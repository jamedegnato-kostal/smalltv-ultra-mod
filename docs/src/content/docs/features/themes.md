---
title: Themes, Uhr-Designs und Playlist
description: .stheme installieren, konfigurieren und auf dem ESP8266 anzeigen.
---

Ein Theme ist ein Paket aus Manifest und Assets. Es wird im Webinterface hochgeladen, vor der Installation geprüft und bleibt als Datei getrennt von der Firmware erhalten. Ein Paket führt keinen eigenen Programmcode aus.

## Themes verwalten

Unter **Themes** lassen sich allgemeine Themes suchen, nach Kategorien filtern, als Karten mit Metadaten/Vorschau ansehen, installieren, auswählen, bearbeiten oder löschen. Der Anzeigename kann geändert werden, ohne die interne ID zu ändern. Doppelte IDs werden abgewiesen; identische Anzeigenamen sind möglich.

Uhr-Designs werden nicht in einem zweiten Dateispeicher abgelegt. Sie liegen mit den übrigen Paketen im gemeinsamen Theme-Verzeichnis und werden anhand der Kategorie `clock` unter **Uhr & Zeit → Uhr-Designs** gefiltert. Ein Uhr-Theme kann unabhängig davon zur Playlist hinzugefügt werden.

Exportieren von Themes aus der Firmware ist nicht vorgesehen.

## Unterstützte Inhalte

- Text, Bitmap-Bilder, Formen und Sprite-Animationen in geordneter Layer-Reihenfolge.
- Transparente Bildebenen, globale Ebenendeckkraft, Theme-Hintergrund und RGB565-Farbwerte.
- Textvorlagen mit Zeit-/Datumstokens und Feldern aus einer Theme-Datenquelle.
- Zahlen- und Farbbindungen, Scrollen sowie eingebaute Zeitwerte für dynamische Anzeigen.
- Optionale Vorschaudatei und Metadaten: Name, Beschreibung, Autor, Version und Kategorie.
- Optionale PNG-Bitmap-Schriftatlanten. TTF- und OTF-Schriften werden nicht auf dem Gerät geladen.

### Layer-Typen

| Typ | Inhalt |
| --- | --- |
| text | Text, Tokens, Bitmap-Schrift und optionaler Lauftext |
| rectangle | Rechteck mit optionaler Füllung, Kontur und abgerundeten Ecken (`cornerRadius`) |
| circle | Kreis mit Füllung oder Kontur |
| line | Linie mit Farbe und Stärke |
| image | Einzelnes STI-Bild oder PNG-Atlasbild |
| animation | Sprite-Animation mit begrenzter Framerate |

### Lauftext und dynamische Werte

Text kann über `scroll.width` ein festes Anzeigefenster erhalten. `scroll.speed`
bestimmt die Bewegungsgeschwindigkeit; `"mode": "loop"` wiederholt den Text
fortlaufend und `"mode": "bounce"` lässt ihn an den Fensterenden umkehren.
Pausen und Abstand werden im Theme-Manifest festgelegt.

Zahlen- und Farb-Bindings lesen Werte aus den deklarierten `data`-Quellen. Der
Desktop-Vorschaulauf kann mit `--data` Beispielwerte bekommen. Das Beispiel
`live-status` zeigt eine solche dynamische Anzeige. Fehlt ein Wert oder ist er
ungültig, bleibt der statische Layerwert erhalten.

Farb-Bindings können eine Folge von **color stops** verwenden: Ein Quellwert
wird zwischen den benachbarten Farbstufen interpoliert. Lege die Stopps in
aufsteigender Reihenfolge an; Werte außerhalb des Bereichs erhalten die
nächstgelegene Randfarbe.

Die Theme-Engine unterstützt Paketversionen 1 und 2. Version 2 fügt unter anderem zeilenweise komprimierte STI2-Bilder und zusätzliche Bindungs-/Formoptionen hinzu. Für die genauen Felder und Grenzen sind die Beispielmanifeste in `examples/themes/` und die Parserdefinition in `src/features/theme/ThemeEngine.cpp` maßgeblich.

## Ressourcenlimits

- Höchstens 16 installierte Themes im Webkatalog.
- Höchstens 3 MiB und 256 Dateien pro Paket.
- Höchstens 32 Layer in einem Manifest.
- Animationsassets dürfen bis zu 240 Frames enthalten; die deklarierte Framerate ist auf 1–15 FPS begrenzt.
- Das Gerät validiert Manifest, Assetpfade, Bilddaten und Dateigrößen. Der freie LittleFS-Speicher begrenzt zusätzlich, wie viele große Pakete installiert werden können.

Die Weboberfläche kann Animationen global abschalten, ihre Geschwindigkeit und das Renderlimit anpassen und optional einen Fade-Übergang beim Theme-Wechsel aktivieren. Ein individuelles Animationsprofil pro Paket gibt es nicht.

## Theme-Playlist

Unter **Themes → Automatik** können bis zu acht installierte Pakete geordnet werden. Jeder Eintrag hat eine Anzeigedauer von 5 bis 3600 Sekunden. Zufallsmodus vermeidet direkte Wiederholungen, wenn mehrere Einträge verfügbar sind. Ein optionales tägliches Zeitfenster verwendet die eingestellte Zeitzone und benötigt NTP.

Außerhalb aller aktiven Zeitfenster wird das ausgewählte Einzel-Theme als Rückfallanzeige genutzt. Fehlende oder nicht ladbare Playlist-Einträge werden übersprungen. Ein manuelles Aktivieren eines einzelnen Themes bleibt jederzeit möglich.

Für Uhr-Themes lässt sich unter **Themes → Automatik** eine Standarddauer einstellen und auf vorhandene Uhr-Einträge anwenden. Die Einstellung beeinflusst die Dauer von Uhr-Layouts in der Playlist, nicht die Länge einzelner Animationsframes.

## Fotos zwischen Themes

Die Foto-Bibliothek ist ein eigener WebUI-Bereich. Sie unterstützt höchstens 240 JPEGs mit maximal 128 KiB pro Upload. Der Browser schneidet Bilder mittig auf 240 × 240 Pixel zu und codiert sie vor dem Transfer. Der Firmware-Upload ist gestreamt, validiert das JPEG vollständig und hält eine LittleFS-Reserve frei.

Wenn Foto-Wiedergabe aktiv ist, wird nach einem Playlist-Theme ein Shuffle-Batch gezeigt. Es kommen keine Fotos, wenn keine aktive Theme-Playlist läuft. Der Shuffle-Zyklus versucht, alle verfügbaren Fotos einmal zu zeigen, bevor er neu beginnt, und vermeidet eine Wiederholung am Zykluswechsel.

## Beispiele und Werkzeuge

Im Ordner `examples/themes/` liegen installierbare Pakete, Quellmanifeste und Offline-Vorschauen. Dazu gehören eine dynamische Analoguhr und ein Wetter-Theme für Bellmerei, das aktuelle Wetterdaten von Open-Meteo abruft. Diese Pakete lassen sich über die Theme-Verwaltung in der WebUI importieren; sie werden nicht automatisch in den LittleFS-Speicher des Geräts geschrieben. Der Desktop-Packer befindet sich in `tools/theme_pack.py`; er benötigt Pillow und einen C++11-Compiler für die native Validierung. Die Firmware braucht Pillow nicht.

```powershell
python -m pip install -r tools/requirements-theme.txt
python tools/theme_pack.py examples/themes/pixel-room .pio/pixel-room.stheme
```

Die Endpunkte sind unter [HTTP-API](/api/) aufgeführt. Paketprüfungen liegen unter `tests/theme/`.
