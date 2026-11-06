---
title: Weboberfläche und Einstellungen
description: Die Bereiche im SmallTV Settings-Menü und ihre Funktionen.
---

Die Oberfläche erreichst du über die IP-Adresse des Geräts. Änderungen an den Formularen werden erst mit **Änderungen speichern** übernommen. Theme- und Fotodateien werden separat über ihre jeweiligen Aktionen installiert oder verwaltet.

## Übersicht

Zeigt WLAN-Status, lokale Adresse, Laufzeit, aktives Theme, Helligkeit, Uhr-/NTP-Status und technische Details. Die Schnellzugriffe öffnen passende Bereiche.

## Anzeige

- Helligkeit von 0 bis 100 Prozent, Displayrotation und invertierte Backlight-Ansteuerung.
- Farbreihenfolge RGB/BGR/automatisch, Farbinvertierung, getrennte RGB-Verstärkung, Kontrast und Sättigung.
- Eigene Theme-Hintergrundfarbe und globale Theme-Layer-Transparenz.
- ST7789-Panelrate von 39 bis 60 Hz und Renderlimit von 1 bis 15 FPS.
- Globale Theme-Animation, Animationsgeschwindigkeit und optionaler Fade beim Wechsel.

Es gibt in diesem Build keinen Lichtsensor-Modus.

## Uhr & Zeit

Die Firmware synchronisiert die Uhr über NTP, sobald die Netzverbindung verfügbar ist. In diesem Bereich wählst du Zeitzone, 12-/24-Stunden-Format, Sekunden- und Datumsanzeige sowie ein Format für das `{DATE}`-Token. Der konkrete Aufbau der Uhr bleibt Teil des aktiven Themes. Installierte Uhr-Themes werden hier separat nach Kategorie angezeigt und können dort installiert, angesehen, aktiviert oder gelöscht werden.

Der Nachtmodus kann die Displayhelligkeit in einem täglichen Zeitfenster senken oder ganz abschalten. Das Gerät wartet vor dem Eintritt auf eine bestätigte Uhrzeit. Zeitfenster der Theme-Playlist beziehen sich auf die lokale Zeitzone.

## Themes

Die Unterbereiche **Bibliothek** und **Automatik** enthalten die Paketverwaltung bzw. Playlist. Die Bibliothek bietet Suche, Kategorien, Vorschau, Metadatenbearbeitung, Installation, Auswahl und Löschen. In der Automatik werden bis zu acht Themes mit individueller Laufzeit sortiert; Zufall und Tageszeitfenster sind optional.

Ein einzeln ausgewähltes Theme dient als Anzeige, wenn die Playlist deaktiviert ist, und als Rückfall bei nicht ladbaren Playlist-Einträgen. Theme-Export aus dem Gerät ist nicht enthalten.

## Fotos

Wähle eine oder mehrere Bilddateien aus, um die Gerätebibliothek zu füllen. Die Seite zeigt Anzahl, Dateigrößen und freien Speicher; Upload kann abgebrochen werden. Einzelne und alle Fotos lassen sich löschen. Die Wiedergabeparameter gelten für Foto-Batches zwischen Einträgen einer aktiven Playlist.

## Netzwerk

Scan nach 2,4-GHz-WLANs, Zugangsdaten speichern und Hostname festlegen. Zusätzlich kannst du den Setup-/Fallback-Hotspot benennen und optional absichern. Nach WLAN-Änderungen startet das Gerät gegebenenfalls neu, damit die neue Verbindung aktiv wird.

## System

Konfiguration exportieren oder importieren, manuell ein Firmwareimage hochladen, Update-Check ausführen, Neustart anfordern oder Werkseinstellungen wiederherstellen. Der optionale Update-Check lädt Releaseinformationen aus der in `src/config.h` eingetragenen Repository-Adresse. Ein manueller Upload benötigt ein zum ESP8266 und zum verfügbaren OTA-Slot passendes Image.

## Erweitert

Digest-Passwortschutz und unterstützte Tasteraktionen werden in diesem Bereich verwaltet. Ein Firmware-Reset oder Neu-Flashen kann nötig sein, wenn ein gesetztes Passwort verloren geht; es gibt keinen separaten Passwort-Resetdienst.

## Authentifizierung und lokale Sicherheit

Passwortschutz ist standardmäßig aus. Ohne Passwort kann jedes Gerät im gleichen erreichbaren Netzwerk die Oberfläche bedienen. Schalte Authentifizierung ein, wenn weitere Personen oder Geräte Zugriff auf das WLAN haben. Die Firmware spricht HTTP, nicht HTTPS. Digest verhindert, dass das Passwort im Klartext übertragen wird, verschlüsselt aber weder Konfiguration noch Uploads.
