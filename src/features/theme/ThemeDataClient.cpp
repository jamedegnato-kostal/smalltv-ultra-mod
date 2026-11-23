#include "ThemeDataClient.h"
#if WITH_THEME
#include "Platform.h"
#include "ThemeMode.h"
#include <ArduinoJson.h>
#include <cstring>
#include <climits>
#include <cstdlib>
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

// A deadline- and byte-limited Stream lets ArduinoJson filter large provider
// replies in place instead of allocating a complete JSON body on the ESP8266.
class BoundedBodyStream : public Stream {
 public:
  BoundedBodyStream(Stream& source, Deadline& deadline)
      : source_(source), deadline_(deadline) {}
  int available() override {
    if (deadline_.expired() || bytes_ >= smalltv::MaxDataResponseBytes) return 0;
    const size_t remaining = smalltv::MaxDataResponseBytes - bytes_;
    return std::min(source_.available(), static_cast<int>(remaining));
  }
  int read() override {
    if (deadline_.expired() || bytes_ >= smalltv::MaxDataResponseBytes) return -1;
    int value = source_.read();
    if (value >= 0) ++bytes_;
    return value;
  }
  int peek() override {
    if (deadline_.expired() || bytes_ >= smalltv::MaxDataResponseBytes) return -1;
    return source_.peek();
  }
  size_t write(uint8_t) override { return 0; }
  void flush() override {}
  size_t bytesRead() const { return bytes_; }
 private:
  Stream& source_;
  Deadline& deadline_;
  size_t bytes_ = 0;
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
static bool pathIndex(const char* value, size_t length, size_t& index) {
  if (!length || length > 2) return false;
  index = 0;
  for (size_t i = 0; i < length; ++i) {
    if (value[i] < '0' || value[i] > '9') return false;
    index = index * 10 + size_t(value[i] - '0');
  }
  return index <= 15;
}
JsonVariantConst jsonPath(JsonVariantConst root, std::string& path) {
  size_t start = 0;
  while (start < path.size()) {
    size_t dot = path.find('.', start);
    if (dot == start || (dot != std::string::npos && dot + 1 == path.size())) return JsonVariantConst();
    const size_t end = dot == std::string::npos ? path.size() : dot;
    size_t index = 0;
    if (root.is<JsonArrayConst>()) {
      if (!pathIndex(path.c_str() + start, end - start, index)) return JsonVariantConst();
      root = root[index];
    } else if (root.is<JsonObjectConst>()) {
      // Temporarily terminate this component in the already-owned path string.
      // JsonObject lookup consumes the key immediately; restore the delimiter.
      char separator = 0;
      if (dot != std::string::npos) {
        separator = path[dot];
        path[dot] = '\0';
      }
      root = root[path.c_str() + start];
      if (dot != std::string::npos) path[dot] = separator;
    } else return JsonVariantConst();
    if (dot == std::string::npos) break;
    start = dot + 1;
  }
  return root;
}
static bool addFilterPath(JsonDocument& filter, const std::string& path) {
  JsonVariant node = filter.as<JsonVariant>();
  if (node.isNull()) node = filter.to<JsonObject>();
  size_t start = 0;
  while (start < path.size()) {
    size_t dot = path.find('.', start);
    if (dot == start || (dot != std::string::npos && dot + 1 == path.size())) return false;
    size_t index = 0;
    const size_t end = dot == std::string::npos ? path.size() : dot;
    if (pathIndex(path.c_str() + start, end - start, index)) node = node[index];
    else node = node[path.substr(start, end - start).c_str()];
    if (dot == std::string::npos) break;
    start = dot + 1;
  }
  return node.set(true);
}
static bool jsonInteger(JsonVariantConst value, int& result) {
  if (value.is<int>()) { result = value.as<int>(); return true; }
  if (value.is<long>()) { long n = value.as<long>(); if (n < INT32_MIN || n > INT32_MAX) return false; result = int(n); return true; }
  if (!value.is<const char*>()) return false;
  const char* text = value.as<const char*>();
  if (!text || !*text) return false;
  char* end = nullptr;
  long n = strtol(text, &end, 10);
  if (!end || *end || n < INT32_MIN || n > INT32_MAX) return false;
  result = int(n);
  return true;
}
static void assignTransform(JsonVariantConst root, const smalltv::ThemeDataField& field,
                            std::string& output) {
  if (field.transform == smalltv::ThemeDataTransform::WwoWeatherCode) {
    int code = 0;
    if (!jsonInteger(jsonPath(root, const_cast<std::string&>(field.path)), code)) { output = "--"; return; }
    output = std::to_string(smalltv::wwoWeatherCodeToWmo(code));
    return;
  }
  if (field.transform == smalltv::ThemeDataTransform::WttrDaylight) {
    // The field path points at current_condition[0].localObsDateTime. Sunrise
    // and sunset are included by the same filtered response.
    JsonVariantConst observation = jsonPath(root, const_cast<std::string&>(field.path));
    const char* local = observation.as<const char*>();
    const char* sunrise = root["weather"][0]["astronomy"][0]["sunrise"].as<const char*>();
    const char* sunset = root["weather"][0]["astronomy"][0]["sunset"].as<const char*>();
    int day = 0;
    output = smalltv::wttrIsDaylight(local, sunrise, sunset, day) ? (day ? "1" : "0") : "--";
    return;
  }
  jsonValue(jsonPath(root, const_cast<std::string&>(field.path)), output);
}
bool fetchThemeSource(smalltv::DataFetch& request) {
  if (request.cancelled()) return false;
  PlatformMemoryPhaseScope dataMemory(PLATFORM_MEM_THEME_DATA, true);
#if defined(SMALLTV_ESP8266)
  // ThemeMode releases the renderer/package before calling this synchronous
  // ESP8266 path. Keep a conservative reserve for WiFi and the JSON filter.
  if (ESP.getFreeHeap() < ThemeMode::DataFetchFreeHeap ||
      platformMaxFreeBlock() < ThemeMode::DataFetchLargestBlock) return false;
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
  const int contentLength = http.getSize();
  if (contentLength == 0 || contentLength > static_cast<int>(smalltv::MaxDataResponseBytes)) {
    http.end();
    return false;
  }
  JsonDocument filter;
  for (const auto& field : request.source.fields) {
    if (!addFilterPath(filter, field.path)) { http.end(); return false; }
    if (field.transform == smalltv::ThemeDataTransform::WttrDaylight) {
      filter["weather"][0]["astronomy"][0]["sunrise"] = true;
      filter["weather"][0]["astronomy"][0]["sunset"] = true;
    }
  }
  JsonDocument doc;
  BoundedBodyStream stream(http.getStream(), deadline);
  const auto parsed = deserializeJson(doc, stream,
      DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(8));
  const bool complete = !parsed && !deadline.expired() &&
      stream.bytesRead() <= smalltv::MaxDataResponseBytes;
  http.end();
  if (!complete || request.cancelled()) return false;
  // Filtering keeps only declared paths (plus the two sun times required for
  // wttr-daylight), so the full provider forecast never resides in heap.
  platformMemorySample(true);
  request.values.resize(request.source.fields.size());
  size_t valueIndex = 0;
  JsonVariantConst root = doc.as<JsonVariantConst>();
  for (auto& field : request.source.fields) {
    smalltv::ThemeValue& output = request.values[valueIndex++];
    output.key.assign(request.source.id);
    output.key += '.';
    output.key += field.id;
    assignTransform(root, field, output.value);
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
