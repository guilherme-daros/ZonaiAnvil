#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Protocol {

constexpr uint8_t kStartByte = 0xAA;

enum class Command : uint8_t {
  kPing = 0x00,
  kGetSchema = 0x10,
  kReadValue = 0x20,
  kReadAll = 0x21,
  kWriteValue = 0x30,
  kWriteAll = 0x31,
  kLog = 0x40
};

enum class ParamType : uint8_t { kToggle = 0x01, kSlider = 0x02, kNumeric = 0x03, kString = 0x04 };

#pragma pack(push, 1)
struct PacketHeader {
  uint8_t startByte;
  uint8_t command;
  uint16_t length;
};
#pragma pack(pop)

// Utility to calculate a simple XOR checksum
inline uint8_t CalculateChecksum(const std::vector<uint8_t>& data) {
  uint8_t checksum = 0;
  for (uint8_t b : data) checksum ^= b;
  return checksum;
}

}  // namespace Protocol
