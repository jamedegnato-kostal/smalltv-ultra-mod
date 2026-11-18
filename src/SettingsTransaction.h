#pragma once
#include <new>
#include <utility>
#include <stdint.h>

enum class SettingsTransactionFailure : uint8_t { None, OutOfMemory, SaveFailed };

// Keep the candidate off the small web-server stack, and report whether
// staging or persistence failed. The live object changes only after a
// successful persistent write.
template <typename SettingsT, typename Apply, typename Save>
bool applyAndSaveSettings(SettingsT& settings, Apply apply, Save save,
                          SettingsTransactionFailure* failure = nullptr) {
  if (failure) *failure = SettingsTransactionFailure::None;
  SettingsT* candidate = new (std::nothrow) SettingsT(settings);
  if (!candidate) {
    if (failure) *failure = SettingsTransactionFailure::OutOfMemory;
    return false;
  }
  apply(*candidate);
  if (!save(*candidate)) {
    delete candidate;
    if (failure) *failure = SettingsTransactionFailure::SaveFailed;
    return false;
  }
  settings = std::move(*candidate);
  delete candidate;
  return true;
}
