# Theme JSON data sources

The manifest's optional `data` array describes generic JSON HTTP endpoints.
This firmware fetches only the fields declared by a selected theme; it does not
download or retain the complete response body.

```json
{
  "data": [{
    "id": "sample",
    "url": "https://example.invalid/api/summary",
    "interval": 300,
    "insecureTls": true,
    "fields": [
      { "id": "label", "path": "result.label" },
      { "id": "value", "path": "result.value", "decimals": 1 }
    ]
  }]
}
```

Text layers and bindings refer to values as `{sample.label}`,
`{sample.value}`, or `source: "sample.value"`.

## ESP8266 limits

- At most 4 sources and 8 fields per source (32 declared fields total), matching the existing theme format limits. Spec 2 also exposes `source.age` in whole minutes after the first successful fetch.
- One request at a time, with at least 5 seconds between requests.
- Source intervals are 10 to 86,400 seconds; manifest URLs are limited to 320 bytes.
- HTTP response bodies are limited to 2,048 bytes; known `Content-Length` values are checked, and close-delimited HTTP/1.0 replies are streamed to the same cap.
- JSON nesting is limited to 8; the filter/document allocator has a 2,048-byte
  combined budget. Only selected fields are retained; text values are at most
  31 bytes.
- Fetching is skipped under low free-heap or fragmented-heap conditions. TLS
  uses 512/1,024-byte MFLN buffers when the server supports them. A 4 KiB TLS
  buffer is used only when there is substantially more heap headroom.
- Request/reply timeouts are bounded; failed sources use exponential retry
  backoff. The last good field values remain available as stale data after a
  failed refresh.
- Values and freshness state live only in RAM. Responses are never cached in a
  file or written to flash.

HTTPS requires `insecureTls: true` because certificate trust validation is not
configured in this ESP8266 firmware. This is an explicit opt-in, not secure
certificate verification. Use only trusted API endpoints. Redirects are
disabled. The generic fetcher does not add authentication headers or manage API
secrets; a later provider-specific step must handle those safely if required.

## Open-Meteo example

A ready-to-upload `.stheme` is included at
`examples/open-meteo/open-meteo.stheme`, together with the editable manifest.
It requests only current temperature, humidity, WMO weather code, wind speed,
day/night, and three daily values (minimum/maximum temperature and maximum
precipitation probability). The URL uses `forecast_days=1`, `timezone=auto`
and a 30-minute refresh interval to keep the response small.

Replace the sample latitude/longitude in `theme.json` before packaging for the
actual installation. The demo uses Berlin coordinates only as an example.
Open-Meteo supports current weather variables; daily variables require a
timezone. See the [official API documentation](https://open-meteo.com/en/docs).

Open-Meteo returns `weather_code` as a numeric WMO code rather than a text
description. The example uses theme `show` rules to map code ranges to localized
labels without adding a lookup table to firmware flash:

| WMO code | Display label |
|---|---|
| 0 | Clear / Klar |
| 1–3 | Cloud / Wolkig |
| 45, 48 | Fog / Nebel |
| 51–57 | Drizzle / Nieselregen |
| 61–67, 80–82 | Rain / Regen |
| 71–77, 85–86 | Snow / Schnee |
| 95–99 | Thunderstorm / Gewitter |

For exact code descriptions, see the
[Open-Meteo WMO weather-code table](https://open-meteo.com/en/docs#weathervariables).

