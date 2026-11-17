#include "ThemeSchedule.h"
#include <cassert>
#include <cstdio>

int main() {
  using smalltv::themeScheduleContains;
  // Normal daytime interval: inclusive start, exclusive end.
  assert(!themeScheduleContains(539, 540, 1020));
  assert(themeScheduleContains(540, 540, 1020));
  assert(themeScheduleContains(1019, 540, 1020));
  assert(!themeScheduleContains(1020, 540, 1020));
  // Overnight interval wraps midnight.
  assert(themeScheduleContains(1320, 1320, 360));
  assert(themeScheduleContains(0, 1320, 360));
  assert(themeScheduleContains(359, 1320, 360));
  assert(!themeScheduleContains(360, 1320, 360));
  assert(!themeScheduleContains(800, 1320, 360));
  // Empty/default windows are disabled, never interpreted as all-day.
  assert(!themeScheduleContains(0, 0, 0));
  assert(!themeScheduleContains(720, 720, 720));
  // Invalid minute values fail closed.
  assert(!themeScheduleContains(1440, 0, 10));
  assert(!themeScheduleContains(0, 1440, 10));
  std::puts("theme schedule tests passed");
  return 0;
}
