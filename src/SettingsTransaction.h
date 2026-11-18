#pragma once
#include <new>
#include <utility>
#include <stdint.h>

enum class SettingsTransactionFailure : uint8_t { None, OutOfMemory, CopyIncomplete, SaveFailed };

// Keep the candidate off the small web-server stack, and report whether
// staging or persistence failed. Validate deep-copied String members too:
// Arduino String's allocation failure does not necessarily make new Settings
// return nullptr, and publishing such a partial copy could lose credentials.
template <typename SettingsT, typename Apply, typename Save, typename Validate>
bool applyAndSaveSettings(SettingsT& settings, Apply apply, Save save,
                          SettingsTransactionFailure* failure, Validate validate) {
  if (failure) *failure = SettingsTransactionFailure::None;
  SettingsT* candidate = new (std::nothrow) SettingsT(settings);
  if (!candidate) {
    if (failure) *failure = SettingsTransactionFailure::OutOfMemory;
    return false;
  }
  if (!validate(settings, *candidate)) {
    delete candidate;
    if (failure) *failure = SettingsTransactionFailure::CopyIncomplete;
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

// Compatibility overload for host tests and non-Settings transactions.
template <typename SettingsT, typename Apply, typename Save>
bool applyAndSaveSettings(SettingsT& settings, Apply apply, Save save,
                          SettingsTransactionFailure* failure = nullptr) {
  return applyAndSaveSettings(settings, apply, save, failure,
      [](const SettingsT&, const SettingsT&) { return true; });
}
