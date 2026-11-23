#include "PhotoJpeg.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>

struct MemoryReader {
  explicit MemoryReader(const std::vector<uint8_t>& source) : bytes(source) {}
  uint32_t size() const { return static_cast<uint32_t>(bytes.size()); }
  uint32_t position() const { return static_cast<uint32_t>(cursor); }
  int read() { return cursor < bytes.size() ? bytes[cursor++] : -1; }
  bool seek(uint32_t position) {
    if (position > bytes.size()) return false;
    cursor = position;
    return true;
  }
  const std::vector<uint8_t>& bytes;
  size_t cursor = 0;
};

static std::vector<uint8_t> jpeg(uint8_t frameMarker = 0xc0,
                                 uint16_t width = 240,
                                 uint16_t height = 240,
                                 uint8_t components = 3) {
  std::vector<uint8_t> out = {0xff, 0xd8, 0xff, 0xe0, 0x00, 0x04, 0x4a, 0x46};
  const uint16_t frameLength = static_cast<uint16_t>(8 + 3 * components);
  out.insert(out.end(), {0xff, frameMarker,
    static_cast<uint8_t>(frameLength >> 8), static_cast<uint8_t>(frameLength),
    0x08, static_cast<uint8_t>(height >> 8), static_cast<uint8_t>(height),
    static_cast<uint8_t>(width >> 8), static_cast<uint8_t>(width), components});
  for (uint8_t i = 0; i < components; ++i) {
    out.push_back(static_cast<uint8_t>(i + 1));
    out.push_back(0x11);
    out.push_back(i ? 1 : 0);
  }
  const uint16_t scanLength = static_cast<uint16_t>(6 + 2 * components);
  out.insert(out.end(), {0xff, 0xda,
    static_cast<uint8_t>(scanLength >> 8), static_cast<uint8_t>(scanLength),
    components});
  for (uint8_t i = 0; i < components; ++i) {
    out.push_back(static_cast<uint8_t>(i + 1));
    out.push_back(i ? 0x11 : 0x00);
  }
  out.insert(out.end(), {0x00, 0x3f, 0x00, 0x34, 0xff, 0x00, 0x56, 0xff, 0xd9});
  return out;
}

static bool validate(const std::vector<uint8_t>& bytes) {
  MemoryReader reader(bytes);
  return smalltv_photo::validateBaselineJpeg240(reader);
}

int main() {
  assert(validate(jpeg()));
  assert(!validate(jpeg(0xc2)));               // progressive JPEG is unsupported
  assert(!validate(jpeg(0xc0, 239, 240)));     // wrong width
  assert(!validate(jpeg(0xc0, 240, 239)));     // wrong height
  assert(!validate(jpeg(0xc0, 240, 240, 1))); // grayscale is not accepted
  auto missingEoi = jpeg();
  missingEoi.pop_back();
  missingEoi.pop_back();
  assert(!validate(missingEoi));
  auto truncated = jpeg();
  truncated[10] = 0xff; // corrupt the SOF segment length
  truncated[11] = 0xff;
  assert(!validate(truncated));
  std::puts("photo JPEG validation tests passed");
  return 0;
}
