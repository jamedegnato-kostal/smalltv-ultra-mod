#pragma once
#include "config.h"
#if WITH_PHOTOS
#include "Settings.h"
#include "PhotoShuffle.h"
#include "PhotoLibrary.h"

bool photoValidateJpeg(const String& path);
bool photoValidateJpeg(const String& path, uint32_t offset, uint32_t length);

class PhotoSlideshow {
 public:
  void begin();
  bool startOne(uint8_t photoId, bool randomPhoto, uint16_t dwellSeconds);
  bool service(const Settings& settings); // true when this batch is finished
  void cancel() { active_ = false; showing_ = false; currentAnimated_ = false; }
  bool active() const { return active_; }

 private:
  bool draw(uint8_t id);
  bool wasAttempted(uint8_t id) const;
  void markAttempted(uint8_t id);
  bool active_ = false, showing_ = false;
  bool currentAnimated_ = false;
  uint8_t target_ = 0, shown_ = 0, requestedPhoto_ = 0, currentPhoto_ = 0, animationFrame_ = 0, animationFrameCount_ = 0;
  bool randomPhoto_ = true;
  uint16_t dwellSeconds_ = 10, currentDwellSeconds_ = 10;
  uint16_t generation_ = 0;
  uint32_t shownAt_ = 0, nextFrameAt_ = 0;
  uint8_t attempted_[(smalltv_photo::MaxPhotos + 7) / 8] = {};
  smalltv_photo::AnimationFrameInfo animationFrames_[smalltv_photo::MaxAnimationFrames] = {};
  smalltv_photo::ShuffleBag sequence_;
};

extern PhotoSlideshow g_photoSlideshow;
#endif
