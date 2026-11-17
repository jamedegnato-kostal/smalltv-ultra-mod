#pragma once
#include <stdint.h>

// A daily local-time window. Equal endpoints mean an empty/disabled window,
// not a full day; this makes the zero-initialized UI values safe by default.
namespace smalltv {
inline bool themeScheduleContains(uint16_t minute, uint16_t start, uint16_t end) {
  if (minute >= 1440 || start >= 1440 || end >= 1440 || start == end) return false;
  if (start < end) return minute >= start && minute < end;
  return minute >= start || minute < end;  // wraps midnight
}
}  // namespace smalltv
