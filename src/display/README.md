# Physical display layer

The hardware-facing display code is isolated from the web-only firmware.

## Modules

- `DisplayBoard.h`: ESP8266/SmallTV pin map, ST7789 geometry and board defaults.
- `DisplayBacklight.*`: ESP8266 sigma-delta backlight driver and pure level planning.
- `DisplayPanel.*`: ST7789/SPI adapter implementing the theme renderer's
  scanline `smalltv::Display` interface, plus rotation and RGB/BGR/inversion/
  per-channel gain controls.
- `DisplayText.*`: opaque 6x8 text layout/rendering over the same row sink;
  it has no dependency on SPI or the panel library.

The panel adapter uses SPI mode 3, the SmallTV-specific ST7789 rotation map,
and the 80-row RAM offset for 180-degree rotations. Pixels are colour-corrected
once at the output boundary. Rows are written as one address window and one
streamed pixel run rather than as individual pixel writes.

## Safety and build boundaries

`smalltv_clean` does not compile these files and does not link the display
library. The optional `smalltv_display_check` environment compiles the panel,
backlight, text output and theme renderer together. It never calls
`DisplayPanel::begin()`, has no upload step and is not a flash target.

The default state for an explicitly initialized panel is a dark backlight.
The adapter is not constructed or initialized by the current firmware entry
point; a future display integration must deliberately call `begin()` and
set the desired brightness.
