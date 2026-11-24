#pragma once
#include "../theme/ThemeDisplay.h"
#include <stdint.h>

namespace smalltv {
namespace display {
// Small opaque 6x8 bitmap text output, independent of the ST7789 driver.
// The sink supplies glyph columns and accepts complete RGB565 output rows.
int textWidth(const char* text, uint8_t scale);
uint8_t fitTextScale(const char* text, int maxWidth, uint8_t maxScale);
void drawText(smalltv::Display& sink, const char* text, int x, int y,
              uint8_t scale, uint16_t foreground, uint16_t background);
void drawCenteredText(smalltv::Display& sink, const char* text, int y,
                      uint8_t scale, uint16_t foreground, uint16_t background);
}  // namespace display
}  // namespace smalltv
