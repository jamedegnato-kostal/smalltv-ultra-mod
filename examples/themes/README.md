# Beispiel-Themes

Die folgenden Pakete können mit der SmallTV Settings Firmware über **Themes → Bibliothek** installiert werden:

- [`pixel-room.stheme`](pixel-room.stheme) — Pixel-Art-Zimmer mit animierter Katze.
- [`terminal-ops.stheme`](terminal-ops.stheme) — text- und linienbasiertes Dashboard.
- [`live-status.stheme`](live-status.stheme) — dynamisches Dashboard mit deklarativen Datenbindungen.

Die bearbeitbaren Quellen liegen in den gleichnamigen Verzeichnissen. Öffne die Offline-Vorschauen für [Pixel Room](pixel-room-preview.html), [Terminal Ops](terminal-ops-preview.html) und [Live Status](live-status-preview.html). SmallTV Studio wird separat von dieser Firmware gepflegt.

## Validieren, bauen und Vorschau erzeugen

Voraussetzungen: Python 3.10+, Pillow, ein C++11-Compiler und die ArduinoJson-/GFX-Header. Siehe [Theme-Dokumentation](../../docs/src/content/docs/features/themes.md).

```sh
python3 tools/smalltv_theme.py validate examples/themes/pixel-room
python3 tools/smalltv_theme.py build examples/themes/pixel-room examples/themes/pixel-room.stheme
python3 tools/smalltv_theme.py preview examples/themes/pixel-room examples/themes/pixel-room-preview.html --time 2026-09-18T10:24:55 --seconds 10 --fps 15
```

Die Pixel-Room-PNGs lassen sich mit `python3 examples/themes/make_pixel_room_assets.py` neu erzeugen. Terminal Ops kann mit `python3 tools/create_terminal_theme.py` rekonstruiert werden. Die Desktop-Werkzeuge gehören nicht zum installierten Firmware-Image; die Theme-Pakete enthalten deklarative Daten und keine ausführbaren Skripte.

Live Status besitzt keinen mitgelieferten Daten-Endpunkt. Für eine reproduzierbare Offline-Vorschau werden Beispielwerte injiziert:

```sh
python3 tools/smalltv_theme.py build examples/themes/live-status examples/themes/live-status.stheme
python3 tools/smalltv_theme.py preview examples/themes/live-status \
  examples/themes/live-status-preview.html --seconds 8 --fps 15 \
  --time 2026-09-22T12:00:00 \
  --data status.label='SmallTV dynamic dashboard headline' \
  --data status.level=82 --data status.state=Warning
```

Die Endpunkte sind unter [HTTP-API](../../docs/src/content/docs/api.mdx) aufgeführt. Paketprüfungen liegen unter `tests/theme/`.
