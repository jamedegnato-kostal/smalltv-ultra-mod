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

The firmware starts the generic JSON data-source fetcher only for a selected
theme that declares sources. It services one source at a time after display
rendering, honors each source interval, and retries failures with backoff. The
reply is parsed directly from the network stream through an ArduinoJson filter;
only the declared field values are retained. There is no raw-body cache and no
flash write path for fetched data. NTP remains separately gated by clock use.


## Theme clock

The selected theme is scanned for built-in clock tokens and `time.*` bindings.
Only when one is present does the ESP8266 start SNTP. The engine receives a
local `tm` value after the first successful SNTP synchronization; until then
clock-dependent text remains in its engine fallback state. The POSIX timezone
rule is applied by the ESP8266 time core, including the configured DST
transition dates. Deactivating the theme stops SNTP polling.
