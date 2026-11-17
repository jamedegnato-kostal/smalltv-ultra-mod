---
title: Firmware installieren
description: Settings Firmware auf einem SmallTV oder SmallTV Ultra mit ESP8266 installieren.
---

## Passendes Image

Für SmallTV und SmallTV Ultra mit ESP8266 ist das Hauptimage
`smalltv-settings-firmware.bin` vorgesehen. Verwende es nicht auf ESP32- oder
ESP32-C2-Geräten. Vor dem Wechsel empfiehlt sich ein Export der Einstellungen
und eine Sicherung der installierten Firmware, falls du zurückkehren möchtest.

Das Hauptimage liegt nach einem lokalen Build in
`.pio/build/smalltv_settings/firmware.bin`. Build-Artefakte und Releases
verwenden den Dateinamen `smalltv-settings-firmware.bin`.

## Installation oder Update

Wenn die aktuell laufende Firmware einen Web-Updatebereich anbietet:

1. Öffne die lokale Geräte-Weboberfläche.
2. Gehe zu **System** und wähle den Firmware-Upload.
3. Lade das Settings-Image hoch und warte, bis das Gerät neu startet.

Unterbrich die Stromversorgung während des Schreibens nicht. Ein serielles
Flashen ist eine weitere Möglichkeit, erfordert aber einen 3,3-V-UART-Adapter,
den richtigen Flash-Modus und die zum Gerät passende Flash-Konfiguration.

## Hinweis zum SmallTV Ultra

Die verfügbare Updatefläche hängt vom aktuell installierten Flashlayout ab.
Wenn das vollständige Image abgelehnt wird, kann das separate Loader-Image bei
bestimmten Installationswegen helfen. Der Loader ist **kein normales
Firmware-Update** und wird nicht als reguläres Release-Asset veröffentlicht.
Er kann bei Bedarf lokal gebaut werden:

```sh
python -m platformio run -e smalltv_loader
```

Das Ergebnis liegt unter
`.pio/build/smalltv_loader/firmware.bin`. Verwende es nur, wenn der
gerätespezifische Installationsweg dies ausdrücklich verlangt.

## Beim ersten Start

Ohne gespeichertes WLAN startet die Firmware den Zugangspunkt
`SmallTV-Setup`. Verbinde dich damit und öffne `http://192.168.4.1`, um ein
2,4-GHz-WLAN einzurichten. Anschließend erreichst du die Oberfläche über die
IP-Adresse des Geräts. Weitere Schritte stehen im
[Einrichtungsleitfaden](/manual/settings/).
