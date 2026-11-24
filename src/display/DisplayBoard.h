#pragma once
// Board facts for the original GeekMagic SmallTV Ultra display.
// Kept separate from the web-only configuration so the default firmware
// does not gain GPIO or panel dependencies.
#include <Arduino.h>

namespace smalltv {
namespace display {
constexpr int PanelWidth = 240;
constexpr int PanelHeight = 240;

// ESP8266 hardware SPI uses fixed SCLK/MOSI pins: GPIO14 / GPIO13.
constexpr uint8_t PinSclk = 14;  // D5
constexpr uint8_t PinMosi = 13;  // D7
constexpr uint8_t PinDc = 0;     // D3
constexpr uint8_t PinReset = 2;  // D4
constexpr int8_t PinChipSelect = 15;  // D8
constexpr uint8_t PinBacklight = 5;   // D1

// Original 1.54-inch ST7789 IPS panel. Rotation 2/3 uses the unused
// 80-row controller RAM band as an offset for the 240x240 glass.
constexpr bool DefaultBgr = false;
constexpr uint16_t ColumnOffset1 = 0;
constexpr uint16_t RowOffset1 = 0;
constexpr uint16_t ColumnOffset2 = 0;
constexpr uint16_t RowOffset2 = 80;
constexpr bool BacklightActiveLow = true;
}  // namespace display
}  // namespace smalltv
