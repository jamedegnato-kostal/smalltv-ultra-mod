#pragma once
#include "BacklightPlan.h"
#include <stdint.h>

namespace smalltv {
namespace display {
// ESP8266 hardware sigma-delta output; avoids the timer-interrupt edge jitter
// of software PWM while Wi-Fi is busy.
class DisplayBacklight {
 public:
  void begin(bool activeLow = true);
  void setPercent(uint8_t percent, bool activeLow = true);
  void setSteps(uint8_t steps, bool activeLow = true);

 private:
  BacklightPlan last_{false, false, 0};
  bool valid_ = false;
};
}  // namespace display
}  // namespace smalltv
