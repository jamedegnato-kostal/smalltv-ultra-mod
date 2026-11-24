# Display subsystem structure

The standard ESP8266 firmware initializes the ST7789 panel and can render one
selected `.stheme` package. Rendering uses dirty rectangles and RGB565 scanlines;
there is no persistent full-screen framebuffer.

## Boundaries

- **ThemeEngine.h / ThemeEngine.cpp** — theme model, manifest/data handling,
  layer state, animation/scroll timing, dirty-rectangle calculation. No
  display-driver dependency.
- **ThemeDisplay.h / ThemeDisplay.cpp** — row renderer and the output boundary.
  The renderer composes RGB565 rows in layer order and streams them through a
  `Display::row()` sink; it does not allocate a full framebuffer.
- **ThemeRenderInternal.h** — private shared geometry helpers used by both the
  dirty-region engine and pixel renderer.
- **ThemePackage.h / ThemePackage.cpp** — package validation and asset access.
  `Package` implements `Assets` only in a non-headless build. The headless
  package manager continues validating images and fonts without loading display
  caches or exposing a rendering interface.

## Current state

The standard `smalltv_clean` environment builds the non-headless package path
and `ThemeDisplay.cpp`. `ThemeRuntime` keeps one package file open, one parsed
engine and one asset index. Switching themes releases the prior engine before
loading another. Runtime activation checks heap and largest-block reserves
before and after loading; package installation is blocked while a theme is
active. Image/font bytes are read from LittleFS as needed.

This firmware supplies no clock or external data values: NTP and remote data
source clients remain outside this runtime step.
