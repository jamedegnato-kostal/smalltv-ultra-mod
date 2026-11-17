# Mitwirken

Danke für dein Interesse an SmallTV Settings Firmware. Beiträge sollten nachvollziehbar, eng umrissen und mit der bestehenden Firmware kompatibel sein.

## Vor einer Änderung

1. Prüfe vorhandene Issues und Dokumentation, damit keine Arbeit doppelt entsteht.
2. Beschreibe größere Änderungen zuerst in einem Issue.
3. Arbeite möglichst auf einem eigenen Branch.
4. Halte Änderungen thematisch zusammen und vermeide unnötige Umformatierungen großer Firmware-Dateien.

## Entwicklungsumgebung

Benötigt werden Python 3 und PlatformIO Core. Installiere PlatformIO bei Bedarf mit:

```bash
python -m pip install platformio
```

Der erste Build benötigt Internetzugang, damit PlatformIO Framework und Bibliotheken herunterladen kann. Für die Dokumentationsseite wird Node.js 22 verwendet.

## Build und Prüfungen

Vor einem Pull Request mindestens die zur Änderung passenden Prüfungen ausführen:

```bash
python tools/gzip_webui_settings.py --check
python -m platformio run -e smalltv_settings
```

Die Hosttests und zusätzlichen CI-Prüfungen sind im Workflow unter `.github/workflows/build.yml` definiert. Änderungen an der Dokumentationsseite lassen sich so prüfen:

```bash
cd docs
npm ci
npm run build
```

Wenn du die Weboberfläche änderst, muss der komprimierte Header aktualisiert werden:

```bash
python tools/gzip_webui_settings.py
python tools/gzip_webui_settings.py --check
```

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

Dieses Projekt basiert auf [giovi321/smalltv-mod](https://github.com/giovi321/smalltv-mod). Beiträge müssen bestehende Copyright- und Lizenzhinweise erhalten und die Repository-Lizenz beachten. Details stehen in [NOTICE.md](NOTICE.md) und [LICENSE](LICENSE).
