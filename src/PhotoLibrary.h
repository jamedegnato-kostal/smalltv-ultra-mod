#pragma once
#include "config.h"
#if WITH_PHOTOS
#include <Arduino.h>
#include <LittleFS.h>

namespace smalltv_photo {
#ifndef SMALLTV_PHOTO_MAX_COUNT
#define SMALLTV_PHOTO_MAX_COUNT
constexpr uint8_t MaxPhotos = 240;
#endif
constexpr uint8_t MaxAnimationFrames = 12;

void begin();
void refresh();
uint8_t count();
const uint8_t* slots();
uint16_t generation();
bool contains(uint8_t id);
String path(uint8_t id);
String framePath(uint8_t id, uint8_t frame);
String animationPath(uint8_t id);
bool isAnimation(uint8_t id);
uint8_t frameCount(uint8_t id);
uint16_t frameDelayMs(uint8_t id, uint8_t frame);
uint32_t bytes(uint8_t id);
int nextFreeSlot();
bool remove(uint8_t id);
uint16_t removeAll();
uint32_t totalPhotoBytes();
uint32_t filesystemTotalBytes();
uint32_t filesystemFreeBytes();
} // namespace smalltv_photo
#endif
