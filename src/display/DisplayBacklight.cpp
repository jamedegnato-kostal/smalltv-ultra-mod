#include "DisplayBacklight.h"
#include "DisplayBoard.h"
#include <Arduino.h>
#include <sigma_delta.h>

namespace smalltv {
namespace display {
void DisplayBacklight::begin(bool activeLow) {
  pinMode(PinBacklight, OUTPUT);
  sigmaDeltaEnable();
  sigmaDeltaSetPrescaler(SigmaDeltaPrescaler);
  valid_ = false;
  setSteps(0, activeLow);  // safe default: keep the panel dark
}

void DisplayBacklight::setPercent(uint8_t percent, bool activeLow) {
  if (percent > 100) percent = 100;
  setSteps(static_cast<uint8_t>((uint16_t(percent) * 255u + 50u) / 100u), activeLow);
}

void DisplayBacklight::setSteps(uint8_t steps, bool activeLow) {
  const BacklightPlan next = backlightPlan(steps, activeLow);
  if (valid_ && sameBacklightPlan(next, last_)) return;

  if (next.sigmaDelta) {
    sigmaDeltaWrite(0, next.duty);
    if (!sigmaDeltaIsPinAttached(PinBacklight)) sigmaDeltaAttachPin(PinBacklight);
  } else {
    sigmaDeltaDetachPin(PinBacklight);
    digitalWrite(PinBacklight, next.level ? HIGH : LOW);
  }

  last_ = next;
  valid_ = true;
}
}  // namespace display
}  // namespace smalltv
