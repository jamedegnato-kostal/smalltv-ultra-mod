# Third-party notices

## Theme package and parser components

The theme manifest parser, package/container reader, asset validator, bounded
collection and streaming catalogue helpers in `src/theme/` are derived from
[giovi321/smalltv-mod](https://github.com/giovi321/smalltv-mod), release
[v3.13.0](https://github.com/giovi321/smalltv-mod/releases/tag/v3.13.0),
tag commit `5396e3f80ce868e0fca2f7bbc032df16e66daa7a`.

The extracted components include the theme engine/parser, STH1 package
validation and asset access, theme catalogue, block list, budget allocator,
language helpers and streaming JSON utilities. The upstream source and
component-level license are documented in
[`src/theme/UPSTREAM.md`](src/theme/UPSTREAM.md); the upstream WTFPL 2.0
license text is retained in `src/theme/LICENSE-WTFPL-2.0.txt`.

The ESP8266 runtime integration, display adapter, web portal, photo slideshow,
mixed-playlist controller, settings, networking and OTA integration are
maintained in this repository.

## Build dependencies

The firmware uses the libraries declared in `platformio.ini`, including
ArduinoJson, TJpg_Decoder and GFX Library for Arduino. Wi-Fi, DNS, mDNS,
HTTP server, filesystem, updater and core runtime facilities are provided by
the ESP8266 Arduino platform.

Please consult each dependency's upstream project for its license and notices.
The repository's `LICENSE` file applies as stated there; this notice is not a
replacement for the individual upstream licenses.
