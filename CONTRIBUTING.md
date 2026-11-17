# Mitwirken

Danke für dein Interesse an SmallTV Ultra Mod. Beiträge sollten nachvollziehbar, eng umrissen und mit der bestehenden Firmware kompatibel sein.

## Vor einer Änderung

1. Prüfe vorhandene Issues und Dokumentation, damit keine Arbeit doppelt entsteht.
2. Beschreibe größere Änderungen zuerst in einem Issue.
3. Arbeite möglichst auf einem eigenen Branch.
4. Halte Änderungen thematisch zusammen und vermeide unnötige Umformatierungen großer Firmware-Dateien.

## Entwicklungsumgebung

Benötigt werden Python 3 und PlatformIO Core. Der erste Build lädt Framework und Bibliotheken aus dem Internet. Für Änderungen an der Weboberfläche können zusätzliche Node.js-Prüfungen erforderlich sein.

## Build und Prüfungen

Vor einem Pull Request mindestens die zur Änderung passenden Prüfungen ausführen:

```bash
python tools/gzip_webui_settings.py --check
python -m platformio run -e smalltv_settings
```

Die Hosttests und zusätzlichen CI-Prüfungen sind im Workflow unter `.github/workflows/build.yml` definiert. Wenn du die Weboberfläche änderst, prüfe außerdem, ob der generierte Header aktualisiert werden muss:

```bash
python tools/gzip_webui_settings.py
```

Führe anschließend den Build-Check erneut aus.

## Hinweise für Firmware-Änderungen

- Behalte die begrenzten Flash-, RAM- und LittleFS-Ressourcen des ESP8266 im Blick.
- Vermeide lange blockierende Operationen, insbesondere bei Datei-Uploads und Display-Rendering.
- Berücksichtige Konfigurationswerte beim Laden, Speichern und Importieren.
- Ändere Theme-, Foto- oder Konfigurationsformate nicht ohne Dokumentation und passende Tests.
- Prüfe Auswirkungen auf `smalltv_settings` und `smalltv_loader`; nicht jede Build-Option ist in beiden Umgebungen aktiv.
- Checke keine Passwörter, Gerätekonfigurationen oder WLAN-Zugangsdaten ein.
- Behaupte keinen Hardwaretest, wenn nur ein Host- oder CI-Build ausgeführt wurde.

## Pull Requests

Ein guter Pull Request enthält:

- eine kurze Problembeschreibung und Motivation,
- eine Zusammenfassung der technischen Änderung,
- Angaben zu ausgeführten Builds und Tests,
- bei Änderungen an der Weboberfläche möglichst Vorher-/Nachher-Screenshots,
- bekannte Einschränkungen oder erforderliche Hardwaretests.

Bitte mische keine unabhängigen Refactorings mit funktionalen Änderungen. Das erleichtert Review, Fehlersuche und Rücknahme.

## Lizenz und Herkunft

Beiträge unterliegen der Repository-Lizenz. Bestehende Lizenz- und Copyright-Hinweise in übernommenen Dateien müssen erhalten bleiben.
