#include "ThemeWeb.h"
#include "ThemeCatalog.h"
#include <LittleFS.h>
#include <algorithm>

namespace {
WebServerClass* g_server = nullptr;
File g_upload;
String g_uploadError;
bool g_uploadAllowed = false;
bool g_uploadEnded = false;
unsigned g_uploadParts = 0;
uint32_t g_uploaded = 0;
size_t g_uploadBudget = 0;

constexpr unsigned MaxThemes = 16;
constexpr size_t SettingsReserveMinimum = 16 * 1024;
const char* const ThemeDirectory = "/themes";
const char* const UploadStagingPath = "/themes/.upload";
const char* const ThemeIndexPath = "/themes/.index";
const char* const ThemeIndexNextPath = "/themes/.index.new";

class ThemeFile : public smalltv::Source {
 public:
  bool open(const String& path) {
    file_ = LittleFS.open(path, "r");
    return bool(file_);
  }
  uint32_t size() const override { return file_ ? file_.size() : 0; }
  bool read(uint32_t offset, void* destination, size_t count) override {
    if (!file_ || !file_.seek(offset, SeekSet)) return false;
    return file_.read(static_cast<uint8_t*>(destination), count) == count;
  }
  void pause() override { yield(); }
 private:
  File file_;
};

struct FileSink : smalltv::Sink {
  File& file;
  explicit FileSink(File& value) : file(value) {}
  bool write(const char* data, size_t count) override {
    return file.write(reinterpret_cast<const uint8_t*>(data), count) == count;
  }
};

size_t freeStorageBytes() {
  FSInfo info;
  if (!LittleFS.info(info) || info.usedBytes > info.totalBytes) return 0;
  return info.totalBytes - info.usedBytes;
}

unsigned installedThemeCount() {
  unsigned count = 0;
  Dir directory = LittleFS.openDir(ThemeDirectory);
  while (directory.next()) {
    if (directory.fileName().endsWith(".stheme")) ++count;
    yield();
  }
  return count;
}

void reply(int code, const String& message) {
  JsonDocument document;
  document[code < 300 ? "id" : "error"] = message;
  String body;
  serializeJson(document, body);
  g_server->send(code, "application/json", body);
}

void publishIndex(bool written) {
  // A missing index is recoverable: the next listing validates and rebuilds it.
  LittleFS.remove(ThemeIndexPath);
  if (!written || !LittleFS.rename(ThemeIndexNextPath, ThemeIndexPath)) {
    LittleFS.remove(ThemeIndexNextPath);
  }
}

void updateIndex(const String& id, const smalltv::InstalledTheme* added) {
  bool written = false;
  {
    ThemeFile previous;
    previous.open(ThemeIndexPath);
    File next = LittleFS.open(ThemeIndexNextPath, "w");
    FileSink sink(next);
    written = next && smalltv::copyThemeIndex(previous, sink, id.c_str(), added);
  }
  publishIndex(written);
}

void resetUpload() {
  g_upload.close();
  LittleFS.remove(UploadStagingPath);
  g_uploadAllowed = false;
  g_uploadEnded = false;
  g_uploadParts = 0;
  g_uploaded = 0;
  g_uploadBudget = 0;
  g_uploadError = "";
}

void receiveUpload() {
  HTTPUpload& incoming = g_server->upload();
  if (incoming.status == UPLOAD_FILE_START) {
    ++g_uploadParts;
    if (g_uploadParts > 1) {
      g_uploadError = "Upload exactly one .stheme package";
      g_upload.close();
      return;
    }

    g_uploadError = "";
    g_uploaded = 0;
    g_uploadEnded = false;
    g_uploadAllowed = true;

    if (!incoming.filename.endsWith(".stheme")) {
      g_uploadError = "Expected a .stheme file";
      return;
    }
    if (installedThemeCount() >= MaxThemes) {
      g_uploadError = "Theme limit reached (16 packages)";
      return;
    }

    // Leave room for a settings rewrite and filesystem metadata.
    File settings = LittleFS.open("/config.json", "r");
    size_t reserve = settings ? settings.size() + 4096 : SettingsReserveMinimum;
    if (settings) settings.close();
    if (reserve < SettingsReserveMinimum) reserve = SettingsReserveMinimum;

    const size_t free = freeStorageBytes();
    if (free <= reserve) {
      g_uploadError = "Not enough filesystem space; remove a theme first";
      return;
    }
    g_uploadBudget = std::min<size_t>(smalltv::MaxPackage, free - reserve);
    LittleFS.remove(UploadStagingPath);
    g_upload = LittleFS.open(UploadStagingPath, "w");
    if (!g_upload) g_uploadError = "Cannot create staging file";
  } else if (incoming.status == UPLOAD_FILE_WRITE) {
    if (!g_uploadAllowed || g_uploadError.length()) return;
    if (!g_upload || g_uploaded > g_uploadBudget ||
        incoming.currentSize > g_uploadBudget - g_uploaded) {
      g_uploadError = "Package exceeds available storage or 3 MiB";
      g_upload.close();
      return;
    }
    if (g_upload.write(incoming.buf, incoming.currentSize) != incoming.currentSize) {
      g_uploadError = "Filesystem write failed";
      g_upload.close();
      return;
    }
    g_uploaded += incoming.currentSize;
  } else if (incoming.status == UPLOAD_FILE_END) {
    g_upload.close();
    g_uploadEnded = g_uploadAllowed && !g_uploadError.length() &&
                    g_uploadParts == 1 && g_uploaded == incoming.totalSize;
    if (!g_uploadEnded && !g_uploadError.length()) g_uploadError = "Incomplete package upload";
  } else if (incoming.status == UPLOAD_FILE_ABORTED) {
    resetUpload();
    g_uploadError = "Upload aborted";
  }
  yield();
}

void finishUpload() {
  g_upload.close();
  if (!g_uploadAllowed || !g_uploadEnded || g_uploadError.length()) {
    const String message = g_uploadError.length() ? g_uploadError : String("No complete package uploaded");
    reply(400, message);
    resetUpload();
    return;
  }

  bool valid = false;
  smalltv::Theme theme;
  std::string error;
  {
    ThemeFile staged;
    if (staged.open(UploadStagingPath)) {
      smalltv::Package package(staged);
      valid = package.load(theme, error);
    } else {
      error = "Cannot read staged package";
    }
  }

  if (!valid) {
    reply(400, error.c_str());
    resetUpload();
    return;
  }

  const String id = theme.id.c_str();
  const String destination = String(ThemeDirectory) + "/" + id + ".stheme";
  if (LittleFS.exists(destination)) {
    reply(409, "Theme ID already installed; remove it or use a different ID");
    resetUpload();
    return;
  }

  // The staged file becomes visible only after the complete package passed validation.
  if (!LittleFS.rename(UploadStagingPath, destination)) {
    reply(500, "Could not publish validated package");
    resetUpload();
    return;
  }

  smalltv::InstalledTheme entry;
  entry.id = theme.id;
  entry.name = theme.name;
  entry.author = theme.author;
  entry.version = theme.version;
  entry.bytes = g_uploaded;
  entry.valid = true;
  updateIndex(id, &entry);

  reply(201, id);
  resetUpload();
}

bool requestThemeId(String& id) {
  if (!g_server->hasArg("plain")) {
    reply(400, "Expected JSON containing an id");
    return false;
  }
  const String body = g_server->arg("plain");
  if (body.length() > 128) {
    reply(413, "Theme ID request is too large");
    return false;
  }
  JsonDocument document;
  if (deserializeJson(document, body) || !document["id"].is<const char*>()) {
    reply(400, "Expected JSON containing an id");
    return false;
  }
  id = document["id"].as<String>();
  if (!smalltv::validId(id.c_str()) || id.length() != strlen(id.c_str())) {
    reply(400, "Invalid theme ID");
    return false;
  }
  return true;
}

using Listing = smalltv::BlockList<smalltv::ListedTheme>;

uint32_t fillFromIndex(Listing& list, bool& rewrite) {
  uint32_t listed = 0;
  ThemeFile file;
  if (!file.open(ThemeIndexPath)) return 0;
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
    smalltv::ListedTheme& theme = list[match];
    theme.set(theme.id(), line.name.c_str(), line.author.c_str(), line.version.c_str(), theme.error());
    theme.valid = true;
    listed |= 1u << match;
  }
  return listed;
}

bool inspectInto(smalltv::ListedTheme& listed) {
  const String id = listed.id();
  smalltv::InstalledTheme info;
  {
    ThemeFile file;
    file.open(String(ThemeDirectory) + "/" + id + ".stheme");
    info = smalltv::inspectInstalledTheme(file, id.c_str());
  }
  listed.set(id.c_str(), info.name.c_str(), info.author.c_str(), info.version.c_str(), info.error.c_str());
  listed.valid = info.valid;
  return info.valid;
}

void writeIndex(const Listing& list) {
  bool written = false;
  {
    File next = LittleFS.open(ThemeIndexNextPath, "w");
    FileSink sink(next);
    written = bool(next);
    for (const smalltv::ListedTheme& theme : list) {
      if (!written) break;
      if (!theme.valid) continue;
      smalltv::InstalledTheme entry;
      entry.id = theme.id();
      entry.name = theme.name();
      entry.author = theme.author();
      entry.version = theme.version();
      entry.bytes = theme.bytes;
      const std::string line = smalltv::themeIndexLine(entry);
      written = sink.write(line.data(), line.size());
    }
  }
  publishIndex(written);
}

struct ReplySink : smalltv::Sink {
  bool write(const char* data, size_t count) override {
    const bool sent = g_server->client().write(reinterpret_cast<const uint8_t*>(data), count) == count;
    yield();
    return sent;
  }
  void abandon() override { g_server->client().abort(); }
};

struct CountSink : smalltv::Sink {
  size_t bytes = 0;
  bool write(const char*, size_t count) override {
    bytes += count;
    return true;
  }
};

void sendList(const Listing& list, size_t storage) {
  CountSink count;
  smalltv::writeThemeList<128>("", "", static_cast<uint32_t>(storage), list, count);
  g_server->setContentLength(count.bytes);
  g_server->send(200, "application/json", "");
  ReplySink sink;
  g_server->client().setTimeout(1500);
  smalltv::writeThemeList<128>("", "", static_cast<uint32_t>(storage), list, sink);
}

void listThemes() {
  Listing list;
  list.reserve(MaxThemes);

  Dir directory = LittleFS.openDir(ThemeDirectory);
  while (directory.next() && list.size() < MaxThemes) {
    String name = directory.fileName();
    if (!name.endsWith(".stheme")) continue;
    const int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.substring(slash + 1);
    const String id = name.substring(0, name.length() - 7);
    if (!smalltv::validId(id.c_str())) continue;

    smalltv::ListedTheme& item = list.emplace_back();
    item.set(id.c_str(), id.c_str(), "", "", "");
    item.bytes = directory.fileSize();
    yield();
  }

  bool rewrite = false;
  const uint32_t indexed = fillFromIndex(list, rewrite);
  for (unsigned i = 0; i < list.size(); ++i) {
    if (indexed & (1u << i)) continue;
    if (inspectInto(list[i])) rewrite = true;
    yield();
  }
  if (rewrite) writeIndex(list);
  sendList(list, freeStorageBytes());
}

void deleteTheme() {
  String id;
  if (!requestThemeId(id)) return;
  const String path = String(ThemeDirectory) + "/" + id + ".stheme";
  if (!LittleFS.exists(path)) {
    reply(404, "Theme not found");
    return;
  }
  if (!LittleFS.remove(path)) {
    reply(500, "Could not remove theme package");
    return;
  }
  updateIndex(id, nullptr);
  reply(200, id);
}
}  // namespace

void themeWebBegin(WebServerClass& server) {
  g_server = &server;
  LittleFS.mkdir(ThemeDirectory);
  // An interrupted upload must never appear as an installed package.
  LittleFS.remove(UploadStagingPath);
  LittleFS.remove(ThemeIndexNextPath);
  server.on("/api/themes", HTTP_GET, listThemes);
  server.on("/api/themes/install", HTTP_POST, finishUpload, receiveUpload);
  server.on("/api/themes/delete", HTTP_POST, deleteTheme);
}
