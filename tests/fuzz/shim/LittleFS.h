// The filesystem is only reached by loadSettings()/saveSettings(), which the
// harness never calls; this exists so Settings.cpp compiles.
#pragma once
#include "Arduino.h"
struct File {
  explicit operator bool() const { return false; }
  void close() {}
  size_t write(const uint8_t*, size_t) { return 0; }
  size_t write(uint8_t) { return 0; }
  void flush() {}
  size_t size() const { return 0; }
  bool getWriteError() const { return true; }
  int read() { return -1; }
  int peek() { return -1; }
  size_t readBytes(char*, size_t) { return 0; }
};
struct FSInfo {
  uint32_t totalBytes = 0;
  uint32_t usedBytes = 0;
};
namespace littlefs_impl {
struct LittleFSConfig { explicit LittleFSConfig(bool) {} };
}
using littlefs_impl::LittleFSConfig;
struct FSClass {
  bool setConfig(const LittleFSConfig&) { return true; }
  bool begin() { return false; }
  bool format() { return false; }
  bool info(FSInfo&) { return false; }
  File open(const char*, const char*) { return File(); }
  bool rename(const char*, const char*) { return false; }
  bool remove(const char*) { return false; }
};
extern FSClass LittleFS;
