#include "PhotoLibrary.h"
#if WITH_PHOTOS
#include <stdio.h>
#include <string.h>
namespace smalltv_photo {
namespace {
uint8_t installed[MaxPhotos] = {};
uint8_t installedCount = 0;
uint16_t currentGeneration = 0;
uint32_t cachedPhotoBytes = 0;
uint32_t cachedOrphanBytes = 0;
uint16_t cachedLegacyFileCount = 0;
constexpr uint8_t LegacyFramesPerAnimation = 12;
constexpr size_t LegacySlotBytes = (MaxPhotos + 7U) / 8U;
uint8_t legacyOccupied[LegacySlotBytes] = {};
uint8_t legacyMetadata[LegacySlotBytes] = {};
void setLegacyBit(uint8_t* bits, uint8_t id) {
  bits[id >> 3] |= static_cast<uint8_t>(1u << (id & 7));
}
bool getLegacyBit(const uint8_t* bits, uint8_t id) {
  return (bits[id >> 3] & static_cast<uint8_t>(1u << (id & 7))) != 0;
}
bool parsePhotoName(const String& name, uint8_t& id) {
  if (name.length() != 8 || name[0] != 'p' || name[4] != '.') return false;
  if (name[1] < '0' || name[1] > '9' || name[2] < '0' || name[2] > '9' ||
      name[3] < '0' || name[3] > '9' ||
      name[5] != 'j' || name[6] != 'p' || name[7] != 'g') return false;
  const uint16_t value = uint16_t(name[1] - '0') * 100 +
                         uint16_t(name[2] - '0') * 10 +
                         uint16_t(name[3] - '0');
  if (value >= MaxPhotos) return false;
  id = static_cast<uint8_t>(value);
  return true;
}
// Old GIF imports used pNNN.anim metadata and pNNNfNN.jpg frame files.
// Recognize old names for storage diagnostics and slot reservation. They are
// never decoded or changed automatically; explicit delete-all removes them.
bool parseLegacyFrameName(const String& name, uint8_t& id) {
  if (name.length() != 11 || name[0] != 'p' || name[4] != 'f' || name[7] != '.' ||
      name[8] != 'j' || name[9] != 'p' || name[10] != 'g') return false;
  if (name[1] < '0' || name[1] > '9' || name[2] < '0' || name[2] > '9' ||
      name[3] < '0' || name[3] > '9' || name[5] < '0' || name[5] > '9' ||
      name[6] < '0' || name[6] > '9') return false;
  const uint16_t value = uint16_t(name[1] - '0') * 100 +
                         uint16_t(name[2] - '0') * 10 +
                         uint16_t(name[3] - '0');
  const uint8_t frame = static_cast<uint8_t>((name[5] - '0') * 10 + (name[6] - '0'));
  if (value >= MaxPhotos || frame >= LegacyFramesPerAnimation) return false;
  id = static_cast<uint8_t>(value);
  return true;
}
bool parseLegacyAnimationName(const String& name, uint8_t& id) {
  if (name.length() != 9 || name[0] != 'p' || name[4] != '.' ||
      name[5] != 'a' || name[6] != 'n' || name[7] != 'i' || name[8] != 'm') return false;
  if (name[1] < '0' || name[1] > '9' || name[2] < '0' || name[2] > '9' ||
      name[3] < '0' || name[3] > '9') return false;
  const uint16_t value = uint16_t(name[1] - '0') * 100 +
                         uint16_t(name[2] - '0') * 10 +
                         uint16_t(name[3] - '0');
  if (value >= MaxPhotos) return false;
  id = static_cast<uint8_t>(value);
  return true;
}
bool alreadyInstalled(uint8_t id) {
  for (uint8_t i = 0; i < installedCount; ++i)
    if (installed[i] == id) return true;
  return false;
}
} // namespace
String path(uint8_t id) {
  char name[20];
  snprintf(name, sizeof(name), "/photos/p%03u.jpg", unsigned(id));
  return String(name);
}
void refresh() {
  installedCount = 0;
  memset(legacyOccupied, 0, sizeof(legacyOccupied));
  memset(legacyMetadata, 0, sizeof(legacyMetadata));
  cachedLegacyFileCount = 0;
  Dir dir = LittleFS.openDir("/photos");
  while (dir.next()) {
    if (!dir.isFile()) continue;
    uint8_t id;
    const String name = dir.fileName();
    if (parsePhotoName(name, id)) {
      if (installedCount < MaxPhotos && !alreadyInstalled(id))
        installed[installedCount++] = id;
    } else if (parseLegacyFrameName(name, id)) {
      setLegacyBit(legacyOccupied, id);
      ++cachedLegacyFileCount;
    } else if (parseLegacyAnimationName(name, id)) {
      setLegacyBit(legacyOccupied, id);
      setLegacyBit(legacyMetadata, id);
      ++cachedLegacyFileCount;
    } else if (name == ".upload" || name == ".anim-upload") {
      ++cachedLegacyFileCount;
    }
  }
  for (uint8_t i = 1; i < installedCount; ++i) {
    uint8_t value = installed[i], j = i;
    while (j && installed[j - 1] > value) {
      installed[j] = installed[j - 1]; --j;
    }
    installed[j] = value;
  }
  cachedPhotoBytes = 0;
  for (uint8_t i = 0; i < installedCount; ++i) cachedPhotoBytes += bytes(installed[i]);
  // Refresh only accounts for orphan legacy frames; it never deletes user files.
  cachedOrphanBytes = 0;
  Dir frames = LittleFS.openDir("/photos");
  while (frames.next()) {
    if (!frames.isFile()) continue;
    uint8_t id;
    const String name = frames.fileName();
    if (!parseLegacyFrameName(name, id)) continue;
    if (getLegacyBit(legacyMetadata, id)) continue;
    char fileName[32];
    snprintf(fileName, sizeof(fileName), "/photos/%s", name.c_str());
    File frame = LittleFS.open(fileName, "r");
    if (frame) cachedOrphanBytes += frame.size();
    yield();
  }
  ++currentGeneration;
}
void begin() {
  LittleFS.mkdir("/photos");
  // Never clean user media on boot. Old GIF files remain untouched; they are
  // simply not listed or played by this JPEG-only build.
  refresh();
}
uint8_t count() { return installedCount; }
const uint8_t* slots() { return installed; }
uint16_t generation() { return currentGeneration; }
bool contains(uint8_t id) {
  for (uint8_t i = 0; i < installedCount; ++i)
    if (installed[i] == id) return true;
  return false;
}
uint32_t bytes(uint8_t id) {
  File file = LittleFS.open(path(id), "r");
  return file ? file.size() : 0;
}
int nextFreeSlot() {
  // Do not reuse an ID still occupied by legacy GIF metadata/frame files.
  for (uint16_t candidate = 0; candidate < MaxPhotos; ++candidate) {
    const uint8_t id = static_cast<uint8_t>(candidate);
    if (contains(id) || getLegacyBit(legacyOccupied, id)) continue;
    return static_cast<int>(id);
  }
  return -1;
}
bool remove(uint8_t id) {
  if (!contains(id)) return false;
  const String fileName = path(id);
  if (LittleFS.exists(fileName) && !LittleFS.remove(fileName)) return false;
  refresh();
  return true;
}
uint16_t removeAll() {
  uint16_t removed = 0;
  // This is an explicit, user-confirmed destructive action. Remove the current
  // JPEG library and every filename format used by the old GIF importer, while
  // normal boot/refresh continues to leave legacy media untouched.
  char fileName[32];
  for (uint16_t rawId = 0; rawId < MaxPhotos; ++rawId) {
    const uint8_t id = static_cast<uint8_t>(rawId);
    snprintf(fileName, sizeof(fileName), "/photos/p%03u.jpg", unsigned(id));
    if (LittleFS.exists(fileName) && LittleFS.remove(fileName)) ++removed;
    snprintf(fileName, sizeof(fileName), "/photos/p%03u.anim", unsigned(id));
    if (LittleFS.exists(fileName) && LittleFS.remove(fileName)) ++removed;
    for (uint8_t frame = 0; frame < LegacyFramesPerAnimation; ++frame) {
      snprintf(fileName, sizeof(fileName), "/photos/p%03uf%02u.jpg", unsigned(id), unsigned(frame));
      if (LittleFS.exists(fileName) && LittleFS.remove(fileName)) ++removed;
    }
    yield();
  }
  static const char* const temporaryFiles[] = {
    "/photos/.upload", "/photos/.anim-upload"
  };
  for (const char* temporary : temporaryFiles) {
    if (LittleFS.exists(temporary) && LittleFS.remove(temporary)) ++removed;
  }
  refresh();
  return removed;
}
uint16_t legacyFileCount() { return cachedLegacyFileCount; }
uint32_t totalPhotoBytes() { return cachedPhotoBytes; }
uint32_t orphanFrameBytes() { return cachedOrphanBytes; }
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
  return LittleFS.info(info) && info.totalBytes >= info.usedBytes
      ? info.totalBytes - info.usedBytes : 0;
#else
  const uint32_t total = LittleFS.totalBytes();
  const uint32_t used = LittleFS.usedBytes();
  return total >= used ? total - used : 0;
#endif
}
} // namespace smalltv_photo
#endif
