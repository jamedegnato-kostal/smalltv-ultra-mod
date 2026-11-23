"""Regression guards for the ESP8266 V1 photo upload and playback path."""
import os
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")


def read(path):
    with open(os.path.join(ROOT, path), encoding="utf-8") as stream:
        return stream.read()


class PhotoPipelineSafety(unittest.TestCase):
    def test_upload_is_flushed_and_verified_before_jpeg_validation(self):
        source = read("src/WebPortal.cpp")
        start = source.index("} else if (up.status == UPLOAD_FILE_END) {")
        end = source.index("} else if (up.status == UPLOAD_FILE_ABORTED) {", start)
        block = source[start:end]
        flush = block.index("photoUpload.flush()")
        write_error = block.index("photoUpload.getWriteError()")
        size = block.index("photoUpload.size() == photoUploadBytes")
        close = block.index("photoUpload.close()")
        reopen = block.index('LittleFS.open(PhotoUploadPath, "r")')
        persisted_size = block.index("verify.size() == photoUploadBytes")
        validate = block.index("photoValidateJpeg(PhotoUploadPath)")
        self.assertLess(flush, write_error)
        self.assertLess(write_error, size)
        self.assertLess(size, close)
        self.assertLess(close, reopen)
        self.assertLess(reopen, persisted_size)
        self.assertLess(persisted_size, validate)
        self.assertIn("LittleFS.remove(PhotoUploadPath)", block)

    def test_upload_validation_uses_bounded_markers_not_full_decode(self):
        source = read("src/PhotoSlideshow.cpp")
        start = source.index("bool photoValidateJpeg")
        end = source.index("void PhotoSlideshow::begin()", start)
        validation = source[start:end]
        self.assertIn("validateBaselineJpeg240(reader)", validation)
        self.assertNotIn("TJpgDec.drawFsJpg", validation)
        self.assertIn("MinFreeHeap = 3UL * 1024UL", source)
        self.assertIn("MinLargestBlock = 1280UL", source)

    def test_active_theme_is_released_before_photo_batch_starts(self):
        source = read("src/main.cpp")
        start = source.index("if (!g_themePhotosChecked)")
        end = source.index("#endif", start)
        block = source[start:end]
        self.assertLess(block.index("g_themeMode.setPlaylistTheme(nullptr)"),
                        block.index("g_photoSlideshow.start"))

    def test_esp8266_http_upload_buffer_is_bounded(self):
        self.assertIn("-D HTTP_UPLOAD_BUFLEN=1024", read("platformio.ini"))


if __name__ == "__main__":
    unittest.main()
