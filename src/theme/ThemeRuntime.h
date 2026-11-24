#pragma once
#include <stdint.h>
#include <string>
#include "../display/DisplayPanel.h"

namespace smalltv {
void themeRuntimeBegin(display::DisplayPanel& display);
bool themeRuntimeSelect(const char* id, std::string& error);
void themeRuntimeStop();
bool themeRuntimeLoop();
void themeRuntimeInvalidate();
bool themeRuntimeActive();
const char* themeRuntimeActiveId();
const char* themeRuntimeError();
uint32_t themeRuntimeFreeHeap();
}
