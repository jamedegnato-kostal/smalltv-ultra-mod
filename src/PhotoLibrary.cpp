#include "PhotoLibrary.h"
#if WITH_PHOTOS
#include <stdio.h>
#include <string.h>

namespace smalltv_photo {
namespace {
uint8_t installed[MaxPhotos] = {};
uint8_t installedCount = 0;
uint16_t currentGeneration = 0;

bool parsePhotoName(const String& name, uint8_t& id, bool& animation) {
  if ((name.length() != 8 && name.length() != 9) || name[0] != 'p' || name[4] != '.') return false;
  if (name[1] < '0' || name[1] > '9' || name[2] < '0' || name[2] > '9' ||
      name[3] < '0' || name[3] > '9') return false;
  animation = name.length() == 9 && name[5] == 'a' && name[6] == 'n' && name[7] == 'i' && name[8] == 'm';
  if (!animation && !(name.length() == 8 && name[5] == 'j' && name[6] == 'p' && name[7] == 'g')) return false;
  uint16_t value = uint16_t(name[1] - '0') * 100 + uint16_t(name[2] - '0') * 10 + uint16_t(name[3] - '0');
  if (value >= MaxPhotos) return false;
  id = static_cast<uint8_t>(value);
  return true;
}
bool alreadyInstalled(uint8_t id) {
  for (uint8_t i = 0; i < installedCount; ++i) if (installed[i] == id) return true;
  return false;
}
bool parseAnimationFrameName(const String& name, uint8_t& id) {
  if (name.length() != 11 || name[0] != 'p' || name[4] != 'f' || name[7] != '.' ||
      name[8] != 'j' || name[9] != 'p' || name[10] != 'g') return false;
  if (name[1] < '0' || name[1] > '9' || name[2] < '0' || name[2] > '9' ||
      name[3] < '0' || name[3] > '9' || name[5] < '0' || name[5] > '9' ||
      name[6] < '0' || name[6] > '9') return false;
  const uint16_t value = uint16_t(name[1] - '0') * 100 + uint16_t(name[2] - '0') * 10 + uint16_t(name[3] - '0');
  if (value >= MaxPhotos) return false;
  id = static_cast<uint8_t>(value);
  return true;
}
uint8_t readAnimationFrameCount(uint8_t id) {
  File meta = LittleFS.open(animationPath(id), "r");
  if (!meta || meta.size() < 5) return 0;
  char header[5];
  if (meta.readBytes(header, sizeof(header)) != sizeof(header) || memcmp(header, "PGA1", 4)) return 0;
  const uint8_t frames = static_cast<uint8_t>(header[4]);
  return frames >= 2 && frames <= MaxAnimationFrames && meta.size() == 5 + uint32_t(frames) * 2 ? frames : 0;
}
}

String path(uint8_t id) {
  char name[20];
  snprintf(name, sizeof(name), "/photos/p%03u.jpg", unsigned(id));
  return String(name);
}
String framePath(uint8_t id, uint8_t frame) {
  char name[24];
  snprintf(name, sizeof(name), "/photos/p%03uf%02u.jpg", unsigned(id), unsigned(frame));
  return String(name);
}
String animationPath(uint8_t id) {
  char name[24];
  snprintf(name, sizeof(name), "/photos/p%03u.anim", unsigned(id));
  return String(name);
}

void refresh() {
  installedCount = 0;
  Dir animations = LittleFS.openDir("/photos");
  while (animations.next() && installedCount < MaxPhotos) {
    if (!animations.isFile()) continue;
    uint8_t id;
    bool animation = false;
    if (parsePhotoName(animations.fileName(), id, animation) && animation && !alreadyInstalled(id)) installed[installedCount++] = id;
  }
  Dir dir = LittleFS.openDir("/photos");
  while (dir.next() && installedCount < MaxPhotos) {
    if (!dir.isFile()) continue;
    uint8_t id;
    bool animation = false;
    if (parsePhotoName(dir.fileName(), id, animation) && !animation && !alreadyInstalled(id) &&
        LittleFS.exists(path(id))) installed[installedCount++] = id;
  }
  Dir frames = LittleFS.openDir("/photos");
  while (frames.next()) {
    if (!frames.isFile()) continue;
    uint8_t id;
    if (parseAnimationFrameName(frames.fileName(), id) && !LittleFS.exists(animationPath(id)))
      LittleFS.remove(String("/photos/") + frames.fileName());
  }
  // Sort the small directory snapshot so IDs and previews have a stable order.
  for (uint8_t i = 1; i < installedCount; ++i) {
    uint8_t value = installed[i], j = i;
    while (j && installed[j - 1] > value) { installed[j] = installed[j - 1]; --j; }
    installed[j] = value;
  }
  ++currentGeneration;
}

void begin() {
  LittleFS.mkdir("/photos");
  Dir dir = LittleFS.openDir("/photos");
  while (dir.next()) {
    if (!dir.isFile()) continue;
    const String name = dir.fileName();
    if (name.length() && name[0] == '.') LittleFS.remove(String("/photos/") + name);
  }
  refresh();
}
uint8_t count() { return installedCount; }
const uint8_t* slots() { return installed; }
uint16_t generation() { return currentGeneration; }
bool contains(uint8_t id) {
  for (uint8_t i = 0; i < installedCount; ++i) if (installed[i] == id) return true;
  return false;
}
bool isAnimation(uint8_t id) {
  return contains(id) && LittleFS.exists(animationPath(id));
}
uint8_t frameCount(uint8_t id) {
  return isAnimation(id) ? readAnimationFrameCount(id) : 0;
}
uint16_t frameDelayMs(uint8_t id, uint8_t frame) {
  const uint8_t frames = frameCount(id);
  if (frame >= frames) return 100;
  File meta = LittleFS.open(animationPath(id), "r");
  if (!meta || !meta.seek(5 + uint32_t(frame) * 2, SeekSet)) return 100;
  uint8_t delay[2];
  if (meta.read(delay, 2) != 2) return 100;
  const uint16_t value = delay[0] | (uint16_t(delay[1]) << 8);
  return value >= 20 && value <= 2000 ? value : 100;
}
uint32_t bytes(uint8_t id) {
  if (isAnimation(id)) {
    uint32_t total = 0;
    File meta = LittleFS.open(animationPath(id), "r");
    if (meta) total += meta.size();
    for (uint8_t i = 0; i < frameCount(id); ++i) {
      File frame = LittleFS.open(framePath(id, i), "r");
      if (frame) total += frame.size();
    }
    return total;
  }
  File file = LittleFS.open(path(id), "r");
  return file ? file.size() : 0;
}
int nextFreeSlot() {
  uint8_t at = 0;
  for (uint8_t i = 0; i < installedCount; ++i) {
    if (installed[i] != at) return at;
    ++at;
  }
  return at < MaxPhotos ? at : -1;
}
bool remove(uint8_t id) {
  if (!contains(id)) return false;
  bool ok = true;
  if (LittleFS.exists(path(id)) && !LittleFS.remove(path(id))) ok = false;
  if (LittleFS.exists(animationPath(id)) && !LittleFS.remove(animationPath(id))) ok = false;
  for (uint8_t i = 0; i < MaxAnimationFrames; ++i)
    if (LittleFS.exists(framePath(id, i)) && !LittleFS.remove(framePath(id, i))) ok = false;
  if (!ok) return false;
  refresh();
  return true;
}
uint16_t removeAll() {
  uint16_t removed = 0;
  const uint8_t count = installedCount;
  uint8_t ids[MaxPhotos];
  memcpy(ids, installed, count);
  for (uint8_t i = 0; i < count; ++i) {
    const uint8_t id = ids[i];
    bool complete = true;
    if (LittleFS.exists(path(id)) && !LittleFS.remove(path(id))) complete = false;
    if (LittleFS.exists(animationPath(id)) && !LittleFS.remove(animationPath(id))) complete = false;
    for (uint8_t frame = 0; frame < MaxAnimationFrames; ++frame)
      if (LittleFS.exists(framePath(id, frame)) && !LittleFS.remove(framePath(id, frame))) complete = false;
    if (complete) ++removed;
  }
  refresh();
  return removed;
}
uint32_t totalPhotoBytes() {
  uint32_t total = 0;
  for (uint8_t i = 0; i < installedCount; ++i) total += bytes(installed[i]);
  return total;
}
uint32_t filesystemTotalBytes() {
#if defined(SMALLTV_ESP8266)
  FSInfo info;
  return LittleFS.info(info) ? info.totalBytes : 0;
#else
  return LittleFS.totalBytes();
#endif
}
uint32_t filesystemFreeBytes() {
#if defined(SMALLTV_ESP8266)
  FSInfo info;
  return LittleFS.info(info) && info.totalBytes >= info.usedBytes ? info.totalBytes - info.usedBytes : 0;
#else
  return LittleFS.totalBytes() - LittleFS.usedBytes();
#endif
}
} // namespace smalltv_photo
#endif
