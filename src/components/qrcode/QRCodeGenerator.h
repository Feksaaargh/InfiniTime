#pragma once
#include "lvgl/src/lv_misc/lv_mem.h"
#include <cstdint>
#include <cstring>
#include <cassert>
#include <memory>

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
  // If no alignment patterns are to be generated for the given version, patternStart must be 0.
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
    AppendableBitArray(const AppendableBitArray& other);
    ~AppendableBitArray();
    AppendableBitArray& operator=(const AppendableBitArray& other);

    // Appends number of bits from the LEAST significant side of the value to the array.
    // So AppendBits(0b00001101, 3) would append 0b101 to the array.
    void AppendBits(uint8_t value, uint8_t numBits);

    [[nodiscard]] uint8_t GetByte(uint32_t position) const;
    [[nodiscard]] uint32_t GetUsedBits() const;
    [[nodiscard]] uint32_t GetUsedBytes() const;
  };

  // A class for creating and holding a 2D canvas of bits representing a QR code.
  class QRCodeModules {
  private:
    uint8_t version;
    uint32_t arrSize;
    uint8_t* arr;

  public:
    QRCodeModules(uint8_t version);
    QRCodeModules(QRCodeModules& other);
    ~QRCodeModules();
    QRCodeModules& operator=(const QRCodeModules& other);

    [[nodiscard]] uint16_t GetSize() const;
    [[nodiscard]] uint16_t GetVersion() const;

    // Get or set bits in the field
    void SetModule(uint16_t x, uint16_t y, bool value);
    [[nodiscard]] bool GetModule(uint16_t x, uint16_t y) const;
    // Fill is a convenience function and is not faster than setting each pixel individually
    // Bounds are INCLUSIVE
    // TODO: Make it faster?
    void Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool value);
  };

  // A block of a QR code. Also used to generate EC data.
  class QRBlockWithEC {
  private:
    uint8_t* message;
    uint16_t messageLen;
    uint8_t* ecData;
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
    static void MakeGeneratorPolynomial(uint8_t ecCodewordCount, Alpha* destination);

  public:
    QRBlockWithEC(uint16_t messageLen, uint16_t ecDataLen);
    QRBlockWithEC(QRBlockWithEC& other);
    QRBlockWithEC() : QRBlockWithEC(0, 0) {};
    ~QRBlockWithEC();
    QRBlockWithEC& operator=(QRBlockWithEC& other);

    [[nodiscard]] uint16_t GetMsgLen() const;
    [[nodiscard]] uint16_t GetECLen() const;

    void SetToMsg(uint16_t index, uint8_t value);
    [[nodiscard]] uint8_t GetFromMsg(uint16_t index) const;
    [[nodiscard]] uint8_t GetFromECData(uint16_t index) const;

    void GenerateECData();
  };

  // A class to contain the main generation process
  class QRCodeGenerator {
  private:
    // Returns data of appropriate size given the version. Data must fit in the given version.
    static std::unique_ptr<uint8_t[]> GenerateQRContents(const char* data, int dataLen, int version);
    // Structures the final code from the given contents. Deletes the contents variable.
    static QRCodeModules StructureFinalCode(std::unique_ptr<uint8_t[]> contents, int version);

    // Used while generating the data contents. Returns 41 if data cannot fit in any version.
    static int FindMinFittingVersion(uint16_t dataLength);

    // Used while structuring the final code
    static void PlaceReservedModules(QRCodeModules& qrCode);
    static bool IsReservedModule(uint8_t version, int x, int y);
    static int FindAndApplyOptimalMask(QRCodeModules& qrCode); // Returns optimal mask ID
    static void ApplyMask(QRCodeModules& qrCode, int mask);
    static void PlaceMetadata(QRCodeModules& qrCode, int chosenMask); // Assumes low error correction
    static void PlaceVersionInfo(QRCodeModules& qrCode);

  public:
    QRCodeGenerator() = delete;

    // TODO: Change weird uint16_t things to regular ints
    // Returns QRCodeModules of size 0 in case of error
    static QRCodeModules GenerateQRCode(const char* data, uint16_t dataLen);
  };
}