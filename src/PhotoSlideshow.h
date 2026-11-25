#pragma once
#include "Platform.h"
#include "display/DisplayPanel.h"

namespace smalltv {
namespace photos {
void begin(display::DisplayPanel& display);
void webBegin(WebServerClass& server);
void loop();
void stop();
bool active();
uint8_t count();
void redraw();
bool showByName(const char* name);
}  // namespace photos
}  // namespace smalltv
