#pragma once

// Apply a mutation atomically from the caller's perspective. A failed
// persistent write restores the exact live settings that preceded it, so a
// route that reports failure never leaves the in-memory `settings` object
// disagreeing with what is actually on disk.
template <typename SettingsT, typename Apply, typename Save, typename Restore>
bool applyAndSaveSettings(SettingsT& settings, Apply apply, Save save, Restore restore) {
  apply(settings);
  if (save(settings)) return true;
  // Atomic save leaves the old file intact; reload only on failure rather than
  // allocating a second copy of every stored Wi-Fi String for rollback.
  restore(settings);
  return false;
}
