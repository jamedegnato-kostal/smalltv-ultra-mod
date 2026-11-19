# Änderungsprotokoll

Hier werden Änderungen an SmallTV Settings Firmware nachvollziehbar festgehalten.

Die bisherige Versionshistorie wird nicht rückwirkend rekonstruiert. Für veröffentlichte Firmware-Versionen sind die jeweiligen Einträge und Assets auf der [Releases-Seite](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/releases) maßgeblich.


## 0.1.8 – ESP8266 Foto-only-Verteilbuild (18. November 2026)

- Behebt den Settings-Speicherpfad: Theme-Renderer und sämtliche Theme-Cache-Vektoren werden bei Heap-Druck vollständig freigegeben.
- Prüft den Heap nach dem JSON-Parsing erneut, bevor die Settings-Kopie erzeugt wird.
- Beschränkt die Fotobibliothek auf JPEG-Einzelbilder; PNG und WebP werden im Browser nach JPEG konvertiert.
- Entfernt GIF-Import, GIF-Frame-Validierung, GIF-HTTP-Endpunkte und GIF-Frame-Wiedergabe.
- Bewahrt vorhandene GIF-Metadaten und Frame-Dateien auf LittleFS; sie werden nicht automatisch gelöscht oder überschrieben.
- Beibehaltung atomarer Settings-Speicherung, LittleFS ohne automatische Formatierung und erweiterten RAM-/LittleFS-Diagnosen.

**Hinweis:** CI-Build und automatisierte Tests ersetzen keine Prüfung auf realer ESP8266-Hardware. Vor einem breiten Rollout sind Geräte-Tests mit wiederholtem Theme-Speichern, Foto-Upload und Dauerbetrieb erforderlich.

## 0.1.9 – 4m2m LittleFS als Standard (19. November 2026)

- Setzt das reguläre ESP8266-Image auf das 4-MB-Flashlayout mit 2.024 KiB LittleFS.
- Behält ein separat benanntes 4m1m-Legacyprofil für bereits mit dem alten LittleFS-Offset formatierte Geräte.
- Trennt die OTA-Assetnamen nach Flashlayout; der bisherige generische Assetname bleibt vorübergehend ein 4m1m-Kompatibilitätsalias für ältere Firmware.
- Dokumentiert ESP-12/ESP8266-Hardware, Flashadressen, OTA-/IROM-Grenzen, Theme-/Foto-/RAM-Limits und die logische Modul-Landkarte.
- Ergänzt einen CI-Build für Standard-, Legacy- und Loader-Image.

**Migrationshinweis:** Ein vorhandenes 4m1m-Dateisystem wird nicht automatisch auf 4m2m verschoben. Vor dem Wechsel sind Konfiguration, Themes und Fotos separat zu sichern und nach dem Layoutwechsel wiederherzustellen. Der automatische LittleFS-Migrationspfad ist in dieser Version nicht enthalten.

## Unreleased

### Dokumentation und Repository

- README als Firmware-Projektübersicht mit schematischer WebUI-Ansicht neu strukturiert.
- Herkunft des ursprünglichen Projekts [giovi321/smalltv-mod](https://github.com/giovi321/smalltv-mod) deutlich ausgewiesen.
- Online-Dokumentation und Theme-Beispiele auf das ESP8266-Settings-Profil ausgerichtet.
- Nicht referenzierte, irreführende Hardware-/Modusgrafiken, CAD-Gehäusedateien und das ungenutzte PlatformIO-Bootstrap-Bundle entfernt.
- Build- und Beitragsanleitungen präzisiert.
