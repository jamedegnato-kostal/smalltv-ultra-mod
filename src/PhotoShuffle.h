#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace smalltv_photo {
#ifndef SMALLTV_PHOTO_MAX_COUNT
#define SMALLTV_PHOTO_MAX_COUNT
constexpr uint8_t MaxPhotos = 240;
#endif

// A compact shuffle bag: at most 240 byte-sized IDs plus a 30-byte membership
// map. Fisher-Yates runs once per bag; crossing a bag boundary avoids
// immediately repeating the previous photo when another choice exists.
class ShuffleBag {
 public:
  void seed(uint32_t value) { state_ = value ? value : 0x9e3779b9UL; }

  bool setPhotos(const uint8_t* ids, uint8_t count) {
    if (count > MaxPhotos) count = MaxPhotos;
    uint8_t nextPresent[(MaxPhotos + 7) / 8] = {};
    for (uint8_t i = 0; i < count; ++i) nextPresent[ids[i] >> 3] |= uint8_t(1u << (ids[i] & 7));
    if (count == count_ && memcmp(nextPresent, present_, sizeof(present_)) == 0) return false;
    count_ = count;
    memcpy(present_, nextPresent, sizeof(present_));
    if (count_) memcpy(order_, ids, count_);
    position_ = count_;
    shuffle();
    return true;
  }

  bool next(uint8_t& id) {
    if (!count_) return false;
    if (position_ >= count_) {
      shuffle();
      if (count_ > 1 && order_[0] == last_) {
        const uint8_t swap = order_[0]; order_[0] = order_[1]; order_[1] = swap;
      }
      position_ = 0;
    }
    id = order_[position_++];
    last_ = id;
    return true;
  }

  uint8_t count() const { return count_; }
  uint8_t last() const { return last_; }
  uint8_t randomDwellSeconds() {
    static const uint8_t choices[] = {5, 10, 15, 20, 30, 60};
    return choices[randomBelow(sizeof(choices))];
  }

 private:
  uint32_t randomBelow(uint32_t upper) {
    state_ ^= state_ << 13;
    state_ ^= state_ >> 17;
    state_ ^= state_ << 5;
    return upper ? state_ % upper : 0;
  }
  void shuffle() {
    for (uint16_t i = count_; i > 1; --i) {
      const uint8_t j = static_cast<uint8_t>(randomBelow(i));
      const uint8_t t = order_[i - 1]; order_[i - 1] = order_[j]; order_[j] = t;
    }
  }

  uint8_t order_[MaxPhotos] = {};
  uint8_t present_[(MaxPhotos + 7) / 8] = {};
  uint8_t count_ = 0, position_ = 0, last_ = 0xff;
  uint32_t state_ = 0x9e3779b9UL;
};

inline uint8_t photosInBatch(uint8_t available, uint8_t requested) {
  return available < requested ? available : requested;
}
} // namespace smalltv_photo
