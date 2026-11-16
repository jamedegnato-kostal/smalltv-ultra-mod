#include "ThemeDataClient.h"
#if WITH_THEME
#include "Platform.h"
#include "ThemeMode.h"
#include <ArduinoJson.h>
#include <cstring>
#if !defined(SMALLTV_ESP8266)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif
#include <new>

namespace {
struct Deadline {
  smalltv::DataFetch& request;
  uint32_t started = millis();
  explicit Deadline(smalltv::DataFetch& r) : request(r) {}
  bool expired() const {
    return request.cancelled() || uint32_t(millis() - started) >= smalltv::DataTimeoutMs;
  }
};

// HTTPClient's timeout is an idle timeout, including for header parsing. Stop
// its transport at the overall deadline too, so trickling headers cannot keep
// the worker alive indefinitely. No other task touches this socket.
template <class Transport>
class BoundedClient : public Transport {
 public:
  explicit BoundedClient(Deadline& deadline) : deadline_(deadline) {}
  using Transport::connect;
  // ESP8266's Client has no connect(host,port,timeout) overload; allowed()
  // already bounds every socket call regardless, so the missing explicit
  // connect timeout costs nothing there.
#if defined(SMALLTV_ESP8266)
  int connect(const char* host, uint16_t port) override {
    if (!allowed()) return 0;
    int result = Transport::connect(host, port);
    return allowed() ? result : 0;
  }
#else
  int connect(const char* host, uint16_t port, int32_t timeout) override {
    if (!allowed()) return 0;
    int result = Transport::connect(host, port, timeout);
    return allowed() ? result : 0;
  }
#endif
  int available() override { return allowed() ? Transport::available() : 0; }
  uint8_t connected() override { return allowed() ? Transport::connected() : 0; }
  int read() override { return allowed() ? Transport::read() : -1; }
  int read(uint8_t* buffer, size_t count) override { return allowed() ? Transport::read(buffer, count) : -1; }
  size_t write(uint8_t byte) override { return write(&byte, 1); }
  size_t write(const uint8_t* buffer, size_t count) override { return allowed() ? Transport::write(buffer, count) : 0; }
 private:
  bool allowed() {
    if (!deadline_.expired()) return true;
    Transport::stop();
    // Also end Stream::readStringUntil's internal timedRead after cancellation.
    this->setTimeout(1);
    return false;
  }
  Deadline& deadline_;
};

// After GET(), read the body through HTTPClient's stream (the same pattern
// StockClient and OtaUpdate use): HTTPClient owns the response from that
// point, and on ESP8266 the raw client behind it never yields body bytes, so
// every theme-data fetch failed at the body-read stage despite HTTP 200.
struct BodyStream {
  Stream& stream;
  NetClient& client;
  Deadline& deadline;
  uint32_t now() const { return millis(); }
  bool cancelled() const { return deadline.expired(); }
  int available() { return stream.available(); }
  bool connected() { return client.connected(); }
  int read(uint8_t* buffer, size_t count) { return (int)stream.readBytes(buffer, count); }
  void idle() { delay(1); }
};

void jsonValue(JsonVariantConst value, std::string& out) {
  if (value.isNull()) { out = "--"; return; }
  if (value.is<const char*>()) {
    const char* text = value.as<const char*>();
    if (!text) { out = "--"; return; }
    out.assign(text, std::min(strlen(text), size_t(31)));
    return;
  }
  if (!value.is<bool>() && !value.is<int>() && !value.is<long>() && !value.is<float>() && !value.is<double>()) {
    out = "--";
    return;
  }
  char encoded[40] = {};
  size_t n = serializeJson(value, encoded, sizeof(encoded) - 1);
  if (n >= sizeof(encoded) - 1) { out = "--"; return; }
  out.assign(encoded, n);
}
JsonVariantConst jsonPath(JsonVariantConst root, std::string& path) {
  size_t start = 0;
  while (start < path.size()) {
    size_t dot = path.find('.', start);
    if (dot == start || (dot != std::string::npos && dot + 1 == path.size())) return JsonVariantConst();
    if (!root.is<JsonObjectConst>()) return JsonVariantConst();
    // Temporarily terminate this component in the already-owned path string.
    // JsonObject lookup consumes the key immediately; restore the delimiter
    // before the next field/update, without creating a substring allocation.
    char separator = 0;
    if (dot != std::string::npos) {
      separator = path[dot];
      path[dot] = '\0';
    }
    root = root[path.c_str() + start];
    if (dot != std::string::npos) path[dot] = separator;
    if (dot == std::string::npos) break;
    start = dot + 1;
  }
  return root;
}
bool fetchThemeSource(smalltv::DataFetch& request) {
  if (request.cancelled()) return false;
  PlatformMemoryPhaseScope dataMemory(PLATFORM_MEM_THEME_DATA, true);
#if defined(SMALLTV_ESP8266)
  // Keep external fetches below the same conservative request limits used
  // by the web portal; the active theme cannot be released while it is the
  // caller of this fetch.
  if (ESP.getFreeHeap() < ThemeMode::RequestFreeHeap ||
      platformMaxFreeBlock() < ThemeMode::RequestLargestBlock) return false;
#endif
  Deadline deadline(request);
  std::unique_ptr<NetClient> client;
  if (request.source.url.rfind("https://", 0) == 0) {
    if (ESP.getFreeHeap() < 18000 ||
        platformMaxFreeBlock() < ThemeMode::RequestLargestBlock) return false;
    auto secure = new(std::nothrow) BoundedClient<SecureClient>(deadline);
    if (!secure) return false;
    // HTTPS reaches this path only after the manifest explicitly opted in.
    secure->setInsecure();
#if !defined(SMALLTV_ESP8266)
    secure->setHandshakeTimeout(smalltv::DataTimeoutMs / 1000);
#endif
    client.reset(secure);
  } else client.reset(new(std::nothrow) BoundedClient<WiFiClient>(deadline));
  if (!client) return false;
  // ESP8266's WiFiClient/HTTPClient have no separate connect-phase timeout;
  // Deadline (via BoundedClient::allowed()) already bounds every socket call
  // on every platform, so these are ESP32-only belt-and-suspenders.
#if !defined(SMALLTV_ESP8266)
  client->setConnectionTimeout(smalltv::DataTimeoutMs);
#endif
  HTTPClient http;
#if !defined(SMALLTV_ESP8266)
  http.setConnectTimeout(smalltv::DataTimeoutMs);
#endif
  http.setTimeout(smalltv::DataTimeoutMs);
  http.setReuse(false);
  http.useHTTP10(true);
  if (!http.begin(*client, request.source.url.c_str())) return false;
  http.addHeader("Accept", "application/json");
  if (http.GET() != HTTP_CODE_OK || deadline.expired()) return false;
  BodyStream stream{http.getStream(), *client, deadline};
  // The response is bounded to MaxDataBody and held only for this request.
  // Parsing the mutable string in place avoids a second copy of every JSON
  // key/value while returning the body allocation to the heap afterwards.
  std::string body;
  bool complete = smalltv::readDataBody(stream, http.getSize(), body);
  http.end();
  if (!complete || request.cancelled() || body.empty()) return false;
  JsonDocument doc;
#if defined(SMALLTV_ESP8266)
  if (deserializeJson(doc, &body[0], DeserializationOption::NestingLimit(8))) return false;
#else
  if (deserializeJson(doc, body, DeserializationOption::NestingLimit(8))) return false;
#endif
  // Sample while the parsed JSON document and retained body buffer are both
  // live; this is the peak-memory phase of a data refresh.
  platformMemorySample(true);
  // Resize preserves existing ThemeValue/string capacities for repeated
  // polls of the same source; assign() below only grows when a value is longer.
  request.values.resize(request.source.fields.size());
  size_t valueIndex = 0;
  for (auto& field : request.source.fields) {
    smalltv::ThemeValue& output = request.values[valueIndex++];
    output.key.assign(request.source.id);
    output.key += '.';
    output.key += field.id;
    jsonValue(jsonPath(doc.as<JsonVariantConst>(), field.path), output.value);
  }
  platformMemorySample(true);
  return !request.cancelled();
}
#if !defined(SMALLTV_ESP8266)
void runFetch(void* parameter) {
  {
    std::unique_ptr<std::shared_ptr<smalltv::DataFetch>> context(
        static_cast<std::shared_ptr<smalltv::DataFetch>*>(parameter));
    auto request = std::move(*context);
    context.reset();
    request->finish(fetchThemeSource(*request));
  } // release every C++ object before FreeRTOS deletes the task's stack
  vTaskDelete(nullptr);
}
#endif
}

#if defined(SMALLTV_ESP8266)
// No FreeRTOS on this chip: run the bounded fetch synchronously on the
// caller's own stack, the same "one blocking call per main-loop tick"
// pattern used by the network layer here. Deadline (inside
// fetchThemeSource) already bounds this to DataTimeoutMs plus body-read
// time, the same order of magnitude a TLS handshake already costs this
// chip elsewhere.
bool startThemeDataFetch(const std::shared_ptr<smalltv::DataFetch>& request) {
  request->finish(fetchThemeSource(*request));
  return true;
}
#else
bool startThemeDataFetch(const std::shared_ptr<smalltv::DataFetch>& request) {
  constexpr uint32_t StackBytes = 8192;
  if (ESP.getFreeHeap() < StackBytes + 18000) return false;
  auto context = new(std::nothrow) std::shared_ptr<smalltv::DataFetch>(request);
  if (!context) return false;
  if (xTaskCreate(runFetch, "theme-json", StackBytes, context, 1, nullptr) != pdPASS) {
    delete context;
    return false;
  }
  return true;
}
#endif
#endif
