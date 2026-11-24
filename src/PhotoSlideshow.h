#pragma once
#include "Platform.h"
#include "display/DisplayPanel.h"

namespace smalltv {
namespace photos {
void begin(display::DisplayPanel& display);
void webBegin(WebServerClass& server);
void loop();
bool active();
uint8_t count();
void redraw();
}  // namespace photos
}  // namespace smalltv
