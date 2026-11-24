---
title: Web-only Clean Image lokal bauen
description: Die minimale SmallTV-Ultra-Firmware mit PlatformIO kompilieren.
---

## Voraussetzungen

- PlatformIO Core oder die PlatformIO-Erweiterung für Visual Studio Code
- Python 3
- Internetzugang beim ersten Build

## Firmware bauen

Im Repository-Ordner:

~~~sh
pio run -e smalltv_clean
~~~

Das Image liegt unter .pio/build/smalltv_clean/firmware.bin.

Es enthält nur WLAN-Client, Setup-Hotspot, lokale Weboberfläche, Konfigurationsspeicher, Statusdiagnose, Werksreset, Neustart, manuelles OTA und GitHub-OTA. Display, Uhr, Theme-Engine, Fotos, Login und optionale Anzeige-Modi sind nicht Bestandteil des Builds.

## Qualitätssicherung

GitHub Actions baut ausschließlich smalltv_clean, kontrolliert den Veröffentlichungsdateinamen und prüft den freien Platz im OTA-App-Slot. Das ersetzt keinen Test auf echter SmallTV-Ultra-Hardware.
