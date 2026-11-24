#pragma once
// Private bridge shared by the theme state engine and its row renderer.
// Keep these declarations out of ThemeEngine.h: they are implementation
// details needed only where dirty-region geometry and pixel drawing meet.
#include "ThemeEngine.h"

namespace smalltv {
constexpr int Turn16 = 5760;
struct Segment {
  int x0, y0, x1, y1;
};
struct ArcWedge {
  int sx, sy, ex, ey;
  bool full, reflex;
};

int textPixelWidth(const Theme& theme, const Layer& layer, int size, const std::string& text);
int pixelAngle16(int dx, int dy);
Segment handSegment(const Layer& layer, const ResolvedLayer& resolved);
Segment tickSegment(const Layer& layer, const ResolvedLayer& resolved, int index);
bool rotationShape(Shape shape);
ArcWedge arcWedge(const ResolvedLayer& resolved);
bool inWedge(const ArcWedge& wedge, int dx, int dy);
}
