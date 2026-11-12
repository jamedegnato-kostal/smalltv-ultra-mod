#pragma once
#include "ThemeEngine.h"
#include <algorithm>
#include <atomic>
#include <memory>

namespace smalltv {
constexpr size_t MaxDataBody = 2048;
constexpr uint32_t DataTimeoutMs = 3000;

// Stream supplies a monotonic clock and nonblocking socket reads. Limit total
// elapsed time, including slow trickles; a socket's idle timeout is insufficient.
// `length` is the declared Content-Length, or -1 for chunked/unknown length
// (HTTPClient already de-chunks the stream; here that just means "read until
// the connection closes" instead of "read until N bytes"), capped either way
// at MaxDataBody so a source cannot force an unbounded allocation.
template <class Stream>
bool readDataBody(Stream& stream, int length, std::string& body) {
  body.clear();
  if (length == 0 || length < -1 || length > static_cast<int>(MaxDataBody)) return false;
  body.reserve(length > 0 ? static_cast<size_t>(length) : MaxDataBody);
  const uint32_t started = stream.now();
  uint8_t buffer[128];
  for (;;) {
    if (stream.cancelled() || uint32_t(stream.now() - started) >= DataTimeoutMs) return false;
    if (length > 0 && body.size() == static_cast<size_t>(length)) return true;
    int available = stream.available();
    if (available > 0) {
      const size_t limit = length > 0 ? static_cast<size_t>(length) : MaxDataBody;
      if (body.size() == limit) return false;
      size_t count = std::min(sizeof(buffer), std::min(static_cast<size_t>(available), limit - body.size()));
      int read = stream.read(buffer, count);
      if (read > 0) {
        body.append(reinterpret_cast<const char*>(buffer), read);
        continue;
      }
    } else if (!stream.connected()) {
      // On the ESP8266, available() lets the network stack run when it finds
      // nothing and still returns 0, even if the rest of the reply arrived
      // with the server's close meanwhile. Those bytes are waiting: read them.
      if (stream.available() > 0) continue;
      return length < 0 && !body.empty();
    }
    stream.idle();
  }
}

// One in-flight (or just-completed) fetch, shared between the display loop
// and whatever actually performs the request -- a FreeRTOS task on ESP32, or
// a direct synchronous call on ESP8266 (see startThemeDataFetch in
// ThemeDataClient.cpp for both). `cancelled_`/`finished_` are the only fields
// either side may touch after construction, so they are atomic; `source`,
// `index`, and `values` are written once by their respective owner and never
// contended. `finish()`'s release / `finished()`'s acquire pairing is what
// makes `values` (written just before finish()) visible to the reader once
// finished() observes true.
struct DataFetch {
  ThemeDataSource source;
  size_t index;
  std::vector<ThemeValue> values;
  bool success = false;
  DataFetch(const ThemeDataSource& s, size_t i) : source(s), index(i) {}
  void cancel() { cancelled_.store(true); }
  bool cancelled() const { return cancelled_.load(); }
  void finish(bool ok) {
    success = ok;
    finished_.store(true, std::memory_order_release);
  }
  bool finished() const { return finished_.load(std::memory_order_acquire); }
 private:
  std::atomic<bool> cancelled_{false}, finished_{false};
};

// The worker owns a shared reference, so changing/unloading a theme never frees
// its request underneath it. Only the display loop consumes completed values.
class DataRequests {
 public:
  ~DataRequests() { cancel(); }
  bool busy() const { return bool(pending_); }
  // Refuses a second request while one is already pending -- callers must
  // check busy() first; this only guards against a caller that does not.
  std::shared_ptr<DataFetch> start(const ThemeDataSource& source, size_t index) {
    if (pending_) return {};
    pending_ = std::make_shared<DataFetch>(source, index);
    return pending_;
  }
  void cancel() { if (pending_) pending_->cancel(); }
  // Returns the finished result once, or nothing if still pending or if it
  // was cancelled (a cancelled fetch's values are stale by definition, e.g.
  // from a theme that has since been switched away from).
  std::shared_ptr<DataFetch> take() {
    if (!pending_ || !pending_->finished()) return {};
    auto result = std::move(pending_);
    return result->cancelled() ? std::shared_ptr<DataFetch>{} : result;
  }
 private:
  std::shared_ptr<DataFetch> pending_;
};

// Spec 2 exposes the minutes since each data source last succeeded as
// "<source>.age". Ages use millis() only, so NTP is not required and wrap is
// handled with unsigned subtraction. Unknown ages stay absent until a success.
class DataAges {
 public:
  void reset(size_t count, bool enabled = true) {
    ageMs_.assign(enabled ? count : 0, Unknown);
    published_.assign(count, Unpublished);
    started_ = false;
  }
  void tick(uint32_t now) {
    const uint32_t delta = started_ ? uint32_t(now - last_) : 0;
    started_ = true;
    last_ = now;
    for (auto& age : ageMs_) if (age != Unknown) age = delta >= MaxAge - age ? MaxAge : age + delta;
  }
  void result(size_t index, bool success) { if (success && index < ageMs_.size()) ageMs_[index] = 0; }
  int minutes(size_t index) const { return index < ageMs_.size() && ageMs_[index] != Unknown ? int(ageMs_[index] / 60000) : -1; }
  bool publish(const std::vector<ThemeDataSource>& sources, std::vector<ThemeValue>& values) {
    if (published_.size() < sources.size()) published_.resize(sources.size(), Unpublished);
    bool changed = false;
    for (size_t i = 0; i < sources.size(); ++i) {
      const int age = minutes(i);
      if (age == published_[i]) continue;
      published_[i] = age;
      bool own = false;
      for (const auto& field : sources[i].fields) own = own || field.id == "age";
      if (own) continue;
      const std::string key = sources[i].id + ".age";
      auto it = std::find_if(values.begin(), values.end(), [&](const ThemeValue& v) { return v.key == key; });
      if (age < 0) { if (it != values.end()) { values.erase(it); changed = true; } continue; }
      const std::string text = std::to_string(age);
      if (it == values.end()) { values.push_back({key, text}); changed = true; }
      else if (it->value != text) { it->value = text; changed = true; }
    }
    return changed;
  }
 private:
  enum : uint32_t { Unknown = 0xffffffffu, MaxAge = 0xfffffffeu };
  enum : int { Unpublished = -2 };
  std::vector<uint32_t> ageMs_;
  std::vector<int> published_;
  uint32_t last_ = 0;
  bool started_ = false;
};
}
