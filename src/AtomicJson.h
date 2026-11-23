#pragma once
#include <ArduinoJson.h>
#include <stdint.h>

enum class AtomicJsonResult : uint8_t {
  Ok, DocumentOverflow, TemporaryOpenFailed, ShortWrite, FlushFailed, SizeMismatch, RenameFailed
};

// Write and verify a staging file before publication. The existing target is
// never removed first, so a short write, flash error or power loss cannot
// replace the last known-good settings with a partial document.
template <typename FileSystem>
AtomicJsonResult saveJsonAtomicallyDetailed(FileSystem& fs, const JsonDocument& doc,
                                             const char* target, const char* temporary) {
  if (doc.overflowed()) return AtomicJsonResult::DocumentOverflow;
  auto file = fs.open(temporary, "w");
  if (!file) return AtomicJsonResult::TemporaryOpenFailed;

  const size_t expected = measureJson(doc);
  const size_t written = serializeJson(doc, file);
  file.flush();
  const bool writeError = file.getWriteError();
  const size_t actual = file.size();
  file.close();

  if (written != expected) {
    fs.remove(temporary);
    return AtomicJsonResult::ShortWrite;
  }
  if (writeError) {
    fs.remove(temporary);
    return AtomicJsonResult::FlushFailed;
  }
  if (actual != expected) {
    fs.remove(temporary);
    return AtomicJsonResult::SizeMismatch;
  }
  if (!fs.rename(temporary, target)) {
    fs.remove(temporary);
    return AtomicJsonResult::RenameFailed;
  }
  return AtomicJsonResult::Ok;
}

template <typename FileSystem>
bool saveJsonAtomically(FileSystem& fs, const JsonDocument& doc,
                        const char* target, const char* temporary) {
  return saveJsonAtomicallyDetailed(fs, doc, target, temporary) == AtomicJsonResult::Ok;
}
