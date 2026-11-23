---
title: Firmware lokal bauen
description: Die SmallTV Settings Firmware mit PlatformIO kompilieren und das Ergebnis finden.
---

## Voraussetzungen

- Python 3
- PlatformIO Core (Installation mit `python -m pip install platformio`)
- Internetzugang beim ersten Build, damit PlatformIO Framework und Bibliotheken laden kann

## Hauptfirmware

Im Repository-Ordner ausführen:

```sh
python tools/gzip_webui_settings.py --check
python -m platformio run -e smalltv_settings
```

Das Image wird unter `.pio/build/smalltv_settings/firmware.bin` erzeugt. Es enthält die Settings-Weboberfläche, Theme- und Playlist-Funktionen sowie die Foto-Bibliothek.

Der optionale Loader ist ein separates, minimales Image:

```sh
python -m platformio run -e smalltv_loader
```

Er ist nur für Installationswege gedacht, bei denen das bisherige Ultra-Updateverfahren das Hauptimage nicht annimmt. Er ersetzt die Hauptfirmware nicht.

## Weboberfläche bearbeiten

Die HTML-Quelle liegt in `src/webui_settings.html`. Der Firmware-Build verwendet den daraus generierten Header `src/webui_settings.h`. Nach Änderungen:

```sh
python tools/gzip_webui_settings.py
python tools/gzip_webui_settings.py --check
```

Den generierten Header nicht manuell bearbeiten.

## Vorhandene Prüfungen

Die GitHub Actions bauen die Settings-Firmware und prüfen unter anderem, dass die komprimierte Weboberfläche mit ihrer HTML-Quelle übereinstimmt, Theme-Pakete gültig sind und genügend Platz im App-Slot bleibt. Die Hosttests unter `tests/` decken Parser, Speicher, Rendering, Statusdaten und WebUI-Code ab.

Die Tests benötigen je nach Skript einen C++-Compiler und unter Linux teils 32-Bit-Unterstützung. Sie simulieren kein vollständiges ESP8266-Gerät.

## Dokumentation bauen

Die Starlight-Dokumentation liegt unter `docs/` und wird mit Node.js 22 gebaut:

```sh
cd docs
npm ci
npm run build
```

## Veröffentlichungsdateien

Der Firmware-Workflow stellt das Hauptimage und Beispiel-Themes als separate Dateien bereit: `smalltv-settings-firmware.bin`, `dynamische-uhr.stheme` und `wetter-bellmerei.stheme`. Ein Git-Tag im Format `vX.Y.Z` veröffentlicht dieselben Dateien als einzelne Release-Assets. Der Name des Firmware-Assets wird über `FIRMWARE_ASSET` in `src/config.h` geprüft. Das Gerät lädt Releases nicht selbst herunter; Firmware wird lokal über die Weboberfläche manuell eingespielt.
