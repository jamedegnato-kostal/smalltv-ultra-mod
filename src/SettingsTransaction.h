#pragma once

#include <new>
#include <utility>

// Apply a mutation atomically from the caller's perspective. A failed
// persistent write restores the exact live settings that preceded it, so a
// route that reports failure never leaves the in-memory `settings` object
// disagreeing with what is actually on disk. Settings is large on the
// ESP8266, so keep the staging copy off the small web-server task stack.
template <typename SettingsT, typename Apply, typename Save>
bool applyAndSaveSettings(SettingsT& settings, Apply apply, Save save) {
  SettingsT* candidate = new (std::nothrow) SettingsT(settings);
  if (!candidate) return false;

  apply(*candidate);
  if (!save(*candidate)) {
    delete candidate;
    return false;
  }

  settings = std::move(*candidate);
  delete candidate;
  return true;
}
