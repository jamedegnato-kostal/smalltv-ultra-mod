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
- NTP time for themes with clock tokens or `time.*` bindings
- Selectable POSIX timezones with automatic daylight-saving transitions
- Persistent German/English theme language selection for localized spec 2 text
- Bounded JSON data-source fetching with streaming extraction and an in-memory field cache
- Open-Meteo weather demo theme with current conditions and a one-day forecast

## Not included yet

- Language/locale scheduling or night mode
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
scrolling. NTP starts only when a selected theme contains clock text tokens
(e.g. `{HH}`) or `time.*` bindings; it stops when that theme is deactivated.
The timezone is configurable in the Display settings, including Europe/Berlin
with automatic CET/CEST changes. The Display settings select German or English
for localized theme text and month/weekday names; missing translations fall back
to English. Generic JSON sources declared by a theme are fetched one at a time
with a 2 KiB response cap, a 2 KiB JSON heap budget and short timeouts with retry
backoff. Only declared fields (up to 32 total, the pre-existing format maximum) plus spec-2 source ages are cached in RAM; no raw response
body is buffered or written to flash. HTTPS requires the theme's explicit
`insecureTls` opt-in because ESP8266 certificate validation is not configured.
Open-Meteo is the first provider example; it uses the generic fetcher and maps
WMO weather codes to localized labels with theme `show` rules, not extra firmware
lookup tables. It requests only five weather values; the `.stheme` demo package
is stored separately in LittleFS, not linked into the firmware image. Change the
example Berlin coordinates in `examples/open-meteo/theme.json` for the
installation. The parser
and package format are extracted from
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

## Flash layout and migration

The firmware target is configured for an ESP-12E module with **4 MB flash**:
`eagle.flash.4m2m.ld` reserves a 2 MB LittleFS partition while retaining an
OTA-capable firmware area of approximately 1 MB. LittleFS metadata means the
entire nominal 2 MB is not available for package data. Keep the firmware image
below the OTA slot limit.

Switching from the previous `4m1m` layout changes the LittleFS partition
boundary. Files in the old filesystem are not migrated automatically. Before
updating, export the settings configuration and keep separate copies of the
`.stheme` packages. The firmware transition can also be installed with the existing OTA
updater; serial flashing is only a fallback if OTA is unavailable. The old
LittleFS contents are not migrated automatically and may no longer mount with
the new partition boundary. If the device reports that LittleFS is unavailable
after boot, open the web interface and use the confirmed **Factory Reset**
action to initialize the new filesystem. This formats LittleFS and erases its
stored settings and themes; reconnect to the setup AP if necessary, then import
the settings backup and upload the theme packages again. This layout must not
be used on modules with less than 4 MB flash.

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

Use `python tools/pack_stheme.py examples/open-meteo/theme.json --output examples/open-meteo/open-meteo.stheme`
to rebuild the manifest-only demo package after changing its location or text.

GitHub Actions also verifies that the checked-in demo package matches its
manifest. It builds the standard target and publishes the file as
`smalltv-clean-firmware.bin` for tags named `clean-v*`. The display-check
target is compile-only; it does not upload or flash anything.

See `NOTICE.md` and `LICENSE` for project origin and license information.
