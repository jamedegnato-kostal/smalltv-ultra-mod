#pragma once
#include <stdint.h>
#include <string>
#include "../display/DisplayPanel.h"

namespace smalltv {
void themeRuntimeBegin(display::DisplayPanel& display);
bool themeRuntimeSelect(const char* id, const char* timezone, const char* language, std::string& error);
void themeRuntimeSetLanguage(const char* language);
uint8_t themeRuntimeDataSourceCount();
uint8_t themeRuntimeDataReadyCount();
uint8_t themeRuntimeDataStaleCount();
uint8_t themeRuntimeDataFailureCount();
const char* themeRuntimeDataError();
void themeRuntimeStop();
bool themeRuntimeLoop();
void themeRuntimeInvalidate();
bool themeRuntimeActive();
const char* themeRuntimeActiveId();
const char* themeRuntimeError();
uint32_t themeRuntimeFreeHeap();
}
