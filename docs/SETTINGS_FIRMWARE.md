# Technische Übersicht

## Ziel und Profile

`smalltv_settings` ist der Hauptbuild für SmallTV und SmallTV Ultra mit
ESP8266, 4 MB Flash und einem ST7789-Display mit 240 × 240 Pixeln. Er nutzt
LittleFS für Einstellungen, Themes und Fotos.

| PlatformIO-Profil | Zweck |
| --- | --- |
| `smalltv_settings` | Hauptfirmware mit Weboberfläche, `.stheme`, Playlist und Fotos |
| `smalltv_clean` | Basisprofil ohne die Foto-Bibliothek |
| `smalltv_loader` | separates Hilfsimage für bestimmte Erstinstallationswege |

Ticker, Nutzungsanzeige, Radar, Home-Assistant/MQTT und Benachrichtigungen sind
in diesen Profilen deaktiviert. Das Loader-Image ist keine vollständige
Firmware.

## Funktionen

- **Anzeige:** manuelle Helligkeit, Rotation, Backlight-Polarität, RGB/BGR,
  Invertierung, RGB-Verstärkung, Kontrast, Sättigung, Hintergrundfarbe,
  Theme-Deckkraft, Panelrate und Renderlimit.
- **Zeit:** NTP, Zeitzone, 12-/24-Stundenformat, Sekunden, Datum, eigenes
  Datumsformat und zeitgesteuerte Nachtabsenkung. Es gibt keinen
  Umgebungslichtsensor-Modus.
- **Themes:** `.stheme`-Bibliothek mit Suche, Kategorien, Metadaten,
  Vorschau, Aktivierung, Löschung und Layer-Reihenfolge. Uhr-Themes erscheinen
  im Uhr-Bereich und verwenden dieselbe Theme-Ablage.
- **Playlist:** bis zu acht Themes in fester oder zufälliger Reihenfolge,
  individuelle Laufzeiten und optionale lokale Zeitfenster. Zufallswahl
  vermeidet direkte Wiederholungen, wenn mehrere passende Themes existieren.
- **Fotos:** JPEG-Bibliothek mit Speicheranzeige, Upload und Löschfunktionen.
  Fotos werden in 240 × 240 Pixel große JPEGs umgewandelt und zwischen
  Playlist-Themes eingeblendet.
- **Gerät:** WLAN-Konfiguration, Konfigurationssicherung, Firmware-Upload,
  Updateprüfung, Neustart, Werksreset und optionaler Zugriffsschutz.

Die Weboberfläche zeigt nur Funktionen an, die in diesem Build vorgesehen
sind. Änderungen an Formularwerten werden gespeichert; Theme- und Fotodateien
werden durch ihre Verwaltungsaktionen geschrieben.

## Ressourcen- und Dateigrenzen

- Bis zu 16 installierte Themes; Paketgröße höchstens 3 MiB, höchstens 256
  Dateien und 32 Layer je Theme.
- Bis zu acht Playlist-Einträge, jeweils 5 bis 3600 Sekunden.
- Bis zu 240 Fotos; ein JPEG-Upload darf höchstens 128 KiB groß sein.
- Foto-Wiedergabe lädt und dekodiert Bilder einzeln. Die Firmware weist
  fehlerhafte Dateien zurück und hält freien LittleFS-Speicher für Betrieb und
  Einstellungen zurück.
- Themes enthalten deklarative Layoutdaten, keinen ausführbaren Code.
  Unterstützte Schriftgrafik ist ein vorbereitetes PNG-Atlasformat; TTF/OTF
  wird nicht auf dem ESP geladen.

## Laufzeit und Wiederherstellung

Beim Start lädt die Firmware die Konfiguration und verbindet sich mit einem
gespeicherten WLAN. Wenn kein gespeichertes Netz erreichbar ist, stellt sie
einen Setup-Zugangspunkt bereit. NTP wird für Uhranzeige, Zeitfenster und
Nachtabsenkung verwendet. Ohne bestätigte Zeit bleibt die zeitabhängige
Helligkeitsregelung außer Kraft.

Theme- und Fotoanzeige laufen zustandsbasiert in der Geräteschleife. Große
Dateioperationen sind begrenzt und dürfen Netzwerkbedienung und Watchdog nicht
blockieren. Eine Recovery-Ansicht hilft beim Start, wenn kein nutzbares Theme
geladen werden kann.

## Bekannte Grenzen

- Die Firmware ist für ESP8266/SmallTV-Ultra vorgesehen, nicht für ESP32 oder
  ESP32-C2.
- Ein erfolgreicher Host- oder CI-Build ersetzt keinen Test auf dem echten
  240 × 240 Display.
- Das Projekt enthält keinen vollständigen ESP8266-Emulator. Das lokale
  QEMU-Experiment ist kein unterstützter Testweg.
- Der Selbstupdate-Release muss unter dem in `src/config.h` eingetragenen
  Repository liegen und das Asset `smalltv-settings-firmware.bin` anbieten.
- Der Ultra kann je nach vorherigem Flashlayout einen gesonderten
  Erstinstallationsweg benötigen. Prüfe vor dem Flashen die verfügbare
  Updatefläche und sichere bei Bedarf die bestehende Firmware.

## Web-API

Routen und Methoden sind in [API.md](API.md) dokumentiert. Authentifizierung
und Schreibschutz werden zentral durch den Webserver angewendet.
