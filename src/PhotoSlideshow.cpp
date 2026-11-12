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

void PhotoSlideshow::begin() {
  sequence_.seed(ESP.getCycleCount());
}

bool PhotoSlideshow::start(uint8_t photosPerTheme, uint8_t dwellSeconds) {
  // TJpg_Decoder allocates working buffers while decoding.  Refuse a batch
  // before entering the decoder when the heap is fragmented rather than
  // risking an ESP8266 watchdog/exception reset loop.
  constexpr uint32_t MinFreeHeap = 16UL * 1024UL;
  constexpr uint32_t MinLargestBlock = 8UL * 1024UL;
  if (ESP.getFreeHeap() < MinFreeHeap || platformMaxFreeBlock() < MinLargestBlock) return false;
  const uint8_t available = smalltv_photo::count();
  if (!available || !photosPerTheme) return false;
  sequence_.setPhotos(smalltv_photo::slots(), available);
  target_ = smalltv_photo::photosInBatch(available, photosPerTheme);
  shown_ = 0;
  dwellSeconds_ = dwellSeconds;
  memset(attempted_, 0, sizeof(attempted_));
  generation_ = smalltv_photo::generation();
  showing_ = false;
  currentDwellSeconds_ = dwellSeconds_ ? dwellSeconds_ : 10;
  active_ = target_ != 0;
  return active_;
}

bool PhotoSlideshow::wasAttempted(uint8_t id) const {
  return (attempted_[id >> 3] & uint8_t(1u << (id & 7))) != 0;
}
void PhotoSlideshow::markAttempted(uint8_t id) {
  attempted_[id >> 3] |= uint8_t(1u << (id & 7));
}

bool PhotoSlideshow::draw(uint8_t id) {
  if (!smalltv_photo::contains(id)) return false;
  File file = LittleFS.open(smalltv_photo::path(id), "r");
  if (!file) return false;
  TJpgDec.setJpgScale(0);
  TJpgDec.setSwapBytes(false);
  TJpgDec.setCallback(photoOutput);
  return TJpgDec.drawFsJpg(0, 0, file) == JDR_OK;
}

bool PhotoSlideshow::service(const Settings& settings) {
  if (!active_) return true;
  if (!settings.photosEnabled || settings.mode != MODE_THEME || !settings.themePlaylistEnabled) {
    active_ = false;
    showing_ = false;
    return true;
  }

  if (generation_ != smalltv_photo::generation()) {
    generation_ = smalltv_photo::generation();
    const uint8_t available = smalltv_photo::count();
    sequence_.setPhotos(smalltv_photo::slots(), available);
    const uint8_t newTarget = smalltv_photo::photosInBatch(available, settings.photosPerTheme);
    if (newTarget < target_) target_ = newTarget;
  }

  if (showing_) {
    if (uint32_t(millis() - shownAt_) < uint32_t(currentDwellSeconds_) * 1000UL) return false;
    showing_ = false;
    if (shown_ >= target_) { active_ = false; return true; }
  }

  const uint8_t available = smalltv_photo::count();
  for (uint8_t attempt = 0; attempt < available; ++attempt) {
    uint8_t id;
    if (!sequence_.next(id)) break;
    if (wasAttempted(id)) continue;
    markAttempted(id);
    if (!draw(id)) return false; // retry another ID on the next loop pass
    ++shown_;
    if (dwellSeconds_ == 0) {
      currentDwellSeconds_ = sequence_.randomDwellSeconds();
    } else currentDwellSeconds_ = dwellSeconds_;
    shownAt_ = millis();
    showing_ = true;
    return false;
  }
  active_ = false; // all currently available files were tried once
  return true;
}
#endif
