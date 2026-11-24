#include "Settings.h"
#include "AtomicJson.h"
#include "Platform.h"
#include <LittleFS.h>
#include <string.h>

static const char* CONFIG_PATH = "/config.json";
static bool g_storageReady = false;

void Settings::setDefaults() {
  wifiCount = 0;
  for (uint8_t i = 0; i < MAX_WIFI_NETS; ++i) {
    wifi[i].ssid = "";
    wifi[i].pass = "";
  }
  apSsid = DEFAULT_AP_SSID;
  hostname = String(DEFAULT_HOSTNAME) + "-" + String(platformChipId() & 0xFFFF, HEX);
  httpTimeout = DEFAULT_HTTP_TIMEOUT;
  displayBrightness = 90;
  displayRotation = 0;
  displayBgr = false;
  displayInvert = false;
  displayRedGain = 100;
  displayGreenGain = 100;
  displayBlueGain = 100;
}

bool settingsBegin() {
  g_storageReady = LittleFS.begin();
  return g_storageReady;
}

bool settingsStorageReady() {
  return g_storageReady;
}

// Formatting is destructive and only available through an explicitly confirmed reset.
bool settingsFormat() {
  if (g_storageReady) return false;
  if (!LittleFS.format()) return false;
  g_storageReady = LittleFS.begin();
  return g_storageReady;
}

bool loadSettings(Settings& s) {
  s.setDefaults();
  if (!g_storageReady) return false;
  File file = LittleFS.open(CONFIG_PATH, "r");
  if (!file) return false;
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error || !doc.is<JsonObject>()) return false;
  if (settingsValidateJson(doc.as<JsonObjectConst>())) return false;
  settingsApplyJson(s, doc.as<JsonObjectConst>());
  return true;
}

bool saveSettings(const Settings& s) {
  if (!g_storageReady) return false;
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
  root["httpTimeout"] = s.httpTimeout;
  root["displayBrightness"] = s.displayBrightness;
  root["displayRotation"] = s.displayRotation;
  root["displayBgr"] = s.displayBgr;
  root["displayInvert"] = s.displayInvert;
  root["displayRedGain"] = s.displayRedGain;
  root["displayGreenGain"] = s.displayGreenGain;
  root["displayBlueGain"] = s.displayBlueGain;

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
  }
}


static bool validHostname(const char* value) {
  if (!value) return false;
  const size_t length = strlen(value);
  if (!length || length > 63 || value[0] == '-' || value[length - 1] == '-') return false;
  for (size_t i = 0; i < length; ++i) {
    const char c = value[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '-')) return false;
  }
  return true;
}

// Static error strings and bounded iteration only: this validation allocates no heap memory.
const __FlashStringHelper* settingsValidateJson(JsonObjectConst root) {
  if (!root["hostname"].isNull()) {
    if (!root["hostname"].is<const char*>() ||
        !validHostname(root["hostname"].as<const char*>())) return F("invalid hostname");
  }
  if (!root["apSsid"].isNull()) {
    if (!root["apSsid"].is<const char*>()) return F("invalid setup hotspot name");
    const char* ap = root["apSsid"].as<const char*>();
    const size_t length = strlen(ap);
    if (!length || length > 32) return F("setup hotspot name must be 1-32 bytes");
  }
  if (!root["httpTimeout"].isNull()) {
    if (!root["httpTimeout"].is<int>()) return F("invalid HTTP timeout");
    const int timeout = root["httpTimeout"].as<int>();
    if (timeout < 1000 || timeout > 20000) return F("HTTP timeout must be 1000-20000 ms");
  }
  if (!root["displayBrightness"].isNull()) {
    if (!root["displayBrightness"].is<int>()) return F("invalid display brightness");
    const int value = root["displayBrightness"].as<int>();
    if (value < 0 || value > 100) return F("display brightness must be 0-100 percent");
  }
  if (!root["displayRotation"].isNull()) {
    if (!root["displayRotation"].is<int>()) return F("invalid display rotation");
    const int value = root["displayRotation"].as<int>();
    if (value < 0 || value > 3) return F("display rotation must be 0-3");
  }
  if (!root["displayBgr"].isNull() && !root["displayBgr"].is<bool>())
    return F("invalid display color order");
  if (!root["displayInvert"].isNull() && !root["displayInvert"].is<bool>())
    return F("invalid display inversion setting");
  if (!root["displayRedGain"].isNull()) {
    if (!root["displayRedGain"].is<int>()) return F("invalid red gain");
    const int value = root["displayRedGain"].as<int>();
    if (value < 50 || value > 150) return F("red gain must be 50-150 percent");
  }
  if (!root["displayGreenGain"].isNull()) {
    if (!root["displayGreenGain"].is<int>()) return F("invalid green gain");
    const int value = root["displayGreenGain"].as<int>();
    if (value < 50 || value > 150) return F("green gain must be 50-150 percent");
  }
  if (!root["displayBlueGain"].isNull()) {
    if (!root["displayBlueGain"].is<int>()) return F("invalid blue gain");
    const int value = root["displayBlueGain"].as<int>();
    if (value < 50 || value > 150) return F("blue gain must be 50-150 percent");
  }
  if (!root["wifi"].isNull()) {
    if (!root["wifi"].is<JsonArrayConst>()) return F("wifi must be an array");
    JsonArrayConst networks = root["wifi"].as<JsonArrayConst>();
    if (networks.size() > MAX_WIFI_NETS) return F("maximum four Wi-Fi networks");
    for (JsonVariantConst value : networks) {
      if (!value.is<JsonObjectConst>()) return F("invalid Wi-Fi entry");
      JsonObjectConst item = value.as<JsonObjectConst>();
      if (!item["ssid"].is<const char*>()) return F("Wi-Fi SSID is missing");
      const char* ssid = item["ssid"].as<const char*>();
      const size_t ssidLength = strlen(ssid);
      if (!ssidLength || ssidLength > 32) return F("Wi-Fi SSID must be 1-32 bytes");
      if (!item["pass"].isNull()) {
        if (!item["pass"].is<const char*>()) return F("invalid Wi-Fi password");
        if (strlen(item["pass"].as<const char*>()) > 64) return F("Wi-Fi password exceeds 64 bytes");
      }
    }
  }
  // Legacy single-network backups remain accepted.
  if (!root["staSsid"].isNull()) {
    if (!root["staSsid"].is<const char*>()) return F("invalid legacy Wi-Fi SSID");
    if (strlen(root["staSsid"].as<const char*>()) > 32) return F("legacy Wi-Fi SSID exceeds 32 bytes");
  }
  if (!root["staPass"].isNull()) {
    if (!root["staPass"].is<const char*>()) return F("invalid legacy Wi-Fi password");
    if (strlen(root["staPass"].as<const char*>()) > 64) return F("legacy Wi-Fi password exceeds 64 bytes");
  }
  return nullptr;
}

void settingsApplyJson(Settings& s, JsonObjectConst root) {
  if (root["hostname"].is<const char*>()) {
    s.hostname = root["hostname"].as<String>();
    if (s.hostname.length() > 63) s.hostname.remove(63);
  }

  if (root["wifi"].is<JsonArrayConst>()) {
    const JsonArrayConst networks = root["wifi"].as<JsonArrayConst>();
    const uint8_t oldCount = s.wifiCount;
    // Preserve blank password fields without heap-allocating copies of String.
    char preservedPass[MAX_WIFI_NETS][65] = {};
    bool preservePass[MAX_WIFI_NETS] = {false};
    uint8_t index = 0;
    for (JsonObjectConst item : networks) {
      const char* ssid = item["ssid"] | "";
      const char* pass = item["pass"] | "";
      if (!pass[0]) {
        for (uint8_t oldIndex = 0; oldIndex < oldCount; ++oldIndex) {
          if (s.wifi[oldIndex].ssid == ssid) {
            const size_t length = s.wifi[oldIndex].pass.length() > 64
              ? 64 : s.wifi[oldIndex].pass.length();
            memcpy(preservedPass[index], s.wifi[oldIndex].pass.c_str(), length);
            preservedPass[index][length] = '\0';
            preservePass[index] = true;
            break;
          }
        }
      }
      ++index;
    }
    index = 0;
    for (JsonObjectConst item : networks) {
      WifiCred& target = s.wifi[index];
      target.ssid = item["ssid"] | "";
      const char* pass = item["pass"] | "";
      if (pass[0]) target.pass = pass;
      else target.pass = preservePass[index] ? preservedPass[index] : "";
      ++index;
    }
    s.wifiCount = index;
    for (uint8_t i = index; i < MAX_WIFI_NETS; ++i) {
      s.wifi[i].ssid = "";
      s.wifi[i].pass = "";
    }
  } else if (root["staSsid"].is<const char*>()) {
    const char* ssid = root["staSsid"] | "";
    if (ssid[0]) {
      s.wifi[0].ssid = ssid;
      if (root["staPass"].is<const char*>()) {
        const char* pass = root["staPass"] | "";
        if (pass[0]) s.wifi[0].pass = pass;
      }
      if (s.wifiCount < 1) s.wifiCount = 1;
    }
  }

  if (root["apSsid"].is<const char*>()) {
    s.apSsid = root["apSsid"].as<String>();
    if (s.apSsid.length() > 32) s.apSsid.remove(32);
  }
  if (root["httpTimeout"].is<int>()) {
    s.httpTimeout = constrain((int)root["httpTimeout"], 1000, 20000);
  }
  if (root["displayBrightness"].is<int>())
    s.displayBrightness = (uint8_t)root["displayBrightness"].as<int>();
  if (root["displayRotation"].is<int>())
    s.displayRotation = (uint8_t)root["displayRotation"].as<int>();
  if (root["displayBgr"].is<bool>())
    s.displayBgr = root["displayBgr"].as<bool>();
  if (root["displayInvert"].is<bool>())
    s.displayInvert = root["displayInvert"].as<bool>();
  if (root["displayRedGain"].is<int>())
    s.displayRedGain = (uint8_t)root["displayRedGain"].as<int>();
  if (root["displayGreenGain"].is<int>())
    s.displayGreenGain = (uint8_t)root["displayGreenGain"].as<int>();
  if (root["displayBlueGain"].is<int>())
    s.displayBlueGain = (uint8_t)root["displayBlueGain"].as<int>();
}
