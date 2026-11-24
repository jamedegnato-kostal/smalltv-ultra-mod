#include "DisplayPanel.h"
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <font/glcdfont.h>
#include <algorithm>
#include <new>
#ifdef ARDUINO
#include <pgmspace.h>
#endif

namespace {
using smalltv::display::PanelWidth;
using smalltv::display::PanelHeight;

static bool s_bgr = smalltv::display::DefaultBgr;
static Arduino_DataBus* s_bus = nullptr;
static Arduino_GFX* s_gfx = nullptr;

// The stock ESP8266 Arduino_ST7789 selects SPI mode 2. The SmallTV panel
// requires mode 3; this subclass preserves the rotation mapping and allows
// the RGB/BGR MADCTL bit to follow the selected panel profile.
class Arduino_ST7789_SmallTV final : public Arduino_ST7789 {
 public:
  using Arduino_ST7789::Arduino_ST7789;

  bool begin(int32_t speed = GFX_NOT_DEFINED) override {
    _override_datamode = SPI_MODE3;
    return Arduino_TFT::begin(speed);
  }

  void setRotation(uint8_t rotation) override {
    Arduino_TFT::setRotation(rotation);
    uint8_t madctl = 0;
    switch (_rotation) {
      case 1: madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MV; break;
      case 2: madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MY; break;
      case 3: madctl = ST7789_MADCTL_MY | ST7789_MADCTL_MV; break;
      default: break;
    }
    if (s_bgr) madctl |= 0x08;
    _bus->beginWrite();
    _bus->writeC8D8(ST7789_MADCTL, madctl);
    _bus->endWrite();
  }
};

static uint8_t clampGain(uint8_t value) {
  return value < 50 ? 50 : (value > 150 ? 150 : value);
}

static uint16_t applyGain(uint16_t rgb565, const smalltv::display::PanelColorProfile& profile) {
  if (profile.redGain == 100 && profile.greenGain == 100 && profile.blueGain == 100) return rgb565;
  uint32_t red = (rgb565 >> 11) & 0x1f;
  uint32_t green = (rgb565 >> 5) & 0x3f;
  uint32_t blue = rgb565 & 0x1f;
  red = std::min<uint32_t>(31, red * profile.redGain / 100);
  green = std::min<uint32_t>(63, green * profile.greenGain / 100);
  blue = std::min<uint32_t>(31, blue * profile.blueGain / 100);
  return static_cast<uint16_t>((red << 11) | (green << 5) | blue);
}
}  // namespace

namespace smalltv {
namespace display {
bool DisplayPanel::begin() {
  if (ready()) return true;

  // Keep the active-low backlight dark until the controller has started.
  backlight_.begin(BacklightActiveLow);
  s_bus = new (std::nothrow) Arduino_HWSPI(PinDc, PinChipSelect);
  if (!s_bus) return false;

  s_gfx = new (std::nothrow) Arduino_ST7789_SmallTV(
      s_bus, PinReset, 0, true, PanelWidth, PanelHeight,
      ColumnOffset1, RowOffset1, ColumnOffset2, RowOffset2);
  if (!s_gfx) {
    delete s_bus;
    s_bus = nullptr;
    return false;
  }
  if (!s_gfx->begin()) {
    delete s_gfx;
    delete s_bus;
    s_gfx = nullptr;
    s_bus = nullptr;
    return false;
  }

  s_gfx->setTextWrap(false);
  applyColorProfile();
  s_gfx->fillScreen(0x0000);
  return true;
}

bool DisplayPanel::ready() const {
  return s_gfx != nullptr;
}

void DisplayPanel::clear(uint16_t rgb565) {
  if (s_gfx) s_gfx->fillScreen(applyGain(rgb565, profile_));
}

void DisplayPanel::setColorProfile(const PanelColorProfile& profile) {
  profile_ = profile;
  profile_.rotation &= 3;
  profile_.redGain = clampGain(profile_.redGain);
  profile_.greenGain = clampGain(profile_.greenGain);
  profile_.blueGain = clampGain(profile_.blueGain);
  applyColorProfile();
}

void DisplayPanel::setRotation(uint8_t rotation) {
  profile_.rotation = rotation & 3;
  if (s_gfx) s_gfx->setRotation(profile_.rotation);
}

void DisplayPanel::setBrightness(uint8_t percent, bool activeLow) {
  backlight_.setPercent(percent, activeLow);
}

void DisplayPanel::setBrightnessSteps(uint8_t steps, bool activeLow) {
  backlight_.setSteps(steps, activeLow);
}

void DisplayPanel::applyColorProfile() {
  s_bgr = profile_.bgr;
  if (!s_gfx) return;
  s_gfx->setRotation(profile_.rotation);
  s_gfx->invertDisplay(profile_.invert);
}

uint8_t DisplayPanel::glyphColumn(uint8_t character, uint8_t column) const {
  if (column >= 5) return 0;
  return pgm_read_byte(font + (unsigned(character) * 5u) + column);
}

void DisplayPanel::row(int x, int y, const uint16_t* pixels, int count) {
  if (!s_gfx || !pixels || count <= 0 || y < 0 || y >= PanelHeight) return;
  if (x < 0) {
    const int skip = -x;
    if (skip >= count) return;
    pixels += skip;
    count -= skip;
    x = 0;
  }
  if (x >= PanelWidth) return;
  if (count > PanelWidth - x) count = PanelWidth - x;
  if (count <= 0) return;

  uint16_t tinted[PanelWidth];
  for (int i = 0; i < count; ++i) tinted[i] = applyGain(pixels[i], profile_);

  auto* tft = static_cast<Arduino_TFT*>(s_gfx);
  tft->startWrite();
  tft->writeAddrWindow(x, y, count, 1);
  tft->writePixels(tinted, static_cast<uint32_t>(count));
  tft->endWrite();

  if ((y & 15) == 15) yield();
}
}  // namespace display
}  // namespace smalltv
