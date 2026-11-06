#include "PhotoShuffle.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

using smalltv_photo::ShuffleBag;

int main() {
  ShuffleBag bag;
  bag.seed(0x12345678UL);
  uint8_t id = 0;

  // Empty and short libraries must never promise more photos than exist.
  assert(!bag.next(id));
  assert(smalltv_photo::photosInBatch(0, 10) == 0);
  assert(smalltv_photo::photosInBatch(2, 10) == 2);
  assert(smalltv_photo::photosInBatch(10, 3) == 3);

  // A one-photo library repeats only because no alternative exists.
  const uint8_t one[] = {7};
  bag.setPhotos(one, 1);
  for (int i = 0; i < 8; ++i) { assert(bag.next(id)); assert(id == 7); }

  // Each full 240-photo bag contains every ID once; bag boundaries do not
  // repeat the previous photo when another choice is available.
  uint8_t all[smalltv_photo::MaxPhotos];
  bool seen[smalltv_photo::MaxPhotos] = {};
  for (uint16_t i = 0; i < smalltv_photo::MaxPhotos; ++i) all[i] = uint8_t(i);
  bag.setPhotos(all, smalltv_photo::MaxPhotos);
  uint8_t previous = 0xff;
  for (int cycle = 0; cycle < 3; ++cycle) {
    for (uint16_t i = 0; i < smalltv_photo::MaxPhotos; ++i) {
      assert(bag.next(id));
      assert(id != previous);
      assert(!seen[id]);
      seen[id] = true;
      previous = id;
    }
    for (uint16_t i = 0; i < smalltv_photo::MaxPhotos; ++i) seen[i] = false;
  }

  // A library change rebuilds the bag and excludes deleted IDs.
  const uint8_t changed[] = {4, 19, 201};
  bag.setPhotos(changed, sizeof(changed));
  for (int i = 0; i < 30; ++i) {
    assert(bag.next(id));
    assert(id == 4 || id == 19 || id == 201);
    const uint8_t dwell = bag.randomDwellSeconds();
    assert(dwell == 5 || dwell == 10 || dwell == 15 || dwell == 20 || dwell == 30 || dwell == 60);
  }

  puts("photo shuffle sequence tests passed");
  return 0;
}
