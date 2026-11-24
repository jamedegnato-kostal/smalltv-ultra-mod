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
  String apPass;
  String hostname;
  uint16_t httpTimeout;
  void setDefaults();
};

bool settingsBegin();
bool loadSettings(Settings& s);
bool saveSettings(const Settings& s);
void factoryReset(Settings& s);
void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets);
void settingsApplyJson(Settings& s, JsonObjectConst root);
