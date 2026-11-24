// ESP8266-only platform layer required for Wi-Fi, HTTP and OTA.
#pragma once
#include "Target.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <Updater.h>
extern "C" {
#include <user_interface.h>
}

using WebServerClass = ESP8266WebServer;
using SecureClient = BearSSL::WiFiClientSecure;

static inline void platformSetHostname(const char* hostname) {
  WiFi.hostname(hostname);
}
static inline void platformMdnsUpdate() {
  MDNS.update();
}
static inline bool platformScanIsOpen(int index) {
  return WiFi.encryptionType(index) == ENC_TYPE_NONE;
}
static inline String platformUpdateError() {
  return Update.getErrorString();
}
static inline uint32_t platformChipId() {
  return ESP.getChipId();
}
static inline uint32_t platformMaxFreeBlock() {
  return ESP.getMaxFreeBlockSize();
}

struct PlatformReset {
  String reason;
  bool wasCrash;
};
static inline PlatformReset platformResetInfo() {
  PlatformReset info;
  info.reason = ESP.getResetReason();
  struct rst_info* reset = ESP.getResetInfoPtr();
  info.wasCrash = reset && (
    reset->reason == REASON_EXCEPTION_RST ||
    reset->reason == REASON_WDT_RST ||
    reset->reason == REASON_SOFT_WDT_RST
  );
  return info;
}
