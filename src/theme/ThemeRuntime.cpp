#include "ThemeRuntime.h"
#include "ThemePackage.h"
#include "../Platform.h"
#include <LittleFS.h>
#include <stdio.h>
#include <string.h>
#include <utility>
#include <vector>

namespace {
class ThemeFileSource final : public smalltv::Source {
 public:
  bool open(const char* id) {
    close();
    char path[64];
    const int length = snprintf(path, sizeof(path), "/themes/%s.stheme", id);
    if (length <= 0 || static_cast<size_t>(length) >= sizeof(path)) return false;
    file_ = LittleFS.open(path, "r");
    return bool(file_);
  }
  void close() { file_.close(); }
  uint32_t size() const override { return file_ ? static_cast<uint32_t>(file_.size()) : 0; }
  bool read(uint32_t offset, void* destination, size_t count) override {
    if (!file_ || !file_.seek(offset, SeekSet)) return false;
    return file_.read(static_cast<uint8_t*>(destination), count) == static_cast<int>(count);
  }
  void pause() override { yield(); }
 private:
  File file_;
};

ThemeFileSource g_source;
smalltv::Package g_package(g_source);
smalltv::Engine g_engine;
smalltv::display::DisplayPanel* g_display = nullptr;
bool g_active = false;
uint32_t g_lastUpdate = 0;
char g_activeId[49] = {};
std::string g_error;

bool validThemeId(const char* id) {
  if (!id) return false;
  const size_t length = strlen(id);
  if (!length || length > 48) return false;
  for (size_t i = 0; i < length; ++i) {
    const char c = id[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '-' || c == '_')) return false;
  }
  return true;
}

void releaseRuntime() {
  g_active = false;
  g_activeId[0] = '\0';
  g_engine.setTheme(smalltv::Theme{}, millis());
  g_package.reset();
  g_source.close();
}
}  // namespace

namespace smalltv {
void themeRuntimeBegin(display::DisplayPanel& display) {
  g_display = &display;
}

void themeRuntimeStop() {
  releaseRuntime();
  g_error.clear();
}

bool themeRuntimeSelect(const char* id, std::string& error) {
  if (!id || !id[0]) {
    themeRuntimeStop();
    error.clear();
    return true;
  }
  if (!validThemeId(id)) {
    error = "Invalid theme ID";
    g_error = error;
    return false;
  }
  if (g_active && strcmp(g_activeId, id) == 0) {
    error.clear();
    return true;
  }

  releaseRuntime();
  if (!g_display || !g_display->ready()) {
    error = "Display is not initialized";
    g_error = error;
    return false;
  }

  // Keep a reserve for Wi-Fi, HTTP replies and the theme render stack.
  const uint32_t freeBefore = ESP.getFreeHeap();
  const uint32_t maxBefore = platformMaxFreeBlock();
  if (freeBefore < 12UL * 1024UL || maxBefore < 2048UL) {
    error = "Not enough free RAM to load a theme (need 12 KiB free heap and a 2 KiB block)";
    g_error = error;
    return false;
  }

  if (!g_source.open(id)) {
    error = "Theme package not found or cannot be opened";
    g_error = error;
    return false;
  }

  Theme theme;
  if (!g_package.load(theme, error)) {
    if (error.empty()) error = "Theme package validation failed";
    releaseRuntime();
    g_error = error;
    return false;
  }

  g_engine.setTheme(std::move(theme), millis());
  const uint32_t freeAfter = ESP.getFreeHeap();
  const uint32_t maxAfter = platformMaxFreeBlock();
  if (freeAfter < 6UL * 1024UL || maxAfter < 1024UL) {
    error = "Theme rejected: less than 6 KiB heap reserve would remain";
    releaseRuntime();
    g_error = error;
    return false;
  }

  strncpy(g_activeId, id, sizeof(g_activeId) - 1);
  g_activeId[sizeof(g_activeId) - 1] = '\0';
  g_active = true;
  g_lastUpdate = millis() - 25;
  g_error.clear();
  error.clear();
  return true;
}

bool themeRuntimeLoop() {
  if (!g_active || !g_display || !g_display->ready()) return false;
  const uint32_t now = millis();
  if (static_cast<uint32_t>(now - g_lastUpdate) < 25) return false;
  g_lastUpdate = now;

  // Let HTTP/Wi-Fi work first if free heap is temporarily low.
  if (ESP.getFreeHeap() < 4096UL) return false;

  const std::vector<Rect> dirty = g_engine.update(now, nullptr);
  if (dirty.empty()) return false;
  if (render(g_engine, dirty, g_package, *g_display)) return false;

  g_error = "Theme rendering stopped: an image/font asset could not be read";
  releaseRuntime();
  return true;
}

void themeRuntimeInvalidate() {
  if (g_active) g_engine.invalidate();
}

bool themeRuntimeActive() { return g_active; }
const char* themeRuntimeActiveId() { return g_active ? g_activeId : ""; }
const char* themeRuntimeError() { return g_error.c_str(); }
uint32_t themeRuntimeFreeHeap() { return ESP.getFreeHeap(); }
}  // namespace smalltv
