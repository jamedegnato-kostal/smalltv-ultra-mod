---
title: Gerät einrichten
description: WLAN verbinden, die Weboberfläche öffnen und die erste Theme-Playlist einrichten.
---

Beim ersten Start hat das SmallTV noch kein gespeichertes WLAN. Es startet deshalb den Hotspot `SmallTV-Setup`. Die weitere Einrichtung erfolgt im Browser.

## Mit dem WLAN verbinden

1. Verbinde Laptop oder Smartphone mit dem WLAN **SmallTV-Setup**.
2. Öffnet sich kein Einrichtungsfenster automatisch, rufe `http://192.168.4.1` auf.
3. Öffne **WiFi**, suche dein 2,4-GHz-WLAN und trage das Passwort ein.
4. Speichere die Verbindung. Das SmallTV startet neu und verbindet sich mit deinem Netzwerk.
5. Notiere die IP-Adresse, die das Display anzeigt. Öffne diese Adresse im Browser, zum Beispiel `http://192.168.1.42`.

Der ESP8266 unterstützt nur 2,4 GHz. Die IP-Adresse kann sich ändern; wenn dein Router lokale Gerätenamen unterstützt, kannst du auch den angezeigten Hostnamen verwenden.

## Ein Theme installieren

Öffne für allgemeine Themes den Bereich **Themes**. Uhr-Themes installierst und verwaltest du unter **Uhr & Zeit**; sie verwenden das dort gewählte Zeitformat und Datumsformat. Theme-Dateien sind von der Firmware getrennte Inhalte.

## Eine Playlist einrichten

Unter **Themes → Automatik** findest du die Playlist. Füge installierte Themes hinzu, sortiere sie mit den Pfeilen und stelle für jeden Eintrag eine Dauer zwischen 5 und 3600 Sekunden ein. Aktiviere die Playlist und speichere die Konfiguration. Reihenfolge, Aktivierung und Zeiten bleiben nach Neustarts erhalten.

Wenn du die Playlist ausschaltest, wird das einzeln gewählte Theme angezeigt. Nicht mehr vorhandene oder nicht ladbare Playlist-Einträge überspringt die Firmware automatisch.

## Die Oberfläche

- **Status:** WLAN-Verbindung, IP-Adresse, Speicher und Firmware-Informationen.
- **WiFi:** WLANs suchen und speichern sowie Gerätenamen ändern.
- **Anzeige:** Helligkeit, Ausrichtung, Farben und Bildrate einstellen.
- **Uhr & Zeit:** Zeitformat, Zeitzone, Nachtmodus und Uhr-Themes verwalten.
- **Themes:** allgemeine `.stheme`-Pakete verwalten und die automatische Playlist konfigurieren.
- **System:** Konfiguration sichern oder laden, Firmware manuell hochladen und das Gerät neu starten.

Die Oberfläche ist für lokale Verwendung im Heim- oder Büronetz gedacht. Sie benötigt keine Verbindung zu einem Unternehmensnetz, wenn du das SmallTV direkt mit einem eigenen 2,4-GHz-WLAN oder einem Laptop-Hotspot verbindest.
