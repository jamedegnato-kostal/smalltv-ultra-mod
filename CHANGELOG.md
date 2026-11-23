# Änderungsprotokoll

Hier werden Änderungen an SmallTV Settings Firmware nachvollziehbar festgehalten.

Die bisherige Versionshistorie wird nicht rückwirkend rekonstruiert. Für veröffentlichte Firmware-Versionen sind die jeweiligen Einträge und Assets auf der [Releases-Seite](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/releases) maßgeblich.

## Unreleased

### SmallTV Ultra V2 (ESP8266)

- Settings- und Import-Transaktionen gegen unvollständige Arduino-String-Kopien, Heap-Fragmentierung und fehlgeschlagene Persistenz absichern.
- LittleFS-Mount ohne automatisches Formatieren; Konfiguration erst nach geprüftem temporärem Schreiben und erfolgreichem Rename veröffentlichen.
- Theme-Caches unter Speicherdruck vollständig freigeben; Heap-Grenzen aus der tatsächlichen Größe von `Settings` ableiten.
- Fotoverwaltung und WebUI auf JPEG-Einzelbilder reduzieren; GIF-Frameimport, Wiedergabe und API-Endpunkt entfernen.
- Speicherdiagnose um Heap-Low-Water-Marks, größte freie Blöcke, Speicherphasen und LittleFS-Belegung erweitern.
- GitHub-Online-Self-Update entfernen; lokaler manueller Firmware-Upload und Recovery bleiben verfügbar.


### Dokumentation und Repository

- README als Firmware-Projektübersicht mit schematischer WebUI-Ansicht neu strukturiert.
- Herkunft des ursprünglichen Projekts [giovi321/smalltv-mod](https://github.com/giovi321/smalltv-mod) deutlich ausgewiesen.
- Online-Dokumentation und Theme-Beispiele auf das ESP8266-Settings-Profil ausgerichtet.
- Nicht referenzierte, irreführende Hardware-/Modusgrafiken, CAD-Gehäusedateien und das ungenutzte PlatformIO-Bootstrap-Bundle entfernt.
- Build- und Beitragsanleitungen präzisiert.
