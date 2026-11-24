---
title: Clean Image lokal bauen
description: Die minimale SmallTV-Ultra-Firmware mit PlatformIO kompilieren.
---

## Voraussetzungen

- PlatformIO Core oder die PlatformIO-Erweiterung für Visual Studio Code
- Python 3 für die eingebettete Weboberfläche und Prüfwerkzeuge
- Internetzugang beim ersten Build, damit PlatformIO Framework und Bibliotheken laden kann

## Clean Image

Im Repository-Ordner ausführen:

```sh
python tools/gzip_webui_settings.py --check
pio run -e smalltv_clean
```

Das Image wird unter `.pio/build/smalltv_clean/firmware.bin` erzeugt.

Das Clean Image enthält Display, WLAN-/Captive-Portal-Einrichtung, lokale Weboberfläche, NTP-Uhr, installierbare einzelne Themes, OTA-Wartung und Crash-Recovery. Foto-Bibliothek/JPEG-Decoding, Theme-Playlist und Zeitpläne, Ticker, Usage-Monitor, Radar, Home Assistant/MQTT, Benachrichtigungs-Overlay und das separate Loader-Image sind nicht Bestandteil des Builds.

## Vorhandene Prüfungen

GitHub Actions baut ausschließlich `smalltv_clean`, prüft die komprimierte Weboberfläche, Theme-Pakete und Parser sowie den verfügbaren Platz im OTA-App-Slot. Lokal können die gleichen relevanten Schritte ausgeführt werden:

```sh
python tools/gzip_webui_settings.py --check
pio run -e smalltv_clean
python tools/ci_checks.py size --envs smalltv_clean --min-free 8192
python tools/ci_checks.py assets
```

Die Tests unter `tests/` benötigen je nach Skript einen C++-Compiler und unter Linux teils 32-Bit-Unterstützung. Sie decken Hostcode ab, simulieren aber kein vollständiges ESP8266-Gerät.

## Veröffentlichungsdatei

Der Firmware-Workflow legt `smalltv-clean-firmware.bin` als Build-Artefakt ab. Ein Git-Tag im Format `vX.Y.Z` löst zusätzlich die Release-Erstellung aus. Der Asset-Name entspricht dem Selbstupdate-Eintrag `UPDATE_ASSET` in `src/config.h`.
