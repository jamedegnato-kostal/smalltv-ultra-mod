# SmallTV Ultra – Speicher-Audit

**Stand:** 18. November 2026  
**Analysierter Stand:** `main`, Commit `a413e0b0e0e9673268f79523e85e98a03b8a1bb6`  
**Arbeitsbranch:** `audit/memory-footprint-2026-11`

## Messbare Ausgangsbasis

Der letzte erfolgreiche GitHub-Actions-Lauf vor diesem Branch ist
[Firmware build #111](https://github.com/jamedegnato-kostal/smalltv-ultra-mod/actions/runs/44268403847).

| Größe | ESP8266 Settings | Loader |
| --- | ---: | ---: |
| Statisches RAM laut PlatformIO | 58.624 / 81.920 B (71,6 %) | 28.824 / 81.920 B (35,2 %) |
| Firmware-/App-Slot laut Größenprüfung | 886.128 B | 315.920 B |
| Freier App-Slot | 158.336 B | 728.544 B |

Das sind Linker-/Buildwerte, keine Messung des freien Laufzeit-Heaps,
größten Heap-Blocks oder freien LittleFS. Das Linkerskript ist
`eagle.flash.4m1m.ld`; LittleFS ist vom App-Slot getrennt.

## Befunde

### RAM / Heap

1. **Settings-POST hat mehrere aktive Strukturen.** Der Webserver hält den
   Request-Body, ArduinoJson parst ihn in ein Dokument, die Transaktion legt
   eine vollständige `Settings`-Stagingkopie auf dem Heap an und
   `saveSettings()` serialisiert die Konfiguration in ein weiteres
   ArduinoJson-Dokument. Das Request-Dokument wird vor dem Speichern explizit
   mit `doc.clear()` freigegeben; die Stagingkopie bleibt während des
   persistenten Schreibens bestehen. Das ist ein relevanter Peak-Verbraucher,
   besonders mit festen Theme-Alias- und Showflow-Arrays.

2. **Die Settings-Stagingkopie konnte an einem zu kleinen Heap-Block scheitern.**
   `applyAndSaveSettings()` benötigt einen zusammenhängenden Block von
   `sizeof(Settings)` Byte. Der vorherige Theme-Request-Guard verlangte nur
   3.072 Byte Largest-Block. Bei einer größeren Settings-Struktur konnte
   `appRelease()` deshalb das Theme behalten, obwohl die Transaktionskopie
   nicht mehr in einen einzelnen Block passte. Der Guard wird nun aus
   `sizeof(Settings)` plus Allokatorreserve abgeleitet. Zusätzlich prüft die
   Transaktion, ob String-Mitglieder beim Kopieren vollständig erhalten
   blieben; String-Allokationsfehler können sonst trotz erfolgreichem
   `new Settings` leere Kopien erzeugen.

4. **Der bisherige Fehlertext fasste Ursachen zusammen.** `false` konnte
   sowohl bei nicht erfolgreicher Staging-Allokation als auch bei
   fehlgeschlagener Persistierung entstehen. Der Branch unterscheidet diese
   Fälle sowie JSON-Overflow, Temp-Open, Short Write, Flush, Größenprüfung
   und Rename.

3. **Der Netzwerk-Fingerprint allokierte temporäre Strings.** SSIDs,
   Passwörter und Hostname wurden vor/nach POST verkettet. Der Branch ersetzt
   dies durch einen allocation-freien 64-Bit-Fingerprint.

5. **Heap-Telemetrie existierte bereits.** `/api/status` liefert freien Heap,
   größten zusammenhängenden Block, Heap-/Block-Minima und Phasenwerte für
   Theme-Parse, Theme-Render, Theme-Daten, Web-Request und Foto. Neu ist
   `settingsSave`; Upload/Commit werden ebenfalls der festen Photo-Phase
   zugeordnet. Die Messstrukturen selbst allozieren keinen Heap.

6. **Theme-/JPEG-Wiedergabe lädt Medien nicht komplett in den ESP8266-Heap.**
   Themes werden abschnittsweise aus LittleFS gelesen. JPEG wird direkt aus
   LittleFS bzw. einem PGA1-Offset decodiert. Für Animationen hält die
   Firmware eine feste Tabelle mit maximal zwölf Frame-Metadaten; JPEG-
   Payloads bleiben im Flash-Dateisystem.

7. **GIF-Import benötigt vor allem Browser-RAM.** Bis zu zwölf 240×240-RGBA-
   Snapshots entsprechen rund 2,64 MiB Browser-RAM (230.400 Byte pro Bild),
   zuzüglich Encoder-/Blob-Puffern. Die Snapshots werden nicht zum ESP8266
   übertragen: PGA1 wird in Upload-Chunks direkt in eine temporäre LittleFS-
   Datei geschrieben. Der Branch reduziert weder JPEG-Qualität noch
   Frame-Anzahl; das wäre erst nach visuellem Qualitätsvergleich vertretbar.

### Flash / LittleFS

1. **Atomare Speicherung braucht vorübergehend zwei Konfigurationsdateien.**
   `AtomicJson.h` schreibt `/config.json.tmp`, prüft Schreibanzahl, Flush
   und endgültige Größe und veröffentlicht erst danach per Rename. Die alte
   `/config.json` bleibt bis zum erfolgreichen Rename bestehen. Diesen
   Sicherheitsgewinn nicht durch vorheriges Löschen der Zieldatei aufgeben.

2. **Fehlgeschlagenes Speichern bedeutet nicht automatisch „LittleFS voll“.**
   Mögliche Ursachen sind Platzmangel, fehlgeschlagener Open, Short Write,
   Flush-/Flashfehler oder Rename-/Metadatenfehler. Der Branch meldet die
   konkrete Stufe; `/api/status` liefert FS-Kapazität, belegt/frei,
   Fotodaten, verwaiste Frame-Daten, Config-Größe und Größe einer ggf.
   vorhandenen `config.json.tmp`.

3. **Verwaiste Animationsframes wurden automatisch gelöscht.**
   `PhotoLibrary::refresh()` entfernte `pNNNfNN.jpg`, wenn die
   `.anim`-Metadatei fehlte. Nach einer unterbrochenen Speicherung können
   diese JPEGs die einzigen verbliebenen Bilddaten sein. Der Branch bewahrt
   sie und zählt ihre Bytes separat. Das ist konservativer, kann aber
   ungenutzten LittleFS-Platz binden.

4. **Upload-Temp-Dateien.** Foto-/GIF-Uploads landen zunächst in versteckten
   Temp-Dateien und werden erst nach Validierung umbenannt. Fehlerpfade
   entfernen unvollständige Uploads; ein Stromausfall kann Reste hinterlassen.
   `config.json.tmp` wird nicht automatisch als gültige Konfiguration
   übernommen.

5. **Weboberfläche.** Die HTML-Quelle wird zu `webui_settings.h` gzip-
   komprimiert und liegt in PROGMEM. Die Größe des generierten C-Headers ist
   nicht mit Laufzeit-Heap gleichzusetzen.

## Ursache des Fotoshow-Speicherfehlers

Der Quellcode allein beweist **noch nicht**, ob ein konkreter Geräte-Save an
vollem LittleFS oder an RAM-Mangel scheiterte. Die frühere HTTP-Antwort
unterschied diese Ursachen nicht; historische Heap-/FS-Werte zum Zeitpunkt
des Fehlers liegen nicht vor. Ein nachträglicher Ursachennachweis ohne
reproduzierten Fehler oder Geräte-Log wäre Spekulation.

Nach diesem Branch wird die nächste Fehlermeldung einem der Pfade zugeordnet:
Staging-RAM, ArduinoJson-Overflow, Temp-Open/Short-Write, Flush,
Dateigröße oder atomarer Rename. Die alte Konfiguration bleibt bei
fehlgeschlagenem Schreiben/Rename erhalten; aktive Settings werden nur nach
erfolgreicher Persistierung übernommen.

## Änderungen dieses Branches

- Detaillierte AtomicJson-Ergebnisse; der bool-Kompatibilitätswrapper bleibt.
- Unterschiedliche HTTP-Fehler für RAM-Staging und Persistenz.
- Allocation-freier Netzwerk-Fingerprint.
- Heap-Phase `settingsSave` und Photo-Phase für Upload/Commit.
- LittleFS-, Foto-, Orphan- und Config-Temp-Daten in `/api/status`.
- Keine automatische Löschung verwaister Animationsframes.
- Hosttests für AtomicJson-Fehlerklassen und maximales 16-Schritte-Showflow-JSON.

## Noch erforderliche Hardwaretests

- Heap und größter Block vor/nach Settings-Save mit Theme, 16 Showflow-
  Schritten und maximalen Theme-Alias-Einträgen.
- Mindestens 100 wiederholte Saves; Heap-Minimum und Largest-Block-Trend
  beobachten.
- LittleFS knapp unter/über dem Platzbedarf der Temp-Datei: Fehlerantwort
  und anschließendes Lesen von `config.json` prüfen.
- Stromunterbrechung während des Temp-Schreibens sowie vor/nach Rename.
- JPEG-/GIF-Upload bis zum 12-Frame-Limit bei knappem Heap und LittleFS.
- Foto-/GIF-Wiedergabe während Theme-/Showflow-Wechsel.
- Unterbrochenen Animationsimport simulieren; Frames müssen erhalten bleiben
  und als Orphan-Bytes erscheinen.
- 24-Stunden-Test mit Webinterface, Fotos, GIFs, Themes und Showflow.

`/api/status` ist die Quelle für Laufzeit-Heap, Largest Block, Minima,
Phasen und LittleFS. CI-Builds ersetzen diese Hardwaretests nicht.
