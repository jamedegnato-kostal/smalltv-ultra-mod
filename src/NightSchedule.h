#pragma once
#include <stdint.h>

namespace smalltv {

// Half-open local-time interval [start, end). Crossing midnight is supported.
// Equal endpoints mean the schedule is off, not a 24-hour blackout.
inline bool nightScheduleActive(uint16_t nowMinute,
                                uint16_t startMinute,
                                uint16_t endMinute) {
  if (nowMinute >= 1440 || startMinute >= 1440 || endMinute >= 1440 ||
      startMinute == endMinute) return false;
  if (startMinute < endMinute)
    return nowMinute >= startMinute && nowMinute < endMinute;
  return nowMinute >= startMinute || nowMinute < endMinute;
}

}  // namespace smalltv
