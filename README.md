# SmallTV Ultra – Web Management + Display Status

Minimal firmware for the ESP8266-based GeekMagic SmallTV / SmallTV Ultra.

The ST7789 display is initialized at boot and shows startup/network status
until a valid `.stheme` package is selected. The theme engine then renders the
selected package through row-based output without a full-screen framebuffer.

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
- Persistent manual brightness, panel rotation, RGB/BGR order, inversion and per-channel color gains
- `.stheme` package storage, validation, selection and row-based rendering

## Not included yet

- Clock, NTP, language/locale scheduling or night mode
- Theme data sources and network fetchers
- Photos, JPEG decoding or slideshow
- Physical button support
- Web login or access-password system
- ESP32-C2 / classic ESP32 / SmallTV Pro board targets
- Separate loader image

## .stheme package management

The **Themes** tab accepts one `.stheme` package per upload. Each package is
validated before it is published to LittleFS, including its STH1 container,
manifest, paths, image/font assets and declared layer references. Up to 16
packages can be stored. A selected package is saved and restored at boot.
Deactivate a running theme before installing or deleting packages so package
validation does not compete with the renderer for ESP8266 heap.

The renderer supports theme layers, images, fonts, animation frames and
scrolling. This firmware does not start NTP/time services or fetch external
theme data; theme elements depending on clock or remote values may therefore
show placeholders. The parser and package format are extracted from
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
