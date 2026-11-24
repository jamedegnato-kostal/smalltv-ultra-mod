# SmallTV Ultra – Web-only Clean Image

Minimal firmware for the ESP8266-based GeekMagic SmallTV / SmallTV Ultra.

**No display output, clock, themes, images or web login.** The display backlight is held off. The device is operated through its local web interface.

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

## Not included

- Display initialization, graphics or backlight control beyond keeping it off
- Clock, NTP or night mode
- Theme engine, .stheme files, theme upload or playlists
- Photos, JPEG decoding or slideshow
- Ticker, usage monitor, radar, MQTT/Home Assistant or notifications
- Web password login
- Separate loader image

## First start

1. Flash the firmware to the SmallTV ESP8266.
2. Connect to the \`SmallTV-Setup\` Wi-Fi access point.
3. Open \`http://192.168.4.1\`.
4. Enter the home Wi-Fi credentials and save.
5. Reopen the web interface at the device IP address or its mDNS hostname.

The web interface is intentionally open without login. Use it only on a trusted local network. Configuration backups contain Wi-Fi credentials and should be treated as secrets.

## Build

Install Python 3 and PlatformIO Core. From the repository root run:

\`\`\`sh
pio run -e smalltv_clean
\`\`\`

Firmware output:

\`\`\`text
.pio/build/smalltv_clean/firmware.bin
\`\`\`

GitHub Actions builds this target and publishes the file as \`smalltv-clean-firmware.bin\` for tags named \`clean-v*\`.

See \`NOTICE.md\` and \`LICENSE\` for project origin and license information.
