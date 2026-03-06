#pragma once
#include <cstdint>

namespace Pinetime::Tools {
  struct QRInfo {
    uint16_t capacity;
    uint8_t ecCodewords;
    uint8_t group1BlockCount;
    uint8_t group1BlockSize;
    uint8_t group2BlockCount;
    uint8_t group2BlockSize;
  };
}