# SmallTV Ultra Clean Firmware

Schlanke ESP8266-Firmware für GeekMagic SmallTV und SmallTV Ultra mit ST7789-Display (240 × 240 Pixel). Dieses Image enthält nur die Funktionen, die für den eigenständigen Betrieb, die Einrichtung und die Wartung des Geräts benötigt werden.

## Enthalten

- **Display-Kern:** ST7789-Ansteuerung, Rendering und Helligkeits-/Ausrichtungseinstellungen.
- **Netzwerk:** WLAN-Client, Einrichtungs-Hotspot bei fehlender Verbindung, mehrere gespeicherte WLANs und mDNS-Hostname.
- **Lokale Weboberfläche:** Gerätestatus, Display- und Uhr-Einstellungen, Theme-Verwaltung, WLAN-Konfiguration und Systemwartung.
- **Uhr:** NTP-Synchronisierung, Zeitzone, Zeit-/Datumsdarstellung und optionaler Nachtmodus.
- **Einzelnes Theme:** Installieren und Auswählen eines `.stheme`-Pakets sowie Rendering der Theme-Ebenen. Das Image enthält ein internes Recovery-Theme.
- **Wartung:** Konfigurations-Backup, Werkseinstellungen, Neustart, manuelles OTA-Update und Crash-Recovery.

## Bewusst nicht enthalten

Diese Funktionen werden beim Clean Build nicht kompiliert:

- Foto-Bibliothek, JPEG-Decodierung und Foto-Diashow
- Theme-Playlist, Zeitpläne und automatische Theme-Wechsel
- Aktien-/Krypto-Ticker
- Usage-Monitor
- Flugradar
- Home-Assistant-/MQTT-Integration
- Benachrichtigungs-Overlay
- Separates Loader-/Installationsimage

Die Theme-Engine bleibt enthalten, weil sie das installierbare Zifferblatt-/Theme-Format rendert. Die WLAN- und OTA-Funktionen bleiben erhalten, damit das Gerät ohne serielle Konfiguration eingerichtet und gewartet werden kann.

## Zielhardware und Grenzen

- GeekMagic SmallTV / SmallTV Ultra mit ESP8266
- ST7789 IPS-Panel, 240 × 240 Pixel
- WLAN mit 2,4 GHz; 5 GHz wird vom ESP8266 nicht unterstützt
- PlatformIO-Umgebung: `smalltv_clean`
- Dateisystem: LittleFS; Einstellungen und installierte Themes liegen dort

## Clean Image bauen

Benötigt werden Python 3, PlatformIO Core und beim ersten Build eine Internetverbindung zum Laden von Framework und Bibliotheken.

```sh
python tools/gzip_webui_settings.py --check
python -m platformio run -e smalltv_clean
```

Das Image entsteht unter:

```text
.pio/build/smalltv_clean/firmware.bin
```

Die Settings-Weboberfläche wird gzip-komprimiert aus `src/webui_settings.html` eingebunden. Nach Änderungen an der HTML-Datei den Header mit folgendem Befehl neu erzeugen:

```sh
python tools/gzip_webui_settings.py
```

## Projektstruktur

| Pfad | Zweck |
| --- | --- |
| `src/` | Firmware-Kern, Display, Uhr, Netzwerk, OTA und Einstellungen |
| `src/features/theme/` | Theme-Paketparser, Renderer und Theme-Web-API |
| `src/webui_settings.html` | Quelle der lokalen Weboberfläche |
| `examples/themes/` | editierbare Theme-Beispiele und fertige Pakete |
| `examples/recovery-theme/` | separates Beispiel-Recovery-Theme |
| `tools/` | Theme-Packer, Vorschau, UI-Komprimierung und CI-Hilfen |
| `tests/` | Parser-, Speicher-, Theme- und API-nahe Hosttests |
| `docs/` | Bedienungs- und Entwicklungsdokumentation |

## Sicherheit

Der Webzugriff ist standardmäßig nicht passwortgeschützt. Bei Bedarf Digest-Authentifizierung in den erweiterten Geräteeinstellungen aktivieren und das Gerät nur in einem vertrauenswürdigen lokalen Netzwerk betreiben. HTTP verschlüsselt den übrigen Datenverkehr nicht. Konfigurations-Backups können WLAN-Zugangsdaten enthalten und gehören nicht in öffentliche Fehlerberichte.

## Lizenz und Herkunft

Die Lizenzdatei bleibt unverändert erhalten. Eine separate Notiz zu Firmware-Basis und eingebundenen Bibliotheken steht in [NOTICE.md](NOTICE.md). Beispiel-Themes können abweichende Autor- und Lizenzangaben besitzen.
