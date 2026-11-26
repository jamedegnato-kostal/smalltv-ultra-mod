# Contributing

Thanks for helping improve SmallTV Stable.

## Before opening a change

- Base firmware work on the current `main` branch.
- Keep `clean-image` untouched; it is retained as a separate reference branch.
- Keep ESP8266 heap usage bounded. Avoid full-screen framebuffers and large
  transient JSON documents.
- Preserve the existing LittleFS partition layout unless a migration is
  deliberately designed and documented.
- Keep the firmware, display-check and loader targets building.

## Local setup

Install Python 3, PlatformIO Core and a C++ compiler.

```sh
python -m pip install platformio
pio run -e smalltv_stable
pio run -e smalltv_display_check
pio run -e smalltv_loader
```

Run the host-side regression tests:

```sh
g++ -std=c++11 -Wall -Wextra -Werror tests/playback/test_playback_order.cpp -o /tmp/playback-order-tests
/tmp/playback-order-tests

g++ -std=c++11 -Wall -Wextra -Werror -Isrc tests/night_schedule/test_night_schedule.cpp -o /tmp/night-schedule-tests
/tmp/night-schedule-tests

python tests/factory_reset/check_factory_reset.py
python tools/pack_stheme.py examples/open-meteo/theme.json --check examples/open-meteo/open-meteo.stheme
```

GitHub Actions runs these checks and builds all three PlatformIO targets for
pushes to `main` and `stable-image`.

## Pull requests

Please include:

1. A short explanation of the user-visible behavior or bug.
2. The affected hardware and firmware environment, when relevant.
3. Test/build results and any remaining device-side validation needed.
4. Memory/flash impact for changes that add runtime or UI features.

Keep changes focused. Do not commit generated PlatformIO output, firmware
binaries, Wi-Fi credentials, exported configuration backups or personal API
keys.
