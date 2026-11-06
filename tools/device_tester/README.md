# SmallTV Ultra Hardware-Testbench für Windows

Die Testbench arbeitet **mit dem echten SmallTV Ultra**. Sie führt keine
Firmware-Emulation aus. Die `.bin` wird entweder über die OTA-Route der Firmware
oder seriell mit esptool auf das ESP8266 geschrieben. Funktionsprüfungen laufen
gegen die echten Geräte-APIs.

## Start

1. Python 3.10 oder neuer installieren (Windows `py -3` muss funktionieren).
2. Einmal `Install-Requirements.bat` starten.
3. `Start-SmallTV-Tester.bat` starten.
4. SmallTV Ultra einschalten und mit demselben WLAN wie den PC verbinden. In der
   Testbench „Gerät suchen“ drücken. Sie fragt den HTTP-mDNS-Dienst ab und prüft
   zusätzlich bereits bekannte Windows-ARP-Geräte, ohne IP-Bereiche zu scannen.
   Bei Setup-Hotspot auf „Einrichtungs-Hotspot verwenden“ klicken. Alternativ
   eine IP aus dem Router eintragen. Digest-Zugangsdaten ergänzen, falls nötig.

Die GUI kann:

- Geräte-Status, Konfiguration, Theme-Liste und Fotobibliothek auslesen.
- Eine passende `.bin` per OTA nach `/update` oder über USB/COM flashen.
- Serielle Boot-/Diagnosemeldungen mit 115200 Baud anzeigen.
- `.stheme` an `/api/themes/install` senden; die Firmware validiert das Paket.
  Anschließend prüft die Testbench Listeneintrag, Preview-Endpoint und optional
  die Aktivierung.
- Ein Bild mittig auf 240×240 zuschneiden, als JPEG an `/api/photos/upload`
  senden und die bestätigte Foto-ID in der Gerätebibliothek nachprüfen.
- Foto-Wiedergabe und Theme-Playlist nach bewusster Bestätigung gemeinsam
  aktivieren. Die Firmware zeigt Fotos zwischen den Themes einer Playlist.
- Die echte Geräte-WebUI zum manuellen Testen der übrigen Funktionen öffnen.

## Flashen und Daten

- **OTA** setzt einen erreichbaren Webserver, freie OTA-Sketch-Partition und ein
  OTA-kompatibles Image voraus. Ein Upload ohne `OK` wird als Fehler gemeldet.
- **USB/COM** nutzt das im lokalen PlatformIO-Setup vorhandene esptool. Das
  Schreiben beginnt bei `0x0`. Flash löschen ist standardmäßig ausgeschaltet.
  Wenn aktiviert, löscht es auch WLAN-Einstellungen, installierte Themes und
  Fotos; die GUI fragt dafür separat nach.
- Zum Flashen muss der SmallTV im seriellen Flash-Modus sein. Falls die
  automatische Reset-Leitung nicht greift, BOOT/FLASH beim Einschalten halten.
- Nach OTA startet das Gerät neu; danach erneut verbinden und den API-Smoke-Test
  ausführen. Für USB-Flash muss der serielle Monitor beendet sein.
- API-Smoke-Test verändert keine Einstellungen. „Foto-Diashow aktivieren“
  verändert `photoPlayback`, `themePlaylist`, `themeId` und den Anzeigemodus.
  Zeitgesteuerte Playlist-Einträge blockieren die Ein-Klick-Aktivierung, weil
  die Firmware in diesem Modus keine Foto-Batches zwischen Themes startet. Die
  vorhandenen Zeitpläne bleiben dabei unverändert.
- Logs liegen in `%LOCALAPPDATA%\SmallTVHardwareTester\logs`.
- Die zuletzt verwendete Geräteadresse wird lokal in
  `%LOCALAPPDATA%\SmallTVHardwareTester\device.json` gespeichert; Passwörter
  werden nicht gespeichert.

Das Werkzeug ist auf ESP8266/SmallTV Ultra eingestellt. Es ersetzt keine
Langzeit-, Heap-, Display-Farb-, WLAN-Roaming- oder Stromversorgungsprüfung;
solche Eigenschaften hängen vom realen Gerät ab.

Für diese Firmware ist normalerweise `.pio/build/smalltv_settings/firmware.bin`
das passende Image für den SmallTV Ultra. Verwende kein Loader-Image für den
OTA-Test.
