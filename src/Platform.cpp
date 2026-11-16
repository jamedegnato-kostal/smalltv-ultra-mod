// Platform.cpp — the few things that need a real definition (not header-inline)
// but are still core-specific. Keeps the per-chip SNTP callback wiring out of the
// feature code, same as the inline shims in Platform.h.
#include "Platform.h"
#include "config.h"

namespace {
uint32_t s_minFreeHeap = 0xffffffffu;
uint32_t s_minMaxBlock = 0xffffffffu;
uint32_t s_lastMemorySampleMs = 0;
}
void platformMemorySample(bool force) {
  const uint32_t now = millis();
  if (!force && s_lastMemorySampleMs && uint32_t(now - s_lastMemorySampleMs) < 250) return;
  s_lastMemorySampleMs = now;
  const uint32_t heap = ESP.getFreeHeap();
  const uint32_t block = platformMaxFreeBlock();
  if (heap < s_minFreeHeap) s_minFreeHeap = heap;
  if (block < s_minMaxBlock) s_minMaxBlock = block;
}
void platformMemoryStatsGet(PlatformMemoryStats& out) {
  platformMemorySample(true);
  out.minHeap = s_minFreeHeap == 0xffffffffu ? ESP.getFreeHeap() : s_minFreeHeap;
  out.minMaxBlock = s_minMaxBlock == 0xffffffffu ? platformMaxFreeBlock() : s_minMaxBlock;
}

#if defined(SMALLTV_FAMILY_ESP32)
#include <esp_sntp.h>

static void (*s_syncCb)() = nullptr;
static void sntpSyncNotify(struct timeval*) { if (s_syncCb) s_syncCb(); }

void platformOnTimeSync(void (*cb)()) {
  s_syncCb = cb;
  sntp_set_time_sync_notification_cb(sntpSyncNotify);
}

#else  // ESP8266
#include <coredecls.h>
#include "CrashRecord.h"
extern "C" {
extern void* umm_last_fail_alloc_addr;
extern int umm_last_fail_alloc_size;
}

static void (*s_syncCb)() = nullptr;
// settimeofday_cb fires whenever the clock is set; from_sntp distinguishes an
// SNTP update (what we care about) from a manual settimeofday (which we never do).
static void sntpSyncNotify(bool from_sntp) { if (from_sntp && s_syncCb) s_syncCb(); }

void platformOnTimeSync(void (*cb)()) {
  s_syncCb = cb;
  settimeofday_cb(sntpSyncNotify);
}

// RTC words 0..31 are reserved by the OTA hand-off. Keep this record at 96.
static const uint32_t CrashRtcWord = 96;
static_assert(sizeof(CrashRtc) % 4 == 0 && sizeof(CrashRtc) / 4 + CrashRtcWord <= 128,
              "crash record must fit RTC user area");
static bool crashLoad(CrashRtc& r) {
  return ESP.rtcUserMemoryRead(CrashRtcWord, reinterpret_cast<uint32_t*>(&r), sizeof(r)) &&
         r.magic == CRASH_MAGIC && r.check == crashCheck(r);
}
static void crashStore(CrashRtc& r) {
  r.magic = CRASH_MAGIC;
  r.check = crashCheck(r);
  ESP.rtcUserMemoryWrite(CrashRtcWord, reinterpret_cast<uint32_t*>(&r), sizeof(r));
}
extern "C" void custom_crash_callback(struct rst_info* ri, uint32_t stack, uint32_t stackEnd) {
  CrashRtc r{};
  r.flags = CRASH_PENDING; r.exccause = ri->exccause; r.epc1 = ri->epc1;
  r.excvaddr = ri->excvaddr; r.uptimeS = millis() / 1000; r.rstReason = ri->reason;
  // The core records these when DEBUG_ESP_OOM is enabled. Zero means no
  // failed allocation was recorded; stack bounds are supplied by postmortem.
  r.allocCaller = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(umm_last_fail_alloc_addr));
  r.allocSize = umm_last_fail_alloc_size > 0 ? static_cast<uint32_t>(umm_last_fail_alloc_size) : 0;
  r.stack = stack; r.stackEnd = stackEnd;
  r.uptimeKnown = 1; strlcpy(r.fw, FW_VERSION, sizeof(r.fw)); crashStore(r);
}
void platformCrashRecordBegin() {
  struct rst_info* ri = ESP.getResetInfoPtr();
  if (!ri) return;
  CrashRtc r{}; const bool have = crashLoad(r);
  if (have && (r.flags & CRASH_PENDING)) { r.flags = 0; crashStore(r); return; }
  if (ri->reason != REASON_WDT_RST && ri->reason != REASON_EXCEPTION_RST && ri->reason != REASON_SOFT_WDT_RST) return;
  r.exccause = ri->exccause; r.epc1 = ri->epc1; r.excvaddr = ri->excvaddr;
  r.rstReason = ri->reason; strlcpy(r.fw, FW_VERSION, sizeof(r.fw)); crashStore(r);
}
bool platformCrashRecordGet(PlatformCrashRecord& out) {
  CrashRtc r{}; if (!crashLoad(r)) return false;
  memset(&out, 0, sizeof(out)); strlcpy(out.reason, crashReasonLabel(r.rstReason), sizeof(out.reason));
  strlcpy(out.fw, r.fw, sizeof(out.fw)); out.hasRegisters = crashHasRegisters(r.rstReason);
  out.exccause = r.exccause; out.epc1 = r.epc1; out.excvaddr = r.excvaddr;
  out.uptimeS = r.uptimeS; out.uptimeKnown = r.uptimeKnown != 0;
  out.allocCaller = r.allocCaller; out.allocSize = r.allocSize;
  out.stack = r.stack; out.stackEnd = r.stackEnd;
  return true;
}
void platformCrashRecordClear() { uint32_t zero[sizeof(CrashRtc) / 4] = {}; ESP.rtcUserMemoryWrite(CrashRtcWord, zero, sizeof(zero)); }
#endif
