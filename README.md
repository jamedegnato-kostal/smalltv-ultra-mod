# SmallTV Settings Firmware

Eigenständige ESP8266-Firmware für GeekMagic SmallTV und SmallTV Ultra. Sie stellt die Konfiguration über eine lokale Weboberfläche bereit und nutzt installierbare `.stheme`-Pakete für die Anzeige. Das Projekt liegt separat von anderen SmallTV-Firmwareordnern.

## Funktionen

- **Weboberfläche:** responsives Dashboard mit WLAN-, Laufzeit-, Theme-, Helligkeits- und NTP-Status. Navigation für Anzeige, Uhr & Zeit, Themes, Fotos, Netzwerk, System und erweiterte Einstellungen. Hell- und Dunkelmodus.
- **Display:** manuelle Helligkeit, Ausrichtung, Backlight-Polarität, RGB/BGR-Reihenfolge, Invertierung, RGB-Korrektur, Kontrast, Sättigung, Theme-Hintergrund und Ebenen-Transparenz. Einstellbare ST7789-Bildrate von 39 bis 60 Hz.
- **Uhr:** NTP-Zeitsynchronisation mit Zeitzone, 12-/24-Stunden-Format, Sekunden- und Datumsanzeige, Datums-Token `{DATE}` sowie zeitgesteuerter Nachtmodus. Für die Nachtfunktion wird eine bestätigte NTP-Zeit verlangt.
- **Themes:** `.stheme` installieren, suchen, nach Kategorien filtern, Vorschau ansehen, auswählen, umbenennen, Metadaten bearbeiten, Ebenen anordnen und Pakete löschen. Der interne Theme-Identifier bleibt dabei stabil. Installierte Uhr-Themes erscheinen in einem eigenen Abschnitt unter **Uhr & Zeit**.
- **Theme-Automatik:** Playlist mit bis zu acht Einträgen, fester oder zufälliger Reihenfolge, Zeitfenstern und einer Dauer von 5 bis 3600 Sekunden je Eintrag. Der Zufallsmodus vermeidet direkte Wiederholungen. Uhr-Themes können eine eigene Standard-Anzeigedauer erhalten.
- **Fotos:** getrennte Foto-Bibliothek im LittleFS-Speicher mit bis zu 240 JPEGs. Mehrfachauswahl im Browser, Vorschau, Fortschritt, Abbruch und Löschen. Der Browser skaliert und beschneidet Bilder vor dem Upload auf 240 × 240 Pixel. Fotos erscheinen als Shuffle-Wiedergabe zwischen Themes einer aktiven Playlist.
- **Theme-Renderer:** Text, Bilder, Formen, Animationen, transparente Ebenen, optionale PNG-Schriftatlanten und gebundene Werte. Animationsschalter, Geschwindigkeit, Renderlimit und weicher Theme-Übergang sind konfigurierbar. Externe Theme-Datenquellen werden durch das deklarative Theme-Format begrenzt.
- **Geräteverwaltung:** WLAN-Einrichtung über `SmallTV-Setup`, mehrere gespeicherte Netzwerke, optionaler Digest-Passwortschutz, Gerätekonfiguration exportieren/importieren, manueller Firmware-Upload, Neustart und Werkseinstellungen.
- **Recovery:** nach wiederholten Startfehlern kann ein in die Firmware eingebettetes Recovery-Theme angezeigt werden. Es ist getrennt von den installierten Paketen.

Die Weboberfläche zeigt keine Sensor-Helligkeitsfunktion. Der Nachtmodus verwendet feste Uhrzeiten und ändert die Helligkeit zeitgesteuert.

## Zielhardware und Grenzen

- Ziel: SmallTV / SmallTV Ultra mit ESP8266 und ST7789-Panel, 240 × 240 Pixel.
- WLAN: 2,4 GHz; 5 GHz wird vom ESP8266 nicht unterstützt.
- PlatformIO-Umgebung `smalltv_settings` baut das Settings-Image. `smalltv_loader` ist ein separates Installationshilfsimage und kein Ersatz für die Firmware.
- Theme-Speicher und Fotos teilen sich LittleFS. Das Gerät begrenzt Paket-, Bild- und Playlistgrößen und hält beim Upload eine Reserve frei.
- Ein vollständiger Lauf auf echter Ultra-Hardware ist für diese Dokumentationsänderung nicht erfolgt. Das lokale QEMU-Experiment ist kein funktionsfähiger Display-Emulator.

Weitere Hardware- und Formatgrenzen stehen in [Firmware- und Architekturhinweisen](docs/SETTINGS_FIRMWARE.md), die Bedienung in [der Dokumentation](docs/src/content/docs/index.mdx).

## Lokal bauen

Benötigt werden Python 3, PlatformIO Core und beim ersten Build eine Internetverbindung zum Laden von Framework und Bibliotheken.

```powershell
python tools/gzip_webui_settings.py --check
python -m platformio run -e smalltv_settings
```

Das Image entsteht unter:

```text
.pio/build/smalltv_settings/firmware.bin
```

Die Weboberfläche ist in `src/webui_settings.html` gepflegt und wird als gzip-komprimiertes Header-Array eingebunden. Nach Änderungen die Datei mit `python tools/gzip_webui_settings.py` regenerieren.

Für lokale Theme-Werkzeuge:

```powershell
python -m pip install -r tools/requirements-theme.txt
python tools/theme_pack.py examples/themes/pixel-room .pio/pixel-room.stheme
```

## Projektstruktur

| Pfad | Zweck |
| --- | --- |
| `src/` | Firmware, Settings, Display, Uhr, Netzwerk und Web-API |
| `src/features/theme/` | `.stheme`-Parser, Renderer, Datenbindung und Theme-API |
| `src/webui_settings.html` | Quelltext der Settings-Weboberfläche |
| `examples/themes/` | editierbare Theme-Beispiele und fertige Pakete |
| `examples/recovery-theme/` | separates Recovery-Theme |
| `tools/` | Theme-Packer, Vorschau, UI-Komprimierung und CI-Hilfen |
| `tools/device_tester/` | Windows-Testoberfläche für ein echtes Gerät; kein Emulator |
| `tests/` | Parser-, Speicher-, Theme-, Foto- und API-nahe Hosttests |
| `docs/` | ausführliche Bedienungs- und Entwicklungsdokumentation |

## GitHub-Releases

Ein Push eines Versions-Tags (`v*`) startet den Workflow unter `.github/workflows/build.yml`. Er baut `smalltv_settings` und veröffentlicht `smalltv-settings-firmware.bin` als Release-Asset. Firmware-Version, Release-Dateiname und Repository-Adresse für den integrierten Update-Check stehen in `src/config.h`.

## Sicherheit und Supportdaten

Der Webzugriff ist standardmäßig nicht passwortgeschützt. Aktiviere unter **Erweitert** bei Bedarf Digest-Authentifizierung und verwende das Gerät nur in einem vertrauenswürdigen lokalen Netzwerk. HTTP ist unverschlüsselt; Digest schützt das Passwort beim Login, aber nicht den gesamten Datenverkehr. Konfigurations-Backups können WLAN-Zugangsdaten enthalten und gehören nicht in öffentliche Fehlerberichte.

## Lizenz und Herkunft

Die Lizenzdatei im Projekt bleibt unverändert erhalten. Eine separate Notiz zu Firmware-Basis und eingebundenen Bibliotheken steht in [NOTICE.md](NOTICE.md). Beispiel-Themes können abweichende Autor- und Lizenzangaben besitzen.
