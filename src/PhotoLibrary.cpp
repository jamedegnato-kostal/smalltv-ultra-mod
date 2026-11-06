#include "PhotoLibrary.h"
#if WITH_PHOTOS
#include <stdio.h>
#include <string.h>

namespace smalltv_photo {
namespace {
uint8_t installed[MaxPhotos] = {};
uint8_t installedCount = 0;
uint16_t currentGeneration = 0;

bool parsePhotoName(const String& name, uint8_t& id) {
  if (name.length() != 8 || name[0] != 'p' || name[4] != '.' ||
      name[5] != 'j' || name[6] != 'p' || name[7] != 'g') return false;
  if (name[1] < '0' || name[1] > '9' || name[2] < '0' || name[2] > '9' ||
      name[3] < '0' || name[3] > '9') return false;
  uint16_t value = uint16_t(name[1] - '0') * 100 + uint16_t(name[2] - '0') * 10 + uint16_t(name[3] - '0');
  if (value >= MaxPhotos) return false;
  id = static_cast<uint8_t>(value);
  return true;
}
}

String path(uint8_t id) {
  char name[20];
  snprintf(name, sizeof(name), "/photos/p%03u.jpg", unsigned(id));
  return String(name);
}

void refresh() {
  installedCount = 0;
  Dir dir = LittleFS.openDir("/photos");
  while (dir.next() && installedCount < MaxPhotos) {
    if (!dir.isFile()) continue;
    uint8_t id;
    if (parsePhotoName(dir.fileName(), id)) installed[installedCount++] = id;
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
  LittleFS.remove("/photos/.upload");
  refresh();
}
uint8_t count() { return installedCount; }
const uint8_t* slots() { return installed; }
uint16_t generation() { return currentGeneration; }
bool contains(uint8_t id) {
  for (uint8_t i = 0; i < installedCount; ++i) if (installed[i] == id) return true;
  return false;
}
uint32_t bytes(uint8_t id) {
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
  if (!contains(id) || !LittleFS.remove(path(id))) return false;
  refresh();
  return true;
}
uint16_t removeAll() {
  uint16_t removed = 0;
  for (uint8_t i = 0; i < installedCount; ++i) if (LittleFS.remove(path(installed[i]))) ++removed;
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
