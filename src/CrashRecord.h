#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum CrashReasonCode : uint8_t {
  CRASH_RST_WDT = 1, CRASH_RST_EXCEPTION = 2, CRASH_RST_SOFT_WDT = 3,
  CRASH_RST_STACK_SMASH = 253, CRASH_RST_SW_EXC = 254,
};
static const uint32_t CRASH_MAGIC = 0x43525348;
static const uint32_t CRASH_PENDING = 1;
struct CrashRtc {
  uint32_t magic, check, flags, exccause, epc1, excvaddr, uptimeS;
  uint8_t rstReason, uptimeKnown;
  uint16_t pad;
  char fw[16];
};
static inline uint32_t crashCheck(const CrashRtc& r) {
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&r) + 8;
  uint32_t h = 2166136261u;
  for (size_t i = 8; i < sizeof(r); ++i) { h ^= *p++; h *= 16777619u; }
  return h;
}
static inline const char* crashReasonLabel(uint8_t reason) {
  switch (reason) {
    case CRASH_RST_EXCEPTION: return "Exception";
    case CRASH_RST_SOFT_WDT: return "Software watchdog";
    case CRASH_RST_WDT: return "Hardware watchdog";
    case CRASH_RST_SW_EXC: return "Abort/out of memory";
    case CRASH_RST_STACK_SMASH: return "Stack overflow";
    default: return "Unknown";
  }
}
static inline bool crashHasRegisters(uint8_t reason) {
  return reason != CRASH_RST_SW_EXC && reason != CRASH_RST_STACK_SMASH;
}
