#include "PhotoSlideshow.h"
#if WITH_PHOTOS
#include "Gfx.h"
#include "Platform.h"
#include "PhotoLibrary.h"
#include "PhotoJpeg.h"
#include <Arduino_GFX_Library.h>
#include <TJpg_Decoder.h>
namespace {
bool photoOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data) {
  if (x < 0 || y < 0 || x + w > 240 || y + h > 240 || !gfxDev()) return false;
  const uint32_t pixels = uint32_t(w) * h;
  if (pixels > 256) return false;
  for (uint32_t i = 0; i < pixels; ++i) data[i] = gfxTint(data[i]);
  gfxDev()->draw16bitRGBBitmap(x, y, data, w, h);
  yield();
  return true;
}
}
PhotoSlideshow g_photoSlideshow;
namespace {
class PhotoFileReader {
 public:
  explicit PhotoFileReader(File& file) : file_(file) {}
  uint32_t size() const { return static_cast<uint32_t>(file_.size()); }
  uint32_t position() const { return static_cast<uint32_t>(file_.position()); }
  int read() { return file_.read(); }
  bool seek(uint32_t position) { return file_.seek(position, SeekSet); }
 private:
  File& file_;
};
}
bool photoValidateJpeg(const String& path) {
  File file = LittleFS.open(path, "r");
  if (!file) return false;
  PhotoFileReader reader(file);
  return smalltv_photo::validateBaselineJpeg240(reader);
}
void PhotoSlideshow::begin() {
  sequence_.seed(ESP.getCycleCount());
}
bool PhotoSlideshow::startOne(uint8_t photoId, bool randomPhoto, uint16_t dwellSeconds) {
  // TJpg_Decoder keeps its ~3.5 KiB workspace in its global static
  // object, not on the heap. The theme is unloaded before this call; retain
  // enough heap for LittleFS/File bookkeeping without imposing an 8 KiB /
  // 3 KiB floor that rejects otherwise usable ESP8266 devices.
  constexpr uint32_t MinFreeHeap = 3UL * 1024UL;
  constexpr uint32_t MinLargestBlock = 1280UL;
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
  File file = LittleFS.open(smalltv_photo::path(id), "r");
  if (!file) return false;
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(false);
  TJpgDec.setCallback(photoOutput);
  return TJpgDec.drawFsJpg(0, 0, file) == JDR_OK;
}
bool PhotoSlideshow::service(const Settings& settings) {
  if (!active_) return true;
  if (settings.mode != MODE_THEME) {
    active_ = false;
    showing_ = false;
    return true;
  }
  if (generation_ != smalltv_photo::generation()) {
    generation_ = smalltv_photo::generation();
    sequence_.setPhotos(smalltv_photo::slots(), smalltv_photo::count());
    if (!randomPhoto_ && !smalltv_photo::contains(requestedPhoto_)) target_ = 0;
  }
  if (showing_) {
    if (uint32_t(millis() - shownAt_) < uint32_t(currentDwellSeconds_) * 1000UL) return false;
    showing_ = false;
    if (shown_ >= target_) {
      active_ = false;
      return true;
    }
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
    if (!draw(id)) continue;
    ++shown_;
    currentDwellSeconds_ = dwellSeconds_;
    shownAt_ = millis();
    showing_ = true;
    return false;
  }
  active_ = false;
  return true;
}
#endif
