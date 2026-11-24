#pragma once
#include "ThemeEngine.h"

namespace smalltv {
// Physical-output boundary for the theme renderer.
//
// The model/runtime above this API stays independent of the display driver.
// Assets supply pixels and font coverage a row at a time; Display receives
// finished RGB565 scanlines. No full 240x240 framebuffer is required.
// Assets deliver RGB565 and 8-bit opacity; a row never exceeds 240 pixels.
using AssetHandle = uint16_t;
constexpr AssetHandle InvalidAsset = 0xffff;
// The widest ink of one package-font glyph: a glyph lies within its advance,
// and an advance is at most 120 (the package check, Package::checkFont()).
constexpr size_t MaxGlyphWidth = 120;
// Alpha-composites an RGB565 foreground pixel over a background one, blending
// each of the 5/6/5 channels independently with rounding (+127 before the
// /255 integer division). Used for image/animation alpha and a package font's
// coverage -- shapes and built-in font text are fully opaque where drawn.
inline uint16_t blend(uint16_t bg, uint16_t fg, uint8_t a) {
  if (a == 255) return fg;
  if (a == 0) return bg;
  unsigned b = 255 - a;
  return ((((fg >> 11) * a + (bg >> 11) * b + 127) / 255) << 11) |
    (((((fg >> 5) & 63) * a + ((bg >> 5) & 63) * b + 127) / 255) << 5) |
    (((fg & 31) * a + (bg & 31) * b + 127) / 255);
}
// Row-based asset provider so render() never needs a whole image or animation
// frame in memory at once -- only the pixel rows a dirty rectangle actually
// touches are read.
class Assets {
 public:
  virtual ~Assets() = default;
  // Handles belong to this asset provider and stay valid until it is reloaded.
  virtual AssetHandle resolve(const std::string& path) = 0;
  virtual bool row(AssetHandle handle, int y, int x, int count,
                   uint16_t* colors, uint8_t* alpha) = 0;
  bool row(const std::string& path, int y, int x, int count,
           uint16_t* colors, uint8_t* alpha) {
    return row(resolve(path), y, x, count, colors, alpha);
  }
  // Like row(), but blends the pixels over `pixels[0..count)` (blend()) instead
  // of handing them back, so the caller needs no colour and alpha buffers
  // (720 bytes of the loop stack while a frame is drawn). The base class
  // does it with row() in small pieces; a provider that can read and blend
  // in one pass overrides it.
  virtual bool blendRow(AssetHandle handle, int y, int x, int count, uint16_t* pixels);
  // One row of a package font's glyph: line `y` of the line box, as 0-255
  // coverage for the `width` pixels from `left` within the glyph's advance;
  // `width` is 0 where the glyph has no ink. `coverage` holds MaxGlyphWidth
  // bytes, and no glyph is wider.
  virtual bool glyphRow(AssetHandle, uint8_t, int, uint8_t*, int&, int&) { return false; }
};
// Font/output sink render() writes into. glyphColumn() returns the built-in
// bitmap font's column bits for one character; row() streams one already
// composited scanline to the physical display (or, on desktop, a buffer).
class Display {
 public:
  virtual ~Display() = default;
  virtual uint8_t glyphColumn(uint8_t character, uint8_t column) const = 0;
  virtual void row(int x, int y, const uint16_t* pixels, int count) = 0;
};
// Rebuilds each dirty row in layer order; no display readback or full framebuffer.
bool render(const Engine& engine, const std::vector<Rect>& dirty, Assets& assets, Display& display);
}
