# SmallTV Ultra – Web-only Clean Image

Minimal firmware for the ESP8266-based GeekMagic SmallTV / SmallTV Ultra.

**No display output, clock, theme rendering, images or web login.** The display backlight is held off. The web interface can manage `.stheme` packages (upload, validate, list and delete), but does not select or display them.

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
- `.stheme` package storage, manifest/container validation, asset validation, listing and deletion

## Not included

- Display initialization, graphics or backlight control beyond keeping it off
- Clock, NTP or night mode
- Theme selection, display rendering, theme playlists and theme data-source fetching
- Photos, JPEG decoding or slideshow
- Ticker, usage monitor, radar, MQTT/Home Assistant or notifications
- Any web login or access-password system
- Separate loader image

## .stheme package management

The **Themes** tab accepts one `.stheme` package per upload. Each package is validated before it is published to LittleFS, including its STH1 container, manifest, paths, image/font assets and declared layer references. Up to 16 packages are listed; corrupt packages remain visible with an error and can be removed.

This profile deliberately has no theme selection or renderer. The display remains dark. Package management does not start clock, NTP, data-source fetching or image output. The parser and package format are extracted from [giovi321/smalltv-mod v3.13.0](https://github.com/giovi321/smalltv-mod/releases/tag/v3.13.0); see `src/theme/UPSTREAM.md`.

## First start

1. Flash the firmware to the SmallTV ESP8266.
2. Connect to the open `SmallTV-Setup` Wi-Fi access point.
3. Open `http://192.168.4.1`.
4. Enter the home Wi-Fi credentials and save.
5. Reopen the web interface at the device IP address or its mDNS hostname.

The setup access point and web interface are open without a password. Use the device only on a trusted local network. Configuration backups contain Wi-Fi credentials and should be treated as secrets.

## Build

Install Python 3 and PlatformIO Core. From the repository root run:

```sh
pio run -e smalltv_clean
```

Firmware output:

```text
.pio/build/smalltv_clean/firmware.bin
```

GitHub Actions builds this target and publishes the file as `smalltv-clean-firmware.bin` for tags named `clean-v*`.

See `NOTICE.md` and `LICENSE` for project origin and license information.
