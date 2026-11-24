# SmallTV Ultra – Web-only Clean Image

Minimal firmware for the GeekMagic SmallTV / SmallTV Ultra (ESP8266).

**There is no on-screen interface.** The display and backlight are not initialised; the backlight is held off. All operation is performed through the local web interface.

## Included

- Wi-Fi client and stored network credentials
- Setup access point when no saved network is configured or a connection fails
- Captive-portal redirects and mDNS hostname
- Small local web interface
- Configuration storage in LittleFS
- Wi-Fi scanning and network configuration
- Configuration export/import
- Factory reset and restart
- Manual firmware upload and GitHub online OTA update
- Network/status diagnostics and OTA recovery after a failed boot

## Removed

- Theme engine, .stheme format, themes, package manager and theme APIs
- Clock, NTP and night mode
- Display rendering, ST7789/GFX library and display settings
- Photo library, JPEG decoding and slideshow
- Ticker, usage monitor, aircraft radar, MQTT/Home Assistant and notifications
- Digest login and web password settings
- Loader image and theme build tools

## Setup

1. Flash the firmware to the ESP8266 SmallTV.
2. Connect a phone or computer to the SmallTV-Setup Wi-Fi access point.
3. Open http://192.168.4.1.
4. Enter the home Wi-Fi details and save. The device restarts and the web interface becomes available at its assigned IP address or mDNS hostname.

The web interface has **no login or password protection**. Use it only on a trusted local network. Configuration exports contain Wi-Fi credentials and should be handled as secrets.

The display remains dark by design. This is a network and web-management image, not a display firmware.

## Build

Install Python 3 and PlatformIO Core, then run:

~~~sh
pio run -e smalltv_clean
~~~

Output:

~~~text
.pio/build/smalltv_clean/firmware.bin
~~~

Only the web-only firmware is compiled. There is no loader, display renderer, clock service, theme engine or optional mode registry.

The existing project LICENSE and NOTICE.md files remain in place.
