"""JPEG upload durability and GIF-free firmware regression checks."""
import os
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")


def read(path):
    with open(os.path.join(ROOT, path), encoding="utf-8") as stream:
        return stream.read()


class PhotoUploadDurability(unittest.TestCase):
    def test_staging_file_is_flushed_and_verified_before_jpeg_validation(self):
        source = read("src/WebPortal.cpp")
        start = source.index("} else if (up.status == UPLOAD_FILE_END) {")
        end = source.index("#if WITH_THEME", start)
        block = source[start:end]

        flush = block.index("photoUpload.flush()")
        write_error = block.index("photoUpload.getWriteError()")
        file_size = block.index("photoUpload.size() == photoUploadBytes")
        close = block.index("photoUpload.close()")
        reopen = block.index('LittleFS.open(PhotoUploadPath, "r")')
        persisted_size = block.index("verify.size() == photoUploadBytes")
        self.assertLess(flush, write_error)
        self.assertLess(write_error, file_size)
        self.assertLess(file_size, close)
        self.assertLess(close, reopen)
        self.assertLess(reopen, persisted_size)
        self.assertIn("LittleFS.remove(PhotoUploadPath)", block)
        self.assertIn("photoValidateJpeg(PhotoUploadPath)", source)

    def test_old_gif_upload_and_playback_paths_are_absent(self):
        portal = read("src/WebPortal.cpp")
        library = read("src/PhotoLibrary.cpp")
        slideshow = read("src/PhotoSlideshow.cpp")
        for old_symbol in (
            "PhotoAnimationUploadPath", "validatePhotoAnimation",
            "handlePhotoFrame", "photoUploadAnimated", "isAnimation",
        ):
            self.assertNotIn(old_symbol, portal)
        self.assertNotIn("/api/photos/frame", portal)
        self.assertNotIn("smalltv_photo::isAnimation", slideshow)
        # Legacy files are accounted for, but no longer decoded or played.
        self.assertIn("orphanFrameBytes", library)

    def test_legacy_slot_reservation_is_cached_from_directory_scan(self):
        library = read("src/PhotoLibrary.cpp")
        self.assertIn("legacyOccupied[LegacySlotBytes]", library)
        self.assertIn("parseLegacyAnimationName", library)
        self.assertIn("setLegacyBit(legacyOccupied, id)", library)
        self.assertIn("getLegacyBit(legacyOccupied, id)", library)
        self.assertNotIn("legacySlotOccupied", library)

    def test_legacy_gif_files_are_removed_only_by_explicit_delete_all(self):
        library = read("src/PhotoLibrary.cpp")
        portal = read("src/WebPortal.cpp")
        begin = library[library.index("void begin()"):library.index("uint8_t count()")]
        remove_all = library[library.index("uint16_t removeAll()"):library.index("uint16_t legacyFileCount()")]
        self.assertNotIn("LittleFS.remove", begin)
        self.assertIn("LegacyFramesPerAnimation = 12", library)
        self.assertIn('"/photos/p%03u.anim"', remove_all)
        self.assertIn('"/photos/p%03uf%02u.jpg"', remove_all)
        self.assertIn('"/photos/.upload"', remove_all)
        self.assertIn('"/photos/.anim-upload"', remove_all)
        self.assertIn("legacyFileCount()", remove_all + library[library.index("uint16_t legacyFileCount()"):])
        self.assertIn("smalltv_photo::legacyFileCount()", portal)
        self.assertIn("remaining == 0 && legacyRemaining == 0", portal)
        self.assertIn(r'\"legacyFiles\":%u', portal)


if __name__ == "__main__":
    unittest.main()
