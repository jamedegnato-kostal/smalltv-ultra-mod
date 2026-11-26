#!/usr/bin/env python3
"""Regression check for playlist theme discovery while a theme is active."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
webui = (root / "src/webui_stable.h").read_text(encoding="utf-8")
theme_web = (root / "src/theme/ThemeWeb.cpp").read_text(encoding="utf-8")
playback = (root / "src/Playback.cpp").read_text(encoding="utf-8")

# ThemeWeb intentionally defers package validation while a theme is running.
deferred_reason = "Deactivate active theme to validate this package"
assert deferred_reason in theme_web

# The playlist must still offer deferred packages, rather than filtering them
# out together with packages that actually failed validation.
assert 'theme.valid === true ||' in webui
assert f'theme.error === "{deferred_reason}"' in webui
assert '(Prüfung beim Start)' in webui

# On playback, the current renderer is released before the package is loaded;
# ThemeRuntime then performs the real package validation.
assert 'smalltv::themeRuntimeSelect(e.id' in playback
assert 'smalltv::photos::stop();' in playback

# Do not reserve all 32 playlist records in static RAM when the saved playlist
# contains only one or two items; this memory is needed by the ESP8266 theme runtime.
assert "std::vector<PlaylistEntry> g_entries;" in playback
assert "PlaylistEntry g_entries[MaxPlaylistEntries]" not in playback
assert "g_entries.push_back(e);" in playback
assert "std::vector<PlaylistEntry>().swap(g_entries);" in playback

print("Playlist theme-selection regression check passed.")
