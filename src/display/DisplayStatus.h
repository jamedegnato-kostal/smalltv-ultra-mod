#pragma once
#include "DisplayPanel.h"

namespace smalltv {
namespace display {
void showBoot(DisplayPanel& panel, const char* detail);
void showAccessPoint(DisplayPanel& panel, const char* ssid, const char* ip, const char* reason);
void showConnected(DisplayPanel& panel, const char* ssid, const char* ip, const char* hostname);
void showSafeMode(DisplayPanel& panel, const char* reason, const char* ip);
void showUpdating(DisplayPanel& panel, const char* detail);
}  // namespace display
}  // namespace smalltv
