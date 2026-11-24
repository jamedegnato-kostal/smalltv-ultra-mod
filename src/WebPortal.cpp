// Minimal HTTP configuration portal and OTA service. No login, display or UI modes.
#include "WebPortal.h"
#include "Platform.h"
#include "Settings.h"
#include "SettingsTransaction.h"
#include "Net.h"
#include "OtaUpdate.h"
#include "theme/ThemeWeb.h"
#include "theme/ThemeRuntime.h"
#include "config.h"
#include "webui_clean.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFiUdp.h>

extern const char* appResetReason();
extern void appShowNetworkStatus();
extern void applyDisplaySettings(const Settings& settings);

static WebServerClass server(80);
static Settings* S = nullptr;
static bool g_reboot = false;
static uint32_t g_rebootAt = 0;
static bool g_selfUpdate = false;
static String g_updateMsg;
static uint32_t g_uploadMax = 0;
static uint32_t g_uploadBytes = 0;
static bool g_uploadStarted = false;
static bool g_uploadMagicOk = false;
static String g_uploadError;
static constexpr uint32_t MinFirmwareImageSize = 16UL * 1024UL;
// This cap applies only to settings/import JSON. Future .stheme APIs should use
// their own streaming handler and format-specific size limit.
static constexpr size_t MaxSettingsJsonBytes = 2048;
static bool g_uploadComplete = false;

static void scheduleReboot(uint32_t inMs) {
  g_reboot = true;
  g_rebootAt = millis() + inMs;
}

static void sendJson(JsonDocument& doc, int code = 200) {
  String body;
  serializeJson(doc, body);
  server.send(code, "application/json", body);
}

static void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html", WEBUI_CLEAN_HTML, sizeof(WEBUI_CLEAN_HTML) - 1);
}

static void handleGetConfig() {
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  settingsToJson(*S, root, false);
  root["chip"] = "esp8266";
  sendJson(doc);
}

static void handleStatus() {
  const String ssid = netSSID();
  const String ip = netIP();
  JsonDocument doc;
  doc["fw"] = FW_NAME;
  doc["version"] = FW_VERSION;
  doc["variant"] = FW_VARIANT;
  doc["mode"] = netMode() == NET_AP ? "ap" : "sta";
  doc["connected"] = netConnected();
  doc["ssid"] = ssid;
  doc["ip"] = ip;
  doc["rssi"] = netRSSI();
  doc["wifiFail"] = netFailReason();
  doc["heap"] = ESP.getFreeHeap();
  doc["maxBlock"] = platformMaxFreeBlock();
  doc["uptime"] = millis() / 1000;
  doc["reset"] = appResetReason();
  doc["storageReady"] = settingsStorageReady();
  doc["themeLoaded"] = smalltv::themeRuntimeActive();
  doc["theme"] = smalltv::themeRuntimeActiveId();
  doc["themeError"] = smalltv::themeRuntimeError();
  doc["themeHeap"] = smalltv::themeRuntimeFreeHeap();
  doc["updateMsg"] = g_updateMsg;
  sendJson(doc);
}

static uint32_t netFingerprint(const Settings& settings) {
  // FNV-1a over network-relevant settings; avoids temporary heap-allocated Strings.
  uint32_t hash = 2166136261UL;
  const auto feed = [&hash](uint8_t value) {
    hash ^= value;
    hash *= 16777619UL;
  };
  feed(settings.wifiCount);
  for (uint8_t i = 0; i < settings.wifiCount; ++i) {
    for (size_t n = 0; n < settings.wifi[i].ssid.length(); ++n)
      feed((uint8_t)settings.wifi[i].ssid[n]);
    feed(0);
    for (size_t n = 0; n < settings.wifi[i].pass.length(); ++n)
      feed((uint8_t)settings.wifi[i].pass[n]);
    feed(0xFF);
  }
  for (size_t n = 0; n < settings.hostname.length(); ++n)
    feed((uint8_t)settings.hostname[n]);
  feed(0);
  for (size_t n = 0; n < settings.apSsid.length(); ++n)
    feed((uint8_t)settings.apSsid[n]);
  return hash;
}

static uint32_t displayFingerprint(const Settings& settings) {
  uint32_t hash = 2166136261UL;
  const uint8_t values[] = {
    settings.displayBrightness, settings.displayRotation,
    (uint8_t)settings.displayBgr, (uint8_t)settings.displayInvert,
    settings.displayRedGain, settings.displayGreenGain, settings.displayBlueGain
  };
  for (uint8_t value : values) {
    hash ^= value;
    hash *= 16777619UL;
  }
  return hash;
}

static void sendParseError(DeserializationError error) {
  if (error == DeserializationError::NoMemory) {
    server.send(503, "application/json", "{\"ok\":false,\"error\":\"not enough memory\"}");
  } else {
    server.send(400, "application/json",
                String("{\"ok\":false,\"error\":\"bad json (" ) + error.c_str() + ")\"}");
  }
}

static bool readSettingsJson(JsonDocument& doc) {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "no body");
    return false;
  }
  const String body = server.arg("plain");
  if (body.length() > MaxSettingsJsonBytes) {
    server.send(413, F("application/json"), F("{\"ok\":false,\"error\":\"configuration JSON exceeds 2048 bytes\"}"));
    return false;
  }
  if (DeserializationError error = deserializeJson(doc, body)) {
    sendParseError(error);
    return false;
  }
  if (!doc.is<JsonObject>()) {
    server.send(400, F("application/json"), F("{\"ok\":false,\"error\":\"JSON root must be an object\"}"));
    return false;
  }
  if (const __FlashStringHelper* error = settingsValidateJson(doc.as<JsonObjectConst>())) {
    String response = F("{\"ok\":false,\"error\":\"");
    response += error;
    response += F("\"}");
    server.send(400, "application/json", response);
    return false;
  }
  return true;
}

static void handlePostConfig() {
  JsonDocument doc;
  if (!readSettingsJson(doc)) return;

  const uint32_t oldNetwork = netFingerprint(*S);
  const uint32_t oldDisplay = displayFingerprint(*S);
  const bool saved = applyAndSaveSettings(
    *S,
    [&](Settings& settings) {
      settingsApplyJson(settings, doc.as<JsonObjectConst>());
      doc.clear();
    },
    [](const Settings& settings) { return saveSettings(settings); },
        [](Settings& settings) { loadSettings(settings); }
  );

  if (!saved) {
    server.send(500, "application/json", "{\"ok\":false,\"error\":\"could not save settings\"}");
    return;
  }

  if (displayFingerprint(*S) != oldDisplay) applyDisplaySettings(*S);

  const bool reboot = netFingerprint(*S) != oldNetwork;
  JsonDocument result;
  result["ok"] = true;
  result["reboot"] = reboot;
  sendJson(result);
  if (reboot) scheduleReboot(800);
}

static void handleScan() {
  const int found = WiFi.scanNetworks();
  JsonDocument doc;
  JsonArray items = doc.to<JsonArray>();
  for (int i = 0; i < found && i < 25; ++i) {
    JsonObject item = items.add<JsonObject>();
    item["ssid"] = WiFi.SSID(i);
    item["rssi"] = WiFi.RSSI(i);
    item["open"] = platformScanIsOpen(i);
  }
  WiFi.scanDelete();
  sendJson(doc);
}

static void handleReboot() {
  server.send(200, "application/json", "{\"ok\":true}");
  scheduleReboot(400);
}

static void handleFactory() {
  // Formatting is destructive and only allowed after the user confirms reset.
  if (!settingsStorageReady() && !settingsFormat()) {
    server.send(500, F("application/json"), F("{\"ok\":false,\"error\":\"could not initialize LittleFS\"}"));
    return;
  }
  if (!applyAndSaveSettings(
        *S,
        [](Settings& settings) { factoryReset(settings); },
        [](const Settings& settings) { return saveSettings(settings); },
        [](Settings& settings) { loadSettings(settings); })) {
    server.send(500, "application/json", "{\"ok\":false,\"error\":\"could not save factory defaults\"}");
    return;
  }
  server.send(200, "application/json", "{\"ok\":true}");
  scheduleReboot(400);
}

static void handleExport() {
  if (!settingsStorageReady()) {
    server.send(503, F("application/json"), F("{\"ok\":false,\"error\":\"LittleFS unavailable; use factory reset to initialize it\"}"));
    return;
  }
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  settingsToJson(*S, root, true);
  String body;
  serializeJson(doc, body);
  server.sendHeader("Content-Disposition", "attachment; filename=smalltv-config.json");
  server.send(200, "application/json", body);
}

static void handleImport() {
  JsonDocument doc;
  if (!readSettingsJson(doc)) return;

  if (!applyAndSaveSettings(
        *S,
        [&](Settings& settings) {
          settingsApplyJson(settings, doc.as<JsonObjectConst>());
          doc.clear();
        },
        [](const Settings& settings) { return saveSettings(settings); },
        [](Settings& settings) { loadSettings(settings); })) {
    server.send(500, "application/json", "{\"ok\":false,\"error\":\"could not save imported config\"}");
    return;
  }

  server.send(200, "application/json", "{\"ok\":true,\"reboot\":true}");
  scheduleReboot(800);
}

static void handleCheckUpdate() {
  OtaLatest latest = otaCheckLatest(*S);
  JsonDocument doc;
  doc["current"] = FW_VERSION;
  doc["ok"] = latest.ok;
  doc["latest"] = latest.tag;
  doc["newer"] = latest.newer;
  if (!latest.ok) doc["error"] = latest.error;
  sendJson(doc);
}

static void handleSelfUpdate() {
  g_selfUpdate = true;
  g_updateMsg = "starting...";
  server.send(200, "application/json", "{\"ok\":true}");
}

static void abortFirmwareUpload(const char* reason) {
  g_uploadComplete = false;
  g_uploadError = reason;
  if (!g_uploadStarted) return;
  uint8_t poison[2] = {0, 0};
  const size_t remaining = Update.remaining();
  const size_t poisonSize = remaining < sizeof(poison) ? remaining + 1 : 1;
  Update.write(poison, poisonSize);
  Update.end(false);
  g_uploadStarted = false;
}

static void handleUpdateUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    g_uploadMax = 0;
    g_uploadBytes = 0;
    g_uploadStarted = false;
    g_uploadMagicOk = false;
    g_uploadComplete = false;
    g_uploadError = "";

    WiFiUDP::stopAll();
    const uint32_t freeSpace = ESP.getFreeSketchSpace();
    if (freeSpace <= 0x1000) {
      g_uploadError = "no OTA slot available";
    } else {
      g_uploadMax = (freeSpace - 0x1000) & 0xFFFFF000;
      if (g_uploadMax < MinFirmwareImageSize) g_uploadError = "no OTA slot available";
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (g_uploadError.length()) return;
    if (!upload.currentSize || g_uploadBytes > g_uploadMax ||
        upload.currentSize > g_uploadMax - g_uploadBytes) {
      abortFirmwareUpload("firmware image exceeds the available OTA slot");
      return;
    }

    if (!g_uploadMagicOk) {
      if (upload.buf[0] != 0xE9) {
        g_uploadError = "invalid firmware image: expected ESP image magic 0xE9";
        return;
      }
      g_uploadMagicOk = true;
      if (!Update.begin(g_uploadMax)) {
        g_uploadError = platformUpdateError();
        return;
      }
      g_uploadStarted = true;
    }

    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      const String error = platformUpdateError();
      abortFirmwareUpload(error.c_str());
      return;
    }
    g_uploadBytes += upload.currentSize;
  } else if (upload.status == UPLOAD_FILE_END) {
    if (g_uploadError.length()) return;
    if (!g_uploadStarted || !g_uploadMagicOk ||
        g_uploadBytes < MinFirmwareImageSize ||
        upload.totalSize != g_uploadBytes) {
      abortFirmwareUpload(g_uploadBytes < MinFirmwareImageSize
        ? "firmware image is too small" : "incomplete firmware upload");
      return;
    }
    if (!Update.end(true)) {
      g_uploadError = platformUpdateError();
      g_uploadStarted = false;
      g_uploadComplete = false;
    } else {
      g_uploadStarted = false;
      g_uploadComplete = true;
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    abortFirmwareUpload("firmware upload was interrupted");
  }
  yield();
}

static void handleUpdateDone() {
  const bool ok = g_uploadComplete && g_uploadError.length() == 0 && !Update.hasError();
  server.sendHeader("Connection", "close");
  if (ok) {
    server.send(200, "text/plain", "OK");
    scheduleReboot(1200);
  } else {
    const String error = g_uploadError.length() ? g_uploadError
      : (g_uploadComplete ? platformUpdateError() : String(F("no complete firmware image received")));
    server.send(400, "text/plain", error);
  }
  g_uploadComplete = false;
  g_uploadStarted = false;
  g_uploadBytes = 0;
  g_uploadError = "";
}

static void handleNotFound() {
  if (netMode() == NET_AP) {
    server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    server.send(302, "text/plain", "");
  } else {
    server.send(404, "text/plain", "Not found");
  }
}

void webPortalBegin(Settings& settings) {
  S = &settings;
  themeWebBegin(server, settings);
  g_updateMsg = otaTakeBootResult();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handlePostConfig);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/scan", HTTP_GET, handleScan);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.on("/api/factory", HTTP_POST, handleFactory);
  server.on("/api/export", HTTP_GET, handleExport);
  server.on("/api/import", HTTP_POST, handleImport);
  server.on("/api/checkupdate", HTTP_GET, handleCheckUpdate);
  server.on("/api/selfupdate", HTTP_POST, handleSelfUpdate);
  server.on("/update", HTTP_POST, handleUpdateDone, handleUpdateUpload);
  server.on("/generate_204", HTTP_GET, handleNotFound);
  server.on("/gen_204", HTTP_GET, handleNotFound);
  server.on("/hotspot-detect.html", HTTP_GET, handleNotFound);
  server.on("/connecttest.txt", HTTP_GET, handleNotFound);
  server.onNotFound(handleNotFound);
  server.begin();
}

void webPortalLoop() {
  server.handleClient();
  if (!g_selfUpdate) return;
  g_selfUpdate = false;

  OtaLatest latest = otaCheckLatest(*S);
  if (!latest.ok) {
    g_updateMsg = "check failed: " + latest.error;
  } else if (!latest.newer) {
    g_updateMsg = "already up to date (" FW_VERSION ")";
  } else if (otaRequestBootUpdate(latest.tag.c_str())) {
    g_updateMsg = "updating...";
    scheduleReboot(400);
  } else {
    g_updateMsg = "could not queue update (storage error)";
  }
}

bool webPortalRebootDue() {
  return g_reboot && (int32_t)(millis() - g_rebootAt) >= 0;
}
