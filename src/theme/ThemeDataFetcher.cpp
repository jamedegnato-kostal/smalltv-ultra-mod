#include "ThemeDataFetcher.h"
#include "ThemeEngine.h"
#include "../Net.h"
#include "../Platform.h"
#include "../config.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
constexpr uint16_t HttpTimeoutMs = 1400;
constexpr uint32_t ReplyTimeoutMs = 2200;
constexpr size_t MaxResponseBytes = 2048;
// Keep the original spec-2 parser allowance: valid existing themes can need
// up to 2 KiB for the filtered document and its selected strings.
constexpr size_t JsonHeapBudget = 2048;
constexpr uint32_t AgeUnknown = 0xffffffffUL;
constexpr uint32_t AgeMaximum = 0xfffffffeUL;
constexpr int32_t AgeUnpublished = -2;
constexpr uint32_t MinRequestSpacingMs = 5000;
constexpr uint32_t BaseRetryMs = 15000;
constexpr uint32_t MaxRetryMs = 240000;

struct SourceState {
  uint32_t nextDue = 0;
  uint32_t ageMs = AgeUnknown;
  int32_t publishedAge = AgeUnpublished;
  uint8_t failures = 0;
  bool hasValue = false;
  bool stale = false;
  bool tlsChecked = false;
  uint16_t tlsBuffer = 0;
};

SourceState g_state[smalltv::MaxDataSources];
uint8_t g_sourceCount = 0;
uint8_t g_nextSource = 0;
uint32_t g_lastRequestAt = 0;
uint32_t g_lastAgeTick = 0;
bool g_ageTickStarted = false;
bool g_enabled = false;
char g_lastError[96] = {};

bool due(uint32_t now, uint32_t deadline) {
  return static_cast<int32_t>(now - deadline) >= 0;
}

void setError(const char* message) {
  if (!message) message = "data request failed";
  strncpy(g_lastError, message, sizeof(g_lastError) - 1);
  g_lastError[sizeof(g_lastError) - 1] = '\0';
}

void markCachedValuesStale() {
  for (uint8_t i = 0; i < g_sourceCount; ++i)
    if (g_state[i].hasValue) g_state[i].stale = true;
}

bool parseAuthority(const std::string& url, char* host, size_t capacity,
                    uint16_t& port, bool& secure) {
  const size_t scheme = url.find("://");
  if (scheme == std::string::npos) return false;
  secure = url.compare(0, scheme, "https") == 0;
  if (!secure && url.compare(0, scheme, "http") != 0) return false;

  const size_t start = scheme + 3;
  size_t end = url.find_first_of("/?#", start);
  if (end == std::string::npos) end = url.size();
  if (end <= start || url.find('@', start) < end) return false;

  size_t hostEnd = end;
  const size_t colon = url.find(':', start);
  port = secure ? 443 : 80;
  if (colon < end) {
    hostEnd = colon;
    const char* digits = url.c_str() + colon + 1;
    if (digits >= url.c_str() + end) return false;
    unsigned long parsed = 0;
    for (const char* p = digits; p < url.c_str() + end; ++p) {
      if (*p < '0' || *p > '9') return false;
      parsed = parsed * 10 + static_cast<unsigned>(*p - '0');
      if (parsed > 65535) return false;
    }
    if (!parsed) return false;
    port = static_cast<uint16_t>(parsed);
  }
  const size_t length = hostEnd - start;
  if (!length || length >= capacity) return false;
  memcpy(host, url.data() + start, length);
  host[length] = '\0';
  return true;
}

// Age is measured with millis(), not wall time. The accumulator survives the
// millis() wrap and saturates after roughly 49.7 days, matching spec 2.
void tickAges(uint32_t now) {
  if (!g_ageTickStarted) {
    g_ageTickStarted = true;
    g_lastAgeTick = now;
    return;
  }
  const uint32_t elapsed = now - g_lastAgeTick;
  g_lastAgeTick = now;
  for (uint8_t i = 0; i < g_sourceCount; ++i) {
    SourceState& state = g_state[i];
    if (state.ageMs != AgeUnknown)
      state.ageMs = elapsed >= AgeMaximum - state.ageMs ? AgeMaximum : state.ageMs + elapsed;
  }
}

void publishAges(const smalltv::Theme& theme, smalltv::Engine& engine) {
  if (theme.spec < 2) return;
  std::vector<smalltv::ThemeValue> updates;
  updates.reserve(g_sourceCount);
  for (uint8_t i = 0; i < g_sourceCount && i < theme.data.size(); ++i) {
    SourceState& state = g_state[i];
    const int32_t age = state.ageMs == AgeUnknown ? -1 : static_cast<int32_t>(state.ageMs / 60000UL);
    if (age == state.publishedAge) continue;
    state.publishedAge = age;
    if (age < 0) continue;  // no value until the first successful fetch
    bool explicitAge = false;
    for (const auto& field : theme.data[i].fields) explicitAge = explicitAge || field.id == "age";
    if (explicitAge) continue;
    updates.push_back({theme.data[i].id + ".age", std::to_string(age)});
  }
  if (!updates.empty()) engine.mergeValues(std::move(updates));
}

// ArduinoJson reads directly from this adapter. No response String/body buffer
// is allocated; total bytes and wall-clock time are both bounded.
class HttpDataReply final : public smalltv::DataReply {
 public:
  HttpDataReply(Stream& stream, Client& client, int expectedLength)
      : stream_(stream), client_(client), expectedLength_(expectedLength), started_(millis()) {}

  int read() override {
    if (expectedLength_ >= 0 && bytes_ >= static_cast<size_t>(expectedLength_)) return -1;
    if (bytes_ >= MaxResponseBytes) {
      overflow_ = true;
      return -1;
    }
    while (!stream_.available()) {
      if (!client_.connected()) return -1;
      if (static_cast<uint32_t>(millis() - started_) >= ReplyTimeoutMs) {
        timedOut_ = true;
        return -1;
      }
      if (ESP.getFreeHeap() < 5200UL) {
        lowMemory_ = true;
        return -1;
      }
      delay(1);
    }
    const int value = stream_.read();
    if (value >= 0) ++bytes_;
    return value;
  }

  size_t readBytes(char* destination, size_t count) override {
    size_t readCount = 0;
    while (readCount < count) {
      const int value = read();
      if (value < 0) break;
      destination[readCount++] = static_cast<char>(value);
    }
    return readCount;
  }

  bool healthy() const {
    return !timedOut_ && !overflow_ && !lowMemory_ &&
      (expectedLength_ < 0 || bytes_ == static_cast<size_t>(expectedLength_));
  }
  const char* failure() const {
    if (overflow_) return "JSON response exceeds 2048 bytes";
    if (timedOut_) return "JSON response timeout";
    if (lowMemory_) return "data parsing stopped: low heap";
    return "invalid or incomplete JSON response";
  }

 private:
  Stream& stream_;
  Client& client_;
  int expectedLength_;
  uint32_t started_;
  size_t bytes_ = 0;
  bool timedOut_ = false;
  bool overflow_ = false;
  bool lowMemory_ = false;
};

template <typename ClientType>
bool requestSource(ClientType& socket, const smalltv::ThemeDataSource& source,
                   std::vector<smalltv::ThemeValue>& values, char* error,
                   size_t errorSize) {
  socket.setTimeout(HttpTimeoutMs);
  HTTPClient http;
  http.setTimeout(HttpTimeoutMs);
  http.setReuse(false);
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  http.useHTTP10(true);
  if (!http.begin(socket, String(source.url.c_str()))) {
    strncpy(error, "could not start HTTP request", errorSize - 1);
    error[errorSize - 1] = '\0';
    return false;
  }
  http.addHeader("Accept", "application/json");
  http.addHeader("Accept-Encoding", "identity");
  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    char message[48];
    snprintf(message, sizeof(message), "HTTP status %d", code);
    strncpy(error, message, errorSize - 1);
    error[errorSize - 1] = '\0';
    http.end();
    return false;
  }

  const int bodySize = http.getSize();
  if (bodySize == 0 || (bodySize > 0 && static_cast<size_t>(bodySize) > MaxResponseBytes)) {
    strncpy(error, "empty or oversized JSON response", errorSize - 1);
    error[errorSize - 1] = '\0';
    http.end();
    return false;
  }
  if (ESP.getFreeHeap() < 7000UL || platformMaxFreeBlock() < 1400UL) {
    strncpy(error, "not enough free heap to parse JSON", errorSize - 1);
    error[errorSize - 1] = '\0';
    http.end();
    return false;
  }

  values.clear();
  values.reserve(source.fields.size());
  HttpDataReply reply(http.getStream(), socket, bodySize);
  const bool parsed = smalltv::extractThemeValues(reply, source, values, JsonHeapBudget);
  const bool healthy = reply.healthy();
  http.end();
  if (!parsed || !healthy) {
    strncpy(error, healthy ? "invalid JSON or JSON memory budget exceeded" : reply.failure(), errorSize - 1);
    error[errorSize - 1] = '\0';
    values.clear();
    return false;
  }
  return true;
}

uint16_t chooseTlsBuffer(const char* host, uint16_t port, char* error, size_t errorSize) {
  if (BearSSL::WiFiClientSecure::probeMaxFragmentLength(host, port, 512)) return 512;
  if (BearSSL::WiFiClientSecure::probeMaxFragmentLength(host, port, 1024)) return 1024;

  // Without MFLN, BearSSL needs a larger receive buffer. Do not guess a smaller
  // buffer: servers may send a TLS record larger than the ESP8266 can hold.
  const uint32_t freeHeap = ESP.getFreeHeap();
  const uint32_t maxBlock = platformMaxFreeBlock();
  if (freeHeap >= 19000UL && maxBlock >= 4096UL) return 4096;

  snprintf(error, errorSize,
      "TLS probe failed; 4 KiB fallback needs 19000 B heap / 4096 B block (have %lu / %lu)",
      static_cast<unsigned long>(freeHeap), static_cast<unsigned long>(maxBlock));
  return 0;
}

bool fetchSource(const smalltv::ThemeDataSource& source, SourceState& state,
                 std::vector<smalltv::ThemeValue>& values, char* error, size_t errorSize) {
  char host[201] = {};
  uint16_t port = 0;
  bool secure = false;
  if (!parseAuthority(source.url, host, sizeof(host), port, secure)) {
    strncpy(error, "invalid data source URL authority", errorSize - 1);
    error[errorSize - 1] = '\0';
    return false;
  }

  if (!secure) {
    WiFiClient socket;
    return requestSource(socket, source, values, error, errorSize);
  }

  if (!source.insecureTls) {
    strncpy(error, "HTTPS needs explicit insecureTls opt-in", errorSize - 1);
    error[errorSize - 1] = '\0';
    return false;
  }
  if (!state.tlsChecked) {
    state.tlsBuffer = chooseTlsBuffer(host, port, error, errorSize);
    state.tlsChecked = state.tlsBuffer != 0;
  }
  if (!state.tlsBuffer) {
    if (!error[0]) {
      strncpy(error, "TLS needs MFLN or more free heap", errorSize - 1);
      error[errorSize - 1] = '\0';
    }
    return false;
  }

  SecureClient socket;
  socket.setInsecure();  // Certificate validation is not configured in this firmware.
  socket.setBufferSizes(state.tlsBuffer, 512);
  return requestSource(socket, source, values, error, errorSize);
}
}  // namespace

namespace smalltv {
void themeDataFetcherBegin(size_t sourceCount) {
  for (uint8_t i = 0; i < MaxDataSources; ++i) g_state[i] = SourceState{};
  g_sourceCount = static_cast<uint8_t>(std::min(sourceCount, static_cast<size_t>(MaxDataSources)));
  g_nextSource = 0;
  g_lastRequestAt = millis() - MinRequestSpacingMs;
  g_lastAgeTick = millis();
  g_ageTickStarted = false;
  g_enabled = g_sourceCount != 0;
  g_lastError[0] = '\0';
  const uint32_t now = millis();
  for (uint8_t i = 0; i < g_sourceCount; ++i) g_state[i].nextDue = now;
}

void themeDataFetcherStop() {
  g_enabled = false;
  g_sourceCount = 0;
  g_nextSource = 0;
  g_lastError[0] = '\0';
  g_ageTickStarted = false;
  for (uint8_t i = 0; i < MaxDataSources; ++i) g_state[i] = SourceState{};
}

void themeDataFetcherLoop(const Theme& theme, Engine& engine, uint32_t now) {
  if (!g_enabled || theme.data.empty()) return;
  tickAges(now);
  publishAges(theme, engine);
  if (!netConnected()) {
    markCachedValuesStale();
    setError("waiting for an active Wi-Fi connection");
    return;
  }
  if (!due(now, g_lastRequestAt + MinRequestSpacingMs)) return;

  // TEST ONLY: bypass the 10 KiB / 1.8 KiB preflight gate to observe the
  // actual TLS/HTTP/parser failure at lower heap levels. Internal safeguards
  // (TLS buffer selection, parser heap checks, response cap and timeouts) remain.
  for (uint8_t offset = 0; offset < g_sourceCount; ++offset) {
    const uint8_t index = static_cast<uint8_t>((g_nextSource + offset) % g_sourceCount);
    SourceState& state = g_state[index];
    if (!due(now, state.nextDue)) continue;

    g_nextSource = static_cast<uint8_t>((index + 1) % g_sourceCount);
    std::vector<ThemeValue> refreshed;
    char error[96] = {};
    const bool ok = fetchSource(theme.data[index], state, refreshed, error, sizeof(error));
    const uint32_t completedAt = millis();
    tickAges(completedAt);  // include time spent waiting for this request
    g_lastRequestAt = completedAt;

    if (ok) {
      state.ageMs = 0;
      engine.mergeValues(std::move(refreshed));
      state.hasValue = true;
      state.stale = false;
      state.failures = 0;
      state.nextDue = completedAt + theme.data[index].interval * 1000UL;
      g_lastError[0] = '\0';
    } else {
      if (state.failures < 5) ++state.failures;
      state.stale = state.hasValue;
      uint32_t retry = BaseRetryMs << (state.failures ? state.failures - 1 : 0);
      if (retry > MaxRetryMs) retry = MaxRetryMs;
      state.nextDue = completedAt + retry;
      char message[96];
      snprintf(message, sizeof(message), "%.31s: %.60s", theme.data[index].id.c_str(), error[0] ? error : "request failed");
      setError(message);
    }
    publishAges(theme, engine);
    return;
  }
}

uint8_t themeDataFetcherSourceCount() { return g_sourceCount; }
uint8_t themeDataFetcherReadyCount() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < g_sourceCount; ++i) if (g_state[i].hasValue) ++count;
  return count;
}
uint8_t themeDataFetcherStaleCount() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < g_sourceCount; ++i) if (g_state[i].stale) ++count;
  return count;
}
uint8_t themeDataFetcherFailureCount() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < g_sourceCount; ++i) if (g_state[i].failures) ++count;
  return count;
}
const char* themeDataFetcherError() { return g_lastError; }
}  // namespace smalltv
