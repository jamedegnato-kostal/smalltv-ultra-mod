# Hinweise zu Herkunft und Komponenten

Diese Clean-Firmware basiert auf der SmallTV-Firmwarebasis und enthält Beiträge aus dem Projekt [smalltv-mod](https://github.com/giovi321/smalltv-mod). Die ursprüngliche Lizenzdatei liegt unverändert als \`LICENSE\` bei.

Für den Build wird ArduinoJson verwendet. WLAN, DNS, mDNS, HTTP-Server und OTA kommen aus dem ESP8266-Arduino-Core und den dazugehörigen Plattformpaketen. Die konkreten Build-Abhängigkeiten sind in \`platformio.ini\` aufgeführt.

Die frühere Theme-, Grafik-, Bild- und Anzeige-Software ist in diesem Clean-Branch nicht enthalten.
