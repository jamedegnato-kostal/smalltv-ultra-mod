"""Guard the ESP8266WebServer HTTPUpload buffer size and build flag scope."""
import os
import re
import unittest
ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
def read(path):
    with open(os.path.join(ROOT, path), encoding="utf-8") as stream:
        return stream.read()
def section(text, name):
    match = re.search(r"^\[env:%s\]\n(.*?)(?=^\[|\Z)" % re.escape(name), text, re.S | re.M)
    if not match:
        raise AssertionError("missing PlatformIO environment: " + name)
    return match.group(1)
def flag(section_text, name):
    match = re.search(r"^%s\s*=(.*?)(?=^\S|\Z)" % re.escape(name), section_text, re.S | re.M)
    return match.group(1) if match else ""
class UploadBufferConfig(unittest.TestCase):
    def test_one_kib_buffer_is_a_build_flag_for_all_firmware_sources(self):
        ini=read("platformio.ini"); clean=section(ini,"smalltv_clean")
        self.assertRegex(flag(clean,"build_flags"),r"-D\s*HTTP_UPLOAD_BUFLEN=1024\b")
        self.assertNotIn("HTTP_UPLOAD_BUFLEN",flag(clean,"build_src_flags"))
        settings=section(ini,"smalltv_settings")
        self.assertIn("smalltv_clean.build_flags",flag(settings,"build_flags"))
    def test_firmware_compile_fails_if_buffer_grows_again(self):
        self.assertRegex(read("src/WebPortal.cpp"),r"static_assert\(HTTP_UPLOAD_BUFLEN\s*<=\s*1024")
if __name__ == "__main__":
    unittest.main()
