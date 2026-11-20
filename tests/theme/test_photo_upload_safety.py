"""Regression guard for durable photo-upload staging before image validation."""
import os
import unittest
ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
def read(path):
    with open(os.path.join(ROOT, path), encoding="utf-8") as stream:
        return stream.read()
class PhotoUploadDurability(unittest.TestCase):
    def test_staging_file_is_flushed_and_verified_before_validation(self):
        source = read("src/WebPortal.cpp")
        start = source.index("} else if (up.status == UPLOAD_FILE_END) {")
        end = source.index("#if WITH_THEME", start)
        block = source[start:end]
        flush = block.index("photoUpload.flush()")
        write_error = block.index("photoUpload.getWriteError()")
        file_size = block.index("photoUpload.size() == photoUploadBytes")
        close = block.index("photoUpload.close()")
        self.assertLess(flush, write_error)
        self.assertLess(flush, file_size)
        self.assertLess(file_size, close)
        self.assertIn("LittleFS.remove(PhotoUploadPath)", block)
        self.assertIn("LittleFS.remove(PhotoAnimationUploadPath)", block)
    def test_littlefs_validation_is_after_storage_check(self):
        source = read("src/WebPortal.cpp")
        start = source.index("} else if (up.status == UPLOAD_FILE_END) {")
        self.assertLess(source.index("photoUpload.size() == photoUploadBytes", start),
                        source.index("validatePhotoAnimation(PhotoAnimationUploadPath)", start))
        self.assertLess(source.index("photoUpload.size() == photoUploadBytes", start),
                        source.index("photoValidateJpeg(PhotoUploadPath)", start))
if __name__ == "__main__":
    unittest.main()
