# SmallTV Ultra Mod

Eigenständige ESP8266-Firmware für **GeekMagic SmallTV und SmallTV Ultra** mit lokaler Weboberfläche, installierbaren Themes und Foto-Wiedergabe.

[![Build](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/actions/workflows/build.yml)
[![Latest release](https://img.shields.io/github/v/release/jamedegnato-kostal/smalltv-ultra-mod?display_name=tag)](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/releases)
[![License](https://img.shields.io/github/license/jamedegnato-kostal/smalltv-ultra-mod)](LICENSE)

---

## Überblick

SmallTV Ultra Mod erweitert das Gerät um eine browserbasierte Konfiguration und ein flexibles Anzeige-System. Themes lassen sich installieren und organisieren; Fotos können lokal gespeichert und in die Wiedergabe eingebunden werden.

**Auf einen Blick**

- **Geräte:** GeekMagic SmallTV / SmallTV Ultra
- **Plattform:** ESP8266, ST7789, 240 × 240 Pixel
- **Konfiguration:** lokale, responsive Weboberfläche
- **Theme-Format:** installierbare `.stheme`-Pakete
- **Speicher:** LittleFS für Themes, Fotos und Einstellungen
- **Firmware-Build:** PlatformIO, Umgebung `smalltv_settings`

## Funktionen

| Bereich | Funktionen |
| --- | --- |
| **Weboberfläche** | Dashboard, WLAN- und Laufzeitstatus, Hell-/Dunkelmodus, Einstellungen für Anzeige, Uhr, Themes, Fotos, Netzwerk und System |
| **Display** | Helligkeit, Ausrichtung, Backlight-Polarität, RGB/BGR, Invertierung, RGB-Korrektur, Kontrast, Sättigung und Bildrate |
| **Uhr & Zeit** | NTP, Zeitzone, 12-/24-Stunden-Format, Datum, Sekunden und zeitgesteuerter Nachtmodus |
| **Themes** | `.stheme` installieren, suchen, filtern, Vorschau, Metadaten bearbeiten, Ebenen sortieren und Themes verwalten |
| **Wiedergabe** | Playlist mit bis zu 8 Einträgen, Zufallsmodus, Zeitfenster und konfigurierbarer Anzeigedauer |
| **Fotos** | JPEG- und GIF-Import, Vorschau, Upload-Fortschritt, Abbruch, Löschen und Shuffle-Wiedergabe |
| **Renderer** | Text, Bilder, Formen, Animationen, transparente Ebenen und optionale PNG-Schriftatlanten |
| **Geräteverwaltung** | WLAN-Einrichtung, mehrere gespeicherte Netzwerke, optionaler Digest-Login, Konfigurations-Backup, Firmware-Upload und Recovery |

> **Hinweis:** Die Helligkeit wird manuell oder zeitgesteuert geregelt. Eine automatische Sensor-Helligkeitsmessung ist nicht enthalten.

## Schnellstart

### Firmware herunterladen

Öffne [Releases](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/releases) und lade das passende Firmware-Image herunter. Das veröffentlichte Settings-Image heißt:

```text
smalltv-settings-firmware.bin
```

Beachte vor dem Flashen die gerätespezifischen Hinweise in der [Firmware-Dokumentation](docs/SETTINGS_FIRMWARE.md). Insbesondere kann die erstmalige Installation auf Ultra-Geräten von der vorhandenen Flash-Aufteilung abhängen.

### Selbst bauen

Voraussetzungen: Python 3, PlatformIO Core und eine Internetverbindung für den erstmaligen Download der Build-Abhängigkeiten.

```bash
python tools/gzip_webui_settings.py --check
python -m platformio run -e smalltv_settings
```

Das Firmware-Image wird hier abgelegt:

```text
.pio/build/smalltv_settings/firmware.bin
```

Die Weboberfläche wird in `src/webui_settings.html` bearbeitet. Nach Änderungen muss der komprimierte Header aktualisiert werden:

```bash
python tools/gzip_webui_settings.py
```

### Theme-Pakete erstellen

```bash
python -m pip install -r tools/requirements-theme.txt
python tools/theme_pack.py examples/themes/pixel-room .pio/pixel-room.stheme
```

## Dokumentation

| Thema | Beschreibung |
| --- | --- |
| [Firmware- und Architekturhinweise](docs/SETTINGS_FIRMWARE.md) | Hardware, Speichergrenzen, Theme- und Fotoformate |
| [API-Referenz](docs/API.md) | HTTP-Endpunkte und Geräteschnittstellen |
| [Bedienungsdokumentation](docs/src/content/docs/index.mdx) | Einstellungen und Bedienabläufe |
| [Mitwirken](CONTRIBUTING.md) | Build, Tests und Hinweise für Beiträge |
| [Änderungsprotokoll](CHANGELOG.md) | Nachvollziehbare Projektänderungen |
| [Herkunft und Drittanbieter](NOTICE.md) | Ursprünge und Lizenzhinweise |

## Repository-Struktur

```text
src/                    Firmware und Weboberfläche
  features/theme/       Theme-Parser, Renderer und Theme-API
examples/               Beispiel-Themes und Recovery-Theme
tools/                  Theme-Werkzeuge, UI-Komprimierung und CI-Hilfen
tools/device_tester/    Windows-Testwerkzeug für ein echtes Gerät
tests/                  Hosttests für Parser, Speicher, Themes und APIs
docs/                   Bedienungs- und Entwicklungsdokumentation
.github/workflows/      Build-, Test- und Release-Automatisierung
```

## Hardware- und Speicherhinweise

- Der ESP8266 unterstützt WLAN mit **2,4 GHz**, nicht 5 GHz.
- Das Ziel-Display ist ein ST7789-Panel mit **240 × 240 Pixeln**.
- Themes und Fotos teilen sich den begrenzten LittleFS-Speicher.
- `smalltv_settings` ist das reguläre Settings-Firmware-Image. `smalltv_loader` ist ein separates Installationshilfsimage.
- Ein erfolgreicher Host-/CI-Build ersetzt keinen vollständigen Test auf echter SmallTV-Ultra-Hardware. Das vorhandene QEMU-Experiment ist kein funktionsfähiger Display-Emulator.

## Sicherheit

Der Webzugriff ist standardmäßig nicht passwortgeschützt. Aktiviere bei Bedarf die Digest-Authentifizierung und betreibe das Gerät in einem vertrauenswürdigen lokalen Netzwerk. **HTTP bleibt unverschlüsselt**; Digest schützt die Anmeldung, aber nicht den gesamten Datenverkehr.

Konfigurations-Backups können WLAN-Zugangsdaten enthalten. Veröffentliche solche Dateien nicht in Issues, Diskussionen oder Support-Anfragen.

## Mitwirken

Fehlerberichte, Verbesserungsvorschläge und Pull Requests sind willkommen. Bitte lies zuerst die [Beitragsrichtlinien](CONTRIBUTING.md) und erstelle für Fehler möglichst einen reproduzierbaren Bericht mit Firmware-Version und relevanten Logs.

## Lizenz und Herkunft

Beachte die im Repository enthaltene [Lizenz](LICENSE) sowie die ergänzenden Hinweise in [NOTICE.md](NOTICE.md). Beispiel-Themes können eigene Autoren- und Lizenzangaben haben.
