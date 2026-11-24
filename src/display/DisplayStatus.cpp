#include "DisplayStatus.h"
#include "DisplayText.h"
#include <stdio.h>

namespace {
constexpr uint16_t Black = 0x0000;
constexpr uint16_t White = 0xffff;
constexpr uint16_t Gray = 0x8410;
constexpr uint16_t Green = 0x07e0;
constexpr uint16_t Red = 0xf800;
constexpr uint16_t Yellow = 0xffe0;

void line(smalltv::display::DisplayPanel& panel, const char* text, int y,
          uint16_t color, uint8_t maxScale = 2) {
  if (!panel.ready() || !text || !text[0]) return;
  const uint8_t scale = smalltv::display::fitTextScale(text, 224, maxScale);
  smalltv::display::drawCenteredText(panel, text, y, scale, color, Black);
}

void heading(smalltv::display::DisplayPanel& panel, const char* text, uint16_t color) {
  line(panel, text, 18, color, 2);
}
}  // namespace

namespace smalltv {
namespace display {
void showBoot(DisplayPanel& panel, const char* detail) {
  if (!panel.ready()) return;
  panel.clear(Black);
  line(panel, "SMALLTV", 70, White, 3);
  line(panel, "WEB CONTROL", 110, Gray, 1);
  line(panel, detail ? detail : "starting", 158, Green, 2);
}

void showAccessPoint(DisplayPanel& panel, const char* ssid, const char* ip, const char* reason) {
  if (!panel.ready()) return;
  panel.clear(Black);
  heading(panel, "SETUP MODE", Yellow);
  line(panel, "Connect to WiFi", 55, Gray, 1);
  line(panel, ssid && ssid[0] ? ssid : "SmallTV-Setup", 78, White, 2);
  line(panel, "Open in browser", 124, Gray, 1);
  line(panel, ip && ip[0] ? ip : "192.168.4.1", 148, Green, 2);
  if (reason && reason[0]) line(panel, reason, 196, Red, 1);
}

void showConnected(DisplayPanel& panel, const char* ssid, const char* ip, const char* hostname) {
  if (!panel.ready()) return;
  panel.clear(Black);
  heading(panel, "CONNECTED", Green);
  line(panel, "WiFi network", 57, Gray, 1);
  line(panel, ssid && ssid[0] ? ssid : "-", 78, White, 2);
  line(panel, "Web interface", 124, Gray, 1);
  line(panel, ip && ip[0] ? ip : "-", 146, Green, 2);
  if (hostname && hostname[0]) {
    char address[64];
    snprintf(address, sizeof(address), "%s.local", hostname);
    line(panel, address, 190, Gray, 1);
  }
}

void showSafeMode(DisplayPanel& panel, const char* reason, const char* ip) {
  if (!panel.ready()) return;
  panel.clear(Black);
  heading(panel, "SAFE MODE", Red);
  line(panel, "Last reset:", 60, Gray, 1);
  line(panel, reason && reason[0] ? reason : "Unknown", 82, White, 1);
  line(panel, "Web / OTA recovery", 136, Gray, 1);
  line(panel, ip && ip[0] ? ip : "-", 160, Green, 2);
}

void showUpdating(DisplayPanel& panel, const char* detail) {
  if (!panel.ready()) return;
  panel.clear(Black);
  line(panel, "SMALLTV", 75, White, 3);
  line(panel, "UPDATING", 125, Yellow, 2);
  line(panel, detail ? detail : "", 174, Gray, 1);
}
}  // namespace display
}  // namespace smalltv
