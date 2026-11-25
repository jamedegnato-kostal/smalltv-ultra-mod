from pathlib import Path

settings = Path("src/Settings.cpp").read_text(encoding="utf-8")
portal = Path("src/WebPortal.cpp").read_text(encoding="utf-8")
photo_header = Path("src/PhotoSlideshow.h").read_text(encoding="utf-8")
photos = Path("src/PhotoSlideshow.cpp").read_text(encoding="utf-8")
webui = Path("src/webui_stable.h").read_text(encoding="utf-8")
readme = Path("README.md").read_text(encoding="utf-8")

# The format operation must run whether LittleFS was already mounted or not.
assert "const bool formatted = LittleFS.format();" in settings
assert "if (g_storageReady) return false;" not in settings
assert "g_storageReady = LittleFS.begin();" in settings

# Stop active filesystem consumers before formatting the partition.
factory_start = portal.index("static void handleFactory() {")
factory_end = portal.index("static void handleExport()", factory_start)
factory = portal[factory_start:factory_end]
assert factory.index("smalltv::themeRuntimeStop();") < factory.index("settingsFormat()")
assert factory.index("smalltv::photos::stop();") < factory.index("settingsFormat()")
assert "if (!settingsFormat())" in factory

# Clear the slideshow's in-memory state as its files are erased.
assert "void stop();" in photo_header
stop_start = photos.index("void stop() {")
stop_end = photos.index("bool active()", stop_start)
stop = photos[stop_start:stop_end]
assert "g_active = false;" in stop
assert "g_current = -1;" in stop
assert "g_photoCount = 0;" in stop

# Confirm the destructive scope to the user before issuing the reset request.
assert "Alle gespeicherten Einstellungen" in webui
assert ".stheme-Pakete und Fotos" in webui
assert "Die Firmware bleibt installiert" in webui
assert "reformats the complete LittleFS data" in readme

print("Factory-reset regression checks passed.")
