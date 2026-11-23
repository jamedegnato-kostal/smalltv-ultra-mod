#pragma once
#include "ThemeEngine.h"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace smalltv {
constexpr size_t MaxDataBody = 2048;
// Public weather endpoints can take several seconds to answer from an ESP8266.
// Three seconds caused valid requests to time out before the filtered JSON arrived.
constexpr uint32_t DataTimeoutMs = 10000;

constexpr size_t MaxDataResponseBytes = 16 * 1024;

// wttr.in uses WorldWeatherOnline condition codes. Convert them to the WMO
// 4677 groups used by the Bellmerei clock theme's existing sprite bindings.
inline int wwoWeatherCodeToWmo(int code) {
  switch (code) {
    case 113: return 1;                 // clear / sunny (WMO mainly clear group)
    case 116: return 2;                 // partly cloudy
    case 119: case 122: return 3;       // cloudy / overcast
    case 143: case 248: case 260: return 48; // mist / fog
    case 176: case 263: case 266: return 51;
    case 179: case 323: case 326: return 71;
    case 182: case 185: case 281: case 284:
    case 311: case 314: case 317: case 320: return 67;
    case 200: case 386: case 389: case 392: case 395: return 95;
    case 227: case 230: case 329: case 332:
    case 335: case 338: return 75;
    case 293: case 296: return 61;
    case 299: case 302: return 63;
    case 305: case 308: return 65;
    case 350: return 77;
    case 353: return 80;
    case 356: return 81;
    case 359: return 82;
    case 362: case 365: case 368: return 85;
    case 371: return 86;
    case 374: case 377: return 87;
    default: return 3;                  // safe cloudy fallback for new codes
  }
}

// wttr.in returns its observation time, sunrise and sunset in local 12-hour
// format. All three share the same location timezone, so no device timezone
// conversion or NTP clock comparison is needed.
inline bool wttrClockMinutes(const char* value, bool hasDate, int& minutes) {
  if (!value || !*value) return false;
  const char* time = value;
  if (hasDate) {
    const char* lastSpace = strrchr(value, ' ');
    if (!lastSpace) return false;
    time = lastSpace;
    while (time > value && time[-1] != ' ') --time;
  }
  int hour = 0, minute = 0;
  char meridiem[4] = {};
  if (sscanf(time, "%d:%d %3s", &hour, &minute, meridiem) != 3 ||
      hour < 1 || hour > 12 || minute < 0 || minute > 59) return false;
  if (!strcmp(meridiem, "AM")) hour %= 12;
  else if (!strcmp(meridiem, "PM")) hour = hour % 12 + 12;
  else return false;
  minutes = hour * 60 + minute;
  return true;
}
inline bool wttrIsDaylight(const char* observation, const char* sunrise,
                           const char* sunset, int& isDay) {
  int now = 0, rise = 0, set = 0;
  // j2's documented observation_time is local HH:MM AM/PM. Accept the older
  // full localObsDateTime form too, in case the provider returns it.
  if ((!wttrClockMinutes(observation, false, now) &&
       !wttrClockMinutes(observation, true, now)) ||
      !wttrClockMinutes(sunrise, false, rise) ||
      !wttrClockMinutes(sunset, false, set) || rise == set) return false;
  isDay = now >= rise && now < set ? 1 : 0;
  return true;
}

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
  // A single request is in flight at a time. Reinitialize the retained object
  // after take() rather than allocating a new shared object on every refresh.
  void reset(const ThemeDataSource& s, size_t i) {
    source = s;
    index = i;
    // Keep the value objects and their string capacities. The fetcher
    // overwrites every configured field before marking the request successful.
    success = false;
    cancelled_.store(false, std::memory_order_relaxed);
    finished_.store(false, std::memory_order_relaxed);
  }
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
    // Do not mutate a completed result while a caller still holds its
    // shared_ptr. In the normal ThemeMode path it is released before the next
    // poll, so the same object and its capacities are reused.
    if (!storage_ || storage_.use_count() > 1) storage_ = std::make_shared<DataFetch>(source, index);
    else storage_->reset(source, index);
    pending_ = storage_;
    return pending_;
  }
  void cancel() { if (pending_) pending_->cancel(); }
  // Release the reusable request object only when no request is still pending.
  // A worker on other targets may still own a cancelled in-flight request.
  void releaseStorage() { if (!pending_) storage_.reset(); }
  // Returns the finished result once, or nothing if still pending or if it
  // was cancelled (a cancelled fetch's values are stale by definition, e.g.
  // from a theme that has since been switched away from).
  std::shared_ptr<DataFetch> take() {
    if (!pending_ || !pending_->finished()) return {};
    auto result = std::move(pending_);
    return result->cancelled() ? std::shared_ptr<DataFetch>{} : result;
  }
 private:
  // Keep one allocation alive and reuse it for the next round-robin source.
  std::shared_ptr<DataFetch> storage_;
  std::shared_ptr<DataFetch> pending_;
};

// Spec 2 exposes the minutes since each data source last succeeded as
// "<source>.age". Ages use millis() only, so NTP is not required and wrap is
// handled with unsigned subtraction. Unknown ages stay absent until a success.
class DataAges {
 public:
  void reset(size_t count, bool enabled = true) {
    if (!count) {
      // assign(0) is permitted to keep vector capacity. A full theme unload
      // must return the data-age backing stores to the ESP8266 heap.
      std::vector<uint32_t>().swap(ageMs_);
      std::vector<int>().swap(published_);
    } else {
      ageMs_.assign(enabled ? count : 0, Unknown);
      published_.assign(count, Unpublished);
    }
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
      const std::string& id = sources[i].id;
      auto it = std::find_if(values.begin(), values.end(), [&](const ThemeValue& v) {
        return v.key.size() == id.size() + 4 &&
               v.key.compare(0, id.size(), id) == 0 &&
               v.key.compare(id.size(), 4, ".age", 4) == 0;
      });
      if (age < 0) { if (it != values.end()) { values.erase(it); changed = true; } continue; }
      char ageText[12];
      snprintf(ageText, sizeof(ageText), "%d", age);
      if (it == values.end()) {
        // Reserve once for the small bounded number of source-age values.
        if (values.size() == values.capacity()) values.reserve(values.size() + sources.size() - i);
        ThemeValue value;
        value.key = id;
        value.key += ".age";
        value.value = ageText;
        values.push_back(std::move(value));
        changed = true;
      } else if (it->value != ageText) {
        it->value = ageText;
        changed = true;
      }
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
