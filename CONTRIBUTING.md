# Mitwirken

Danke, dass du die SmallTV Settings Firmware verbessern möchtest. Änderungen
sollen zum ESP8266-Ziel, zum begrenzten Gerätespeicher und zu den Funktionen
passen, die in [der technischen Übersicht](docs/SETTINGS_FIRMWARE.md) beschrieben
sind.

## Lokal bauen

Benötigt werden PlatformIO, Python 3 und Node.js. Der Hauptbuild lautet:

```sh
pio run -e smalltv_settings
```

Der optionale Loader wird separat gebaut:

```sh
pio run -e smalltv_loader
```

## Änderungen prüfen

- Nach Änderungen an `src/webui_settings.html` den eingebetteten Stand mit
  `python tools/gzip_webui_settings.py` neu erzeugen.
- Neue Konfigurationswerte sowohl beim Laden als auch beim Speichern und beim
  Import berücksichtigen.
- Eingaben, Dateigrößen und Schleifen begrenzen; lange Abläufe müssen dem
  ESP8266 Gelegenheit geben, Netzwerk und Watchdog zu bedienen.
- Keine Passwörter, Gerätekonfigurationen oder persönlichen WLAN-Daten
  einchecken.
- Bei Firmwareänderungen den Settings-Build und die betroffenen vorhandenen
  Prüfungen ausführen. Hardwareabhängiges Verhalten zusätzlich auf einem
  SmallTV Ultra verifizieren, wenn das Gerät verfügbar ist.

## Lizenz und Herkunft

Der Beitrag unterliegt der Repository-Lizenz. Bestehende Lizenz- und
Copyright-Hinweise in übernommenen Dateien müssen erhalten bleiben.
