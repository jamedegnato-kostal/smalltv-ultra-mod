#pragma once
#include "DisplayBacklight.h"
#include "DisplayBoard.h"
#include "../theme/ThemeDisplay.h"
#include <stdint.h>

namespace smalltv {
namespace display {
struct PanelColorProfile {
  uint8_t rotation = 0;
  bool bgr = DefaultBgr;
  bool invert = false;
  uint8_t redGain = 100;
  uint8_t greenGain = 100;
  uint8_t blueGain = 100;
};

// Concrete ST7789 output for the theme row renderer. Construction does not
// touch GPIO/SPI; only an explicit begin() call initializes hardware.
class DisplayPanel final : public smalltv::Display {
 public:
  bool begin();
  bool ready() const;
  void clear(uint16_t rgb565 = 0x0000);

  void setColorProfile(const PanelColorProfile& profile);
  void setRotation(uint8_t rotation);
  void setBrightness(uint8_t percent, bool activeLow = BacklightActiveLow);
  void setBrightnessSteps(uint8_t steps, bool activeLow = BacklightActiveLow);

  uint8_t glyphColumn(uint8_t character, uint8_t column) const override;
  void row(int x, int y, const uint16_t* pixels, int count) override;

 private:
  void applyColorProfile();

  PanelColorProfile profile_;
  DisplayBacklight backlight_;
};
}  // namespace display
}  // namespace smalltv
