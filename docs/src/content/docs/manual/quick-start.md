---
title: Schnellstart
description: SmallTV Settings Firmware mit WLAN verbinden und Themes einrichten.
---

## Gerät verbinden

Schließe das SmallTV an und warte, bis es gestartet ist. Wenn kein gespeichertes
WLAN verfügbar ist, stellt es den Zugangspunkt **SmallTV-Setup** bereit.
Verbinde Laptop oder Smartphone damit und öffne `http://192.168.4.1`.

## WLAN einrichten

Wähle in **Netzwerk** dein 2,4-GHz-WLAN aus, gib das Passwort ein und speichere.
Nach dem Verbindungsaufbau zeigt das Gerät seine Netzwerkadresse an. Öffne
diese Adresse im Browser, während dein Rechner im gleichen Netzwerk ist.

## Theme installieren

Öffne **Themes → Bibliothek**, lade ein `.stheme`-Paket hoch und prüfe die
Vorschau. Ein Theme kann direkt aktiviert oder zur Playlist hinzugefügt
werden. Uhr-Themes verwaltest du im Bereich **Uhr & Zeit**.

## Automatische Wiedergabe

Unter **Themes → Automatik** stellst du eine feste oder zufällige Folge ein.
Jeder Eintrag erhält eine eigene Laufzeit. Zeitfenster richten sich nach der
konfigurierten Zeitzone und benötigen eine synchronisierte Uhr. Fotos können
zwischen Theme-Einträgen der aktiven Playlist gezeigt werden.

## Nützliche Hinweise

- Das Hauptimage `smalltv_settings` ist für SmallTV und SmallTV Ultra mit
  ESP8266 vorgesehen.
- Das optionale Loader-Image wird nur gebraucht, wenn ein bestimmter
  Erstinstallationsweg das Hauptimage nicht annimmt.
- Systemeinstellungen und Theme-/Fotodateien werden getrennt verwaltet.
- Im Bereich **System** kannst du Einstellungen sichern und Firmware manuell
  aktualisieren.
