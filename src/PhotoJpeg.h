#pragma once
#include <stdint.h>

// Validate the bounded JPEG structure needed by the ESP8266 photo player.
// Reader must provide size(), position(), read() and seek(position).
// This deliberately inspects markers only: full JPEG decompression is done
// later by TJpg_Decoder after the active theme has released its heap.
namespace smalltv_photo {

template <typename Reader>
static bool jpegReadByte(Reader& reader, uint8_t& value) {
  const int byte = reader.read();
  if (byte < 0) return false;
  value = static_cast<uint8_t>(byte);
  return true;
}

template <typename Reader>
static bool jpegReadWord(Reader& reader, uint16_t& value) {
  uint8_t high = 0, low = 0;
  if (!jpegReadByte(reader, high) || !jpegReadByte(reader, low)) return false;
  value = static_cast<uint16_t>((uint16_t(high) << 8) | low);
  return true;
}

template <typename Reader>
bool validateBaselineJpeg240(Reader& reader) {
  const uint32_t fileSize = reader.size();
  if (fileSize < 32 || fileSize > 128UL * 1024UL || !reader.seek(0)) return false;

  uint8_t first = 0, second = 0;
  if (!jpegReadByte(reader, first) || !jpegReadByte(reader, second) ||
      first != 0xff || second != 0xd8) return false;

  bool frameFound = false;
  bool scanFound = false;
  uint8_t frameComponents = 0;

  // JPEG headers are a finite sequence of marker segments before SOS.
  for (uint16_t markers = 0; markers < 128 && reader.position() < fileSize; ++markers) {
    uint8_t prefix = 0, marker = 0;
    if (!jpegReadByte(reader, prefix) || prefix != 0xff) return false;
    do {
      if (!jpegReadByte(reader, marker)) return false;
    } while (marker == 0xff);  // marker fill bytes

    // Stuffed bytes and nested SOI/EOI are not valid in the header.
    if (marker == 0x00 || marker == 0xd8 || marker == 0xd9) return false;

    // TEM and restart markers do not carry a segment length.
    if (marker == 0x01 || (marker >= 0xd0 && marker <= 0xd7)) continue;

    uint16_t segmentLength = 0;
    if (!jpegReadWord(reader, segmentLength) || segmentLength < 2) return false;
    const uint32_t payloadBytes = uint32_t(segmentLength) - 2UL;
    const uint32_t payloadStart = reader.position();
    if (payloadStart > fileSize || payloadBytes > fileSize - payloadStart) return false;
    const uint32_t segmentEnd = payloadStart + payloadBytes;

    if (marker == 0xc0) {  // Baseline DCT; the player does not support progressive JPEG.
      if (frameFound || segmentLength < 17) return false;
      uint8_t precision = 0, components = 0;
      uint16_t height = 0, width = 0;
      if (!jpegReadByte(reader, precision) ||
          !jpegReadWord(reader, height) ||
          !jpegReadWord(reader, width) ||
          !jpegReadByte(reader, components)) return false;
      if (precision != 8 || width != 240 || height != 240 ||
          components != 3 || segmentLength != uint16_t(8 + 3 * components)) return false;
      frameComponents = components;
      frameFound = true;
    } else if (marker == 0xda) {  // Start of scan.
      if (!frameFound || segmentLength != uint16_t(6 + 2 * frameComponents)) return false;
      uint8_t scanComponents = 0;
      if (!jpegReadByte(reader, scanComponents) || scanComponents != frameComponents) return false;
      scanFound = true;
      break;
    }

    if (!reader.seek(segmentEnd)) return false;
  }

  if (!frameFound || !scanFound || fileSize < 2 || !reader.seek(fileSize - 2)) return false;
  uint8_t endMarkerA = 0, endMarkerB = 0;
  return jpegReadByte(reader, endMarkerA) && jpegReadByte(reader, endMarkerB) &&
         endMarkerA == 0xff && endMarkerB == 0xd9;
}

}  // namespace smalltv_photo
