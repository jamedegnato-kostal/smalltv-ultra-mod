#include "PhotoSlideshow.h"
#if WITH_PHOTOS
#include "Gfx.h"
#include "Platform.h"
#include "PhotoLibrary.h"
#include <Arduino_GFX_Library.h>
#include <TJpg_Decoder.h>

namespace {
bool photoOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data) {
  if (x < 0 || y < 0 || x + w > 240 || y + h > 240 || !gfxDev()) return false;
  const uint32_t pixels = uint32_t(w) * h;
  if (pixels > 256) return false; // baseline JPEG MCU is at most 16x16
  for (uint32_t i = 0; i < pixels; ++i) data[i] = gfxTint(data[i]);
  gfxDev()->draw16bitRGBBitmap(x, y, data, w, h);
  yield();
  return true;
}
bool validateOutput(int16_t, int16_t, uint16_t, uint16_t, uint16_t*) {
  yield();
  return true;
}
}

PhotoSlideshow g_photoSlideshow;

bool photoValidateJpeg(const String& path) {
  uint16_t width = 0, height = 0;
  TJpgDec.setJpgScale(0);
  TJpgDec.setCallback(validateOutput);
  if (TJpgDec.getFsJpgSize(&width, &height, path, LittleFS) != JDR_OK || width != 240 || height != 240) return false;
  File file = LittleFS.open(path, "r");
  return file && TJpgDec.drawFsJpg(0, 0, file) == JDR_OK;
}

bool photoValidateJpeg(const String& path, uint32_t offset, uint32_t length) {
  if (length < 4) return false;
  File marker = LittleFS.open(path, "r");
  if (!marker || offset > marker.size() || length > marker.size() - offset ||
      !marker.seek(offset, SeekSet)) return false;
  uint8_t start[2], end[2];
  if (marker.read(start, sizeof(start)) != sizeof(start) ||
      !marker.seek(offset + length - 2, SeekSet) ||
      marker.read(end, sizeof(end)) != sizeof(end) ||
      start[0] != 0xFF || start[1] != 0xD8 ||
      end[0] != 0xFF || end[1] != 0xD9) return false;
  marker.close();

  // TJpg_Decoder reads from the current file position and stops at JPEG EOI.
  uint16_t width = 0, height = 0;
  TJpgDec.setJpgScale(0);
  TJpgDec.setCallback(validateOutput);
  File header = LittleFS.open(path, "r");
  if (!header || !header.seek(offset, SeekSet)) return false;
  const JRESULT sizeResult = TJpgDec.getFsJpgSize(&width, &height, header);
  header.close();
  if (sizeResult != JDR_OK || width != 240 || height != 240) return false;

  File frame = LittleFS.open(path, "r");
  if (!frame || !frame.seek(offset, SeekSet)) return false;
  const JRESULT drawResult = TJpgDec.drawFsJpg(0, 0, frame);
  frame.close();
  return drawResult == JDR_OK;
}

void PhotoSlideshow::begin() {
  sequence_.seed(ESP.getCycleCount());
}

bool PhotoSlideshow::startOne(uint8_t photoId, bool randomPhoto, uint16_t dwellSeconds) {
  // Require a contiguous heap block before touching LittleFS/JPEG state.
  constexpr uint32_t MinFreeHeap = 8UL * 1024UL;
  constexpr uint32_t MinLargestBlock = 3UL * 1024UL;
  if (ESP.getFreeHeap() < MinFreeHeap || platformMaxFreeBlock() < MinLargestBlock) return false;
  const uint8_t available = smalltv_photo::count();
  if (!available || (!randomPhoto && !smalltv_photo::contains(photoId))) return false;
  sequence_.setPhotos(smalltv_photo::slots(), available);
  target_ = 1;
  shown_ = 0;
  requestedPhoto_ = photoId;
  randomPhoto_ = randomPhoto;
  dwellSeconds_ = static_cast<uint16_t>(constrain((int)dwellSeconds, 5, 3600));
  currentDwellSeconds_ = dwellSeconds_;
  memset(attempted_, 0, sizeof(attempted_));
  generation_ = smalltv_photo::generation();
  showing_ = false;
  currentAnimated_ = false;
  active_ = true;
  return true;
}

bool PhotoSlideshow::wasAttempted(uint8_t id) const {
  return (attempted_[id >> 3] & uint8_t(1u << (id & 7))) != 0;
}
void PhotoSlideshow::markAttempted(uint8_t id) {
  attempted_[id >> 3] |= uint8_t(1u << (id & 7));
}

bool PhotoSlideshow::draw(uint8_t id) {
  PlatformMemoryPhaseScope photoMemory(PLATFORM_MEM_PHOTO, true);
  if (!smalltv_photo::contains(id)) return false;

  String filePath;
  if (currentAnimated_) {
    if (animationFrame_ >= animationFrameCount_) return false;
    const smalltv_photo::AnimationFrameInfo& frame = animationFrames_[animationFrame_];
    filePath = frame.embedded ? smalltv_photo::animationPath(id)
                              : smalltv_photo::framePath(id, animationFrame_);
  } else {
    filePath = smalltv_photo::path(id);
  }
  File file = LittleFS.open(filePath, "r");
  if (!file) return false;
  if (currentAnimated_ && animationFrames_[animationFrame_].embedded &&
      !file.seek(animationFrames_[animationFrame_].offset, SeekSet)) return false;

  TJpgDec.setJpgScale(0);
  TJpgDec.setSwapBytes(false);
  TJpgDec.setCallback(photoOutput);
  return TJpgDec.drawFsJpg(0, 0, file) == JDR_OK;
}

bool PhotoSlideshow::service(const Settings& settings) {
  if (!active_) return true;
  if (settings.mode != MODE_THEME) {
    active_ = false;
    showing_ = false;
    currentAnimated_ = false;
    return true;
  }

  if (generation_ != smalltv_photo::generation()) {
    generation_ = smalltv_photo::generation();
    sequence_.setPhotos(smalltv_photo::slots(), smalltv_photo::count());
    if (!randomPhoto_ && !smalltv_photo::contains(requestedPhoto_)) target_ = 0;
  }

  if (showing_) {
    const uint32_t now = millis();
    if (uint32_t(now - shownAt_) < uint32_t(currentDwellSeconds_) * 1000UL) {
      if (currentAnimated_ && int32_t(now - nextFrameAt_) >= 0) {
        animationFrame_ = uint8_t((animationFrame_ + 1) % animationFrameCount_);
        if (draw(currentPhoto_)) nextFrameAt_ = now + animationFrames_[animationFrame_].delayMs;
        else currentAnimated_ = false;
      }
      return false;
    }
    showing_ = false;
    currentAnimated_ = false;
    if (shown_ >= target_) { active_ = false; return true; }
  }

  const uint8_t available = smalltv_photo::count();
  for (uint8_t attempt = 0; attempt < available; ++attempt) {
    uint8_t id = requestedPhoto_;
    if (randomPhoto_) {
      if (!sequence_.next(id)) break;
    } else if (attempt || shown_ || wasAttempted(requestedPhoto_)) {
      break;
    }
    if (wasAttempted(id)) continue;
    markAttempted(id);
    currentPhoto_ = id;
    animationFrameCount_ = 0;
    currentAnimated_ = smalltv_photo::isAnimation(id);
    if (currentAnimated_ &&
        !smalltv_photo::loadAnimationFrames(id, animationFrames_, smalltv_photo::MaxAnimationFrames, animationFrameCount_)) {
      currentAnimated_ = false;
      continue;
    }
    animationFrame_ = 0;
    if (!draw(id)) { currentAnimated_ = false; continue; }
    ++shown_;
    currentDwellSeconds_ = dwellSeconds_;
    shownAt_ = millis();
    nextFrameAt_ = shownAt_ + (currentAnimated_ ? animationFrames_[0].delayMs : 0);
    showing_ = true;
    return false;
  }
  active_ = false; // this step's selected photo could not be displayed
  return true;
}
#endif
