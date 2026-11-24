# Open-Meteo demo theme

Upload `open-meteo.stheme` from this directory in the SmallTV Themes tab.

The manifest is also provided as `theme.json` for editing. Before uploading,
replace `latitude=52.52&longitude=13.41` with coordinates for the installation
and rebuild the STH1 package if the location differs from Berlin.

The package requests eight fields only, refreshes every 30 minutes, and limits
the daily forecast horizon to one day. It contains text layers only; no image,
font, JPEG or framebuffer asset is embedded.

HTTPS is explicitly marked `insecureTls: true` because the current ESP8266
firmware does not validate the server certificate. Use only the official
Open-Meteo endpoint. This is an acknowledged TLS-validation limitation, not
certificate verification.

See `src/theme/DATA_SOURCES.md` for memory limits and WMO condition mapping.
