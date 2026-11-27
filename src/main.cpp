// SmallTV Stable: ESP8266 web management, themes, photos and night mode.
#include <Arduino.h>
#include "Platform.h"
#include "config.h"
#include "Settings.h"
#include "Net.h"
#include "WebPortal.h"
#include "display/DisplayBoard.h"
#include "display/DisplayPanel.h"
#include "display/DisplayStatus.h"
#include "theme/ThemeRuntime.h"
#include "Clock.h"
#include "NightSchedule.h"
#include "PhotoSlideshow.h"
#include "Playback.h"
#include <string>

static Settings g_settings;
static String g_resetReason;
static bool g_safeMode = false;
static smalltv::display::DisplayPanel g_display;
static bool g_displayReady = false;
static uint16_t g_displayFailSeq = 0;
static uint8_t g_lastAppliedBrightness = 0xFF;

const char* appResetReason() {
  return g_resetReason.c_str();
}

static void bootProgress(const char* message) {
  Serial.print("[net] ");
  Serial.println(message ? message : "");
  if (g_displayReady) smalltv::display::showBoot(g_display, message);
}

static void showNetworkStatus() {
  if (!g_displayReady || smalltv::themeRuntimeActive() || smalltv::photos::active() || smalltv::playback::active()) return;
  const String ip = netIP();
  if (g_safeMode) {
    smalltv::display::showSafeMode(g_display, g_resetReason.c_str(), ip.c_str());
  } else if (netMode() == NET_AP) {
    smalltv::display::showAccessPoint(
        g_display, g_settings.apSsid.c_str(), ip.c_str(), netFailReason());
  } else {
    const String ssid = netSSID();
    smalltv::display::showConnected(
        g_display, ssid.c_str(), ip.c_str(), g_settings.hostname.c_str());
  }
  g_displayFailSeq = netFailSeq();
}

void appShowNetworkStatus() {
  showNetworkStatus();
}

static uint8_t effectiveBrightness() {
  if (g_settings.nightEnabled && smalltv::clockSynced()) {
    tm localTime{};
    if (smalltv::clockNow(localTime)) {
      const uint16_t nowMinute = static_cast<uint16_t>(localTime.tm_hour * 60 + localTime.tm_min);
      if (smalltv::nightScheduleActive(nowMinute, g_settings.nightStartMinute,
                                       g_settings.nightEndMinute))
        return g_settings.nightBrightness;
    }
  }
  return g_settings.displayBrightness;
}

static void applyEffectiveBrightness() {
  if (!g_displayReady) return;
  const uint8_t target = effectiveBrightness();
  if (target == g_lastAppliedBrightness) return;
  g_lastAppliedBrightness = target;
  g_display.setBrightness(target, smalltv::display::BacklightActiveLow);
}

void applyTimeSettings(const Settings& settings) {
  smalltv::clockSetNightModeEnabled(settings.nightEnabled, settings.timezone);
}

void applyDisplaySettings(const Settings& settings) {
  smalltv::display::PanelColorProfile profile;
  profile.rotation = settings.displayRotation;
  profile.bgr = settings.displayBgr;
  profile.invert = settings.displayInvert;
  profile.redGain = settings.displayRedGain;
  profile.greenGain = settings.displayGreenGain;
  profile.blueGain = settings.displayBlueGain;
  g_display.setColorProfile(profile);
  g_lastAppliedBrightness = 0xFF;
  applyEffectiveBrightness();
  if (!g_displayReady) return;
  if (smalltv::playback::active()) smalltv::playback::redraw();
  else if (smalltv::themeRuntimeActive()) smalltv::themeRuntimeInvalidate();
  else if (smalltv::photos::active()) smalltv::photos::redraw();
  else showNetworkStatus();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(FW_NAME " " FW_VERSION);

  const PlatformReset reset = platformResetInfo();
  g_resetReason = reset.reason;
  g_safeMode = reset.wasCrash;

  Serial.println("[boot] LittleFS");
  if (!settingsBegin()) Serial.println("[boot] LittleFS unavailable");
  loadSettings(g_settings);
  // Initialize after settings are loaded. The backlight starts dark and is
  // raised only after the ST7789 controller has initialized successfully.
  Serial.println("[boot] display");
  smalltv::display::PanelColorProfile profile;
  profile.rotation = g_settings.displayRotation;
  profile.bgr = g_settings.displayBgr;
  profile.invert = g_settings.displayInvert;
  profile.redGain = g_settings.displayRedGain;
  profile.greenGain = g_settings.displayGreenGain;
  profile.blueGain = g_settings.displayBlueGain;
  g_display.setColorProfile(profile);
  g_displayReady = g_display.begin();
  smalltv::photos::begin(g_display);
  if (g_displayReady) {
    g_lastAppliedBrightness = 0xFF;
    applyEffectiveBrightness();
    smalltv::display::showBoot(g_display, g_safeMode ? "safe mode" : "starting");
  } else {
    Serial.println("[boot] display initialization failed; continuing headless");
  }

  Serial.println("[boot] Wi-Fi");
  netAllowRetry(!g_safeMode);
  netBegin(g_settings, bootProgress);
  // Night mode is an independent NTP consumer; it remains active when a theme
  // releases its own clock request. SNTP starts once Wi-Fi is connected.
  smalltv::clockSetNightModeEnabled(g_settings.nightEnabled, g_settings.timezone);

  Serial.println("[boot] web server");
  smalltv::themeRuntimeBegin(g_display);
  smalltv::playback::begin(g_settings, g_displayReady);
  webPortalBegin(g_settings);
  if (g_safeMode && g_settings.activeTheme[0]) {
    g_settings.activeTheme[0] = '\0';
    if (settingsStorageReady() && !saveSettings(g_settings))
      Serial.println("[theme] could not clear saved theme in safe mode");
  }
  if (g_displayReady && !g_safeMode && g_settings.activeTheme[0] && !smalltv::playback::autoStartRequested()) {
    std::string error;
    if (!smalltv::themeRuntimeSelect(g_settings.activeTheme, g_settings.timezone, g_settings.language, error)) {
      Serial.print("[theme] could not restore selected theme: ");
      Serial.println(error.c_str());
      g_settings.activeTheme[0] = '\0';
      if (settingsStorageReady() && !saveSettings(g_settings))
        Serial.println("[theme] could not clear unavailable theme selection");
    }
  }
  if (!smalltv::themeRuntimeActive()) showNetworkStatus();
  Serial.println("[boot] ready");
}

void loop() {
  netLoop();
  smalltv::clockService();
  applyEffectiveBrightness();
  webPortalLoop();
  if (smalltv::themeRuntimeLoop()) {
    if (smalltv::playback::active()) {
      smalltv::playback::advanceNow();
    } else {
      g_settings.activeTheme[0] = '\0';
      if (settingsStorageReady() && !saveSettings(g_settings))
        Serial.println("[theme] could not clear selection after render failure");
      showNetworkStatus();
    }
  }

  smalltv::photos::loop();
  smalltv::playback::loop();

  // Background Wi-Fi retries can change the failure reason while the setup AP
  // remains active. Refresh its diagnostic screen only when that reason changes.
  if (g_displayReady && !smalltv::themeRuntimeActive() && !smalltv::photos::active() && !smalltv::playback::active() && netMode() == NET_AP && netFailSeq() != g_displayFailSeq) {
    showNetworkStatus();
  }

  if (webPortalRebootDue()) {
    if (g_displayReady) smalltv::display::showBoot(g_display, "restarting");
    delay(120);
    ESP.restart();
  }
  delay(5);
}
