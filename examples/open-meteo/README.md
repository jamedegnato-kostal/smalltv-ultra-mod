# Open-Meteo demo theme

Upload `open-meteo.stheme` from this directory in the SmallTV Themes tab.

The editable `theme.json` requests five values only: current temperature,
WMO weather code, wind speed and today's minimum/maximum temperature. It
refreshes every 30 minutes, requests a one-day horizon and uses a 194-byte URL.
The manifest uses the existing 200-byte source-URL limit and already points
directly to `https://api.open-meteo.com`. The fetcher does not follow HTTP
redirects, so keep the HTTPS scheme when adapting the URL.

Change the sample Berlin latitude/longitude in `theme.json` for the installation
and rebuild the STH1 package with
`python tools/pack_stheme.py theme.json --output open-meteo.stheme`.

The package contains text layers only; no image, font, JPEG or framebuffer asset
is embedded. The weather labels are manifest text controlled by WMO-code show
rules, not a firmware lookup table.

HTTPS is explicitly marked `insecureTls: true` because the current ESP8266
firmware does not validate the server certificate. Use only the official
Open-Meteo endpoint. This is an acknowledged TLS-validation limitation, not
certificate verification.
