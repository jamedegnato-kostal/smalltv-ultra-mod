// BacklightPlan.h - pure plan for the ESP8266 hardware sigma-delta output.
// Kept independent of Arduino headers so its full level/polarity mapping is host-testable.
#pragma once
#include <stdint.h>

namespace smalltv {
static const uint8_t BL_SD_PRESCALER = 79; // 80 MHz / (79 + 1) = 1 MHz
struct BacklightPlan { bool sigmaDelta; bool level; uint8_t duty; };

// steps is the requested light share (0..255). Fixed GPIO levels at both
// endpoints avoid residual pulses. inverted means the backlight is active-low.
inline BacklightPlan backlightPlan(uint8_t steps, bool inverted) {
  BacklightPlan plan = {false, false, 0};
  if (steps == 0) { plan.level = inverted; return plan; }
  if (steps == 255) { plan.level = !inverted; return plan; }
  const uint32_t high = inverted ? 255u - steps : steps;
  plan.sigmaDelta = true;
  plan.duty = static_cast<uint8_t>((high * 256u + 127u) / 255u);
  return plan;
}
inline bool backlightPlanSame(const BacklightPlan& a, const BacklightPlan& b) {
  if (a.sigmaDelta != b.sigmaDelta) return false;
  return a.sigmaDelta ? a.duty == b.duty : a.level == b.level;
}
inline uint32_t backlightClockHz() { return 80000000u / (BL_SD_PRESCALER + 1u); }
inline uint32_t backlightPulseHz(uint8_t duty) {
  const uint32_t active = duty <= 128 ? duty : 256u - duty;
  return static_cast<uint32_t>((static_cast<uint64_t>(backlightClockHz()) * active) / 256u);
}
}  // namespace smalltv
