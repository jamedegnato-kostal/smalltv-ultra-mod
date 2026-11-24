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

- At most 4 sources and 16 fields total (up to 8 fields per source).
- One request at a time, with at least 5 seconds between requests.
- Source intervals are 10 to 86,400 seconds.
- HTTP response `Content-Length` must be present and no larger than 3,072 bytes.
- JSON nesting is limited to 8; the filter/document allocator has a 768-byte
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

This step provides the transport and field-extraction layer only. Weather
provider selection, response mapping and user-facing weather fields are added
separately.
