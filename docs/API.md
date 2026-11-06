# HTTP-API

Die API wird vom lokalen Geräteserver bereitgestellt. Schreibzugriffe laufen durch denselben Origin-/Request-Guard wie die Weboberfläche. Wenn Digest-Authentifizierung eingeschaltet ist, werden Lese- und Schreibzugriffe authentifiziert.

## Status und Konfiguration

| Methode | Pfad | Zweck |
| --- | --- | --- |
| `GET` | `/api/status` | Firmware-, Netzwerk-, Speicher-, Uhr- und Laufzeitstatus |
| `GET` | `/api/config` | gespeicherte Konfiguration; Geheimnisse werden maskiert |
| `POST` | `/api/config` | validierte Änderungen sichern |
| `GET` | `/api/scan` | WLAN-Scan anfordern |
| `POST` | `/api/refresh` | Anzeige-/Datenaktualisierung anfordern |
| `GET` | `/api/export` | Konfiguration als Sicherung exportieren |
| `POST` | `/api/import` | Konfiguration importieren und validieren |
| `POST` | `/api/factory` | gespeicherte Einstellungen zurücksetzen |
| `POST` | `/api/reboot` | Gerät neu starten |

Eine Sicherungsdatei kann WLAN-Zugangsdaten enthalten. Behandle sie wie ein Passwort und teile sie nicht öffentlich.

## Themes

Alle folgenden Routen sind nur verfügbar, wenn die Theme-Funktion im Build aktiviert ist.

| Methode | Pfad | Zweck |
| --- | --- | --- |
| `GET` | `/api/themes` | installierte Theme-Metadaten auflisten; optional nach Kategorie filtern |
| `POST` | `/api/themes/install` | `.stheme` hochladen und validieren |
| `POST` | `/api/themes/delete` | Paket anhand seiner stabilen ID entfernen |
| `POST` | `/api/themes/select` | Rückfall-/Einzelanzeige auswählen |
| `GET` | `/api/themes/preview` | generierte Paketvorschau abrufen |
| `GET` | `/api/themes/layers` | Ebenen des Themes lesen |
| `POST` | `/api/themes/layers/order` | Ebenenreihenfolge speichern |
| `POST` | `/api/themes/metadata` | editierbare Metadaten speichern |
| `POST` | `/api/themes/rename` | Anzeigenamen ändern, ohne die Theme-ID auszutauschen |

Der Installationsendpunkt akzeptiert für Uhr-Dateien zusätzlich `category=clock`. Ein Löschen, Umbenennen oder Bearbeiten bleibt an die interne Paket-ID gebunden.

## Fotos

Diese Routen sind im `smalltv_settings`-Build verfügbar.

| Methode | Pfad | Zweck |
| --- | --- | --- |
| `GET` | `/api/photos` | Bibliothek, Dateigrößen und Speicherstatus abfragen |
| `GET` | `/api/photos/file?id=…` | Vorschaubild einer Foto-ID abrufen |
| `POST` | `/api/photos/upload` | ein JPEG pro Anfrage streamen und validieren |
| `POST` | `/api/photos/delete` | einzelne Foto-ID entfernen |
| `POST` | `/api/photos/delete-all` | gesamte Fotobibliothek löschen |

Mehrfachauswahl im Browser erzeugt mehrere einzelne Upload-Anfragen. Die maximale Bildgröße ist 128 KiB. Die Firmware lehnt ungültige oder nicht vollständig decodierbare JPEGs ab.

## Firmwarepflege

| Methode | Pfad | Zweck |
| --- | --- | --- |
| `POST` | `/update` | manuelles OTA-Image hochladen |
| `GET` | `/api/checkupdate` | neuesten Release-Tag und passendes Image ermitteln |
| `POST` | `/api/selfupdate` | Update für den frühen Bootpfad vormerken |

Die Update-Prüfung liest Repository und Asset-Namen aus `src/config.h`. Vor einem eigenen Release müssen diese Konstanten zum veröffentlichten Firmware-Image passen. OTA ist abhängig vom freien App-Slot; für ein Ultra mit ungünstigem Bestands-Flashlayout kann ein spezieller Erstinstallationsweg erforderlich sein.

## Build-abhängige Endpunkte

Die Projekteinstellungen schalten Home-Assistant/MQTT-Routen und das Benachrichtigungs-Overlay aus. Sie gehören daher nicht zur aktiven API dieses Images, obwohl generischer Quellcode dafür noch vorhanden ist.
