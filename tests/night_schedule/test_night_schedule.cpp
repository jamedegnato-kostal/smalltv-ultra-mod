#include "NightSchedule.h"
#include <assert.h>

int main() {
  using smalltv::nightScheduleActive;
  assert(!nightScheduleActive(21 * 60 + 59, 22 * 60, 7 * 60));
  assert(nightScheduleActive(22 * 60, 22 * 60, 7 * 60));
  assert(nightScheduleActive(23 * 60 + 59, 22 * 60, 7 * 60));
  assert(nightScheduleActive(0, 22 * 60, 7 * 60));
  assert(nightScheduleActive(6 * 60 + 59, 22 * 60, 7 * 60));
  assert(!nightScheduleActive(7 * 60, 22 * 60, 7 * 60));
  assert(!nightScheduleActive(7 * 60 + 59, 8 * 60, 17 * 60));
  assert(nightScheduleActive(8 * 60, 8 * 60, 17 * 60));
  assert(nightScheduleActive(16 * 60 + 59, 8 * 60, 17 * 60));
  assert(!nightScheduleActive(17 * 60, 8 * 60, 17 * 60));
  assert(!nightScheduleActive(12 * 60, 12 * 60, 12 * 60));
  assert(!nightScheduleActive(1440, 22 * 60, 7 * 60));
  assert(!nightScheduleActive(20, 1440, 7 * 60));
  assert(!nightScheduleActive(20, 22 * 60, 1440));
  return 0;
}
