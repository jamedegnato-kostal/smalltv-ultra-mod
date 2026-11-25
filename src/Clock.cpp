#include "Clock.h"
#include "config.h"
#include "Net.h"
#include <coredecls.h>
#include <lwip/apps/sntp.h>
#include <stdio.h>
#include <string.h>

namespace {
char g_timezone[65] = {};
bool g_themeRequested = false;
bool g_nightRequested = false;
bool g_started = false;
bool g_callbackRegistered = false;
volatile bool g_syncedByNtp = false;

void onTimeSet(bool fromSntp) {
  if (fromSntp) g_syncedByNtp = true;
}

void applyTimezone(const char* timezone) {
  if (!timezone || !timezone[0]) timezone = DEFAULT_TIMEZONE;
  strncpy(g_timezone, timezone, sizeof(g_timezone) - 1);
  g_timezone[sizeof(g_timezone) - 1] = '\0';
}

bool clockRequested() {
  return g_themeRequested || g_nightRequested;
}

void stopNtpIfUnused() {
  if (clockRequested()) return;
  if (g_started) sntp_stop();
  g_started = false;
  g_syncedByNtp = false;
}

void startNtpWhenOnline() {
  if (!clockRequested() || g_started || !netConnected()) return;
  if (!g_callbackRegistered) {
    settimeofday_cb(onTimeSet);
    g_callbackRegistered = true;
  }
  // The ESP8266 time core applies this POSIX rule in localtime_r, including DST.
  configTime(g_timezone, NTP_SERVER1, NTP_SERVER2);
  g_started = true;
  Serial.println(F("[clock] SNTP started"));
}

}  // namespace

namespace smalltv {
void clockBegin(const char* timezone) {
  applyTimezone(timezone);
  g_themeRequested = true;
  startNtpWhenOnline();
}

void clockReconfigure(const char* timezone) {
  applyTimezone(timezone);
  if (g_started && clockRequested()) {
    // Re-arm SNTP only for the changed timezone; keep the device clock itself.
    configTime(g_timezone, NTP_SERVER1, NTP_SERVER2);
  } else {
    startNtpWhenOnline();
  }
}

void clockService() {
  startNtpWhenOnline();
}

void clockStop() {
  g_themeRequested = false;
  stopNtpIfUnused();
}

void clockSetNightModeEnabled(bool enabled, const char* timezone) {
  applyTimezone(timezone);
  g_nightRequested = enabled;
  if (clockRequested()) {
    if (g_started) configTime(g_timezone, NTP_SERVER1, NTP_SERVER2);
    else startNtpWhenOnline();
  } else {
    stopNtpIfUnused();
  }
}

bool clockSynced() {
  return g_syncedByNtp && time(nullptr) > 1609459200;
}

bool clockNow(tm& out) {
  if (!clockSynced()) return false;
  const time_t now = time(nullptr);
  return localtime_r(&now, &out) != nullptr;
}

void clockFormat(char* buffer, size_t size) {
  if (!buffer || !size) return;
  tm local{};
  if (!clockNow(local)) {
    buffer[0] = '\0';
    return;
  }
  snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d",
           local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
           local.tm_hour, local.tm_min, local.tm_sec);
}
}  // namespace smalltv
