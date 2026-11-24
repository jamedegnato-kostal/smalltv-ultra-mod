// Persisted settings required for Wi-Fi, web management and OTA only.
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

struct WifiCred {
  String ssid;
  String pass;
};

struct Settings {
  WifiCred wifi[MAX_WIFI_NETS];
  uint8_t wifiCount;
  String apSsid;
  String hostname;
  uint16_t httpTimeout;
  uint8_t displayBrightness;
  uint8_t displayRotation;
  bool displayBgr;
  bool displayInvert;
  uint8_t displayRedGain;
  uint8_t displayGreenGain;
  uint8_t displayBlueGain;
  char activeTheme[49];
  char timezone[65];  // POSIX TZ rule; includes DST transitions
  void setDefaults();
};

bool settingsBegin();
bool settingsStorageReady();
bool settingsFormat();
const __FlashStringHelper* settingsValidateJson(JsonObjectConst root);
bool loadSettings(Settings& s);
bool saveSettings(const Settings& s);
void factoryReset(Settings& s);
void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets);
void settingsApplyJson(Settings& s, JsonObjectConst root);
