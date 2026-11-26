# Physical display layer

The hardware-facing display code is isolated from the web portal and theme
management code.

## Modules

- `DisplayBoard.h`: ESP8266/SmallTV pin map, ST7789 geometry and board defaults.
- `DisplayBacklight.*`: ESP8266 sigma-delta backlight driver and brightness planning.
- `DisplayPanel.*`: ST7789/SPI adapter implementing the theme renderer's
  scanline `smalltv::Display` interface, including rotation and RGB/BGR,
  inversion and per-channel gain controls.
- `DisplayText.*`: opaque 6x8 text layout and rendering over the same row sink;
  it has no dependency on SPI or the panel library.
- `DisplayStatus.*`: boot, setup and network-status screens.

## Rendering and panel behavior

The panel adapter uses SPI mode 3, the SmallTV-specific ST7789 rotation map,
and the 80-row RAM offset for 180-degree rotations. Pixels are colour-corrected
once at the output boundary. Rows are written as one address window and one
streamed pixel run rather than as individual pixel writes.

The theme renderer writes rows directly to the display. The photo renderer uses
TJpg_Decoder output blocks. Neither path requires a persistent 240×240
framebuffer.

## Build boundaries

The standard `smalltv_stable` environment compiles and initializes the panel,
backlight, text output and theme renderer. The separate
`smalltv_display_check` environment is compile-only validation: it does not
flash the panel or upload firmware.

The backlight starts dark during initialization and is raised only after the
ST7789 controller reports a successful start. If panel initialization fails,
the web and network firmware continues in headless recovery mode.
