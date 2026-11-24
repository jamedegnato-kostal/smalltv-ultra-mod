#include "Settings.h"
#include "AtomicJson.h"
#include "Platform.h"
#include <LittleFS.h>

static const char* CONFIG_PATH = "/config.json";

void Settings::setDefaults() {
  wifiCount = 0;
  for (uint8_t i = 0; i < MAX_WIFI_NETS; ++i) {
    wifi[i].ssid = "";
    wifi[i].pass = "";
  }
  apSsid = DEFAULT_AP_SSID;
  apPass = DEFAULT_AP_PASS;
  hostname = String(DEFAULT_HOSTNAME) + "-" + String(platformChipId() & 0xFFFF, HEX);
  httpTimeout = DEFAULT_HTTP_TIMEOUT;
}

bool settingsBegin() {
  if (LittleFS.begin()) return true;
  return LittleFS.format() && LittleFS.begin();
}

bool loadSettings(Settings& s) {
  s.setDefaults();
  File file = LittleFS.open(CONFIG_PATH, "r");
  if (!file) return false;
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) return false;
  settingsApplyJson(s, doc.as<JsonObjectConst>());
  return true;
}

bool saveSettings(const Settings& s) {
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  settingsToJson(s, root, true);
  return saveJsonAtomically(LittleFS, doc, CONFIG_PATH, "/config.json.tmp");
}

void factoryReset(Settings& s) {
  s.setDefaults();
}

void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets) {
  root["hostname"] = s.hostname;
  root["apSsid"] = s.apSsid;
  root["apPassSet"] = s.apPass.length() > 0;
  root["httpTimeout"] = s.httpTimeout;

  JsonArray networks = root["wifi"].to<JsonArray>();
  for (uint8_t i = 0; i < s.wifiCount; ++i) {
    JsonObject item = networks.add<JsonObject>();
    item["ssid"] = s.wifi[i].ssid;
    item["passSet"] = s.wifi[i].pass.length() > 0;
    if (includeSecrets) item["pass"] = s.wifi[i].pass;
  }
  root["staSsid"] = s.wifiCount ? s.wifi[0].ssid : "";
  root["staPassSet"] = s.wifiCount && s.wifi[0].pass.length() > 0;
  if (includeSecrets) {
    root["staPass"] = s.wifiCount ? s.wifi[0].pass : "";
    root["apPass"] = s.apPass;
  }
}

void settingsApplyJson(Settings& s, JsonObjectConst root) {
  if (root["hostname"].is<const char*>()) {
    s.hostname = root["hostname"].as<String>();
    if (s.hostname.length() > 63) s.hostname.remove(63);
  }

  if (root["wifi"].is<JsonArrayConst>()) {
    WifiCred old[MAX_WIFI_NETS];
    const uint8_t oldCount = s.wifiCount;
    for (uint8_t i = 0; i < oldCount; ++i) old[i] = s.wifi[i];

    s.wifiCount = 0;
    for (JsonObjectConst item : root["wifi"].as<JsonArrayConst>()) {
      if (s.wifiCount >= MAX_WIFI_NETS) break;
      const char* ssid = item["ssid"] | "";
      if (!ssid[0]) continue;

      WifiCred& target = s.wifi[s.wifiCount];
      target.ssid = ssid;
      if (target.ssid.length() > 32) target.ssid.remove(32);
      target.pass = item["pass"] | "";
      if (target.pass.length() > 64) target.pass.remove(64);

      if (!target.pass.length()) {
        for (uint8_t oldIndex = 0; oldIndex < oldCount; ++oldIndex) {
          if (old[oldIndex].ssid == target.ssid) {
            target.pass = old[oldIndex].pass;
            break;
          }
        }
      }
      ++s.wifiCount;
    }
  } else if (root["staSsid"].is<const char*>()) {
    String ssid = root["staSsid"].as<String>();
    if (ssid.length()) {
      s.wifi[0].ssid = ssid;
      if (s.wifi[0].ssid.length() > 32) s.wifi[0].ssid.remove(32);
      if (root["staPass"].is<const char*>()) {
        String pass = root["staPass"].as<String>();
        if (pass.length()) s.wifi[0].pass = pass;
      }
      if (s.wifi[0].pass.length() > 64) s.wifi[0].pass.remove(64);
      if (s.wifiCount < 1) s.wifiCount = 1;
    }
  }

  if (root["apSsid"].is<const char*>()) {
    s.apSsid = root["apSsid"].as<String>();
    if (s.apSsid.length() > 32) s.apSsid.remove(32);
  }
  if (root["apPass"].is<const char*>()) {
    s.apPass = root["apPass"].as<String>();
    if (s.apPass.length() > 63) s.apPass.remove(63);
  }
  if (root["httpTimeout"].is<int>()) {
    s.httpTimeout = constrain((int)root["httpTimeout"], 1000, 20000);
  }
}
