// SmallTV Ultra Clean: Wi-Fi, web interface and firmware recovery only.
#include <Arduino.h>
#include "Platform.h"
#include "config.h"
#include "Settings.h"
#include "Net.h"
#include "WebPortal.h"
#include "OtaUpdate.h"

static Settings g_settings;
static String g_resetReason;
static bool g_safeMode = false;

const char* appResetReason() {
  return g_resetReason.c_str();
}

static void bootProgress(const char* message) {
  Serial.print("[net] ");
  Serial.println(message ? message : "");
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(FW_NAME " " FW_VERSION);

  // Original SmallTV backlight is active-low on GPIO5. Keep the panel dark.
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);

  const PlatformReset reset = platformResetInfo();
  g_resetReason = reset.reason;
  g_safeMode = reset.wasCrash;

  Serial.println("[boot] LittleFS");
  if (!settingsBegin()) Serial.println("[boot] LittleFS unavailable");
  loadSettings(g_settings);

  Serial.println("[boot] Wi-Fi");
  netAllowRetry(!g_safeMode);
  netBegin(g_settings, bootProgress);

  if (otaBootRequested()) {
    Serial.println("[boot] applying queued OTA update");
    otaBootUpdate(g_settings);
  }

  Serial.println("[boot] web server");
  webPortalBegin(g_settings);
  Serial.println("[boot] ready");
}

void loop() {
  netLoop();
  webPortalLoop();
  if (webPortalRebootDue()) {
    delay(120);
    ESP.restart();
  }
  delay(5);
}
