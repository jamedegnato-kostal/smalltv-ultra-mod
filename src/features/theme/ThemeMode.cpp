#include "ThemeMode.h"
#if WITH_THEME
#include "Clock.h"
#include "Gfx.h"
#include "Platform.h"
#include "Net.h"
#include "ThemeDataClient.h"
#include <Arduino_GFX_Library.h>
#include <font/glcdfont.h>
#include <algorithm>
#include <cstdio>
#include <new>
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
#include <pgmspace.h>
#include <recovery_theme_data.h>
#endif

ThemeMode g_themeMode;
extern uint8_t appStatusBrightness();
namespace {
// Adapts the portable engine's Display interface to this device's actual
// panel: glyphColumn() reads the built-in font from flash (PROGMEM), and
// row() applies the usual panel color tint before handing the row to the
// GFX driver. yield() every 16 rows keeps a large dirty region from starving
// WiFi/other tasks during a long render.
class Panel : public smalltv::Display {
 public:
  uint8_t glyphColumn(uint8_t c, uint8_t col) const override {
    return pgm_read_byte(font + unsigned(c) * 5 + col);
  }
  void row(int x, int y, const uint16_t* pixels, int count) override {
    // Theme rendering is single-threaded and sends at most 120 pixels per row.
    // Keep its color-adjustment buffer bounded as well.
    static uint16_t tinted[120];
    for (int i = 0; i < count; ++i) tinted[i] = gfxTint(pixels[i]);
    gfxDev()->draw16bitRGBBitmap(x, y, tinted, count, 1);
    if ((y & 15) == 15) yield();
  }
};
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
class RecoveryThemeSource : public smalltv::Source {
 public:
  uint32_t size() const override { return kRecoveryThemeSize; }
  bool read(uint32_t offset, void* dest, size_t count) override {
    if (offset > kRecoveryThemeSize || count > kRecoveryThemeSize - offset) return false;
    uint8_t* out = static_cast<uint8_t*>(dest);
    for (size_t i = 0; i < count; ++i) out[i] = pgm_read_byte(kRecoveryThemeData + offset + i);
    return true;
  }
};
static constexpr const char* RecoveryThemeId = "office-recovery";
#endif
// What a load needs before it starts: a package parse (ThemeMode.h). A failed
// allocation mid-load restarts the ESP8266, so a load that would not fit
// waits instead.
constexpr uint32_t LoadFreeHeap = ThemeMode::ParseFreeHeap;
constexpr uint32_t LoadLargestBlock = ThemeMode::ParseLargestBlock;
const char* const WaitingForMemory = "Waiting for free memory";

// Validate a theme ID without constructing a temporary std::string. This is
// used from service(), which runs on every main-loop pass on the ESP8266.
bool validThemeId(const char* id) {
  if (!id || !*id) return false;
  size_t length = 0;
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(id); *p; ++p) {
    if (++length > 48) return false;
    if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
          (*p >= '0' && *p <= '9') || *p == '-' || *p == '_')) return false;
  }
  return true;
}
}
void ThemeMode::begin(const Settings&) {
  LittleFS.mkdir("/themes");
  // An interrupted install leaves its staging file behind; boot is the one
  // point it is safe to remove without racing a request in progress.
  LittleFS.remove("/themes/.upload");
}
// Drops every bit of state for the currently loaded theme -- the open
// package/file, any in-flight or fetched data, and the engine itself -- so
// the next service() call starts a clean load from scratch.
void ThemeMode::unload() {
  dataRequests_.cancel();
  dataRequests_.take();
  dataRequests_.releaseStorage();
  package_.reset();
  file_.reset();
  // clear() destroys elements but keeps the vector's backing allocation.
  // A theme switch/final unload should return all theme-specific capacity.
  std::vector<uint32_t>().swap(dataNextMs_);
  std::vector<bool>().swap(dataScheduled_);
  std::vector<smalltv::ThemeValue>().swap(dataValues_);
  dataAges_.reset(0);
  dataCursor_ = 0;
  engine_.setValues({});
  engine_.setTheme(smalltv::Theme{}, millis());
  loadedId_ = "";
  attempted_ = messageDrawn_ = keepData_ = false;
  error_ = "";
}
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
void ThemeMode::useRecoveryTheme(uint8_t crashCount) {
  recoveryMode_ = true;
  recoveryCrashCount_ = crashCount;
  unload();
}
#endif
#if WITH_THEME_PLAYLIST
void ThemeMode::setPlaylistTheme(const char* id) {
  String next = id ? id : "";
  if (next == playlistId_) {
    // The first Showflow step can be a photo while the regular selected theme
    // is still loaded (playlistId_ is empty). Explicitly unload that package
    // before TJpg_Decoder is started; the photo path must never share the
    // theme package's heap allocations, even when the playlist override is
    // already empty.
    if (!next.length() && package_) unload();
    return;
  }
  playlistId_ = next;
  unload();  // cancels the previous fetch and drops all theme-owned heap state
}
bool ThemeMode::playlistLoadFailed() const {
  return attempted_ && !package_ && error_.length() &&
      error_ != WaitingForMemory;
}
#endif
void ThemeMode::release(uint32_t needFree, uint32_t needLargestBlock) {
  if (ESP.getFreeHeap() >= needFree && platformMaxFreeBlock() >= needLargestBlock) return;
  // A settings save needs one contiguous Settings-sized block. Drop all
  // theme-owned allocations (including reusable vectors) so the allocator can
  // coalesce the heap. The next service() call reloads the selected theme.
  unload();
}
// Collects a finished fetch (if any) and, with at most one request in flight
// at a time, starts the next source whose interval has elapsed, round-robin
// over sources so no single slow/failing one starves the others.
bool ThemeMode::refreshData() {
  const auto& sources = engine_.theme().data;
  dataAges_.tick(millis());
  bool valuesChanged = false;
  if (auto result = dataRequests_.take()) {
    dataAges_.result(result->index, result->success);
    if (result->success) {
      // Merge into the running set by key rather than replace it wholesale:
      // a source's fields persist across refreshes of the *other* sources, so
      // a slow source does not blank fields a fast one already delivered.
      for (auto& value : result->values) {
        auto existing = std::find_if(dataValues_.begin(), dataValues_.end(), [&](const smalltv::ThemeValue& old) { return old.key == value.key; });
        // Keep the request's reusable key/value buffers intact for the next
        // poll. dataValues_ owns its stable copy used by the engine.
        if (existing == dataValues_.end()) dataValues_.push_back(value);
        else existing->value = value.value;
      }
      valuesChanged = true;
    }
    // Schedule from completion, preserving the interval even for slow sources.
    // A failed source is retried after at most one minute; a healthy one
    // keeps its manifest-defined interval.
    const uint32_t interval = sources[result->index].interval;
    dataNextMs_[result->index] = millis() + (result->success ? interval : std::min(interval, 60UL)) * 1000UL;
  }
  if (dataAges_.publish(sources, dataValues_)) valuesChanged = true;
  if (valuesChanged) engine_.setValues(dataValues_);
  if (sources.empty() || dataRequests_.busy() || WiFi.status() != WL_CONNECTED) return false;
  // Scan forward from `dataCursor_` for the first source that is due (an
  // unscheduled one always is), start it, and advance the cursor past it --
  // so a source that just fired goes to the back of the round-robin queue.
  for (size_t offset = 0; offset < sources.size(); ++offset) {
    size_t i = (dataCursor_ + offset) % sources.size();
    if (dataScheduled_[i] && static_cast<int32_t>(millis() - dataNextMs_[i]) < 0) continue;
    const uint32_t sourceInterval = sources[i].interval;
    const size_t sourceCount = sources.size();
    auto request = dataRequests_.start(sources[i], i);
    if (!request) return false;
    dataScheduled_[i] = true;
    dataCursor_ = (i + 1) % sourceCount;
#if defined(SMALLTV_ESP8266)
    // The live theme itself holds most of the scarce heap. Keep the request's
    // source copy, release only the renderer/package, fetch from that free-heap
    // window, then reload and repaint with the merged values next tick.
    package_.reset();
    file_.reset();
    engine_.setValues({});
    engine_.setTheme(smalltv::Theme{}, millis());
    keepData_ = true;
    attempted_ = false;
    messageDrawn_ = false;
    error_ = "";
    transitionActive_ = false;
    transitionPending_ = false;
    if (ESP.getFreeHeap() < DataFetchFreeHeap ||
        platformMaxFreeBlock() < DataFetchLargestBlock ||
        !startThemeDataFetch(request)) request->finish(false);
    return true;
#else
    if (!startThemeDataFetch(request)) request->finish(false);
    break;
#endif
  }
  return false;
}
// Called when the theme should be redrawn from scratch, e.g. when a
// notification overlay just finished. Switches to a different theme by
// unloading first; otherwise leaves the loaded package alone and only marks
// the engine's next update() as a full repaint.
void ThemeMode::invalidate(const Settings& s) {
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
  if (recoveryMode_) {
    engine_.invalidate();
    messageDrawn_ = false;
    return;
  }
#endif
  const bool onScreen = s.mode == MODE_THEME || (s.mode == MODE_CAROUSEL && s.carouselTheme);
  const String* desiredId = &s.themeId;
#if WITH_THEME_PLAYLIST
  if (playlistId_.length()) desiredId = &playlistId_;
#endif
  if (loadedId_ != *desiredId || !onScreen) unload();
  // A failed load may be retried after installation or settings changes.
  if (!package_) attempted_ = false;
  engine_.invalidate();
  messageDrawn_ = false;
}
// Per-frame entry point: loads the selected theme at most once per selection
// (via `attempted_`, so a failed load is not retried every call -- only on
// the next invalidate()/selection change), then renders it. `attempted_` and
// `loadedId_` together are the load state machine; everything past the load
// block runs every call once a package is open.
void ThemeMode::service(const Settings& s) {
  const char* desiredId = s.themeId.c_str();
#if WITH_THEME_PLAYLIST
  if (playlistId_.length()) desiredId = playlistId_.c_str();
#endif
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
  if (recoveryMode_) desiredId = RecoveryThemeId;
#endif
  if (loadedId_ != desiredId) {
    transitionPending_ = s.display.themeFadeTransition && loadedId_.length() > 0;
    transitionActive_ = false;
    unload();
    loadedId_ = desiredId;
  }
  // A web request, or the buffers of its reply, can briefly hold the heap a
  // load needs; the load then waits, checked again every frame.
  const bool loadFits = ESP.getFreeHeap() >= LoadFreeHeap && platformMaxFreeBlock() >= LoadLargestBlock;
  if (!attempted_ && !loadFits && error_ != WaitingForMemory) {
    error_ = WaitingForMemory;
    messageDrawn_ = false;
  }
  if (!attempted_ && loadFits) {
    attempted_ = true;
    std::string error;
    smalltv::Theme theme;
    platformMemoryPhaseBegin(PLATFORM_MEM_THEME_PARSE, true);
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
    bool sourceReady = false;
    if (recoveryMode_) {
      file_.reset(new(std::nothrow) RecoveryThemeSource);
      sourceReady = bool(file_);
    } else {
      if (!validThemeId(desiredId)) error = "Select an installed theme";
      else {
        ThemeFile* source = new(std::nothrow) ThemeFile;
        if (source && source->open(String("/themes/") + desiredId + ".stheme")) {
          file_.reset(source);
          sourceReady = true;
        } else {
          delete source;
          error = "Theme file not found";
        }
      }
    }
    if (sourceReady) {
      package_.reset(new(std::nothrow) smalltv::Package(*file_));
      if (!package_) error = "Cannot allocate theme package";
      else if (!package_->load(theme, error)) package_.reset();
      else if (theme.id != desiredId) {
        error = "Theme ID mismatch";
        package_.reset();
      }
    }
#else
    if (!validThemeId(desiredId)) error = "Select an installed theme";
    else {
      ThemeFile* source = new(std::nothrow) ThemeFile;
      if (!source || !source->open(String("/themes/") + desiredId + ".stheme")) {
        delete source;
        error = "Theme file not found";
      } else {
        file_.reset(source);
        package_.reset(new(std::nothrow) smalltv::Package(*file_));
        if (!package_) error = "Cannot allocate theme package";
        else if (!package_->load(theme, error)) package_.reset();
        else if (theme.id != desiredId) {
          error = "Theme ID mismatch";
          package_.reset();
        }
      }
    }
#endif
    platformMemoryPhaseEnd(PLATFORM_MEM_THEME_PARSE, true);
    if (!error.empty()) {
      transitionPending_ = false;
      error_ = error.c_str();
      package_.reset();
      file_.reset();
    } else {
      // After a release the same package loads again, so its sources match.
      if (!keepData_ || dataNextMs_.size() != theme.data.size()) {
        dataNextMs_.assign(theme.data.size(), 0);
        dataScheduled_.assign(theme.data.size(), false);
        dataValues_.clear();
        dataAges_.reset(theme.data.size(), theme.spec >= 2);
        dataCursor_ = 0;
      }
      engine_.setTheme(std::move(theme), millis());
      engine_.setValues(dataValues_);
      error_ = "";
      transitionActive_ = transitionPending_;
      transitionPending_ = false;
      transitionStartedMs_ = millis();
    }
  }
  if (!package_) {
    // A cancelled ESP32 data-fetch task may finish after unload(); keep
    // collecting it here so its shared request buffer is not pinned forever
    // while the error screen is displayed.
    dataRequests_.take();
    dataRequests_.releaseStorage();
    transitionActive_ = false;
    if (!messageDrawn_) {
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
      if (recoveryMode_) gfxSafeTheme(netIP().c_str(), recoveryCrashCount_);
      else
#endif
      gfxMessage("THEMES", error_.c_str(), C_WHITE);
      messageDrawn_ = true;
    }
    return;
  }
  if (transitionActive_) {
    const uint32_t duration = constrain((int)s.display.themeTransitionMs, 100, 1000);
    const uint32_t elapsed = millis() - transitionStartedMs_;
    if (elapsed >= duration) {
      transitionActive_ = false;
      // The fade writes the PWM directly, bypassing appApplyBrightness()'s
      // cached last-value check. Write the resolved target explicitly here.
      gfxSetBrightness(appStatusBrightness(), s.backlightInverted);
    } else {
      const uint32_t half = duration / 2 ? duration / 2 : 1;
      const uint32_t level = elapsed < half ? (half - elapsed) * 100 / half : (elapsed - half) * 100 / (duration - half);
      // Fade the effective brightness (night level / button wake override),
      // not the raw manual setting, so the transition respects those states.
      const uint8_t targetBrightness = appStatusBrightness();
      const uint8_t brightness = uint8_t((uint32_t(targetBrightness) * level) / 100);
      gfxSetBrightness(brightness, s.backlightInverted);
    }
  }
  if (refreshData()) return;  // ESP8266 renderer was released for a synchronous data fetch
  const uint32_t renderInterval = 1000U / constrain((int)s.display.refreshFps, 1, 15);
  if (lastRenderMs_ && (uint32_t)(millis() - lastRenderMs_) < renderInterval) return;
  lastRenderMs_ = millis();
  tm t{};
  bool valid = clockNow(t);
  // Pass the persistent setting directly: constructing ClockOptions here
  // used to allocate/free its std::string at every rendered frame.
  engine_.setClockOptions(s.clock.use24Hour, s.clock.showSeconds,
                          s.clock.showDate, s.clock.dateFormat.c_str());
  uint8_t language = 0;
  if (s.clock.lang == "pt") language = 1;
  else if (s.clock.lang == "es") language = 2;
  else if (s.clock.lang == "fr") language = 3;
  else if (s.clock.lang == "de") language = 4;
  else if (s.clock.lang == "it") language = 5;
  engine_.setLanguage(language);
  engine_.setAnimationOptions(s.display.animationsEnabled, s.display.animationSpeedPct);
  Panel panel;
  platformMemoryPhaseBegin(PLATFORM_MEM_THEME_RENDER, true);
  const auto& dirty = engine_.update(millis(), valid ? &t : nullptr);
  // A render failure (a corrupt or truncated asset read mid-frame) is treated
  // like a load failure: drop the package and show an error rather than
  // risk repeatedly failing on every subsequent frame.
  uint16_t background = s.display.backgroundEnabled ? s.display.background : engine_.theme().background;
  const bool rendered = smalltv::render(engine_, dirty, *package_, panel, background, s.display.themeOpacityPct);
  platformMemoryPhaseEnd(PLATFORM_MEM_THEME_RENDER, true);
  if (!rendered) {
    // A failed asset read is terminal for this load. Drop the engine's parsed
    // layers, cached values, package index and file too; keeping only the
    // error state avoids pinning theme heap while the web UI is used to fix it.
    unload();
    loadedId_ = desiredId;
    attempted_ = true;  // do not retry the same broken package every loop pass
    error_ = "Could not read theme asset";
    messageDrawn_ = false;
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
    if (recoveryMode_) gfxSafeTheme(netIP().c_str(), recoveryCrashCount_);
#endif
    return;
  }
#if defined(SMALLTV_CLEAN) && defined(SMALLTV_FAMILY_ESP8266)
  if (recoveryMode_) {
    char count[24];
    snprintf(count, sizeof(count), "Crash count: %u", (unsigned)recoveryCrashCount_);
    gfxDrawCentered("Open settings:", 176, 1, C_GRAY);
    gfxDrawCentered(netIP().c_str(), 194, 1, C_GREEN);
    gfxDrawCentered(count, 216, 1, C_GRAY);
  }
#endif
}
#endif
