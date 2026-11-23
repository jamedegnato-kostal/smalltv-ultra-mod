"""Regression guard for complete and durable .stheme upload staging."""
import os
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")


def read(path):
    with open(os.path.join(ROOT, path), encoding="utf-8") as stream:
        return stream.read()


class ThemeUploadDurability(unittest.TestCase):
    def test_staged_package_is_flushed_and_read_back_before_validation(self):
        source = read("src/features/theme/ThemeWeb.cpp")
        start = source.index("} else if (up.status == UPLOAD_FILE_END) {")
        end = source.index("// Runs once the multipart body", start)
        block = source[start:end]
        flush = block.index("upload.flush()")
        write_error = block.index("upload.getWriteError()")
        file_size = block.index("upload.size() == uploaded")
        close = block.index("upload.close()")
        reopen = block.index('LittleFS.open(staging, "r")')
        persisted_size = block.index("verify.size() == uploaded")
        self.assertLess(flush, write_error)
        self.assertLess(write_error, file_size)
        self.assertLess(file_size, close)
        self.assertLess(close, reopen)
        self.assertLess(reopen, persisted_size)
        self.assertIn("uploaded == up.totalSize", block)

    def test_stream_budget_cannot_underflow(self):
        source = read("src/features/theme/ThemeWeb.cpp")
        self.assertIn("uploaded > uploadBudget", source)
        self.assertIn("up.currentSize > uploadBudget - uploaded", source)


    def test_deleting_selected_theme_leaves_theme_mode_before_removing_package(self):
        source = read("src/features/theme/ThemeWeb.cpp")
        start = source.index("void deleteTheme()")
        end = source.index("\\nuint16_t blend565", start)
        block = source[start:end]

        self.assertIn("const bool leaveThemeMode", block)
        self.assertIn("candidate.mode = MODE_CAROUSEL", block)
        self.assertIn("candidate.carouselTheme = false", block)
        self.assertIn('if (candidate.themeId == id) candidate.themeId = "";', block)
        self.assertNotIn("Switch away from theme mode", block)

        transaction = block.index("applyAndSaveSettings")
        invalidate = block.index("if (settingsChanged) appInvalidate()")
        remove = block.index("LittleFS.remove(path)")
        self.assertLess(transaction, invalidate)
        self.assertLess(invalidate, remove)

    def test_new_upload_closes_stale_staging_handle_first(self):
        source = read("src/features/theme/ThemeWeb.cpp")
        start = source.index("if (up.status == UPLOAD_FILE_START)")
        close = source.index("upload.close()", start)
        remove = source.index("LittleFS.remove(staging)", start)
        self.assertLess(close, remove)


if __name__ == "__main__":
    unittest.main()
