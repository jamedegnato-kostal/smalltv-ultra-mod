#include "ThemeWeb.h"
#if WITH_THEME
#include "ThemeMode.h"
#include "ThemeCatalog.h"
#include "Clock.h"
#include <ArduinoJson.h>
#include <algorithm>
#include <string.h>
extern void appInvalidate();
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
extern void appClearCrashStreak();
#endif
namespace {
WebServerClass* server;
Settings* settings;
bool (*auth)();
bool (*allowed)();   // Host/Origin guard, for the upload callback
File upload;
String uploadError;
bool uploadAllowed = false, uploadEnded = false;
unsigned uploadParts = 0;
uint32_t uploaded = 0, uploadBudget = 0;
constexpr unsigned MaxThemes = 16;
static_assert(MaxThemes <= 32, "listThemes() marks packages in a 32-bit mask");
const char* staging = "/themes/.upload";
// Every theme API route replies with the same small JSON shape: `{"id":...}`
// on success, `{"error":...}` otherwise, keyed by whether `code` is a success
// status.
void reply(int code, const String& message) {
  JsonDocument doc;
  doc[code < 300 ? "id" : "error"] = message;
  String json;
  serializeJson(doc, json);
  server->send(code, "application/json", json);
}
// ESP32's LittleFS exposes totalBytes()/usedBytes() directly; ESP8266's only
// reports them through info(FSInfo&). Both walk the filesystem's blocks, and
// every list and upload start asks, so the answer is kept for a few seconds
// and dropped after this file's own writes (forgetFreeBytes()).
size_t cachedFree = 0;
uint32_t cachedFreeAt = 0;
bool cachedFreeValid = false;
size_t freeBytes() {
  if (cachedFreeValid && millis() - cachedFreeAt < 5000) return cachedFree;
#if defined(SMALLTV_ESP8266)
  FSInfo info;
  cachedFree = LittleFS.info(info) ? info.totalBytes - info.usedBytes : 0;
#else
  cachedFree = LittleFS.totalBytes() - LittleFS.usedBytes();
#endif
  cachedFreeAt = millis();
  cachedFreeValid = true;
  return cachedFree;
}
void forgetFreeBytes() { cachedFreeValid = false; }
// The listing fields of every installed theme (see ThemeIndexReader). It is
// rewritten through `.index.new`, which replaces it only once complete.
const char* indexPath = "/themes/.index";
const char* indexNext = "/themes/.index.new";
struct FileSink : smalltv::Sink {
  File& file;
  explicit FileSink(File& f) : file(f) {}
  bool write(const char* data, size_t count) override {
    return file.write(reinterpret_cast<const uint8_t*>(data), count) == count;
  }
};
// Puts `.index.new` in place of the index. The old index is removed first, as
// a rename onto an existing file is not the same on every core. Whatever
// fails, no index is left that could be out of date: a missing one only
// costs a full inspection of each package at the next list, which writes it
// again.
void publishIndex(bool written) {
  LittleFS.remove(indexPath);
  if (!written || !LittleFS.rename(indexNext, indexPath)) LittleFS.remove(indexNext);
  forgetFreeBytes();
}
// After an install or a delete: the index without `id`'s old line, plus
// `added` if given.
void updateIndex(const String& id, const smalltv::InstalledTheme* added) {
  bool written;
  {
    ThemeFile from;
    from.open(indexPath);   // none yet: copies nothing
    File to = LittleFS.open(indexNext, "w");
    FileSink sink(to);
    written = to && smalltv::copyThemeIndex(from, sink, id.c_str(), added);
  }
  publishIndex(written);
}
// Firmware 3.1.0 and 3.1.1 kept one `<id>.meta` file per theme, each in an
// 8 KB block of storage; the index replaces them. Removed at boot, a few
// names per pass, so the directory is never changed while it is read.
void removeSidecars() {
  // Recover a package interrupted between metadata editor renames.
  for (int pass = 0; pass < 64; ++pass) {
    String names[4]; unsigned found = 0;
#if defined(SMALLTV_ESP8266)
    Dir editDir = LittleFS.openDir("/themes");
    while (found < 4 && editDir.next()) if (editDir.fileName().endsWith(".stheme.bak")) names[found++] = editDir.fileName();
#else
    File editDir = LittleFS.open("/themes", "r");
    for (File f = editDir.openNextFile(); found < 4 && f; f = editDir.openNextFile()) {
      String fileName = f.name(); if (fileName.endsWith(".stheme.bak")) names[found++] = fileName.substring(fileName.lastIndexOf('/') + 1);
    }
    editDir.close();
#endif
    bool changed = false;
    for (unsigned i = 0; i < found; ++i) {
      String backup = "/themes/" + names[i];
      String original = backup.substring(0, backup.length() - 4);
      if (LittleFS.exists(original)) changed |= LittleFS.remove(backup);
      else changed |= LittleFS.rename(backup, original);
    }
    if (found < 4 || !changed) break;
  }
  LittleFS.remove("/themes/.edit");
  for (int pass = 0; pass < 64; ++pass) {
    String names[4];
    unsigned found = 0;
#if defined(SMALLTV_ESP8266)
    Dir dir = LittleFS.openDir("/themes");
    while (found < 4 && dir.next()) if (dir.fileName().endsWith(".meta")) names[found++] = dir.fileName();
#else
    File dir = LittleFS.open("/themes", "r");
    for (File f = dir.openNextFile(); found < 4 && f; f = dir.openNextFile()) {
      String name = f.name();
      if (name.endsWith(".meta")) names[found++] = name.substring(name.lastIndexOf('/') + 1);
    }
    dir.close();
#endif
    bool removed = false;
    for (unsigned i = 0; i < found; ++i) removed |= LittleFS.remove("/themes/" + names[i]);
    if (found < 4 || !removed) break;
  }
}
unsigned installedCount() {
  unsigned count = 0;
#if defined(SMALLTV_ESP8266)
  // Reads the directory entries only. openNextFile() opens every file in
  // turn, and this runs at the start of an upload, when heap is shortest.
  Dir dir = LittleFS.openDir("/themes");
  while (dir.next()) if (dir.isFile() && dir.fileName().endsWith(".stheme")) ++count;
#else
  File dir = LittleFS.open("/themes", "r");
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) if (!f.isDirectory() && String(f.name()).endsWith(".stheme")) ++count;
#endif
  return count;
}
void resetUpload() {
  upload.close();
  LittleFS.remove(staging);
  forgetFreeBytes();
  uploadParts = 0;
  uploadAllowed = false;
  uploadEnded = false;
  uploaded = 0;
}
// The web server framework streams a multipart upload through repeated calls
// to this handler (START, then one or more WRITE, then END or ABORTED), with
// the request body itself validated and stored in finishUpload() only after
// it is complete. Every check here is about the *stream*, not the package
// format: exactly one file, auth, a plausible filename, the theme count
// limit, and available storage, all determined before a single byte is
// written to the staging file so a doomed upload fails fast.
void receiveUpload() {
  HTTPUpload& up = server->upload();
  if (up.status == UPLOAD_FILE_START) {
    ++uploadParts;
    if (uploadParts > 1) {
      uploadError = "Upload exactly one package";
      upload.close();
      return;
    }
    uploadError = "";
    uploaded = 0;
    uploadEnded = false;
    uploadAllowed = allowed() && (!settings->auth.enabled || !settings->auth.pass.length() || server->authenticate(settings->auth.user.c_str(), settings->auth.pass.c_str()));
    if (!uploadAllowed) return;
    if (!up.filename.endsWith(".stheme")) {
      uploadError = "Expected a .stheme file";
      return;
    }
    if (installedCount() >= MaxThemes) {
      uploadError = "Remove a theme first (limit 16)";
      return;
    }
    // Receiving a package needs the receive buffers and the staging file at
    // once, and then a package parse. On the ESP8266 that did not fit beside
    // a running theme (a 16-byte allocation failed here with 160 B free), so
    // an install always frees it; there the hook in themeWebBegin() has freed
    // it already.
    g_themeMode.release(UINT32_MAX);
    LittleFS.remove(staging);
    // Preserve room for a settings rewrite plus filesystem metadata overhead,
    // so a large upload cannot leave too little space for the device to save
    // its own configuration afterward. The size of the last save stands in
    // for the next one: building the settings document here to measure it
    // took the ESP8266's heap down to 896 B at an install's tightest point.
    size_t reserve = settingsStoredSize() + 4096;
    if (reserve < 16384) reserve = 16384;
    size_t free = freeBytes();
    if (free <= reserve) {
      uploadError = "Not enough storage; remove a theme first";
      return;
    }
    uploadBudget = std::min<size_t>(smalltv::MaxPackage, free - reserve);
    upload = LittleFS.open(staging, "w");
    if (!upload) uploadError = "Cannot create staging file";
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (!uploadAllowed || uploadError.length()) return;
    if (!upload || up.currentSize > uploadBudget - uploaded) {
      uploadError = "Package exceeds available storage or 3 MiB";
      upload.close();
      return;
    }
    if (upload.write(up.buf, up.currentSize) != up.currentSize) {
      uploadError = "Not enough storage";
      upload.close();
      return;
    }
    uploaded += up.currentSize;
  } else if (up.status == UPLOAD_FILE_END) {
    upload.close();
    uploadEnded = uploadAllowed && !uploadError.length() && uploaded == up.totalSize;
    if (!uploadEnded && !uploadError.length()) uploadError = "Incomplete upload";
  } else if (up.status == UPLOAD_FILE_ABORTED) {
    resetUpload();
    uploadError = "Upload aborted";
  }
}
// Runs once the multipart body is fully received: validates the staged file
// with the same Package::load() the device uses at runtime (so an accepted
// upload is guaranteed installable), then publishes it under a filename
// derived from the theme's own declared ID.
void finishUpload() {
  if (!auth()) {
    resetUpload();
    return;
  }
  upload.close();
  if (!uploadAllowed || !uploadEnded || uploadError.length()) {
    reply(400, uploadError.length() ? uploadError : String("No complete package uploaded"));
    resetUpload();
    return;
  }
  // Validation parses a second theme; free the running one first if the heap
  // is short (issue #30).
  g_themeMode.release();
  smalltv::Theme theme;
  std::string error;
  bool valid = false;
  {
    ThemeFile file;
    if (file.open(staging)) {
      smalltv::Package package(file);
      valid = package.load(theme, error);
    } else error = "Cannot read uploaded package";
  }
  if (!valid) {
    reply(400, error.c_str());
    resetUpload();
    return;
  }
  String requiredCategory = server->arg("category");
  if (requiredCategory.length() && requiredCategory != theme.category.c_str()) {
    reply(400, "Package is not in the requested theme category");
    resetUpload();
    return;
  }
  String target = "/themes/" + String(theme.id.c_str()) + ".stheme";
  if (LittleFS.exists(target)) {
    reply(409, "Theme ID already installed; remove it or use a new ID");
    resetUpload();
    return;
  }
  // Rename, not copy: the staging file becomes the installed one atomically,
  // so a partially-installed package is never visible to listThemes().
  if (!LittleFS.rename(staging, target)) {
    reply(500, "Cannot publish package");
    resetUpload();
    return;
  }
  smalltv::InstalledTheme entry;
  entry.id = theme.id;
  entry.name = theme.name;
  entry.author = theme.author;
  entry.version = theme.version;
  entry.description = theme.description;
  entry.category = theme.category;
  entry.preview = theme.preview;
  entry.bytes = uploaded;
  entry.valid = true;
  updateIndex(theme.id.c_str(), &entry);
  reply(201, theme.id.c_str());
  resetUpload();
}
// Shared by select/delete: reads a JSON body's `id` and validates it as a
// well-formed theme ID before either route touches the filesystem.
// `id.length() != strlen(id.c_str())` catches an embedded NUL byte ArduinoJson
// would otherwise pass through silently.
bool requestId(String& id) {
  if (!server->hasArg("plain") || server->arg("plain").length() > 128) {
    reply(400, "Expected JSON with id");
    return false;
  }
  JsonDocument doc;
  if (deserializeJson(doc, server->arg("plain")) || !doc["id"].is<const char*>()) {
    reply(400, "Expected JSON with id");
    return false;
  }
  id = doc["id"].as<String>();
  if (!smalltv::validId(id.c_str()) || id.length() != strlen(id.c_str())) {
    reply(400, "Invalid theme ID");
    return false;
  }
  return true;
}
static uint16_t themeU16(const uint8_t* p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
static uint32_t themeU32(const uint8_t* p) { return uint32_t(themeU16(p)) | (uint32_t(themeU16(p + 2)) << 16); }
static void themePut16(uint8_t* p, uint16_t v) { p[0] = uint8_t(v); p[1] = uint8_t(v >> 8); }
static void themePut32(uint8_t* p, uint32_t v) { themePut16(p, uint16_t(v)); themePut16(p + 2, uint16_t(v >> 16)); }
static size_t jsonStringEnd(const std::string& s, size_t at) {
  if (at >= s.size() || s[at] != '"') return std::string::npos;
  for (size_t i = at + 1; i < s.size(); ++i) {
    if (s[i] == '\\') { if (++i >= s.size()) return std::string::npos; }
    else if (s[i] == '"') return i + 1;
  }
  return std::string::npos;
}
static size_t jsonValueEnd(const std::string& s, size_t at) {
  while (at < s.size() && (s[at] == ' ' || s[at] == '\n' || s[at] == '\r' || s[at] == '\t')) ++at;
  if (at >= s.size()) return std::string::npos;
  if (s[at] == '"') return jsonStringEnd(s, at);
  if (s[at] != '{' && s[at] != '[') {
    size_t end = s.find_first_of(",} \r\n\t", at);
    return end == std::string::npos ? s.size() : end;
  }
  unsigned depth = 0; bool string = false, escaped = false;
  for (size_t i = at; i < s.size(); ++i) {
    char c = s[i];
    if (string) { if (escaped) escaped = false; else if (c == '\\') escaped = true; else if (c == '"') string = false; continue; }
    if (c == '"') string = true;
    else if (c == '{' || c == '[') ++depth;
    else if (c == '}' || c == ']') { if (!depth || --depth == 0) return i + 1; }
  }
  return std::string::npos;
}
static bool rootValueRange(const std::string& manifest, const char* wanted, size_t& begin, size_t& end) {
  size_t p = 0;
  while (p < manifest.size() && manifest[p] <= ' ') ++p;
  if (p >= manifest.size() || manifest[p++] != '{') return false;
  while (p < manifest.size()) {
    while (p < manifest.size() && (manifest[p] <= ' ' || manifest[p] == ',')) ++p;
    if (p >= manifest.size() || manifest[p] == '}') return false;
    size_t keyEnd = jsonStringEnd(manifest, p);
    if (keyEnd == std::string::npos) return false;
    bool matches = manifest.compare(p, keyEnd - p, std::string("\"") + wanted + "\"") == 0;
    p = keyEnd;
    while (p < manifest.size() && manifest[p] <= ' ') ++p;
    if (p >= manifest.size() || manifest[p++] != ':') return false;
    while (p < manifest.size() && manifest[p] <= ' ') ++p;
    size_t valueBegin = p, valueEnd = jsonValueEnd(manifest, p);
    if (valueEnd == std::string::npos) return false;
    if (matches) { begin = valueBegin; end = valueEnd; return true; }
    p = valueEnd;
  }
  return false;
}
struct ThemeLayerRange { size_t begin, end; std::string id, type; };
static bool parseLayerRanges(const std::string& manifest, size_t begin, size_t end, std::vector<ThemeLayerRange>& layers) {
  if (begin >= end || manifest[begin] != '[' || manifest[end - 1] != ']') return false;
  size_t p = begin + 1;
  while (p < end - 1) {
    while (p < end - 1 && (manifest[p] <= ' ' || manifest[p] == ',')) ++p;
    if (p >= end - 1) break;
    size_t valueEnd = jsonValueEnd(manifest, p);
    if (valueEnd == std::string::npos || valueEnd > end || manifest[p] != '{' || layers.size() >= 32) return false;
    JsonDocument layer;
    if (deserializeJson(layer, manifest.data() + p, valueEnd - p) || !layer["id"].is<const char*>()) return false;
    ThemeLayerRange range{p, valueEnd, layer["id"].as<String>().c_str(), layer["type"] | ""};
    if (!smalltv::validId(range.id.c_str())) return false;
    for (const auto& old : layers) if (old.id == range.id) return false;
    layers.push_back(std::move(range));
    p = valueEnd;
  }
  return !layers.empty();
}
static bool readThemeManifest(ThemeFile& source, std::string& manifest, uint16_t& entries) {
  uint8_t header[8];
  if (source.size() < sizeof(header) || !source.read(0, header, sizeof(header)) || memcmp(header, "STH1", 4) || themeU16(header + 6)) return false;
  entries = themeU16(header + 4);
  if (!entries || entries > smalltv::MaxEntries) return false;
  uint32_t pos = 8;
  uint8_t record[6]; char path[121];
  for (uint16_t i = 0; i < entries; ++i) {
    if (pos > source.size() || source.size() - pos < 6 || !source.read(pos, record, 6)) return false;
    uint16_t pathLen = themeU16(record); uint32_t length = themeU32(record + 2); pos += 6;
    if (!pathLen || pathLen > 120 || pathLen > source.size() - pos || !source.read(pos, path, pathLen)) return false;
    path[pathLen] = 0; pos += pathLen;
    if (length > source.size() - pos) return false;
    if (!strcmp(path, "theme.json")) {
      if (!length || length > 16384) return false;
      manifest.resize(length);
      return source.read(pos, &manifest[0], length);
    }
    pos += length;
  }
  return false;
}
static bool writeManifestEdit(File& out, const std::string& manifest, size_t metaBegin, size_t metaEnd,
                              const std::string& metadata, size_t layerBegin, size_t layerEnd,
                              const std::vector<ThemeLayerRange>* orderedLayers) {
  const bool layersFirst = orderedLayers && layerBegin < metaBegin;
  size_t cursor = 0;
  auto raw = [&](size_t at, size_t count) { return !count || out.write(reinterpret_cast<const uint8_t*>(manifest.data() + at), count) == count; };
  auto writeLayers = [&]() {
    if (out.write('[') != 1) return false;
    for (size_t i = 0; i < orderedLayers->size(); ++i) {
      if (i && out.write(',') != 1) return false;
      const auto& layer = (*orderedLayers)[i];
      if (!raw(layer.begin, layer.end - layer.begin)) return false;
    }
    return out.write(']') == 1;
  };
  if (layersFirst) {
    if (!raw(cursor, layerBegin - cursor) || !writeLayers()) return false;
    cursor = layerEnd;
  }
  if (!raw(cursor, metaBegin - cursor) || out.write(reinterpret_cast<const uint8_t*>(metadata.data()), metadata.size()) != metadata.size()) return false;
  cursor = metaEnd;
  if (orderedLayers && !layersFirst) {
    if (!raw(cursor, layerBegin - cursor) || !writeLayers()) return false;
    cursor = layerEnd;
  }
  return raw(cursor, manifest.size() - cursor);
}
static bool rewriteThemeMetadata(const String& id, JsonObjectConst changes, const JsonArrayConst* requestedOrder,
                                 smalltv::Theme& edited, std::string& error) {
  const String target = "/themes/" + id + ".stheme", staging = "/themes/.edit", backup = target + ".bak";
  if (LittleFS.exists(backup)) {
    if (!LittleFS.exists(target)) LittleFS.rename(backup, target);
    else LittleFS.remove(backup);
  }
  ThemeFile original;
  if (!original.open(target)) { error = "Theme not found"; return false; }
  uint16_t entryCount = 0; std::string manifest;
  if (!readThemeManifest(original, manifest, entryCount)) { error = "Could not read theme manifest"; return false; }
  size_t begin, end;
  if (!rootValueRange(manifest, "theme", begin, end) || manifest[begin] != '{') { error = "Could not locate theme metadata"; return false; }
  JsonDocument meta;
  if (deserializeJson(meta, manifest.data() + begin, end - begin) || !meta.is<JsonObject>()) { error = "Invalid theme metadata"; return false; }
  JsonObject object = meta.as<JsonObject>();
  for (JsonPairConst pair : changes) {
    const char* key = pair.key().c_str();
    if (!strcmp(key, "id")) continue;
    if (strcmp(key, "name") && strcmp(key, "author") && strcmp(key, "version") && strcmp(key, "description") && strcmp(key, "category") && strcmp(key, "preview")) continue;
    if (!pair.value().is<const char*>()) { error = std::string("Invalid metadata field: ") + key; return false; }
    const char* value = pair.value().as<const char*>();
    size_t length = strlen(value), max = !strcmp(key, "description") ? 160 : !strcmp(key, "version") ? 32 : !strcmp(key, "category") ? 24 : !strcmp(key, "preview") ? 120 : 96;
    if (!length && (!strcmp(key, "description") || !strcmp(key, "category") || !strcmp(key, "preview"))) { object.remove(key); continue; }
    if (!length || length > max) { error = std::string("Invalid metadata length: ") + key; return false; }
    for (size_t i = 0; i < length; ++i) if ((uint8_t)value[i] < 32) { error = std::string("Control character in metadata: ") + key; return false; }
    if (!strcmp(key, "preview") && !smalltv::validPath(value)) { error = "Preview must be a safe relative asset path"; return false; }
    object[key] = value;
  }
  std::string replacement;
  serializeJson(meta, replacement);
  size_t layerBegin = 0, layerEnd = 0;
  std::vector<ThemeLayerRange> orderedLayers;
  size_t newManifestLength = manifest.size() - (end - begin) + replacement.size();
  if (requestedOrder) {
    if (!rootValueRange(manifest, "layers", layerBegin, layerEnd)) { error = "Could not locate theme layers"; return false; }
    std::vector<ThemeLayerRange> parsed;
    if (!parseLayerRanges(manifest, layerBegin, layerEnd, parsed) || requestedOrder->size() != parsed.size()) { error = "Layer order does not match this theme"; return false; }
    for (JsonVariantConst item : *requestedOrder) {
      if (!item.is<const char*>()) { error = "Layer order must contain layer IDs"; return false; }
      String layerId = item.as<String>();
      auto found = std::find_if(parsed.begin(), parsed.end(), [&](const ThemeLayerRange& layer) { return layer.id == layerId.c_str(); });
      if (found == parsed.end() || std::find_if(orderedLayers.begin(), orderedLayers.end(), [&](const ThemeLayerRange& layer) { return layer.id == found->id; }) != orderedLayers.end()) { error = "Layer order has an unknown or repeated ID"; return false; }
      orderedLayers.push_back(*found);
    }
    size_t arrayLength = 2 + (orderedLayers.empty() ? 0 : orderedLayers.size() - 1);
    for (const auto& layer : orderedLayers) arrayLength += layer.end - layer.begin;
    const size_t oldArrayLength = layerEnd - layerBegin;
    if (arrayLength >= oldArrayLength) newManifestLength += arrayLength - oldArrayLength;
    else newManifestLength -= oldArrayLength - arrayLength;
  }
  if (newManifestLength > 16384) { error = "Edited manifest exceeds 16 KiB"; return false; }
  uint32_t newSize = uint32_t(original.size() - manifest.size() + newManifestLength);
  if (newSize > smalltv::MaxPackage) { error = "Edited theme exceeds package size limit"; return false; }
  LittleFS.remove(staging); LittleFS.remove(backup);
  File out = LittleFS.open(staging, "w");
  if (!out) { error = "Cannot create edited package"; return false; }
  uint8_t header[8]; original.read(0, header, sizeof(header));
  bool ok = out.write(header, sizeof(header)) == sizeof(header);
  uint32_t pos = 8; uint8_t record[6]; char path[121]; uint8_t chunk[512];
  for (uint16_t i = 0; ok && i < entryCount; ++i) {
    if (!original.read(pos, record, sizeof(record))) { ok = false; break; }
    uint16_t pathLen = themeU16(record); uint32_t length = themeU32(record + 2); pos += 6;
    if (!pathLen || pathLen > 120 || !original.read(pos, path, pathLen)) { ok = false; break; }
    path[pathLen] = 0; pos += pathLen;
    if (!strcmp(path, "theme.json")) {
      uint8_t updated[6]; memcpy(updated, record, 6); themePut32(updated + 2, uint32_t(newSize));
      ok = out.write(updated, 6) == 6 && out.write(reinterpret_cast<const uint8_t*>(path), pathLen) == pathLen &&
        writeManifestEdit(out, manifest, begin, end, replacement, layerBegin, layerEnd, requestedOrder ? &orderedLayers : nullptr);
    } else {
      ok = out.write(record, 6) == 6 && out.write(reinterpret_cast<const uint8_t*>(path), pathLen) == pathLen;
      uint32_t copied = 0;
      while (ok && copied < length) {
        size_t n = min(sizeof(chunk), length - copied);
        ok = original.read(pos + copied, chunk, n) && out.write(chunk, n) == n;
        copied += n; yield();
      }
    }
    pos += length;
  }
  out.flush(); out.close();
  if (!ok) { LittleFS.remove(staging); error = "Could not write edited package"; return false; }
  meta.clear(); std::string().swap(manifest); std::string().swap(replacement);
  std::vector<ThemeLayerRange>().swap(orderedLayers);
  ThemeFile staged;
  if (!staged.open(staging)) { LittleFS.remove(staging); error = "Cannot validate edited package"; return false; }
  bool valid;
  {
    smalltv::Package package(staged);
    valid = package.load(edited, error) && edited.id == id.c_str();
  }
  if (!valid) { LittleFS.remove(staging); if (error.empty()) error = "Edited package identity mismatch"; return false; }
  staged.close();
  original.close();
  if (!LittleFS.rename(target, backup)) { LittleFS.remove(staging); error = "Cannot preserve original theme"; return false; }
  if (!LittleFS.rename(staging, target)) {
    LittleFS.rename(backup, target); LittleFS.remove(staging); error = "Cannot publish edited theme"; return false;
  }
  LittleFS.remove(backup);
  return true;
}
static bool applyThemeMetadata(const String& id, JsonObjectConst changes, const JsonArrayConst* order, std::string& error) {
  g_themeMode.release(UINT32_MAX);
  smalltv::Theme updated;
  if (!rewriteThemeMetadata(id, changes, order, updated, error)) return false;
  smalltv::InstalledTheme entry;
  entry.id = updated.id; entry.name = updated.name; entry.author = updated.author;
  entry.version = updated.version; entry.description = updated.description;
  entry.category = updated.category; entry.preview = updated.preview;
  { File f = LittleFS.open("/themes/" + id + ".stheme", "r"); entry.bytes = f ? f.size() : 0; }
  entry.valid = true;
  updateIndex(id, &entry);
  for (uint8_t i = 0; i < settings->themeAliasCount; ++i) if (id == settings->themeAliases[i].id) {
    for (uint8_t j = i + 1; j < settings->themeAliasCount; ++j) settings->themeAliases[j - 1] = settings->themeAliases[j];
    --settings->themeAliasCount; memset(&settings->themeAliases[settings->themeAliasCount], 0, sizeof(settings->themeAliases[0]));
    saveSettings(*settings); break;
  }
  appInvalidate();
  return true;
}
void editThemeMetadata() {
  if (!auth()) return;
  if (!server->hasArg("plain") || server->arg("plain").length() > 768) { reply(400, "Expected JSON with id and metadata"); return; }
  JsonDocument doc;
  if (deserializeJson(doc, server->arg("plain")) || !doc["id"].is<const char*>() || !doc["metadata"].is<JsonObjectConst>()) { reply(400, "Expected JSON with id and metadata"); return; }
  String id = doc["id"].as<String>();
  if (!smalltv::validId(id.c_str()) || id.length() != strlen(id.c_str())) { reply(400, "Invalid theme ID"); return; }
  std::string error;
  if (!applyThemeMetadata(id, doc["metadata"].as<JsonObjectConst>(), nullptr, error)) { reply(400, error.c_str()); return; }
  reply(200, id);
}
void renameTheme() {
  if (!auth()) return;
  if (!server->hasArg("plain") || server->arg("plain").length() > 256) { reply(400, "Expected JSON with id and name"); return; }
  JsonDocument input, changes;
  if (deserializeJson(input, server->arg("plain")) || !input["id"].is<const char*>() || !input["name"].is<const char*>()) { reply(400, "Expected JSON with id and name"); return; }
  String id = input["id"].as<String>();
  if (!smalltv::validId(id.c_str()) || id.length() != strlen(id.c_str())) { reply(400, "Invalid theme ID"); return; }
  changes["name"] = input["name"];
  std::string error;
  if (!applyThemeMetadata(id, changes.as<JsonObjectConst>(), nullptr, error)) { reply(400, error.c_str()); return; }
  reply(200, id);
}
void reorderThemeLayers() {
  if (!auth()) return;
  if (!server->hasArg("plain") || server->arg("plain").length() > 2048) { reply(400, "Expected JSON with id and layer order"); return; }
  JsonDocument doc;
  if (deserializeJson(doc, server->arg("plain")) || !doc["id"].is<const char*>() || !doc["order"].is<JsonArrayConst>()) { reply(400, "Expected JSON with id and layer order"); return; }
  String id = doc["id"].as<String>();
  if (!smalltv::validId(id.c_str()) || id.length() != strlen(id.c_str())) { reply(400, "Invalid theme ID"); return; }
  JsonDocument empty;
  empty.to<JsonObject>();
  JsonArrayConst order = doc["order"].as<JsonArrayConst>();
  std::string error;
  if (!applyThemeMetadata(id, empty.as<JsonObjectConst>(), &order, error)) { reply(400, error.c_str()); return; }
  reply(200, id);
}
void getThemeLayers() {
  if (!auth()) return;
  String id = server->arg("id");
  if (!smalltv::validId(id.c_str()) || id.length() != strlen(id.c_str())) { reply(400, "Invalid theme ID"); return; }
  g_themeMode.release();
  ThemeFile source;
  if (!source.open("/themes/" + id + ".stheme")) { reply(404, "Theme not installed"); return; }
  uint16_t count = 0; std::string manifest;
  if (!readThemeManifest(source, manifest, count)) { reply(400, "Cannot read theme manifest"); return; }
  size_t begin, end;
  std::vector<ThemeLayerRange> layers;
  if (!rootValueRange(manifest, "layers", begin, end) || !parseLayerRanges(manifest, begin, end, layers)) { reply(400, "Invalid theme layers"); return; }
  JsonDocument response;
  response["id"] = id;
  JsonArray array = response["layers"].to<JsonArray>();
  for (const auto& layer : layers) {
    JsonObject item = array.add<JsonObject>();
    item["id"] = layer.id;
    item["type"] = layer.type;
  }
  String body; serializeJson(response, body);
  server->send(200, "application/json", body);
}
// The three steps of a list after the directory walk each run in a function
// of their own, called one after another: kept out of listThemes() (no
// inlining), their locals are on the loop stack only while that step runs.
// Inlined, they made listThemes()'s own frame 576 B; on 3.1.0 the sidecar
// code did the same, and the list lost 160 B of stack (#41).

// The index, read once: a usable line fills the listed package with the
// same id and size. Returns those packages as bits (MaxThemes fits) and sets
// `rewrite` when a line matches no package.
using Listing = smalltv::BlockList<smalltv::ListedTheme>;
__attribute__((noinline)) uint32_t fillFromIndex(Listing& list, bool& rewrite) {
  uint32_t listed = 0;
  ThemeFile file;
  if (!file.open(indexPath)) return 0;
  smalltv::ThemeIndexReader index(file);
  smalltv::InstalledTheme line;
  for (smalltv::IndexEntry kind; (kind = index.next(line)) != smalltv::IndexEntry::End;) {
    int match = -1;
    for (unsigned i = 0; kind == smalltv::IndexEntry::Usable && match < 0 && i < list.size(); ++i) {
      if (!(listed & (1u << i)) && line.id == list[i].id() && line.bytes == list[i].bytes) match = i;
    }
    if (match < 0) {
      rewrite = true;
      continue;
    }
    smalltv::ListedTheme& t = list[match];
    t.set(t.id(), line.name.c_str(), line.author.c_str(), line.version.c_str(), line.description.c_str(), line.category.c_str(), line.preview.c_str(), t.error());
    t.valid = true;
    listed |= 1u << match;
  }
  return listed;
}
// Fills a listed package from the package itself, the way a selection
// checks it. True when it is valid.
__attribute__((noinline)) bool inspectInto(smalltv::ListedTheme& t) {
  String id = t.id();
  smalltv::InstalledTheme info;
  {
    ThemeFile file;
    file.open("/themes/" + id + ".stheme");
    info = smalltv::inspectInstalledTheme(file, id.c_str());
  }
  t.set(id.c_str(), info.name.c_str(), info.author.c_str(), info.version.c_str(), info.description.c_str(), info.category.c_str(), info.preview.c_str(), info.error.c_str());
  t.valid = info.valid;
  return info.valid;
}
// The index from a listing: one line per valid package.
__attribute__((noinline)) void writeIndex(const Listing& list) {
  bool written;
  {
    File to = LittleFS.open(indexNext, "w");
    FileSink sink(to);
    written = bool(to);
    for (const smalltv::ListedTheme& t : list) {
      if (!written) break;
      if (!t.valid) continue;
      smalltv::InstalledTheme entry;
      entry.id = t.id();
      entry.name = t.name();
      entry.author = t.author();
      entry.version = t.version();
      entry.description = t.description();
      entry.category = t.category();
      entry.preview = t.preview();
      entry.bytes = t.bytes;
      std::string line = smalltv::themeIndexLine(entry);
      written = sink.write(line.data(), line.size());
    }
  }
  publishIndex(written);
}
// The body of the current reply, piece by piece, with a pause after each
// (CONTRIBUTING: a loop over input-sized data yields).
struct ReplySink : smalltv::Sink {
  bool write(const char* data, size_t count) override {
    server->sendContent(data, count);
    yield();
    return true;
  }
};
struct CountSink : smalltv::Sink {
  size_t bytes = 0;
  bool write(const char*, size_t count) override {
    bytes += count;
    return true;
  }
};
// The reply, written twice the same way: once only to count its bytes for
// the Content-Length, then straight to the client in 128-byte pieces, never
// whole in one block of heap (#63). A step of its own like the three above,
// so its buffer is on the loop stack only while it sends.
__attribute__((noinline)) void sendList(const Listing& list, const String& error, size_t storage) {
  CountSink count;
  smalltv::writeThemeList<128>(settings->themeId.c_str(), error.c_str(), storage, list, count);
  server->setContentLength(count.bytes);
  server->send(200, "application/json", "");
  ReplySink sink;
  smalltv::writeThemeList<128>(settings->themeId.c_str(), error.c_str(), storage, list, sink);
}
// GET /api/themes: every installed package's metadata and validity, without
// requiring any of them to be the currently active theme. A package with a
// matching line in the index (written at install, see ThemeIndexReader) is
// listed from it; any other one goes through inspectInstalledTheme(), the
// same validator a selection would use, so an invalid or corrupt package
// still gets listed (with its error), just not selectable. When that finds a
// valid package, or the index has a line that matches no package, the index
// is written again, so a package is only inspected once.
void listThemes() {
  if (!auth()) return;
  // The running theme's error and the free storage as the list starts.
  const String error = g_themeMode.error();
  const size_t storage = freeBytes();
  Listing list;
  list.reserve(MaxThemes);
  // First each package's id and size, from the directory alone; the other
  // fields as inspectInstalledTheme() gives them for a package that does not
  // load, until something better is known.
  File dir = LittleFS.open("/themes", "r");
  for (File entry = dir.openNextFile(); entry && list.size() < MaxThemes; entry = dir.openNextFile()) {
    String name = entry.name();
    if (entry.isDirectory() || !name.endsWith(".stheme")) continue;
    int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.substring(slash + 1);
    String id = name.substring(0, name.length() - 7);
    if (!smalltv::validId(id.c_str())) continue;
    smalltv::ListedTheme& t = list.emplace_back();
    t.set(id.c_str(), id.c_str(), "", "", "", "other", "", "");
    t.bytes = entry.size();
    yield();
  }
  dir.close();
  // Then the index; any package it does not cover is inspected. Inspecting
  // a package parses it; before the first one that needs it, free the
  // running theme if the heap is short (issue #30). Its error is already
  // copied above.
  bool rewrite = false;  // the index needs writing again
  const uint32_t listed = fillFromIndex(list, rewrite);
  bool released = false;
  for (unsigned i = 0; i < list.size(); ++i) {
    if (listed & (1u << i)) continue;
    if (!released) {
      g_themeMode.release();
      released = true;
    }
    if (inspectInto(list[i])) rewrite = true;
    yield();
  }
  if (rewrite) writeIndex(list);
  for (smalltv::ListedTheme& t : list) {
    for (uint8_t i = 0; i < settings->themeAliasCount; ++i) {
      const auto& alias = settings->themeAliases[i];
      if (strcmp(alias.id, t.id())) continue;
      t.set(t.id(), alias.name, t.author(), t.version(), t.description(), t.category(), t.preview(), t.error());
      break;
    }
  }
  sendList(list, error, storage);
}
// POST /api/themes/select: re-validates the package before committing to it,
// so a corrupt or invalid installed theme can never become the active
// selection (it can still be listed and removed, just not selected). Settings
// are rolled back in memory if the save itself fails, so a failed selection
// never leaves `settings` disagreeing with what was actually persisted.
void selectTheme() {
  if (!auth()) return;
  String id;
  if (!requestId(id)) return;
  // Validation parses a second theme; free the running one first if the heap
  // is short (issue #30).
  g_themeMode.release();
  {
    ThemeFile file;
    smalltv::Theme theme;
    std::string error;
    if (!file.open("/themes/" + id + ".stheme")) {
      reply(404, "Theme not installed");
      return;
    }
    smalltv::Package package(file);
    if (!package.load(theme, error) || theme.id != id.c_str()) {
      reply(400, "Invalid installed theme");
      return;
    }
  }
  String oldId = settings->themeId;
  uint8_t oldMode = settings->mode;
#if WITH_THEME_PLAYLIST
  bool oldPlaylistEnabled = settings->themePlaylistEnabled;
#endif
  settings->themeId = id;
  settings->mode = MODE_THEME;
#if WITH_THEME_PLAYLIST
  settings->themePlaylistEnabled = false;
#endif
  if (!saveSettings(*settings)) {
    settings->themeId = oldId;
    settings->mode = oldMode;
#if WITH_THEME_PLAYLIST
    settings->themePlaylistEnabled = oldPlaylistEnabled;
#endif
    reply(500, "Could not save selection");
    return;
  }
  clockReapply(*settings);
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
  // Choosing a valid user theme is the explicit recovery action. Clear any
  // accumulated crash streak so the next reboot can start that selection.
  appClearCrashStreak();
#endif
  appInvalidate();
  reply(200, id);
}
// POST /api/themes/delete: refuses to remove a package that is both selected
// and currently valid (it would silently blank the active display); removing
// a selected-but-invalid one is allowed, since keeping it installed serves no
// purpose once it can never render.
void deleteTheme() {
  if (!auth()) return;
  String id;
  if (!requestId(id)) return;
  String path = "/themes/" + id + ".stheme";
  if (!LittleFS.exists(path)) {
    reply(404, "Theme not found");
    return;
  }
  bool onScreen = settings->mode == MODE_THEME || (settings->mode == MODE_CAROUSEL && settings->carouselTheme);
  if (settings->themeId == id && onScreen) {
    // Inspecting parses the package; free the running copy first if the heap
    // is short (issue #30).
    g_themeMode.release();
    ThemeFile file;
    file.open(path);
    auto info = smalltv::inspectInstalledTheme(file, id.c_str());
    if (info.valid) {
      reply(409, "Switch away from theme mode or untick it in the carousel before removing this theme");
      return;
    }
  }
  if (settings->themeId == id) {
    settings->themeId = "";
    if (!saveSettings(*settings)) {
      settings->themeId = id;
      reply(500, "Could not clear saved selection");
      return;
    }
    appInvalidate();
  }
  if (!LittleFS.remove(path)) {
    reply(404, "Theme not found or could not be removed");
    return;
  }
#if WITH_THEME_PLAYLIST
  Settings::ThemePlaylistItem removedItem{};
  bool oldPlaylistEnabled = settings->themePlaylistEnabled;
  for (uint8_t i = 0; i < settings->themePlaylistCount; ++i) {
    if (id != settings->themePlaylist[i].id) continue;
    removedItem = settings->themePlaylist[i];
    for (uint8_t j = i + 1; j < settings->themePlaylistCount; ++j)
      settings->themePlaylist[j - 1] = settings->themePlaylist[j];
    --settings->themePlaylistCount;
    memset(&settings->themePlaylist[settings->themePlaylistCount], 0,
           sizeof(settings->themePlaylist[0]));
    if (!settings->themePlaylistCount) settings->themePlaylistEnabled = false;
    if (!saveSettings(*settings)) {
      for (uint8_t j = settings->themePlaylistCount; j > i; --j)
        settings->themePlaylist[j] = settings->themePlaylist[j - 1];
      settings->themePlaylist[i] = removedItem;
      ++settings->themePlaylistCount;
      settings->themePlaylistEnabled = oldPlaylistEnabled;
      appInvalidate();
      reply(500, "Theme removed, but playlist settings could not be saved");
      return;
    }
    break;
  }
#endif
  updateIndex(id, nullptr);
#if WITH_THEME_PLAYLIST
  appInvalidate();
#endif
  reply(200, id);
}
uint16_t blend565(uint16_t fg, uint8_t alpha, uint16_t bg) {
  if (alpha == 255) return fg;
  uint32_t r = (((fg >> 11) & 31) * alpha + ((bg >> 11) & 31) * (255 - alpha) + 127) / 255;
  uint32_t g = (((fg >> 5) & 63) * alpha + ((bg >> 5) & 63) * (255 - alpha) + 127) / 255;
  uint32_t b = ((fg & 31) * alpha + (bg & 31) * (255 - alpha) + 127) / 255;
  return static_cast<uint16_t>((r << 11) | (g << 5) | b);
}
void themePreview() {
  if (!auth()) return;
  String id = server->arg("id");
  if (!smalltv::validId(id.c_str()) || id.length() != strlen(id.c_str())) { reply(400, "Invalid theme ID"); return; }
  g_themeMode.release();
  ThemeFile file;
  if (!file.open("/themes/" + id + ".stheme")) { reply(404, "Theme not installed"); return; }
  smalltv::Package package(file);
  smalltv::Theme theme;
  std::string error;
  if (!package.load(theme, error) || theme.id != id.c_str()) { reply(400, "Invalid installed theme"); return; }
  if (theme.preview.empty()) { reply(404, "This theme has no preview image"); return; }
  smalltv::AssetHandle asset = package.resolve(theme.preview);
  uint16_t width=0,height=0;
  if (asset == smalltv::InvalidAsset || !package.imageSize(theme.preview,width,height)) { reply(404, "Preview image unavailable"); return; }
  const uint32_t rowBytes = uint32_t(width) * 3;
  const uint32_t paddedRow = (rowBytes + 3) & ~3UL;
  const uint32_t total = 54 + paddedRow * height;
  uint8_t header[54] = {'B','M'};
  auto put16=[&](int at,uint16_t v){header[at]=uint8_t(v);header[at+1]=uint8_t(v>>8);};
  auto put32=[&](int at,uint32_t v){header[at]=uint8_t(v);header[at+1]=uint8_t(v>>8);header[at+2]=uint8_t(v>>16);header[at+3]=uint8_t(v>>24);};
  put32(2,total);put32(10,54);put32(14,40);put32(18,width);put32(22,height);put16(26,1);put16(28,24);put32(34,paddedRow*height);
  server->setContentLength(total);
  server->send(200,"image/bmp","");
  server->sendContent(reinterpret_cast<const char*>(header),sizeof(header));
  uint16_t colors[240];uint8_t alpha[240];uint8_t row[240*3+3];
  for (int y=height-1;y>=0;--y) {
    if (!package.row(asset,y,0,width,colors,alpha)) return;
    size_t n=0;
    for (uint16_t x=0;x<width;++x) {
      uint16_t c=blend565(colors[x],alpha[x],theme.background);
      row[n++]=uint8_t((c&31)*255/31);row[n++]=uint8_t(((c>>5)&63)*255/63);row[n++]=uint8_t(((c>>11)&31)*255/31);
    }
    while (n<paddedRow) row[n++]=0;
    server->sendContent(reinterpret_cast<const char*>(row),n);
    yield();
  }
}
}
// Registers the theme API routes on the shared web server; `requireAuth`
// is the same digest-auth check every other route in the device uses, so
// theme install/select/delete get identical protection without duplicating it.
// `requestAllowed` is the CSRF / DNS-rebinding check without the 403 reply: the
// upload callback runs before finishUpload() and must refuse to stage the body.
void themeWebBegin(WebServerClass& web, Settings& s, bool (*requireAuth)(), bool (*requestAllowed)()) {
  server = &web;
  settings = &s;
  auth = requireAuth;
  allowed = requestAllowed;
  web.on("/api/themes", HTTP_GET, listThemes);
  web.on("/api/themes/preview", HTTP_GET, themePreview);
  web.on("/api/themes/install", HTTP_POST, finishUpload, receiveUpload);
  web.on("/api/themes/select", HTTP_POST, selectTheme);
  web.on("/api/themes/delete", HTTP_POST, deleteTheme);
  web.on("/api/themes/rename", HTTP_POST, renameTheme);
  web.on("/api/themes/metadata", HTTP_POST, editThemeMetadata);
  web.on("/api/themes/layers", HTTP_GET, getThemeLayers);
  web.on("/api/themes/layers/order", HTTP_POST, reorderThemeLayers);
  removeSidecars();
#if defined(SMALLTV_ESP8266)
  // The web server reserves its 2 KB upload buffer before the first call to
  // receiveUpload(), so the release there comes too late: beside a running
  // theme the heap fell to 512 B at that point, and the Wi-Fi driver crashed
  // on an empty heap. This hook runs once the request line is read, before
  // the headers and the body, so an install frees the theme first. (The
  // ESP32 WebServer has no hooks, and has the heap.)
  web.addHook([](const String& method, const String& url, WiFiClient*, WebServerClass::ContentTypeFunction) {
    if (method == "POST" && url == "/api/themes/install") g_themeMode.release(UINT32_MAX);
    return WebServerClass::CLIENT_REQUEST_CAN_CONTINUE;
  });
#endif
}
#endif
