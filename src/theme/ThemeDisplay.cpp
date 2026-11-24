#include "ThemeDisplay.h"
#include "ThemeRenderInternal.h"
#ifdef ARDUINO
#include "config.h"
#endif

#if !defined(ARDUINO) || (WITH_THEME && !defined(SMALLTV_THEME_HEADLESS))
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <cstring>

namespace smalltv {
// The base class's blendRow(): row() in pieces of 16 pixels (48 bytes of
// buffer), blended over `pixels`.
bool Assets::blendRow(AssetHandle handle, int y, int x, int count, uint16_t* pixels) {
  uint16_t colors[16];
  uint8_t alpha[16];
  for (int done = 0; done < count; done += 16) {
    const int piece = std::min(16, count - done);
    if (!row(handle, y, x + done, piece, colors, alpha)) return false;
    for (int i = 0; i < piece; ++i) pixels[done + i] = blend(pixels[done + i], colors[i], alpha[i]);
  }
  return true;
}
// True if (x, y) falls within strokeWidth/2 of the segment (lx,ly)-(lx2,ly2),
// giving it round ends (a "capsule" shape): project the pixel onto the
// segment via the dot product, and if that projection falls before the start
// or past the end, test distance to the nearest endpoint instead of to the
// infinite line. All distances are compared squared (and doubled, `4 *`, to
// compare against strokeWidth rather than strokeWidth/2) to avoid a sqrt, and
// widened to int64_t since squaring a 240-range coordinate can exceed 32 bits.
static bool linePixel(int lx, int ly, int lx2, int ly2, int strokeWidth, int x, int y) {
  int64_t dx = lx2 - lx, dy = ly2 - ly, px = x - lx, py = y - ly;
  int64_t len = dx * dx + dy * dy, dot = px * dx + py * dy;
  int64_t sw = strokeWidth;
  if (dot <= 0 || len == 0) return 4 * (px * px + py * py) <= sw * sw;
  if (dot >= len) {
    px = x - lx2;
    py = y - ly2;
    return 4 * (px * px + py * py) <= sw * sw;
  }
  // Perpendicular distance from the line, via the cross product magnitude
  // divided by the segment length -- kept as cross^2 vs sw^2*len to stay in
  // integer arithmetic (no division, no sqrt).
  int64_t cross = px * dy - py * dx;
  return 4 * cross * cross <= sw * sw * len;
}
// Classifies a pixel against a rounded rectangle's outer outline (fill) and the
// outer-minus-inner ring (stroke), where the inner radius is max(0,radius-strokeWidth).
//
// A pixel outside all four corner quadrants (i.e. in the straight top/bottom/
// left/right bands) uses the same fast distance-to-edge test as the
// radius==0 case, since rounding only ever affects the corners. Only a pixel
// inside a corner quadrant needs the squared-distance circle test, against a
// center placed `radius` pixels in from that corner -- the outer and inner
// arcs share that same center, just at radius and radius-strokeWidth, so one
// distance computation classifies both.
static void roundedRectHit(int x, int y, int rx, int ry, int rw, int rh, int radius, int strokeWidth,
                            bool& inside, bool& edge) {
  if (radius <= 0) {
    inside = true;
    edge = x - rx < strokeWidth || rx + rw - x <= strokeWidth || y - ry < strokeWidth || ry + rh - y <= strokeWidth;
    return;
  }
  const bool left = x < rx + radius, right = x >= rx + rw - radius;
  const bool top = y < ry + radius, bottom = y >= ry + rh - radius;
  if (!((left || right) && (top || bottom))) {
    inside = true;
    edge = x - rx < strokeWidth || rx + rw - x <= strokeWidth || y - ry < strokeWidth || ry + rh - y <= strokeWidth;
    return;
  }
  const int64_t cx = left ? rx + radius : rx + rw - radius - 1;
  const int64_t cy = top ? ry + radius : ry + rh - radius - 1;
  const int64_t ddx = x - cx, ddy = y - cy;
  const int64_t dist = ddx * ddx + ddy * ddy;
  inside = dist <= int64_t(radius) * radius;
  const int inner = std::max(0, radius - strokeWidth);
  edge = inside && (inner == 0 || dist > int64_t(inner) * inner);
}
// Maps a scrolling text layer's viewport-local x to its position within the
// full expanded text, or returns false when that position falls in the gap
// (loop mode) between repeated copies.
static bool scrollContentColumn(const Layer& l, const LayerState& s, int textWidth, int local, int& contentX) {
  if (!l.scroll.enabled) {
    contentX = local;
    return true;
  }
  if (l.scroll.mode == ScrollMode::Loop) {
    const int cycle = textWidth + l.scroll.gap;
    if (cycle <= 0) return false;
    contentX = (local + s.scrollOffset) % cycle;
    return contentX < textWidth;
  }
  contentX = local + s.scrollOffset;
  return contentX < textWidth;
}
// Draws the part `r` (one row) of a text layer in a package font into
// `pixels`, which starts at r.x. Every glyph's ink lies within its advance
// (the package check makes sure), so walking the advances from the left
// finds the one glyph under each pixel; only that glyph's row is read, into
// `coverage`, and blended with the layer's colour over what is below.
// Out of line, and the coverage row is its own: it is not on the image path
// (render() -> Package::blendRow()), the deepest one, so it costs no stack there.
__attribute__((noinline)) static bool fontRow(const Theme& theme, const Layer& l, const LayerState& s, Rect r,
                                              AssetHandle& handle, Assets& assets, uint16_t* pixels) {
  uint8_t coverage[MaxGlyphWidth];
  if (l.font > theme.fonts.size()) return false;
  const ThemeFont& font = theme.fonts[l.font - 1];
  if (handle == InvalidAsset) handle = assets.resolve("fonts/" + font.name + ".stf");
  if (handle == InvalidAsset) return false;
  const int width = textPixelWidth(theme, l, s.resolved.size, s.text), line = r.y - s.bounds.y;
  size_t index = 0, loaded = SIZE_MAX;
  int pen = 0, left = 0, inked = 0;
  for (int x = r.x; x < r.x + r.w; ++x) {
    int contentX = 0;
    if (!scrollContentColumn(l, s, width, x - s.bounds.x, contentX)) continue;
    if (contentX < pen) {   // the next copy of a looping scroll
      index = 0;
      pen = 0;
    }
    while (index < s.text.size() && contentX >= pen + font.advance(s.text[index])) pen += font.advance(s.text[index++]);
    if (index == s.text.size()) continue;
    if (index != loaded) {
      if (!assets.glyphRow(handle, font.glyph(s.text[index]), line, coverage, left, inked)) return false;
      loaded = index;
    }
    const int column = contentX - pen - left;
    if (column >= 0 && column < inked) pixels[x - r.x] = blend(pixels[x - r.x], s.resolved.color, coverage[column]);
  }
  return true;
}
// Rebuilds every dirty row from scratch, in layer order back to front, and
// streams each finished row straight to `display` -- there is no persistent
// 240x240 framebuffer anywhere in this path. `handles` caches one resolved
// AssetHandle per layer for the whole call, so a layer touched by several
// disjoint dirty regions (or several rows of the same region) still resolves
// its asset exactly once.
bool render(const Engine& e, const std::vector<Rect>& dirty, Assets& assets, Display& display) {
  uint16_t pixels[240];
  if (e.theme().layers.size() > MaxLayers || e.states().size() != e.theme().layers.size()) return false;
  AssetHandle handles[MaxLayers];
  std::fill(handles, handles + MaxLayers, InvalidAsset);
  for (Rect region : dirty) {
    region = intersect(region, Rect(0, 0, 240, 240));
    for (int y = region.y; y < region.y + region.h; ++y) {
      std::fill(pixels, pixels + region.w, e.theme().background);
      for (size_t i = 0; i < e.theme().layers.size(); ++i) {
        const auto& l = e.theme().layers[i];
        const auto& s = e.states()[i];
        Rect r = intersect(s.bounds, Rect(region.x, y, region.w, 1));
        if (r.empty()) continue;
        if (l.type == LayerType::Image || l.type == LayerType::Animation) {
          // Assets stream by row too: only the `r.w` pixels this dirty region
          // actually needs are ever read, never a whole image or frame.
          if (handles[i] == InvalidAsset) handles[i] = assets.resolve(assetPath(l, s.frame));
          // The asset blends the row over `pixels` itself: no colour and alpha
          // rows here (720 bytes of the 4 KB loop stack, TRAPS T12).
          if (handles[i] == InvalidAsset || !assets.blendRow(handles[i], y - s.resolved.y, r.x - s.resolved.x, r.w, pixels + (r.x - region.x))) return false;
          continue;
        }
        if (l.type == LayerType::Text && l.font) {
          if (!fontRow(e.theme(), l, s, r, handles[i], assets, pixels + (r.x - region.x))) return false;
          continue;
        }
        // The rotation shapes' per-row setup: one sine for a whole row of
        // pixels instead of one per pixel.
        int tickIndex = -1;
        ArcWedge wedge{};
        Segment hand{}, tick{};
        if (l.type == LayerType::Shape && l.shape == Shape::Arc) wedge = arcWedge(s.resolved);
        if (l.type == LayerType::Shape && l.shape == Shape::Hand) hand = handSegment(l, s.resolved);
        for (int x = r.x; x < r.x + r.w; ++x) {
          uint16_t c = 0;
          bool draw = false;
          if (l.type == LayerType::Text) {
            // `local` is the pixel's position within the viewport (s.bounds);
            // scrollContentColumn maps it to `contentX`, its position within
            // the full expanded text, or reports "no ink here" for a loop
            // mode's inter-copy gap. From there it is an ordinary glyph-cell
            // lookup: which character (charIndex), which column within its
            // 6-wide cell (col, with column 5 always blank as spacing), and
            // which row within the glyph (scaled from pixel row to the font's
            // fixed 8-row bitmap by `resolved.size`).
            const int cell = (s.resolved.size * 6 + 7) / 8, local = x - s.bounds.x;
            int contentX = 0;
            if (scrollContentColumn(l, s, textPixelWidth(e.theme(), l, s.resolved.size, s.text), local, contentX)) {
              const int col = (contentX % cell) * 6 / cell, charIndex = contentX / cell;
              const int row = (y - s.bounds.y) * 8 / s.resolved.size;
              draw = col < 5 && charIndex < static_cast<int>(s.text.size()) &&
                (display.glyphColumn(s.text[charIndex], col) & (1 << row));
              c = s.resolved.color;
            }
          } else if (rotationShape(l.shape)) {
            const int dx = x - s.resolved.x, dy = y - s.resolved.y, dist = dx * dx + dy * dy;
            if (l.shape == Shape::Arc) {
              const int outer = s.resolved.radius, inner = std::max(0, outer - s.resolved.strokeWidth);
              draw = s.resolved.y2 != 0 && dist <= outer * outer && (inner == 0 || dist > inner * inner) &&
                inWedge(wedge, dx, dy);
            } else if (l.shape == Shape::Hand) {
              draw = linePixel(hand.x0, hand.y0, hand.x1, hand.y1, s.resolved.strokeWidth, x, y);
            } else {
              // Only the tick nearest in angle can reach this pixel: the
              // distance to a radial line grows with the angle away from it.
              const int pad = (s.resolved.strokeWidth + 1) / 2 + 1, near = std::max(0, s.resolved.radius - l.y2 - pad),
                far = s.resolved.radius + pad;
              if (dist <= far * far && dist >= near * near) {
                const int k = ((pixelAngle16(dx, dy) * l.x2 + Turn16 / 2) / Turn16) % l.x2;
                if (k != tickIndex) {
                  tickIndex = k;
                  tick = tickSegment(l, s.resolved, k);
                }
                draw = linePixel(tick.x0, tick.y0, tick.x1, tick.y1, s.resolved.strokeWidth, x, y);
              }
            }
            c = s.resolved.stroke;
          } else {
            bool inside = false, edge = false;
            if (l.shape == Shape::Rectangle) {
              roundedRectHit(x, y, s.resolved.x, s.resolved.y, s.resolved.width, s.resolved.height,
                              s.resolved.cornerRadius, s.resolved.strokeWidth, inside, edge);
            } else if (l.shape == Shape::Circle) {
              int dx = x - s.resolved.x, dy = y - s.resolved.y, dist = dx * dx + dy * dy, inner = std::max(0, s.resolved.radius - s.resolved.strokeWidth);
              inside = dist <= s.resolved.radius * s.resolved.radius;
              edge = inside && (inner == 0 || dist > inner * inner);
            } else {
              inside = edge = linePixel(s.resolved.x, s.resolved.y, s.resolved.x2, s.resolved.y2, s.resolved.strokeWidth, x, y);
            }
            // Fill first, stroke second and unconditionally overwriting: a
            // pixel that is both `inside` and `edge` (the stroke band) ends
            // up stroke-colored, exactly as if fill were painted first and
            // the stroke ring painted on top of it.
            if (inside && l.hasFill) {
              draw = true;
              c = s.resolved.fill;
            }
            if (edge && l.hasStroke) {
              draw = true;
              c = s.resolved.stroke;
            }
          }
          if (draw) pixels[x - region.x] = c;
        }
      }
      display.row(region.x, y, pixels, region.w);
    }
  }
  return true;
}
}
#endif
