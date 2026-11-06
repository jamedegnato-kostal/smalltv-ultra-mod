---
title: Firmware lokal bauen
description: Die SmallTV Settings Firmware mit PlatformIO kompilieren und das Ergebnis finden.
---

## Voraussetzungen

- PlatformIO Core oder die PlatformIO-Erweiterung für Visual Studio Code
- Python 3 für die eingebettete Weboberfläche und Prüfwerkzeuge
- Internetzugang beim ersten Build, damit PlatformIO Pakete laden kann

## Hauptfirmware

Im Repository-Ordner ausführen:

```sh
pio run -e smalltv_settings
```

Das Image wird unter `.pio/build/smalltv_settings/firmware.bin` erzeugt. Es
enthält die Settings-Weboberfläche, Theme- und Playlist-Funktionen sowie die
Foto-Bibliothek.

Der optionale Loader ist ein separates, minimales Image:

```sh
pio run -e smalltv_loader
```

Er ist nur für Installationswege gedacht, bei denen das bisherige
Ultra-Updateverfahren das Hauptimage nicht annimmt. Er ersetzt die
Hauptfirmware nicht.

## Vorhandene Prüfungen

Die GitHub Actions bauen die Settings-Firmware und prüfen unter anderem, dass
die komprimierte Weboberfläche mit ihrer HTML-Quelle übereinstimmt, dass Theme-
Pakete gültig sind und dass genügend Platz im App-Slot bleibt. Lokal können
die gleichen relevanten Schritte ausgeführt werden:

```sh
python tools/gzip_webui_settings.py --check
pio run -e smalltv_settings
```

Die Tests unter `tests/` benötigen je nach Skript einen C++-Compiler und unter
Linux teils 32-Bit-Unterstützung. Sie decken Hostcode ab, simulieren aber kein
vollständiges ESP8266-Gerät.

## Veröffentlichungsdatei

Der Firmware-Workflow legt `smalltv-settings-firmware.bin` als Build-Artefakt
ab. Ein Git-Tag im Format `vX.Y.Z` löst zusätzlich die Release-Erstellung aus.
Der Asset-Name muss zum Selbstupdate-Eintrag `UPDATE_ASSET` in `src/config.h`
passen. Erstelle ein Release erst, wenn Repository und Versionsnummer dort
stimmen.
