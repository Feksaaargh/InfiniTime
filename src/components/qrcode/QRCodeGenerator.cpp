#include "QRCodeGenerator.h"

using namespace Pinetime::Tools;

namespace {
  constexpr uint8_t MAX_BLOCK_EC_LENGTH = 30;
  constexpr uint16_t MAX_BLOCK_MESSAGE_LENGTH = 123;

  // TODO: Make this a LOT nicer
  // Information for LOW error correction QR codes. Index 0 is junk, 1-40 contain the respective versions.
  QRInfo versionInfos[41] = {
    {0},
    {.capacity = 19, .ecCodewords = 7, .group1BlockCount = 1, .group1BlockSize = 19, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 34, .ecCodewords = 10, .group1BlockCount = 1, .group1BlockSize = 34, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 55, .ecCodewords = 15, .group1BlockCount = 1, .group1BlockSize = 55, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 80, .ecCodewords = 20, .group1BlockCount = 1, .group1BlockSize = 80, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 108, .ecCodewords = 26, .group1BlockCount = 1, .group1BlockSize = 108, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 136, .ecCodewords = 18, .group1BlockCount = 2, .group1BlockSize = 68, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 156, .ecCodewords = 20, .group1BlockCount = 2, .group1BlockSize = 78, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 194, .ecCodewords = 24, .group1BlockCount = 2, .group1BlockSize = 97, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 232, .ecCodewords = 30, .group1BlockCount = 2, .group1BlockSize = 116, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 274, .ecCodewords = 18, .group1BlockCount = 2, .group1BlockSize = 68, .group2BlockCount = 2, .group2BlockSize = 69},
    {.capacity = 324, .ecCodewords = 20, .group1BlockCount = 4, .group1BlockSize = 81, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 370, .ecCodewords = 24, .group1BlockCount = 2, .group1BlockSize = 92, .group2BlockCount = 2, .group2BlockSize = 93},
    {.capacity = 428, .ecCodewords = 26, .group1BlockCount = 4, .group1BlockSize = 107, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 461, .ecCodewords = 30, .group1BlockCount = 3, .group1BlockSize = 115, .group2BlockCount = 1, .group2BlockSize = 116},
    {.capacity = 523, .ecCodewords = 22, .group1BlockCount = 5, .group1BlockSize = 87, .group2BlockCount = 1, .group2BlockSize = 88},
    {.capacity = 589, .ecCodewords = 24, .group1BlockCount = 5, .group1BlockSize = 98, .group2BlockCount = 1, .group2BlockSize = 99},
    {.capacity = 647, .ecCodewords = 28, .group1BlockCount = 1, .group1BlockSize = 107, .group2BlockCount = 5, .group2BlockSize = 108},
    {.capacity = 721, .ecCodewords = 30, .group1BlockCount = 5, .group1BlockSize = 120, .group2BlockCount = 1, .group2BlockSize = 121},
    {.capacity = 795, .ecCodewords = 28, .group1BlockCount = 3, .group1BlockSize = 113, .group2BlockCount = 4, .group2BlockSize = 114},
    {.capacity = 861, .ecCodewords = 28, .group1BlockCount = 3, .group1BlockSize = 107, .group2BlockCount = 5, .group2BlockSize = 108},
    {.capacity = 932, .ecCodewords = 28, .group1BlockCount = 4, .group1BlockSize = 116, .group2BlockCount = 4, .group2BlockSize = 117},
    {.capacity = 1006, .ecCodewords = 28, .group1BlockCount = 2, .group1BlockSize = 111, .group2BlockCount = 7, .group2BlockSize = 112},
    {.capacity = 1094, .ecCodewords = 30, .group1BlockCount = 4, .group1BlockSize = 121, .group2BlockCount = 5, .group2BlockSize = 122},
    {.capacity = 1174, .ecCodewords = 30, .group1BlockCount = 6, .group1BlockSize = 117, .group2BlockCount = 4, .group2BlockSize = 118},
    {.capacity = 1276, .ecCodewords = 26, .group1BlockCount = 8, .group1BlockSize = 106, .group2BlockCount = 4, .group2BlockSize = 107},
    {.capacity = 1370, .ecCodewords = 28, .group1BlockCount = 10, .group1BlockSize = 114, .group2BlockCount = 2, .group2BlockSize = 115},
    {.capacity = 1468, .ecCodewords = 30, .group1BlockCount = 8, .group1BlockSize = 122, .group2BlockCount = 4, .group2BlockSize = 123},
    {.capacity = 1531, .ecCodewords = 30, .group1BlockCount = 3, .group1BlockSize = 117, .group2BlockCount = 10, .group2BlockSize = 118},
    {.capacity = 1631, .ecCodewords = 30, .group1BlockCount = 7, .group1BlockSize = 116, .group2BlockCount = 7, .group2BlockSize = 117},
    {.capacity = 1735, .ecCodewords = 30, .group1BlockCount = 5, .group1BlockSize = 115, .group2BlockCount = 10, .group2BlockSize = 116},
    {.capacity = 1843, .ecCodewords = 30, .group1BlockCount = 13, .group1BlockSize = 115, .group2BlockCount = 3, .group2BlockSize = 116},
    {.capacity = 1955, .ecCodewords = 30, .group1BlockCount = 17, .group1BlockSize = 115, .group2BlockCount = 0, .group2BlockSize = 0},
    {.capacity = 2071, .ecCodewords = 30, .group1BlockCount = 17, .group1BlockSize = 115, .group2BlockCount = 1, .group2BlockSize = 116},
    {.capacity = 2191, .ecCodewords = 30, .group1BlockCount = 13, .group1BlockSize = 115, .group2BlockCount = 6, .group2BlockSize = 116},
    {.capacity = 2306, .ecCodewords = 30, .group1BlockCount = 12, .group1BlockSize = 121, .group2BlockCount = 7, .group2BlockSize = 122},
    {.capacity = 2434, .ecCodewords = 30, .group1BlockCount = 6, .group1BlockSize = 121, .group2BlockCount = 14, .group2BlockSize = 122},
    {.capacity = 2566, .ecCodewords = 30, .group1BlockCount = 17, .group1BlockSize = 122, .group2BlockCount = 4, .group2BlockSize = 123},
    {.capacity = 2702, .ecCodewords = 30, .group1BlockCount = 4, .group1BlockSize = 122, .group2BlockCount = 18, .group2BlockSize = 123},
    {.capacity = 2812, .ecCodewords = 30, .group1BlockCount = 20, .group1BlockSize = 117, .group2BlockCount = 4, .group2BlockSize = 118},
    {.capacity = 2956, .ecCodewords = 30, .group1BlockCount = 19, .group1BlockSize = 118, .group2BlockCount = 6, .group2BlockSize = 119},
  };
}

AppendableBitArray::AppendableBitArray(uint32_t size) {
  usedBits = 0;
  arrSize = size;
  arr = static_cast<uint8_t*>(lv_mem_alloc(arrSize));
  for (uint32_t i = 0; i < arrSize; i++)
    arr[i] = 0;
}

AppendableBitArray::AppendableBitArray(AppendableBitArray& other) {
  usedBits = other.usedBits;
  arrSize = other.arrSize;
  arr = static_cast<uint8_t*>(lv_mem_alloc(arrSize));
  memcpy(arr, other.arr, arrSize);
}

AppendableBitArray::~AppendableBitArray() {
  lv_mem_free(arr);
}

void AppendableBitArray::AppendBits(uint8_t value, uint8_t numBits) {
  assert(usedBits + numBits < arrSize * 8);

  if (numBits == 0)
    return;
  if (numBits > 8)
    numBits = 8;

  value &= 0xFF >> (8 - numBits);
  uint32_t targetByte = usedBits / 8;
  uint8_t remainingSpaceInByte = 8 - (usedBits & 8);

  if (numBits <= remainingSpaceInByte) {
    // Value fits entirely in current byte
    arr[targetByte] |= value << (remainingSpaceInByte - numBits);
  } else {
    // Value needs to be split across two bytes
    arr[targetByte] |= value >> (numBits - remainingSpaceInByte);
    arr[targetByte + 1] = value << (8 - (numBits - remainingSpaceInByte));
  }

  usedBits += numBits;
}

uint8_t AppendableBitArray::GetByte(uint32_t position) const {
  return arr[position];
}

uint32_t AppendableBitArray::GetUsedBits() const {
  return usedBits;
}

uint32_t AppendableBitArray::GetUsedBytes() const {
  return usedBits / 8 + (usedBits % 8 != 0);
}

BitField2D::BitField2D(uint16_t width, uint16_t height) {
  this->width = width;
  this->height = height;
  const uint32_t area = width * height;
  arrSize = area / 8 + (area % 8 != 0);
  arr = static_cast<uint8_t*>(lv_mem_alloc(arrSize));
}

BitField2D::BitField2D(BitField2D& other) {
  width = other.width;
  height = other.height;
  arrSize = other.arrSize;
  arr = static_cast<uint8_t*>(lv_mem_alloc(arrSize));
  memcpy(arr, other.arr, arrSize);
}

BitField2D::~BitField2D() {
  lv_mem_free(arr);
}

void BitField2D::Set(uint16_t x, uint16_t y, bool value) {
  assert(x < width && y < height); // Accesses must be in bounds

  uint32_t targetByte = (y * width + x) / 8;
  uint8_t targetBit = (y * width + x) % 8;
  uint8_t mask = 0x80 >> targetBit;
  if (value) {
    arr[targetByte] |= mask;
  } else {
    arr[targetByte] &= ~mask;
  }
}

bool BitField2D::Get(uint16_t x, uint16_t y) const {
  assert(x < width && y < height); // Accesses must be in bounds

  uint32_t targetByte = (y * width + x) / 8;
  uint8_t targetBit = (y * width + x) % 8;
  uint8_t mask = 0x80 >> targetBit;
  return arr[targetByte] & mask;
}

void BitField2D::Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool value) {
  assert(x1 <= x2 && y1 <= y2); // (x1, y1) must be smaller than (x2, y2)

  for (uint16_t x = x1; x <= x2; x++) {
    for (uint16_t y = y1; y <= y2; y++) {
      this->Set(x, y, value);
    }
  }
}

QRBlockWithEC::Alpha::Alpha() {
  this->exponent = 1;
}

QRBlockWithEC::Alpha::Alpha(uint8_t exponent) {
  this->exponent = exponent;
}

QRBlockWithEC::Alpha QRBlockWithEC::Alpha::FromInt(uint8_t integer) {
  assert(integer != 0); // Cannot convert integer 0 to Alpha

  static constexpr uint8_t intToAlphaTable[] = {
    0,   0,   1,   25,  2,   50,  26,  198, 3,   223, 51,  238, 27,  104, 199, 75,  4,   100, 224, 14,  52,  141, 239, 129, 28,  193,
    105, 248, 200, 8,   76,  113, 5,   138, 101, 47,  225, 36,  15,  33,  53,  147, 142, 218, 240, 18,  130, 69,  29,  181, 194, 125,
    106, 39,  249, 185, 201, 154, 9,   120, 77,  228, 114, 166, 6,   191, 139, 98,  102, 221, 48,  253, 226, 152, 37,  179, 16,  145,
    34,  136, 54,  208, 148, 206, 143, 150, 219, 189, 241, 210, 19,  92,  131, 56,  70,  64,  30,  66,  182, 163, 195, 72,  126, 110,
    107, 58,  40,  84,  250, 133, 186, 61,  202, 94,  155, 159, 10,  21,  121, 43,  78,  212, 229, 172, 115, 243, 167, 87,  7,   112,
    192, 247, 140, 128, 99,  13,  103, 74,  222, 237, 49,  197, 254, 24,  227, 165, 153, 119, 38,  184, 180, 124, 17,  68,  146, 217,
    35,  32,  137, 46,  55,  63,  209, 91,  149, 188, 207, 205, 144, 135, 151, 178, 220, 252, 190, 97,  242, 86,  211, 171, 20,  42,
    93,  158, 132, 60,  57,  83,  71,  109, 65,  162, 31,  45,  67,  216, 183, 123, 164, 118, 196, 23,  73,  236, 127, 12,  111, 246,
    108, 161, 59,  82,  41,  157, 85,  170, 251, 96,  134, 177, 187, 204, 62,  90,  203, 89,  95,  176, 156, 169, 160, 81,  11,  245,
    22,  235, 122, 117, 44,  215, 79,  174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168, 80,  88,  175};
  return Alpha(intToAlphaTable[integer]);
}

uint8_t QRBlockWithEC::Alpha::ToInt() const {
  static constexpr uint8_t alphaToIntTable[] = {
    1,   2,   4,   8,   16,  32,  64,  128, 29,  58,  116, 232, 205, 135, 19,  38,  76,  152, 45,  90,  180, 117, 234, 201, 143, 3,
    6,   12,  24,  48,  96,  192, 157, 39,  78,  156, 37,  74,  148, 53,  106, 212, 181, 119, 238, 193, 159, 35,  70,  140, 5,   10,
    20,  40,  80,  160, 93,  186, 105, 210, 185, 111, 222, 161, 95,  190, 97,  194, 153, 47,  94,  188, 101, 202, 137, 15,  30,  60,
    120, 240, 253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163, 91,  182, 113, 226, 217, 175, 67,  134, 17,  34,  68,  136,
    13,  26,  52,  104, 208, 189, 103, 206, 129, 31,  62,  124, 248, 237, 199, 147, 59,  118, 236, 197, 151, 51,  102, 204, 133, 23,
    46,  92,  184, 109, 218, 169, 79,  158, 33,  66,  132, 21,  42,  84,  168, 77,  154, 41,  82,  164, 85,  170, 73,  146, 57,  114,
    228, 213, 183, 115, 230, 209, 191, 99,  198, 145, 63,  126, 252, 229, 215, 179, 123, 246, 241, 255, 227, 219, 171, 75,  150, 49,
    98,  196, 149, 55,  110, 220, 165, 87,  174, 65,  130, 25,  50,  100, 200, 141, 7,   14,  28,  56,  112, 224, 221, 167, 83,  166,
    81,  162, 89,  178, 121, 242, 249, 239, 195, 155, 43,  86,  172, 69,  138, 9,   18,  36,  72,  144, 61,  122, 244, 245, 247, 243,
    251, 235, 203, 139, 11,  22,  44,  88,  176, 125, 250, 233, 207, 131, 27,  54,  108, 216, 173, 71,  142, 1};
  return alphaToIntTable[exponent];
}

QRBlockWithEC::Alpha QRBlockWithEC::Alpha::operator*(const Alpha& other) const {
  return Alpha(((uint16_t) exponent + (uint16_t) other.exponent) % 255);
}

QRBlockWithEC::Alpha QRBlockWithEC::Alpha::operator+(const Alpha& other) const {
  return Alpha::FromInt(this->ToInt() ^ other.ToInt());
}

QRBlockWithEC::QRBlockWithEC(uint16_t messageLen, uint16_t ecDataLen) {
  assert(ecDataLen <= MAX_BLOCK_EC_LENGTH);
  assert(messageLen <= MAX_BLOCK_MESSAGE_LENGTH);

  this->messageLen = messageLen;
  this->ecDataLen = ecDataLen;
  message = static_cast<uint8_t*>(lv_mem_alloc(messageLen));
  ecData = static_cast<uint8_t*>(lv_mem_alloc(ecDataLen));
}

QRBlockWithEC::QRBlockWithEC(QRBlockWithEC& other) {
  this->messageLen = other.messageLen;
  this->ecDataLen = other.ecDataLen;
  message = static_cast<uint8_t*>(lv_mem_alloc(messageLen));
  ecData = static_cast<uint8_t*>(lv_mem_alloc(ecDataLen));
  memcpy(message, other.message, messageLen);
  memcpy(ecData, other.ecData, ecDataLen);
}

QRBlockWithEC::~QRBlockWithEC() {
  lv_mem_free(message);
  lv_mem_free(ecData);
}

void QRBlockWithEC::SetToMsg(uint16_t index, uint8_t value) {
  message[index] = value;
}

uint8_t QRBlockWithEC::GetFromMsg(uint16_t index) const {
  return message[index];
}

uint8_t QRBlockWithEC::GetFromECData(uint16_t index) const {
  return ecData[index];
}

void QRBlockWithEC::MakeGeneratorPolynomial(uint8_t ecCodewords, Alpha* destination) {
  assert(ecCodewords < 255); // Cannot make a polynomial for >= 255 codewords

  for (uint8_t i = 0; i <= ecCodewords; i++)
    destination[i] = Alpha::FromInt(0);
  // Repeatedly multiply the polynomial (at destination) by Alpha(0)x^1 + Alpha(multStep)x^0
  for (uint8_t multStep = 0; multStep < ecCodewords; multStep++) {
    for (uint8_t i = ecCodewords - multStep; i < ecCodewords; i++) {
      destination[i] = destination[i] * Alpha(multStep) + destination[i + 1];
    }
    destination[ecCodewords] = destination[ecCodewords] * Alpha(multStep);
  }
}

void QRBlockWithEC::GenerateECData() {
  Alpha generatorPolynomial[MAX_BLOCK_EC_LENGTH + 1];
  MakeGeneratorPolynomial(ecDataLen, generatorPolynomial);

  uint8_t longDivisionWork[MAX_BLOCK_MESSAGE_LENGTH + MAX_BLOCK_EC_LENGTH];
  for (uint16_t i = 0; i < messageLen; i++)
    longDivisionWork[i] = message[i];
  for (uint16_t i = messageLen; i < MAX_BLOCK_MESSAGE_LENGTH + MAX_BLOCK_EC_LENGTH; i++)
    longDivisionWork[i] = 0;

  // Perform long division steps
  uint8_t multipliedGeneratorPolynomial[MAX_BLOCK_EC_LENGTH + 1];
  for (uint8_t divisionStep = 0; divisionStep < messageLen; divisionStep++) {
    if (longDivisionWork[divisionStep] == 0)
      continue;
    // Multiply the generator polynomial by the leading value
    for (uint16_t i = 0; i < ecDataLen + 1; i++)
      multipliedGeneratorPolynomial[i] = (generatorPolynomial[i] * Alpha::FromInt(longDivisionWork[divisionStep])).ToInt();
    // Subtract multiplied generator polynomial from current work
    // Since addition and subtraction are identical in Galois fields, just do an addition.
    // However, since both values are in their integer representations, can do a simple XOR rather than going through the Alpha class.
    for (uint16_t i = 0; i < ecDataLen + 1; i++)
      longDivisionWork[divisionStep + i] ^= multipliedGeneratorPolynomial[i];
  }

  for (uint16_t i = 0; i < ecDataLen; i++) {
    ecData[i] = longDivisionWork[messageLen + i];
  }
}