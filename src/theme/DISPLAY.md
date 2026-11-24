# Display subsystem structure

This branch is intentionally keeping the physical display inactive. The display
code is being separated internally so it can be reviewed and tested before any
firmware is flashed.

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

The `smalltv_clean` environment still defines
`SMALLTV_THEME_HEADLESS=1`. Therefore `ThemeDisplay.cpp` contributes no
renderer code to the current firmware build, and the display remains inactive.
No display initialization, backlight control, rendering loop, or device flash
is enabled by this change.

A future hardware integration should implement a concrete `Display` adapter
for the actual SmallTV Ultra panel and separately decide when to initialize it.
