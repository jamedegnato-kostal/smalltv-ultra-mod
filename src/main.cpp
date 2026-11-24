// SmallTV Ultra Clean: web management, firmware recovery and display status.
#include <Arduino.h>
#include "Platform.h"
#include "config.h"
#include "Settings.h"
#include "Net.h"
#include "WebPortal.h"
#include "OtaUpdate.h"
#include "display/DisplayBoard.h"
#include "display/DisplayPanel.h"
#include "display/DisplayStatus.h"
#include "theme/ThemeRuntime.h"
#include "Clock.h"
#include "PhotoSlideshow.h"
#include <string>

static Settings g_settings;
static String g_resetReason;
static bool g_safeMode = false;
static smalltv::display::DisplayPanel g_display;
static bool g_displayReady = false;
static uint16_t g_displayFailSeq = 0;

const char* appResetReason() {
  return g_resetReason.c_str();
}

static void bootProgress(const char* message) {
  Serial.print("[net] ");
  Serial.println(message ? message : "");
  if (g_displayReady) smalltv::display::showBoot(g_display, message);
}

static void showNetworkStatus() {
  if (!g_displayReady || smalltv::themeRuntimeActive() || smalltv::photos::active()) return;
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

void applyTimeSettings(const Settings& settings) {
  smalltv::clockReconfigure(settings.timezone);
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
  if (!g_displayReady) return;
  g_display.setBrightness(settings.displayBrightness,
                          smalltv::display::BacklightActiveLow);
  if (smalltv::themeRuntimeActive()) smalltv::themeRuntimeInvalidate();
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
    g_display.setBrightness(g_settings.displayBrightness,
                            smalltv::display::BacklightActiveLow);
    smalltv::display::showBoot(g_display, g_safeMode ? "safe mode" : "starting");
  } else {
    Serial.println("[boot] display initialization failed; continuing headless");
  }

  Serial.println("[boot] Wi-Fi");
  netAllowRetry(!g_safeMode);
  netBegin(g_settings, bootProgress);

  if (otaBootRequested()) {
    Serial.println("[boot] applying queued OTA update");
    if (g_displayReady) smalltv::display::showUpdating(g_display, "please wait");
    otaBootUpdate(g_settings);
    if (g_displayReady) smalltv::display::showUpdating(g_display, "update failed");
  }

  Serial.println("[boot] web server");
  smalltv::themeRuntimeBegin(g_display);
  webPortalBegin(g_settings);
  if (g_safeMode && g_settings.activeTheme[0]) {
    g_settings.activeTheme[0] = '\0';
    if (settingsStorageReady() && !saveSettings(g_settings))
      Serial.println("[theme] could not clear saved theme in safe mode");
  }
  if (g_displayReady && !g_safeMode && g_settings.activeTheme[0]) {
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
  webPortalLoop();
  if (smalltv::themeRuntimeLoop()) {
    g_settings.activeTheme[0] = '\0';
    if (settingsStorageReady() && !saveSettings(g_settings))
      Serial.println("[theme] could not clear selection after render failure");
    showNetworkStatus();
  }

  smalltv::photos::loop();

  // Background Wi-Fi retries can change the failure reason while the setup AP
  // remains active. Refresh its diagnostic screen only when that reason changes.
  if (g_displayReady && !smalltv::themeRuntimeActive() && !smalltv::photos::active() && netMode() == NET_AP && netFailSeq() != g_displayFailSeq) {
    showNetworkStatus();
  }

  if (webPortalRebootDue()) {
    if (g_displayReady) smalltv::display::showBoot(g_display, "restarting");
    delay(120);
    ESP.restart();
  }
  delay(5);
}
