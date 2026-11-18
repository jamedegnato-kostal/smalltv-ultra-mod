# SmallTV Ultra – Speicher-Audit und Optimierung

**Stand:** 18. November 2026  
**Basis:** main, Commit a413e0b0e0e9673268f79523e85e98a03b8a1bb6  
**Optimierungsbranch:** audit/memory-footprint-2026-11  
**Pull Request:** #11 (Draft)

## 1. Ergebnisübersicht

Der Branch behebt einen konkreten Heap-Engpass im Settings-Speicherpfad,
unterscheidet RAM- und LittleFS-Fehler, erhält die atomare Speicherung,
verhindert automatische LittleFS-Formatierung und erweitert die Diagnose.
Verwaiste Animations-Frames werden nicht mehr während eines
Bibliotheks-Scans automatisch gelöscht.

Der ursprüngliche Gerätefehler wurde nicht mit einem historischen Heap-
oder LittleFS-Snapshot reproduziert. Der Quellcode zeigt aber einen
konkreten Fehlermechanismus: Der bisherige Theme-Request-Guard verlangte
nur einen 3.072-Byte-Largest-Block, während die Transaktion einen
zusammenhängenden Block in der Größe von sizeof(Settings) benötigt.
Die neue Guard-Schwelle wird deshalb aus der tatsächlichen Objektgröße
abgeleitet. Unvollständige Kopien der dynamischen String-Felder werden vor
einer Mutation oder Speicherung zurückgewiesen.

**Wichtig:** Die CI belegt Build- und Hosttest-Erfolg. Stabilität mit realen
Fotos/GIFs, Themes und Showflow bei knappem Heap und LittleFS muss zusätzlich
auf einem ESP8266-Gerät geprüft werden.

## 2. Speicherwerte vor und nach der Änderung

Quelle der Ausgangswerte ist der erfolgreiche GitHub-Actions-Lauf #111 auf
main:
https://github.com/jamedegnato-kostal/smalltv-ultra-mod/actions/runs/44268403847

Quelle der Werte nach der Änderung ist der erfolgreiche
GitHub-Actions-Firmware-Workflow auf dem Audit-Branch,
Commit c4a0afeb05eeda7a6d112c4730070c9802dec117.

| Messgröße | main | Audit-Branch | Differenz |
| --- | ---: | ---: | ---: |
| ESP8266 statisches RAM (PlatformIO) | 58.624 B | 59.404 B | +780 B |
| ESP8266 RAM-Auslastung | 71,6 % | 72,5 % | +0,9 Prozentpunkte |
| ESP8266 Flash laut PlatformIO | 881.981 B | 817.353 B | −64.628 B |
| App-Slot-Größe laut CI-Prüfung | 886.128 B | 821.504 B | −64.624 B |
| Freier App-Slot laut CI-Prüfung | 158.336 B | 222.960 B | +64.624 B |
| Loader statisches RAM | 28.824 B | 28.824 B | unverändert |
| Loader Flash laut CI-Prüfung | 315.920 B | 315.920 B | unverändert |

Der statische RAM-Wert steigt netto um 780 Byte. Das ist der Preis der
zusätzlichen Fehlerdiagnose und Zustandszähler. Er ist kein
Laufzeit-Heap-Messwert. Für den ESP8266-Laufzeitspeicher zählen weiterhin
freier Heap, größter zusammenhängender Block und Stack-Reserve; diese Werte
müssen am Gerät beobachtet werden.

### WebUI-Kompression

Die main-Version enthielt im generierten src/webui_settings.h
105.538 komprimierte Array-Bytes. Der Audit-Branch enthält 38.879 Byte für
106.419 Byte HTML.

Das sind 66.659 Byte weniger komprimierte WebUI-Ressource (rund 63,2 %).
Die gzip-Ressource wird von der CI durch Dekomprimierung mit der
HTML-Quelldatei verglichen. Die Flash-Ersparnis im Firmware-Image ist mit
rund 64,5 KB in derselben Größenordnung. Keine Einstellungs- oder
WebUI-Funktion wurde dafür entfernt.

## 3. RAM- und Heap-Befunde

### Settings-POST und Fotoshow-Einstellungen

Der bisherige Ablauf enthielt mehrere gleichzeitig benötigte Strukturen:

1. Der Webserver puffert den POST-Body als String.
2. ArduinoJson parst den Request in ein JsonDocument.
3. applyAndSaveSettings() legt eine vollständige Settings-Kopie als
   Transaktionskandidaten an.
4. saveSettings() baut ein weiteres JSON-Dokument für die Datei.

Das Request-JSON-Dokument wird vor dem Speichern mit doc.clear()
freigegeben; der vom Webserver gehaltene Request-Body bleibt bis zum Ende
des Requests bestehen. Die Theme-Freigabe muss deshalb ausreichend
zusammenhängenden Heap für die Settings-Kopie und ausreichend Gesamt-Heap
für die anschließende JSON-Serialisierung hinterlassen.

Settings enthält unter anderem 16 Theme-Aliase und 16 Showflow-Schritte
in festen Arrays sowie mehrere dynamische String-Felder. Der vorherige
ThemeMode::RequestLargestBlock-Grenzwert von 3.072 Byte war nicht an
sizeof(Settings) gekoppelt. Wenn die tatsächliche Struktur größer als
der Grenzwert war, konnte appRelease() das Theme behalten, obwohl für die
Staging-Kopie kein ausreichend großer zusammenhängender Block mehr
vorhanden war.

**Korrektur:** Der Request-Guard verwendet nun sizeof(Settings) + 256
Byte als Largest-Block-Schwelle und mindestens max(8192,
sizeof(Settings) + 4096) Byte freien Heap. Der tatsächliche Wert
sizeof(Settings) wird auch in /api/status ausgegeben.

### Fehlgeschlagene String-Unterallokationen

Ein new Settings kann erfolgreich sein, obwohl ein interner
String-Kopierpuffer nicht allokiert werden konnte. Ein reiner
Null-Pointer-Test auf das äußere Objekt reicht deshalb nicht als
Integritätsprüfung.

**Korrektur:** settingsCopyValid() vergleicht vor der Mutation alle
dynamischen String-Felder der Quelle und der Kandidatenkopie. Eine
unvollständige Kopie wird mit einem eigenen Fehlerstatus verworfen.
Das aktive Settings-Objekt bleibt unverändert.

### Dynamische Allokationen und Fragmentierung

- Der Netzwerk-Fingerprint wurde zuvor durch Verkettung temporärer
  String-Objekte erzeugt. Er wird jetzt als allocation-freier
  64-Bit-Fingerprint berechnet.
- Der Theme-Renderer verwendet eine vollständige Freigabe vor
  speicherintensiven Requests. ThemeMode::release() ruft bei knappem Heap
  jetzt ThemeMode::unload() auf und gibt damit auch die Vektorkapazitäten
  für Datenwerte und Abrufpläne frei. Diese Cache-Werte werden nach dem
  Request neu aufgebaut; Theme-Daten werden erneut abgerufen. So bleiben
  sie nicht als kleine Heap-Blöcke zurück, die den größten zusammenhängenden
  Block für die Settings-Kopie begrenzen.
- Foto-/GIF-Wiedergabe lädt keine vollständigen PGA1-Payloads in den
  ESP8266-Heap. Die Firmware hält höchstens zwölf Frame-Metadatensätze;
  JPEG-Daten werden aus LittleFS gelesen.
- Der GIF-Importer benötigt überwiegend Browser-RAM: bis zu zwölf
  240×240-RGBA-Snapshots benötigen zusammen rund 2,64 MiB, zuzüglich
  Encoder- und Blob-Puffern. Diese Daten werden nicht zum ESP8266 übertragen.
- Die WebUI sendet vorbereitete PGA1-Container in Upload-Chunks. Die
  Firmware schreibt diese direkt in eine temporäre LittleFS-Datei.

JPEG-Qualität, Frame-Anzahl und Bildabmessungen wurden nicht willkürlich
reduziert.

## 4. Flash und LittleFS

### Atomare Konfigurationsspeicherung

AtomicJson.h schreibt die vollständige Konfiguration zunächst nach
/config.json.tmp. Vor der Veröffentlichung werden Serialisierungs-Länge,
Schreibfehler nach flush() und endgültige Dateigröße geprüft. Erst danach
ersetzt ein Rename die aktive /config.json.

Die temporäre Datei benötigt vorübergehend zusätzlichen LittleFS-Platz.
Das ist für den Schutz vor beschädigten Konfigurationsdateien notwendig.
Die aktive Datei darf nicht vor dem Schreiben gelöscht werden.
Bei fehlgeschlagenem Schreiben oder Rename bleibt sie erhalten.

### Fehlerklassifizierung

Vor der Änderung wurden Staging-RAM-Mangel und Speicherfehler als
generisches "could not save settings" zurückgegeben. Jetzt unterscheidet
die Transaktion:

- zu wenig RAM für den Settings-Staging-Block;
- unvollständige Kopie dynamischer String-Felder;
- ArduinoJson-Dokument-Overflow;
- temporäre Datei kann nicht geöffnet werden;
- Short Write;
- Flush-Fehler;
- unerwartete Dateigröße;
- atomarer Rename fehlgeschlagen.

Ein Fehler durch fehlenden LittleFS-Platz wird nur dann so bezeichnet,
wenn die gemessene freie Kapazität kleiner als die erwartete JSON-Größe ist.
Andernfalls werden Open-, Schreib-, Flash- und Rename-Fehler getrennt
gemeldet.

### Keine automatische Formatierung

ESP8266-LittleFS verwendet standardmäßig autoFormat=true. Das wäre bei
einem Mount-Fehler gefährlich, weil dadurch die Nutzerdaten-Partition
formatiert werden könnte.

**Korrektur:** ESP8266 wird mit LittleFSConfig(false) konfiguriert,
ESP32 verwendet LittleFS.begin(false). Ein Mount-Fehler formatiert damit
nicht automatisch. Der vorhandene explizite Recovery-Aufruf kann weiterhin
formatieren, wird aber nicht beim normalen Mount verwendet.

### Fotos, GIFs und temporäre Dateien

- Verwaiste pNNNfNN.jpg-Animationsframes wurden vorher beim
  Bibliotheks-Scan gelöscht, wenn die zugehörige .anim-Metadatei fehlte.
  Das konnte nach einem unterbrochenen Schreibvorgang die letzten
  verbliebenen Frame-Daten entfernen. Sie werden jetzt erhalten und als
  Orphan-Bytes gezählt.
- Foto- und GIF-Uploads werden zuerst in temporäre Dateien geschrieben und
  nach Validierung umbenannt.
- /api/status meldet LittleFS-Gesamt-, belegte und freie Bytes,
  Fotobibliotheksgröße, verwaiste Frame-Bytes, laufende Upload-Temp-Dateien,
  Konfigurationsgröße und die Größe einer vorhandenen
  config.json.tmp.
- Beim Start werden keine versteckten Dateien automatisch entfernt.
  Übriggebliebene Upload-Temp-Dateien bleiben erhalten und werden in
  /api/status gezählt. Erst wenn der Nutzer einen neuen Upload startet,
  werden die dafür vorgesehenen Temp-Pfade ersetzt.

## 5. Speicherdiagnose

/api/status enthält:

**RAM:** freier Heap, größter zusammenhängender Block, bisheriges Minimum
für Heap und Largest-Block sowie Phasenmessungen für Theme-Parse,
Theme-Render, Theme-Daten, Web-Request, Fotoverarbeitung und
Settings-Speicherung.

**LittleFS:** Gesamtkapazität, belegter und freier Speicher, Foto-Bytes,
verwaiste Frame-Bytes, temporäre Upload-Bytes, Größe der Konfigurationsdatei
und Größe einer vorhandenen Konfigurations-Temp-Datei.

Die technische Detailansicht der WebUI stellt diese Werte dar. Die
Phasenmessungen erfassen den Heap vor und nach dem Vorgang; sie ersetzen
keine hochfrequente Spitzenwertmessung während eines Vorgangs.

## 6. Automatisierte Tests

Auf dem Audit-Branch, Commit
c4a0afeb05eeda7a6d112c4730070c9802dec117, war die
GitHub-Actions-Firmware-Pipeline vollständig erfolgreich.

Ausgeführt und erfolgreich:

- WebUI-/gzip-Konsistenzprüfung;
- PlatformIO-Build smalltv_settings (ESP8266);
- PlatformIO-Build smalltv_loader;
- Theme-Paket- und Theme-Parser-Tests;
- WebUI-Theme- und Pollingtests;
- deterministische Parser-Fuzztests mit ASan und UBSan;
- AtomicJson-Tests für Temp-Open-, Short-Write-, Flush-, Rename- und
  Document-Overflow-Fehler sowie Erhalt der alten Konfiguration;
- 16-Schritte-Showflow-JSON speichern und erneut laden;
- Status-JSON-Tests (vier Antworten, bytegenauer Vergleich);
- 32-Bit-Heap-Allokationstest für die Statusantworten;
- Web-Request-Guard-Tests;
- App-Slot-Größenprüfung mit mindestens 8.192 Byte Reserve.

Die Firmware-Builds und Hosttests wurden in CI ausgeführt, nicht auf einem
physischen Gerät.

## 7. Noch erforderliche Hardwaretests

Vor Freigabe für den Dauerbetrieb sind auf einem ESP8266 SmallTV Ultra
mindestens folgende Tests durchzuführen:

1. sizeof(Settings) und /api/status-Heapwerte am Gerät dokumentieren.
2. Einstellungen mit Theme aktiv speichern, einschließlich 16 Showflow-
   Schritten und maximaler Theme-Alias-Liste.
3. Mindestens 100 wiederholte Speichervorgänge; Heap-Minimum und größten
   freien Block auf Fragmentierung beobachten.
4. Konfiguration speichern und neu starten; alle WLAN-, Theme-, Foto-,
   GIF- und Showflow-Einstellungen erneut laden und vergleichen.
5. LittleFS gezielt bis knapp unter den notwendigen Temp-Dateiplatz füllen:
   Fehlerantwort prüfen und anschließend die alte Konfiguration laden.
6. Stromunterbrechung beim Schreiben der Temp-Datei sowie direkt vor und
   nach dem Rename simulieren; keine beschädigte oder verlorene
   Konfiguration zulassen.
7. JPEG-Upload sowie GIF-Upload mit mehreren Frames bis zum 12-Frame-Limit
   bei knappem Heap und knappem LittleFS testen.
8. GIF-Wiedergabe mit Webinterface-Requests und Theme-/Showflow-Wechsel
   kombinieren; Resets, WDTs und Heap-Fragmentierung beobachten.
9. Einen abgebrochenen Animationsimport simulieren und verbliebene Frames
   als Orphan-Bytes kontrollieren.
10. Mount-Fehler / beschädigte FS-Metadaten kontrolliert simulieren und
    verifizieren, dass normaler Start niemals formatiert.
11. 24-Stunden-Stabilitätstest mit Webinterface, Fotos, GIFs, Themes und
    aktiviertem Showflow.

## 8. Plattformabdeckung und Einschränkungen

Der tatsächlich vorhandene PlatformIO-Build in diesem Repository enthält
ESP8266-Targets (smalltv_settings und smalltv_loader). Die LittleFS-
Mount-Änderung enthält zusätzlich den ESP32-Aufruf mit
formatOnFail=false; ein ESP32-PlatformIO-Build-Target ist in der aktuellen
Projektkonfiguration jedoch nicht vorhanden und konnte deshalb in dieser
CI nicht verifiziert werden.

Die ursprüngliche Fehlermeldung eines konkreten Geräts lässt sich ohne
Fehlerzeitpunkt-Snapshot nicht rückwirkend eindeutig einem Heap- oder
LittleFS-Fehler zuweisen. Der Branch sorgt dafür, dass ein erneuter Fehler
eine konkrete Stufe und die relevanten Heap-/LittleFS-Werte meldet.
