---
title: Fehlerbehebung
description: Häufige Probleme mit WLAN, Themes, Fotos und Updates beheben.
---

## Setup-WLAN wird nicht angezeigt

Warte den Start ab und prüfe, ob ein gespeichertes WLAN erreichbar ist. Der
Zugangspunkt `SmallTV-Setup` wird benötigt, wenn kein verwendbares Netz
verbunden ist. Das Gerät unterstützt 2,4-GHz-WLAN.

## Weboberfläche ist nicht erreichbar

Prüfe die angezeigte IP-Adresse und öffne sie mit `http://` auf einem Gerät im
gleichen lokalen Netzwerk. Wenn das Netz nicht erreichbar ist, verbinde dich
mit `SmallTV-Setup` und rufe `http://192.168.4.1` auf.

## Theme fehlt oder startet nicht

Prüfe im Theme-Bereich, ob das Paket installiert und mit diesem Firmwarestand
kompatibel ist. Öffne die Vorschau und aktiviere das Theme testweise direkt.
Eine nicht lesbare oder fehlerhafte Datei wird beim Laden übersprungen. Falls
alle installierten Themes ungültig sind, sollte die Firmware ihre eingebaute
Recovery-Ansicht anbieten.

## Playlist wechselt nicht

Prüfe unter **Themes → Automatik**, ob die Playlist aktiviert ist und gültige
Themes enthält. Zeitfenster benötigen eine synchronisierte Uhr und beziehen
sich auf die konfigurierte Zeitzone. Entfernte Themes werden beim Abspielen
übersprungen.

## Foto-Upload schlägt fehl

Die Bibliothek akzeptiert JPEG-Dateien bis zur dokumentierten Größenbegrenzung
und benötigt zusätzlich freien LittleFS-Speicher. Verwende die Auswahl in der
Weboberfläche, die Bilder passend vorbereitet. Wenn der Speicher knapp ist,
lösche nicht mehr benötigte Fotos und lade erneut hoch.

## Firmware-Upload wird abgelehnt

Prüfe, dass das Image für ESP8266 gebaut wurde und ausreichend OTA-Platz
vorhanden ist. Bei manchen Ultra-Flashlayouts kann das bestehende
Erstinstallationsverfahren ein vollständiges Image ablehnen. Der optionale
Loader kann bei diesem speziellen Weg helfen; er ist für normale Updates nicht
erforderlich.

## Gerät startet wiederholt neu

Trenne die Stromversorgung kurz und starte erneut. Wiederholte Bootfehler
sollten die Recovery-Ansicht verfügbar machen. Wenn die Schleife fortbesteht,
notiere die serielle Diagnoseausgabe mit 115200 Baud und installiere ein
bekannt gültiges Image. Ein vollständiger ESP8266-Emulator gehört nicht zum
Projekt.

## Fehler melden

Nutze den Fehlerbereich des lokalen oder später eingerichteten Projekt-Trackers.
Beschreibe Gerät, Firmwareversion, Schritte und Fehlermeldung. Entferne aus
Protokollen und Exporten WLAN-Passwörter und andere Zugangsdaten.
