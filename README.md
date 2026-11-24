# SmallTV Ultra – Web Management + Display Status

Minimal firmware for the ESP8266-based GeekMagic SmallTV / SmallTV Ultra.

The ST7789 display is initialized at boot and shows startup, setup-AP,
connected and safe-mode recovery information. No theme or content renderer is
activated by the application. The web interface manages `.stheme` packages
(upload, validate, list and delete), but does not select or display them.

## Included

- Wi-Fi client with saved network credentials
- Setup access point and captive-portal redirect
- mDNS hostname
- Local web interface
- LittleFS configuration storage
- Wi-Fi network scan and configuration
- Configuration export/import
- Restart and factory reset
- Manual firmware upload
- GitHub release check and OTA update
- Status and memory diagnostics
- ST7789 panel initialization and boot/network status screens
- SPI mode 3, panel rotation/RGB-order boundary and RGB565 row output
- Backlight initialization with upstream default brightness (90%)
- `.stheme` package storage, manifest/container validation, asset validation,
  listing and deletion

## Not included yet

- Theme selection, theme runtime scheduling and theme rendering
- Clock, NTP, language/locale scheduling or night mode
- Theme data sources and network fetchers
- Photos, JPEG decoding or slideshow
- Ticker, usage monitor, radar, rain radar, MQTT/Home Assistant or notifications
- Physical button support
- Display settings in the web UI (brightness, rotation, color correction)
- Web login or access-password system
- ESP32-C2 / classic ESP32 / SmallTV Pro board targets
- Separate loader image

## .stheme package management

The **Themes** tab accepts one `.stheme` package per upload. Each package is
validated before it is published to LittleFS, including its STH1 container,
manifest, paths, image/font assets and declared layer references. Up to 16
packages are listed; corrupt packages remain visible with an error and can be
removed.

Package management does not select a theme, start a clock, fetch theme data or
render package content. The parser and package format are extracted from
[giovi321/smalltv-mod v3.13.0](https://github.com/giovi321/smalltv-mod/releases/tag/v3.13.0);
see `src/theme/UPSTREAM.md`.

## First start

1. Flash the firmware to the SmallTV ESP8266.
2. On boot, the display shows startup status and then the Wi-Fi/setup information.
3. When no saved network is available, connect to the open `SmallTV-Setup` AP.
4. Open `http://192.168.4.1`.
5. Enter the home Wi-Fi credentials and save.
6. Reopen the web interface at the device IP address or its mDNS hostname.

The setup access point and web interface are open without a password. Use the
device only on a trusted local network. Configuration backups contain Wi-Fi
credentials and should be treated as secrets.

## Build

Install Python 3 and PlatformIO Core. From the repository root run:

```sh
pio run -e smalltv_clean
```

The separate non-headless renderer check can be built with:

```sh
pio run -e smalltv_display_check
```

Firmware output:

```text
.pio/build/smalltv_clean/firmware.bin
```

GitHub Actions builds the standard target and publishes the file as
`smalltv-clean-firmware.bin` for tags named `clean-v*`. The display-check
target is compile-only; it does not upload or flash anything.

See `NOTICE.md` and `LICENSE` for project origin and license information.
