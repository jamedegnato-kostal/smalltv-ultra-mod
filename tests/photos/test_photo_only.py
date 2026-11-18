#!/usr/bin/env python3
"""Static safeguards for the JPEG-only ESP8266 distribution profile."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
read = lambda p: (ROOT / p).read_text(encoding="utf-8")

web = read("src/WebPortal.cpp")
library_h = read("src/PhotoLibrary.h")
library = read("src/PhotoLibrary.cpp")
slideshow_h = read("src/PhotoSlideshow.h")
slideshow = read("src/PhotoSlideshow.cpp")
html = read("src/webui_settings.html")

# GIF conversion, animated frame API and player are absent from the shipped UI
# and firmware photo path.
for marker in ("gifToAnimation", "/api/photos/frame", "data-animation-frames"):
    assert marker not in html, f"GIF UI path must not ship: {marker}"
for marker in ("validatePhotoAnimation", "handlePhotoFrame", "PhotoAnimationUploadPath",
               "photoUploadAnimated", "smalltv_photo::isAnimation",
               "smalltv_photo::loadAnimationFrames", "/api/photos/frame"):
    assert marker not in web, f"GIF web path must not ship: {marker}"
for marker in ("MaxAnimationFrames", "AnimationFrameInfo", "loadAnimationFrames",
               "framePath", "animationPath", "isAnimation"):
    assert marker not in library_h + library + slideshow_h + slideshow, f"GIF player must not ship: {marker}"

# JPEG uploads remain validated and streamed through a temporary file.
assert "photoValidateJpeg(PhotoUploadPath)" in web
assert 'LittleFS.open(PhotoUploadPath, "w")' in web
assert "LittleFS.rename(PhotoUploadPath, target)" in web

# Legacy files are preserved: no format/cleanup call, and their slot IDs stay
# reserved so a new JPEG cannot overwrite user data left by the GIF build.
assert "legacySlotOccupied" in library
assert '"/photos/p%03u.anim"' in library
assert '"/photos/p%03uf%02u.jpg"' in library
assert "LittleFS.format" not in library
remove_impl = library[library.index("bool remove("):library.index("uint16_t removeAll()")]
assert "LittleFS.remove(fileName)" in remove_impl
assert "framePath" not in remove_impl and "animationPath" not in remove_impl

print("JPEG-only photo profile checks passed")
