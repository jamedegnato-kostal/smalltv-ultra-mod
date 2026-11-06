---
title: Wiederherstellung
description: Eingebaute Rückfallansicht und sichere Schritte bei Startproblemen.
---

Wenn beim Start wiederholt Fehler auftreten oder kein nutzbares Theme geladen
werden kann, kann die Firmware in eine Recovery-Ansicht wechseln. Das
Recovery-Theme ist in das Image eingebettet und hängt nicht von den Dateien im
LittleFS-Speicher ab.

Die Quelldatei liegt unter `examples/recovery-theme/recovery.stheme`. Änderungen
an ihr müssen mit den Theme-Werkzeugen geprüft und anschließend in einem neuen
Firmware-Build eingebettet werden.

Die Recovery-Ansicht soll den Zugriff auf Einrichtung und Firmware-Upload
ermöglichen. Ein Werksreset löscht Geräteeinstellungen, ersetzt aber nicht das
Firmware-Image. Sichere benötigte Konfigurationen vor einem Reset.

## Komponenten und Herkunft

Diese Firmware baut auf bestehendem SmallTV-Firmwarecode auf. Herkunft und
Lizenzhinweise stehen in `NOTICE.md` und `LICENSE` im Projektstamm. Zusätzliche
Bibliotheken sind in `platformio.ini` aufgeführt und behalten ihre jeweiligen
Lizenzbedingungen.
