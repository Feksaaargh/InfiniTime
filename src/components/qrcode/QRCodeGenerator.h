#pragma once
#include "lvgl/src/lv_misc/lv_mem.h"
#include <cstdint>
#include <cstring>
#include <cassert>

namespace Pinetime::Tools {
  // https://www.thonky.com/qr-code-tutorial/error-correction-table
  // Information unique to each version + error correction level.
  struct QRInfo {
    uint16_t capacity;
    uint8_t ecCodewords;
    // TODO: Read the spec and figure out if I actually need to store group block sizes/counts
    uint8_t group1BlockCount;
    uint8_t group1BlockSize;
    uint8_t group2BlockCount;
    uint8_t group2BlockSize;
  };


  // https://www.thonky.com/qr-code-tutorial/alignment-pattern-locations
  // The alignment pattern location start and increment for each qr code version.
  // Only unique per version.
  // TODO: Find formal definition of the placements and adjust as needed
  struct QRAlignmentPlacement {
    uint8_t patternStart;
    uint8_t patternIncrement;
  };


  // A uint8_t array that is built up by appending bits to it, ignoring byte boundaries.
  class AppendableBitArray {
  private:
    uint8_t* arr;
    uint32_t arrSize;
    uint32_t usedBits;

  public:
    AppendableBitArray(uint32_t size);
    AppendableBitArray(AppendableBitArray& other);
    ~AppendableBitArray();

    // Appends number of bits from the LEAST significant side of the value to the array.
    // So AppendBits(0b00001101, 3) would append 0b101 to the array.
    void AppendBits(uint8_t value, uint8_t numBits);

    [[nodiscard]] uint8_t GetByte(uint32_t position) const;
    [[nodiscard]] uint32_t GetUsedBits() const;
    [[nodiscard]] uint32_t GetUsedBytes() const;
  };


  // A 2D canvas of bits.
  // TODO: Change to a QR specific structure (was BitField2D)
  // TODO: Make this the class handling putting data into the final QR code and whatnot
  // Also will need a version field
  class BitField2D {
  private:
    uint16_t width;
    uint16_t height;
    uint32_t arrSize;
    uint8_t* arr;

  public:
    BitField2D(uint16_t width, uint16_t height);
    BitField2D(BitField2D& other);
    ~BitField2D();

    // Get or set bits in the field.
    void Set(uint16_t x, uint16_t y, bool value);
    [[nodiscard]] bool Get(uint16_t x, uint16_t y) const;
    // Fill is a convenience function and is not faster than setting each pixel individually.
    // Bounds are INCLUSIVE.
    // TODO: Make it faster?
    void Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool value);
  };


  // A block of a QR code. Also used to generate EC data.
  class QRBlockWithEC {
  private:
    uint8_t *message;
    uint16_t messageLen;
    uint8_t *ecData;
    uint16_t ecDataLen;

    // Helper class for dealing with Galois field math
    // Is GF(256) with byte-wise modulo 285
    class Alpha {
    private:
      uint8_t exponent;
    public:
      Alpha();
      Alpha(uint8_t exponent);
      static Alpha FromInt(uint8_t integer);
      [[nodiscard]] uint8_t ToInt() const;
      Alpha operator+(const Alpha& other) const;
      Alpha operator*(const Alpha& other) const;
    };

    // Size of destination must be at least [ecCodewordCount+1]
    static void MakeGeneratorPolynomial(uint8_t ecCodewordCount, Alpha *destination);

  public:
    QRBlockWithEC(uint16_t messageLen, uint16_t ecDataLen);
    QRBlockWithEC(QRBlockWithEC &other);
    ~QRBlockWithEC();

    void SetToMsg(uint16_t index, uint8_t value);
    [[nodiscard]] uint8_t GetFromMsg(uint16_t index) const;
    [[nodiscard]] uint8_t GetFromECData(uint16_t index) const;

    void GenerateECData();
  };
}