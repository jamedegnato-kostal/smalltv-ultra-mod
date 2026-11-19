# SmallTV Settings Firmware

Firmwareprojekt für **GeekMagic SmallTV und SmallTV Ultra mit ESP8266**. Die Firmware stellt eine lokale Weboberfläche für Geräteeinstellungen bereit und unterstützt Themes, Playlists, Uhr-Designs und eine lokale Fotobibliothek.

[![Build](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/actions/workflows/build.yml)
[![Dokumentation](https://img.shields.io/badge/Dokumentation-online-338b4f)](https://jamedegnato-kostal.github.io/smalltv-ultra-mod/)
[![Latest release](https://img.shields.io/github/v/release/jamedegnato-kostal/smalltv-ultra-mod?display_name=tag)](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/releases)
[![License](https://img.shields.io/github/license/jamedegnato-kostal/smalltv-ultra-mod)](LICENSE)

> **Herkunft:** Dieses Projekt basiert auf dem ursprünglichen [giovi321/smalltv-mod](https://github.com/giovi321/smalltv-mod) von **[@giovi321](https://github.com/giovi321)**. Es ist eine eigenständige, settings-orientierte Weiterentwicklung dieses Firmware-Codes. Die ursprüngliche Lizenz und die Herkunftshinweise bleiben erhalten – siehe [NOTICE.md](NOTICE.md) und [LICENSE](LICENSE).

Dies ist ein **Firmware-Repository**, kein Hardware-Shop und kein offizielles Produkt von GeekMagic. Der Quellcode richtet sich an Entwickler und Besitzer kompatibler Geräte.

---

## Weboberfläche

Die Firmware enthält eine lokal auf dem Gerät gehostete, responsive Weboberfläche. Sie wird im Browser über die IP-Adresse des SmallTV geöffnet. Der [HTML-Quellcode](src/webui_settings.html) ist direkt im Firmware-Repository enthalten. Die folgende Grafik ist eine **schematische Darstellung** der tatsächlichen Navigations- und Statusstruktur; angezeigte Werte sind Platzhalter, keine Live-Gerätedaten.

![Schematische Übersicht der SmallTV Settings Weboberfläche](docs/public/assets/settings-ui-overview.svg)

Die Navigation umfasst Übersicht, Anzeige, Uhr & Zeit, Themes, Fotos, Netzwerk, System und erweiterte Einstellungen. Die Oberfläche unterstützt helle und dunkle Darstellung.

## Funktionsumfang

| Bereich | Funktionen |
| --- | --- |
| **Geräteübersicht** | WLAN-Status, Geräteadresse, Laufzeit, aktives Theme, Helligkeit und NTP-Status |
| **Anzeige** | Helligkeit, Rotation, Backlight-Polarität, RGB/BGR, Invertierung, RGB-Korrektur, Kontrast, Sättigung und Panelrate |
| **Uhr & Zeit** | NTP, Zeitzone, 12-/24-Stunden-Format, Datum, Sekunden und zeitgesteuerter Nachtmodus |
| **Themes** | Installierbare `.stheme`-Pakete, Suche, Filter, Vorschau, Metadaten, Ebenenreihenfolge und Verwaltung |
| **Playlist** | Bis zu 8 Einträge, feste oder zufällige Reihenfolge, Zeitfenster und individuelle Laufzeiten |
| **Fotos** | JPEG-/PNG-/WebP-Einzelbildimport (Umwandlung in JPEG), Vorschau, Upload-Fortschritt, Abbruch, Löschen und Shuffle-Wiedergabe; keine GIF-Animationen |
| **Renderer** | Text, Bilder, Formen, Animationen, transparente Ebenen und optionale PNG-Schriftatlanten |
| **Geräteverwaltung** | WLAN-Einrichtung, mehrere gespeicherte Netzwerke, optionale Digest-Authentifizierung, Konfigurations-Backup, Firmware-Upload und Recovery |

## Unterstützte Hardware

- **Zielplattform:** ESP8266
- **Modul/Ziel:** ESP-12-Familie (PlatformIO-Board `esp12e`), ESP8266
- **SPI-Flash:** 4 MB
- **Display:** ST7789, 240 × 240 Pixel
- **WLAN:** 2,4 GHz
- **PlatformIO-Umgebung:** `smalltv_settings`
- **Separates Installationshilfsimage:** `smalltv_loader`

Dieses Firmware-Profil ist **nicht** für ESP32- oder ESP32-C2-Geräte vorgesehen. Die erstmalige Installation auf manchen SmallTV-Ultra-Flashlayouts kann einen gerätespezifischen Installationsweg erfordern. Lies vor dem Flashen die [Installationshinweise](https://jamedegnato-kostal.github.io/smalltv-ultra-mod/getting-started/flashing/).

## Schnellstart

### Firmware beziehen

Öffne [Releases](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/releases) und lade das passende Image herunter:

- `smalltv-settings-firmware.bin` – reguläres Image mit 2 MB LittleFS (4m2m).
- `smalltv-settings-firmware-4m1m-legacy.bin` – Legacy-Image für Geräte, deren LittleFS bereits mit 4m1m formatiert wurde.

**Achtung:** Der Wechsel von 4m1m auf 4m2m verschiebt den LittleFS-Startoffset. Vor dem Wechsel müssen WLAN-/Geräteeinstellungen, Themes und Fotos gesichert und nach dem Layoutwechsel wiederhergestellt werden. Ein normales OTA-Update migriert diese Daten nicht automatisch.

Das Loader-Image wird im normalen Release nicht mitveröffentlicht. Falls der dokumentierte Erstinstallationsweg es erfordert, kann es lokal mit `python -m platformio run -e smalltv_loader` gebaut werden.

Prüfe vor dem Flashen die Hardware- und Flashlayout-Hinweise in der [Dokumentation](https://jamedegnato-kostal.github.io/smalltv-ultra-mod/).

### Firmware selbst bauen

Voraussetzungen: Python 3 und PlatformIO Core. PlatformIO kann beispielsweise mit `python -m pip install platformio` installiert werden.

```bash
python tools/gzip_webui_settings.py --check
python -m platformio run -e smalltv_settings
# Legacy-Image (nur für bereits im 4m1m-Layout formatierte Geräte)
python -m platformio run -e smalltv_settings_4m1m_legacy
```

Das Hauptimage entsteht unter:

```text
.pio/build/smalltv_settings/firmware.bin
```

Die Quell-Weboberfläche liegt in `src/webui_settings.html`. Der komprimierte Header `src/webui_settings.h` ist eine generierte Build-Datei und muss nach Änderungen mit folgendem Befehl aktualisiert werden:

```bash
python tools/gzip_webui_settings.py
```

## Dokumentation und Entwicklung

- **[Online-Dokumentation](https://jamedegnato-kostal.github.io/smalltv-ultra-mod/)** – Einrichtung, Bedienung, Themes, API und Entwicklung
- [Hardware-, Flashlayout- und Ressourcenlimits](docs/src/content/docs/reference/hardware-storage.mdx)
- [Logische Repository-Landkarte](docs/src/content/docs/reference/repository-map.mdx)
- [Firmware- und Architekturhinweise](docs/SETTINGS_FIRMWARE.md)
- [HTTP-API](docs/API.md)
- [Code-Audit und bekannte technische Punkte](docs/AUDIT_2026-11.md)
- [Mitwirken](CONTRIBUTING.md)
- [Änderungsprotokoll](CHANGELOG.md)
- [Herkunft und Drittanbieter](NOTICE.md)

## Repository-Struktur

```text
src/                    Firmware und eingebettete Weboberfläche
  features/theme/       Theme-Modell, Paketcontainer, Engine, Laufzeit und Web-API
  PhotoLibrary.*        Foto-Slots und Dateispeicher
  PhotoSlideshow.*      Foto-Wiedergabe und Shuffle
  WebPortal.*           HTTP-API, WebUI-Routen und manuelles OTA
examples/               Recovery-Theme und installierbare Theme-Beispiele
docs/                   Bedienungs-, Architektur- und API-Dokumentation
tests/                  Hosttests, Parser-, Speicher- und UI-Prüfungen
tools/                  Build-, Theme- und Geräte-Testwerkzeuge
lib/                    lokal angepasste ESP8266-mDNS-Bibliothek
.github/workflows/      Firmware-, Dokumentations- und Release-CI
```

## Ressourcen- und Sicherheitshinweise

- Themes und Fotos teilen sich den LittleFS-Speicher. Standard: 1.000 KiB; optionales 4m2m-Profil: 2.024 KiB.
- 4m1m und 4m2m haben unterschiedliche LittleFS-Startadressen. Der Wechsel benötigt eine Datenmigration; die Firmware formatiert bei einem Mountfehler nicht automatisch.
- Ein erfolgreicher Host-/CI-Build ersetzt keinen vollständigen Test auf echter SmallTV-Ultra-Hardware.
- Der Webzugriff ist standardmäßig nicht passwortgeschützt. Optionaler Digest-Schutz schützt die Anmeldung, **verschlüsselt aber nicht den HTTP-Datenverkehr**.
- Konfigurations-Backups können WLAN-Zugangsdaten enthalten. Teile solche Dateien nicht öffentlich.
- Das Projekt enthält eine Recovery-Ansicht; sie ersetzt keine sorgfältige Prüfung des Flashlayouts vor einer Erstinstallation.

## Lizenz und Anerkennung

Die ursprüngliche Lizenzdatei aus [giovi321/smalltv-mod](https://github.com/giovi321/smalltv-mod) ist im Repository enthalten. Übernommene Copyright- und Lizenzhinweise bleiben bestehen. Zusätzliche Bibliotheken und Beispiel-Themes können eigene Lizenzbedingungen haben; Details stehen in [NOTICE.md](NOTICE.md).
