#include "PhotoSlideshow.h"
#include "theme/ThemeRuntime.h"
#include "Platform.h"
#include <TJpg_Decoder.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <algorithm>
#include <stdio.h>
#include <string.h>

extern void appShowNetworkStatus();

namespace {
using smalltv::display::DisplayPanel;

constexpr uint8_t MaxPhotos = 16;
constexpr size_t MaxPhotoBytes = 256UL * 1024UL;
constexpr size_t StorageReserve = 32UL * 1024UL;
constexpr uint16_t MaxPhotoDimension = 1920;
constexpr uint32_t MaxPhotoPixels = 4UL * 1024UL * 1024UL;
const char* const PhotoDirectory = "/photos";
const char* const UploadStagingPath = "/photos/.upload";

struct PhotoEntry {
  char name[40];
  uint32_t bytes;
};

DisplayPanel* g_display = nullptr;
WebServerClass* g_server = nullptr;
PhotoEntry g_photos[MaxPhotos];
uint8_t g_photoCount = 0;
bool g_active = false;
int8_t g_current = -1;
uint32_t g_intervalMs = 15000;
uint32_t g_lastSlide = 0;
int16_t g_imageX = 0;
int16_t g_imageY = 0;
File g_upload;
String g_uploadError;
String g_uploadDestination;
bool g_uploadAllowed = false;
bool g_uploadEnded = false;
uint8_t g_uploadParts = 0;
uint32_t g_uploaded = 0;
size_t g_uploadBudget = 0;

size_t freeStorageBytes() {
  FSInfo info;
  if (!LittleFS.info(info) || info.usedBytes > info.totalBytes) return 0;
  return info.totalBytes - info.usedBytes;
}

bool isPhotoName(const char* name) {
  if (!name || !*name) return false;
  const size_t length = strlen(name);
  if (length < 5 || length >= 40 || strcmp(name + length - 4, ".jpg") != 0) return false;
  for (size_t i = 0; i < length - 4; ++i) {
    const char c = name[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '-' || c == '_')) return false;
  }
  return true;
}

void refreshPhotoList() {
  g_photoCount = 0;
  Dir directory = LittleFS.openDir(PhotoDirectory);
  while (directory.next() && g_photoCount < MaxPhotos) {
    String path = directory.fileName();
    const int slash = path.lastIndexOf('/');
    if (slash >= 0) path = path.substring(slash + 1);
    if (!isPhotoName(path.c_str())) continue;
    PhotoEntry& item = g_photos[g_photoCount++];
    strncpy(item.name, path.c_str(), sizeof(item.name) - 1);
    item.name[sizeof(item.name) - 1] = '\0';
    item.bytes = directory.fileSize();
    yield();
  }
  for (uint8_t i = 0; i < g_photoCount; ++i) {
    for (uint8_t j = i + 1; j < g_photoCount; ++j) {
      if (strcmp(g_photos[i].name, g_photos[j].name) > 0) {
        PhotoEntry temp = g_photos[i];
        g_photos[i] = g_photos[j];
        g_photos[j] = temp;
      }
    }
  }
}

unsigned installedPhotoCount() {
  unsigned count = 0;
  Dir directory = LittleFS.openDir(PhotoDirectory);
  while (directory.next()) {
    String path = directory.fileName();
    const int slash = path.lastIndexOf('/');
    if (slash >= 0) path = path.substring(slash + 1);
    if (isPhotoName(path.c_str())) ++count;
    yield();
  }
  return count;
}

void sendReply(int code, const String& value) {
  JsonDocument doc;
  doc[code < 300 ? "id" : "error"] = value;
  String body;
  serializeJson(doc, body);
  g_server->send(code, "application/json", body);
}

void resetUpload() {
  g_upload.close();
  LittleFS.remove(UploadStagingPath);
  g_uploadAllowed = false;
  g_uploadEnded = false;
  g_uploadParts = 0;
  g_uploaded = 0;
  g_uploadBudget = 0;
  g_uploadDestination = "";
  g_uploadError = "";
}

bool makePhotoName(String incoming, String& finalName) {
  const int slash = incoming.lastIndexOf('/');
  const int backslash = incoming.lastIndexOf('\\');
  const int separator = slash > backslash ? slash : backslash;
  if (separator >= 0) incoming = incoming.substring(separator + 1);
  const int dot = incoming.lastIndexOf('.');
  if (dot <= 0) return false;
  String extension = incoming.substring(dot + 1);
  extension.toLowerCase();
  if (extension != "jpg" && extension != "jpeg") return false;
  String base = incoming.substring(0, dot);
  String safe;
  safe.reserve(24);
  for (size_t i = 0; i < base.length() && safe.length() < 24; ++i) {
    char c = base[i];
    if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
    const bool valid = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
    if (valid) safe += c;
    else if (safe.length() && safe[safe.length() - 1] != '_') safe += '_';
  }
  while (safe.length() && safe[safe.length() - 1] == '_') safe.remove(safe.length() - 1);
  if (!safe.length()) safe = "photo";
  if (safe.length() > 24) safe.remove(24);

  String candidate = safe + ".jpg";
  if (!LittleFS.exists(String(PhotoDirectory) + "/" + candidate)) {
    finalName = candidate;
    return true;
  }
  for (uint8_t suffix = 2; suffix <= MaxPhotos + 1; ++suffix) {
    candidate = safe + "-" + String(suffix) + ".jpg";
    if (!LittleFS.exists(String(PhotoDirectory) + "/" + candidate)) {
      finalName = candidate;
      return true;
    }
  }
  return false;
}

static bool jpegOutput(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t* pixels) {
  if (!g_display || !pixels) return false;
  for (uint16_t row = 0; row < height; ++row) {
    // drawFsJpg() already adds the image offset to x/y before invoking us.
    g_display->row(x, y + row, pixels + size_t(row) * width, width);
  }
  yield();
  return true;
}

bool renderPhoto(uint8_t index) {
  if (!g_display || !g_display->ready() || index >= g_photoCount) return false;
  char path[64];
  const int pathLength = snprintf(path, sizeof(path), "%s/%s", PhotoDirectory, g_photos[index].name);
  if (pathLength <= 0 || static_cast<size_t>(pathLength) >= sizeof(path)) return false;

  uint16_t width = 0, height = 0;
  const JRESULT sizeResult = TJpgDec.getFsJpgSize(&width, &height, path, LittleFS);
  if (sizeResult != JDR_OK || width < 8 || height < 8 ||
      width > MaxPhotoDimension || height > MaxPhotoDimension ||
      uint32_t(width) * height > MaxPhotoPixels) {
    Serial.printf("[photo] unsupported JPEG: %s (result=%u, %ux%u)\n",
                  g_photos[index].name, unsigned(sizeResult), width, height);
    return false;
  }

  uint8_t scale = 1;
  while (scale < 8 &&
         ((uint32_t(width) + scale - 1) / scale > 240 ||
          (uint32_t(height) + scale - 1) / scale > 240)) {
    scale <<= 1;
  }
  const uint16_t outputWidth = (width + scale - 1) / scale;
  const uint16_t outputHeight = (height + scale - 1) / scale;
  g_imageX = (240 - outputWidth) / 2;
  g_imageY = (240 - outputHeight) / 2;

  g_display->clear(0x0000);
  TJpgDec.setJpgScale(scale);
  TJpgDec.setSwapBytes(false);
  const JRESULT result = TJpgDec.drawFsJpg(g_imageX, g_imageY, path, LittleFS);
  if (result != JDR_OK) {
    Serial.printf("[photo] JPEG decode failed: %s (result=%u)\n", g_photos[index].name, unsigned(result));
    g_display->clear(0x0000);
    return false;
  }
  Serial.printf("[photo] displayed %s (%ux%u, scale %u)\n",
                g_photos[index].name, width, height, scale);
  return true;
}

bool showRelative(int direction) {
  if (!g_photoCount) refreshPhotoList();
  if (!g_photoCount) return false;
  int next = g_current;
  for (uint8_t attempt = 0; attempt < g_photoCount; ++attempt) {
    if (next < 0) next = direction > 0 ? 0 : int(g_photoCount) - 1;
    else {
      next += direction;
      if (next < 0) next = g_photoCount - 1;
      if (next >= g_photoCount) next = 0;
    }
    if (renderPhoto(static_cast<uint8_t>(next))) {
      g_current = static_cast<int8_t>(next);
      g_lastSlide = millis();
      return true;
    }
  }
  return false;
}

void listPhotos() {
  refreshPhotoList();
  JsonDocument doc;
  JsonArray list = doc["photos"].to<JsonArray>();
  for (uint8_t i = 0; i < g_photoCount; ++i) {
    JsonObject item = list.add<JsonObject>();
    item["name"] = g_photos[i].name;
    item["bytes"] = g_photos[i].bytes;
  }
  doc["count"] = g_photoCount;
  doc["freeBytes"] = freeStorageBytes();
  doc["active"] = g_active;
  doc["interval"] = g_intervalMs / 1000;
  doc["current"] = (g_current >= 0 && g_current < g_photoCount) ? g_photos[g_current].name : "";
  String body;
  serializeJson(doc, body);
  g_server->send(200, "application/json", body);
}

void receivePhotoUpload() {
  HTTPUpload& incoming = g_server->upload();
  if (incoming.status == UPLOAD_FILE_START) {
    ++g_uploadParts;
    if (g_uploadParts > 1) {
      g_uploadError = "Upload one photo at a time";
      g_upload.close();
      return;
    }
    g_upload.close();
    LittleFS.remove(UploadStagingPath);
    g_uploadAllowed = true;
    g_uploadEnded = false;
    g_uploaded = 0;
    g_uploadError = "";
    g_uploadDestination = "";

    if (g_active) g_uploadError = "Stop the slideshow before adding photos";
    else if (smalltv::themeRuntimeActive()) g_uploadError = "Deactivate the active .stheme before adding photos";
    else if (installedPhotoCount() >= MaxPhotos) g_uploadError = "Photo limit reached (16 images)";
    else if (!makePhotoName(incoming.filename, g_uploadDestination)) g_uploadError = "Use a JPEG file (.jpg or .jpeg)";

    File settings = LittleFS.open("/config.json", "r");
    size_t reserve = settings ? settings.size() + 4096 : StorageReserve;
    if (settings) settings.close();
    if (reserve < StorageReserve) reserve = StorageReserve;
    const size_t free = freeStorageBytes();
    if (!g_uploadError.length() && free <= reserve) g_uploadError = "Not enough LittleFS space; remove a theme or photo first";
    if (!g_uploadError.length()) {
      g_uploadBudget = std::min<size_t>(MaxPhotoBytes, free - reserve);
      g_upload = LittleFS.open(UploadStagingPath, "w");
      if (!g_upload) g_uploadError = "Cannot create photo upload file";
    }
  } else if (incoming.status == UPLOAD_FILE_WRITE) {
    if (!g_uploadAllowed || g_uploadError.length()) return;
    if (!g_upload || g_uploaded > g_uploadBudget ||
        incoming.currentSize > g_uploadBudget - g_uploaded) {
      g_uploadError = "Photo exceeds available space or 256 KiB upload limit";
      g_upload.close();
      return;
    }
    if (g_upload.write(incoming.buf, incoming.currentSize) != incoming.currentSize) {
      g_uploadError = "LittleFS write failed";
      g_upload.close();
      return;
    }
    g_uploaded += incoming.currentSize;
  } else if (incoming.status == UPLOAD_FILE_END) {
    g_upload.close();
    g_uploadEnded = g_uploadAllowed && !g_uploadError.length() &&
                    g_uploadParts == 1 && g_uploaded == incoming.totalSize;
    if (!g_uploadEnded && !g_uploadError.length()) g_uploadError = "Incomplete photo upload";
  } else if (incoming.status == UPLOAD_FILE_ABORTED) {
    g_upload.close();
    g_uploadEnded = false;
    g_uploadError = "Photo upload was interrupted";
  }
  yield();
}

void finishPhotoUpload() {
  if (!g_uploadAllowed || !g_uploadEnded || g_uploadError.length()) {
    const String message = g_uploadError.length() ? g_uploadError : String("No complete photo uploaded");
    resetUpload();
    sendReply(400, message);
    return;
  }

  File file = LittleFS.open(UploadStagingPath, "r");
  uint8_t signature[2] = {};
  const bool jpegMagic = file && file.read(signature, sizeof(signature)) == sizeof(signature) &&
                         signature[0] == 0xFF && signature[1] == 0xD8;
  if (file) file.close();
  if (!jpegMagic) {
    resetUpload();
    sendReply(415, "Expected a JPEG image");
    return;
  }

  uint16_t width = 0, height = 0;
  const JRESULT result = TJpgDec.getFsJpgSize(&width, &height, UploadStagingPath, LittleFS);
  if (result != JDR_OK || width < 8 || height < 8 ||
      width > MaxPhotoDimension || height > MaxPhotoDimension ||
      uint32_t(width) * height > MaxPhotoPixels) {
    resetUpload();
    sendReply(415, "Unsupported JPEG; use a baseline colour JPEG up to 1920 x 1920");
    return;
  }

  const String destination = String(PhotoDirectory) + "/" + g_uploadDestination;
  if (!LittleFS.rename(UploadStagingPath, destination)) {
    resetUpload();
    sendReply(500, "Could not publish the photo in LittleFS");
    return;
  }
  const String name = g_uploadDestination;
  resetUpload();
  refreshPhotoList();
  sendReply(200, name);
}

bool requestPhotoName(String& name) {
  if (!g_server->hasArg("plain")) {
    sendReply(400, "Missing photo name");
    return false;
  }
  JsonDocument doc;
  const String body = g_server->arg("plain");
  if (body.length() > 256 || deserializeJson(doc, body)) {
    sendReply(400, "Invalid photo request");
    return false;
  }
  const char* raw = doc["name"] | "";
  if (!isPhotoName(raw)) {
    sendReply(400, "Invalid photo name");
    return false;
  }
  name = raw;
  return true;
}

void deletePhoto() {
  if (g_active) {
    sendReply(409, "Stop the slideshow before deleting photos");
    return;
  }
  String name;
  if (!requestPhotoName(name)) return;
  const String path = String(PhotoDirectory) + "/" + name;
  if (!LittleFS.exists(path)) {
    sendReply(404, "Photo not found");
    return;
  }
  if (!LittleFS.remove(path)) {
    sendReply(500, "Could not delete photo");
    return;
  }
  refreshPhotoList();
  sendReply(200, name);
}

void startSlideshow() {
  if (smalltv::themeRuntimeActive()) {
    sendReply(409, "Deactivate the active .stheme before starting the slideshow");
    return;
  }
  if (!g_display || !g_display->ready()) {
    sendReply(503, "Display is not ready");
    return;
  }
  if (ESP.getFreeHeap() < 12UL * 1024UL || platformMaxFreeBlock() < 2048UL) {
    sendReply(503, "Not enough free RAM to start JPEG decoding");
    return;
  }
  refreshPhotoList();
  if (!g_photoCount) {
    sendReply(409, "Upload at least one photo first");
    return;
  }
  uint16_t interval = 15;
  if (g_server->hasArg("plain")) {
    const String body = g_server->arg("plain");
    if (body.length() > 128) {
      sendReply(400, "Invalid slideshow settings");
      return;
    }
    JsonDocument doc;
    if (body.length() && deserializeJson(doc, body)) {
      sendReply(400, "Invalid slideshow settings");
      return;
    }
    const int configured = doc["interval"] | 15;
    if (configured < 5 || configured > 3600) {
      sendReply(400, "Slideshow interval must be 5 to 3600 seconds");
      return;
    }
    interval = static_cast<uint16_t>(configured);
  }
  g_intervalMs = uint32_t(interval) * 1000UL;
  g_current = -1;
  g_active = true;
  g_lastSlide = millis() - g_intervalMs;
  sendReply(200, "started");
}

void stopSlideshow() {
  g_active = false;
  g_current = -1;
  if (g_display && g_display->ready()) g_display->clear(0x0000);
  appShowNetworkStatus();
  sendReply(200, "stopped");
}

void nextPhoto() {
  if (!g_active) {
    sendReply(409, "Start the slideshow first");
    return;
  }
  if (!showRelative(1)) {
    g_active = false;
    appShowNetworkStatus();
    sendReply(500, "No readable JPEG photos found");
    return;
  }
  sendReply(200, "next");
}

void previousPhoto() {
  if (!g_active) {
    sendReply(409, "Start the slideshow first");
    return;
  }
  if (!showRelative(-1)) {
    g_active = false;
    appShowNetworkStatus();
    sendReply(500, "No readable JPEG photos found");
    return;
  }
  sendReply(200, "previous");
}
}  // namespace

namespace smalltv {
namespace photos {
void begin(display::DisplayPanel& display) {
  g_display = &display;
  LittleFS.mkdir(PhotoDirectory);
  LittleFS.remove(UploadStagingPath);
  refreshPhotoList();
  TJpgDec.setCallback(jpegOutput);
  TJpgDec.setSwapBytes(false);
}

void webBegin(WebServerClass& server) {
  g_server = &server;
  server.on("/api/photos", HTTP_GET, listPhotos);
  server.on("/api/photos/upload", HTTP_POST, finishPhotoUpload, receivePhotoUpload);
  server.on("/api/photos/delete", HTTP_POST, deletePhoto);
  server.on("/api/photos/start", HTTP_POST, startSlideshow);
  server.on("/api/photos/stop", HTTP_POST, stopSlideshow);
  server.on("/api/photos/next", HTTP_POST, nextPhoto);
  server.on("/api/photos/previous", HTTP_POST, previousPhoto);
}

void loop() {
  if (!g_active) return;
  const uint32_t now = millis();
  if (g_current >= 0 && static_cast<uint32_t>(now - g_lastSlide) < g_intervalMs) return;
  if (!showRelative(1)) {
    Serial.println("[photo] slideshow stopped: no readable JPEG photos");
    g_active = false;
    g_current = -1;
    if (g_display) g_display->clear(0x0000);
    appShowNetworkStatus();
  }
}

bool active() { return g_active; }
uint8_t count() { return g_photoCount; }

void redraw() {
  if (g_active && g_current >= 0) renderPhoto(static_cast<uint8_t>(g_current));
}
}  // namespace photos
}  // namespace smalltv
