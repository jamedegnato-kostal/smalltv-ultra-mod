// Pure ESP8266 backlight-level calculation. Kept independent of Arduino APIs
// so its boundary and edge cases can also be exercised by native tests.
#pragma once
#include <stdint.h>

namespace smalltv {
namespace display {
constexpr uint8_t SigmaDeltaPrescaler = 79;

struct BacklightPlan {
  bool sigmaDelta;
  bool level;
  uint8_t duty;
};

inline BacklightPlan backlightPlan(uint8_t steps, bool inverted) {
  BacklightPlan plan{false, false, 0};
  if (steps == 0) {
    plan.level = inverted;
    return plan;
  }
  if (steps == 255) {
    plan.level = !inverted;
    return plan;
  }
  const uint32_t high = inverted ? 255u - steps : steps;
  plan.sigmaDelta = true;
  plan.duty = static_cast<uint8_t>((high * 256u + 127u) / 255u);
  return plan;
}

inline bool sameBacklightPlan(const BacklightPlan& a, const BacklightPlan& b) {
  if (a.sigmaDelta != b.sigmaDelta) return false;
  return a.sigmaDelta ? a.duty == b.duty : a.level == b.level;
}
}  // namespace display
}  // namespace smalltv
