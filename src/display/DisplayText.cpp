#include "DisplayText.h"
#include "DisplayBoard.h"
#include <algorithm>
#include <cstring>

namespace smalltv {
namespace display {
int textWidth(const char* text, uint8_t scale) {
  if (!text || !scale) return 0;
  const size_t length = strlen(text);
  const size_t width = length * 6u * scale;
  return width > static_cast<size_t>(PanelWidth) * 16u ? PanelWidth * 16 : static_cast<int>(width);
}

uint8_t fitTextScale(const char* text, int maxWidth, uint8_t maxScale) {
  if (!text || maxWidth <= 0) return 1;
  for (uint8_t scale = maxScale; scale > 1; --scale) {
    if (textWidth(text, scale) <= maxWidth) return scale;
  }
  return 1;
}

void drawText(smalltv::Display& sink, const char* text, int x, int y,
              uint8_t scale, uint16_t foreground, uint16_t background) {
  if (!text || !text[0] || !scale) return;
  const int width = textWidth(text, scale);
  const int left = std::max(0, x);
  const int right = std::min(PanelWidth, x + width);
  if (right <= left) return;

  uint16_t row[PanelWidth];
  const int visibleWidth = right - left;
  for (int py = 0; py < 8 * scale; ++py) {
    const int targetY = y + py;
    if (targetY < 0 || targetY >= PanelHeight) continue;
    std::fill(row, row + visibleWidth, background);
    const int glyphRow = py / scale;

    for (size_t character = 0; text[character]; ++character) {
      const int cellX = x + static_cast<int>(character * 6u * scale);
      const int glyphLeft = std::max(left, cellX);
      const int glyphRight = std::min(right, cellX + 5 * scale);
      if (glyphRight <= glyphLeft) continue;
      for (int glyphColumn = 0; glyphColumn < 5; ++glyphColumn) {
        const uint8_t bits = sink.glyphColumn(static_cast<uint8_t>(text[character]), static_cast<uint8_t>(glyphColumn));
        if (!(bits & (1u << glyphRow))) continue;
        const int pixelLeft = cellX + glyphColumn * scale;
        const int pixelRight = std::min(right, pixelLeft + scale);
        for (int px = std::max(left, pixelLeft); px < pixelRight; ++px) row[px - left] = foreground;
      }
    }
    sink.row(left, targetY, row, visibleWidth);
  }
}

void drawCenteredText(smalltv::Display& sink, const char* text, int y,
                      uint8_t scale, uint16_t foreground, uint16_t background) {
  const int width = textWidth(text, scale);
  drawText(sink, text, (PanelWidth - width) / 2, y, scale, foreground, background);
}
}  // namespace display
}  // namespace smalltv
