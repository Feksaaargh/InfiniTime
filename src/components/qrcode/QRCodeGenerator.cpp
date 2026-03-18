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

  // Alignment pattern spacings for all QR code versions
  QRAlignmentPlacement versionPatternSpacings[41] = {{0},
                                                     {.patternStart = 0, .patternIncrement = 255},
                                                     {.patternStart = 18, .patternIncrement = 255},
                                                     {.patternStart = 22, .patternIncrement = 255},
                                                     {.patternStart = 26, .patternIncrement = 255},
                                                     {.patternStart = 30, .patternIncrement = 255},
                                                     {.patternStart = 34, .patternIncrement = 255},
                                                     {.patternStart = 22, .patternIncrement = 16},
                                                     {.patternStart = 24, .patternIncrement = 18},
                                                     {.patternStart = 26, .patternIncrement = 20},
                                                     {.patternStart = 28, .patternIncrement = 22},
                                                     {.patternStart = 30, .patternIncrement = 24},
                                                     {.patternStart = 32, .patternIncrement = 26},
                                                     {.patternStart = 34, .patternIncrement = 28},
                                                     {.patternStart = 26, .patternIncrement = 20},
                                                     {.patternStart = 26, .patternIncrement = 22},
                                                     {.patternStart = 26, .patternIncrement = 24},
                                                     {.patternStart = 30, .patternIncrement = 24},
                                                     {.patternStart = 30, .patternIncrement = 26},
                                                     {.patternStart = 30, .patternIncrement = 28},
                                                     {.patternStart = 34, .patternIncrement = 28},
                                                     {.patternStart = 28, .patternIncrement = 22},
                                                     {.patternStart = 26, .patternIncrement = 24},
                                                     {.patternStart = 30, .patternIncrement = 24},
                                                     {.patternStart = 28, .patternIncrement = 26},
                                                     {.patternStart = 32, .patternIncrement = 26},
                                                     {.patternStart = 30, .patternIncrement = 28},
                                                     {.patternStart = 34, .patternIncrement = 28},
                                                     {.patternStart = 26, .patternIncrement = 24},
                                                     {.patternStart = 30, .patternIncrement = 24},
                                                     {.patternStart = 26, .patternIncrement = 26},
                                                     {.patternStart = 30, .patternIncrement = 26},
                                                     {.patternStart = 34, .patternIncrement = 26},
                                                     {.patternStart = 30, .patternIncrement = 28},
                                                     {.patternStart = 34, .patternIncrement = 28},
                                                     {.patternStart = 30, .patternIncrement = 24},
                                                     {.patternStart = 24, .patternIncrement = 26},
                                                     {.patternStart = 28, .patternIncrement = 26},
                                                     {.patternStart = 32, .patternIncrement = 26},
                                                     {.patternStart = 26, .patternIncrement = 28},
                                                     {.patternStart = 30, .patternIncrement = 28}};
}

AppendableBitArray::AppendableBitArray(uint32_t size) {
  usedBits = 0;
  arrSize = size;
  arr = new uint8_t[arrSize];
  for (uint32_t i = 0; i < arrSize; i++)
    arr[i] = 0;
}

AppendableBitArray::AppendableBitArray(const AppendableBitArray& other) {
  usedBits = other.usedBits;
  arrSize = other.arrSize;
  arr = new uint8_t[arrSize];
  memcpy(arr, other.arr, arrSize);
}

AppendableBitArray::~AppendableBitArray() {
  delete[] arr;
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

QRCodeModules::QRCodeModules(uint8_t version) {
  this->version = version;
  const uint32_t area = this->GetSize() * this->GetSize();
  arrSize = area / 8 + (area % 8 != 0);
  arr = new uint8_t[arrSize];
}

QRCodeModules::QRCodeModules(QRCodeModules& other) {
  version = other.version;
  arrSize = other.arrSize;
  arr = new uint8_t[arrSize];
  memcpy(arr, other.arr, arrSize);
}

QRCodeModules::~QRCodeModules() {
  delete[] arr;
}

uint16_t QRCodeModules::GetSize() const {
  return version * 4 + 17;
}

uint16_t QRCodeModules::GetVersion() const {
  return version;
}

void QRCodeModules::SetModule(uint16_t x, uint16_t y, bool value) {
  assert(x < width && y < height); // Accesses must be in bounds

  uint32_t targetByte = (y * this->GetSize() + x) / 8;
  uint8_t targetBit = (y * this->GetSize() + x) % 8;
  uint8_t mask = 0x80 >> targetBit;
  if (value) {
    arr[targetByte] |= mask;
  } else {
    arr[targetByte] &= ~mask;
  }
}

bool QRCodeModules::GetModule(uint16_t x, uint16_t y) const {
  assert(x < width && y < height); // Accesses must be in bounds

  uint32_t targetByte = (y * this->GetSize() + x) / 8;
  uint8_t targetBit = (y * this->GetSize() + x) % 8;
  uint8_t mask = 0x80 >> targetBit;
  return arr[targetByte] & mask;
}

void QRCodeModules::Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool value) {
  assert(x1 <= x2 && y1 <= y2); // (x1, y1) must be smaller than (x2, y2)

  for (uint16_t x = x1; x <= x2; x++) {
    for (uint16_t y = y1; y <= y2; y++) {
      this->SetModule(x, y, value);
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

  // intToAlphaTable[0] is junk
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
  message = new uint8_t[messageLen];
  ecData = new uint8_t[ecDataLen];
}

QRBlockWithEC::QRBlockWithEC(QRBlockWithEC& other) {
  this->messageLen = other.messageLen;
  this->ecDataLen = other.ecDataLen;
  message = new uint8_t[messageLen];
  ecData = new uint8_t[ecDataLen];
  memcpy(message, other.message, messageLen);
  memcpy(ecData, other.ecData, ecDataLen);
}

QRBlockWithEC::~QRBlockWithEC() {
  delete[] message;
  delete[] ecData;
}

uint16_t QRBlockWithEC::GetMsgLen() const {
  return messageLen;
}

uint16_t QRBlockWithEC::GetECLen() const {
  return ecDataLen;
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

std::unique_ptr<uint8_t[]> QRCodeGenerator::GenerateQRContents(const char* data, const int dataLen, const int version) {
  QRInfo versionInfo = versionInfos[version];
  auto baseContents = std::make_unique<AppendableBitArray>(versionInfo.capacity);

  // Byte mode indicator. Currently hardcoded to byte mode.
  baseContents->AppendBits(0b0100, 4);

  // Data length is 8 bits below version 10, and 16 bits at version 10 and above
  if (version < 10) {
    baseContents->AppendBits(dataLen & 0xFF, 8);
  } else {
    baseContents->AppendBits(dataLen >> 8, 8);
    baseContents->AppendBits(dataLen & 0xFF, 8);
  }

  // Append data
  for (int i = 0; i < dataLen; i++) {
    baseContents->AppendBits(data[i], 8);
  }

  // Terminator data
  // Add up to 4 zeroes if they fit in the data, then zero pad to next byte
  baseContents->AppendBits(0x0, std::min((uint32_t) 4, (versionInfo.capacity * 8) - baseContents->GetUsedBits()));
  baseContents->AppendBits(0x0, (baseContents->GetUsedBytes() * 8) - baseContents->GetUsedBits());
  // Repeat 0xEC and 0x11 until full
  while (true) {
    if (baseContents->GetUsedBytes() == versionInfo.capacity)
      break;
    baseContents->AppendBits(0xEC, 8);
    if (baseContents->GetUsedBytes() == versionInfo.capacity)
      break;
    baseContents->AppendBits(0x11, 8);
  }

  // Create blocks
  auto* qrBlocks = new QRBlockWithEC[versionInfo.group1BlockCount + versionInfo.group2BlockCount];
  for (uint8_t i = 0; i < versionInfo.group1BlockCount; i++) {
    qrBlocks[i] = QRBlockWithEC(versionInfo.group1BlockSize, versionInfo.ecCodewords);
  }
  for (uint8_t i = 0; i < versionInfo.group2BlockCount; i++) {
    qrBlocks[versionInfo.group1BlockCount + i] = QRBlockWithEC(versionInfo.group2BlockSize, versionInfo.ecCodewords);
  }

  // Copy contents into blocks and generate their EC datas
  int contentsPosition = 0;
  for (int i = 0; i < versionInfo.group1BlockCount + versionInfo.group2BlockCount; i++) {
    QRBlockWithEC& block = qrBlocks[i];
    for (uint16_t j = 0; j < block.GetMsgLen(); j++) {
      block.SetToMsg(j, baseContents->GetByte(contentsPosition));
      contentsPosition++;
    }
    block.GenerateECData();
  }

  // Delete baseContents since it is no longer needed
  baseContents.reset();

  // Structuring the final message

  // Plus one since some versions require an extra few bits at the end
  int finalDataLen = versionInfo.group1BlockSize * versionInfo.group1BlockCount +
                     versionInfo.group2BlockSize * versionInfo.group2BlockCount +
                     versionInfo.ecCodewords * (versionInfo.group1BlockCount + versionInfo.group2BlockCount) + 1;

  auto finalData = std::make_unique<uint8_t[]>(finalDataLen);
  int finalDataPosition = 0;
  // Interleave message
  for (int dataIdx = 0; dataIdx < std::max(versionInfo.group1BlockSize, versionInfo.group2BlockSize); dataIdx++) {
    for (int blockIdx = 0; blockIdx < versionInfo.group1BlockCount + versionInfo.group2BlockCount; blockIdx++) {
      QRBlockWithEC& block = qrBlocks[blockIdx];
      if (dataIdx >= block.GetMsgLen())
        continue;
      finalData[finalDataPosition] = block.GetFromMsg(dataIdx);
      finalDataPosition++;
    }
  }
  // Interleave error correction
  for (int dataIdx = 0; dataIdx < versionInfo.ecCodewords; dataIdx++) {
    for (int blockIdx = 0; blockIdx < versionInfo.group1BlockCount + versionInfo.group2BlockCount; blockIdx++) {
      finalData[finalDataPosition] = qrBlocks[blockIdx].GetFromECData(dataIdx);
      finalDataPosition++;
    }
  }

  return finalData;
}

QRCodeModules QRCodeGenerator::StructureFinalCode(std::unique_ptr<uint8_t[]> contents, const int version) {
  auto finalCode = QRCodeModules(version);

  PlaceReservedModules(finalCode);

  // Add contents to the qr code
  const int qrSize = finalCode.GetSize();
  int currentByte = 0;
  uint8_t bitMask = 0x80;
  int curCol = qrSize - 1; // Right side of the current 2 module wide column being populated
  // TODO: Make this compact (duplicated code in going up and going down, just the y for loop is different)
  while (curCol > 0) {
    // check if going up or down
    if ((curCol > 5 && (curCol / 2) % 2 == 0) || curCol == 3) {
      // going up
      for (int y = qrSize - 1; y >= 0; y--) {
        for (int x = curCol; x < curCol - 2; x--) {
          if (IsReservedModule(version, x, y))
            continue;
          finalCode.SetModule(x, y, contents[currentByte] & bitMask);
          bitMask >>= 1;
          if (!bitMask) {
            bitMask = 0x80;
            currentByte += 1;
          }
        }
      }
    } else {
      // going down
      for (int y = 0; y < qrSize; y++) {
        for (int x = curCol; x < curCol - 2; x--) {
          if (IsReservedModule(version, x, y))
            continue;
          finalCode.SetModule(x, y, contents[currentByte] & bitMask);
          bitMask >>= 1;
          if (!bitMask) {
            bitMask = 0x80;
            currentByte += 1;
          }
        }
      }
    }
    curCol -= 2;
    if (curCol == 6)
      curCol -= 1;
  }

  // Apply mask
  const int chosenMask = FindAndApplyOptimalMask(finalCode);

  // Place metadata
  PlaceMetadata(finalCode, chosenMask);

  // Place version info if needed
  if (version >= 7)
    PlaceVersionInfo(finalCode);

  return finalCode;
}

int QRCodeGenerator::FindMinFittingVersion(uint16_t dataLength) {
  for (int i = 1; i < 41; i++) {
    if (versionInfos[i].capacity > dataLength) {
      return i;
    }
  }
  return 41;
}

void QRCodeGenerator::PlaceReservedModules(QRCodeModules& qrCode) {
  const uint16_t qrSize = qrCode.GetSize();
  const QRAlignmentPlacement& alignerPlacement = versionPatternSpacings[qrCode.GetVersion()];

  // Finder patterns
  qrCode.Fill(0, 0, 6, 6, true);
  qrCode.Fill(1, 1, 5, 5, false);
  qrCode.Fill(2, 2, 4, 4, false);
  qrCode.Fill(qrSize - 7, 0, qrSize - 1, 6, true);
  qrCode.Fill(qrSize - 6, 1, qrSize - 2, 5, false);
  qrCode.Fill(qrSize - 5, 2, qrSize - 3, 4, true);
  qrCode.Fill(0, qrSize - 7, 6, qrSize - 1, true);
  qrCode.Fill(1, qrSize - 6, 5, qrSize - 2, false);
  qrCode.Fill(2, qrSize - 5, 4, qrSize - 3, true);

  // Alignment patterns. Gets skipped if patternStart is zero
  if (alignerPlacement.patternStart != 0) {
    // Blocks excluding very top and left
    for (int y = alignerPlacement.patternStart; y < qrSize - 6; y += alignerPlacement.patternIncrement) {
      for (int x = alignerPlacement.patternStart; x < qrSize - 6; x += alignerPlacement.patternIncrement) {
        qrCode.Fill(x - 2, y - 2, x + 2, y + 2, true);
        qrCode.Fill(x - 1, y - 1, x + 1, y + 1, false);
        qrCode.SetModule(x, y, true);
      }
    }
    // Blocks along top and left
    for (int pos = alignerPlacement.patternStart; pos < qrSize - 6 - alignerPlacement.patternIncrement;
         pos += alignerPlacement.patternIncrement) {
      // top block
      qrCode.Fill(pos - 2, 4, pos + 2, 8, true);
      qrCode.Fill(pos - 1, 5, pos + 1, 7, false);
      qrCode.SetModule(pos, 6, true);
      // left block
      qrCode.Fill(4, pos - 2, 8, pos + 2, true);
      qrCode.Fill(5, pos - 1, 7, pos + 1, false);
      qrCode.SetModule(6, pos, true);
    }
  }

  // Timing patterns
  for (int pos = 8; pos < qrSize - 8; pos++) {
    qrCode.SetModule(pos, 6, pos % 2 == 0);
    qrCode.SetModule(6, pos, pos % 2 == 0);
  }

  // Dark module
  qrCode.SetModule(8, qrSize - 8, true);
}

bool QRCodeGenerator::IsReservedModule(uint8_t version, int x, int y) {
  int qrSize = (int) version * 4 + 17;
  // Reserved modules are mirrored across y=x, so only need to check one half of the code
  // This flips the axes so only the top right half needs to be checked (assuming (0,0) is at top left)
  if (y > x) {
    std::swap(x, y);
  }
  // timing patterns
  if (y == 6)
    return true;
  // finder patterns (including format information and dark module)
  if (x <= 8 || (x >= qrSize - 8 && y <= 8)) {
    return true;
  }
  // version information, if applicable
  if (version >= 7 && y <= 5 && x >= qrSize - 11) {
    return true;
  }
  // alignment patterns
  if (y < 4) {
    return false; // above the topmost alignment pattern
  }
  int patternStart = versionPatternSpacings[version].patternStart;
  int patternIncrement = versionPatternSpacings[version].patternIncrement;
  if (patternStart == 0) {
    return false;
  }
  // check line of alignment patterns closest to the top
  if (y < patternStart - 2) {
    if (y - 4 < 5 and x < qrSize - 9) {
      return (x - patternStart + 2) % patternIncrement < 5;
    }
    return false;
  }
  // check alignment patterns not immediately at the top or left edges
  if ((y - patternStart + 2) % patternIncrement < 5 && (x - patternStart + 2) % patternIncrement < 5) {
    return true;
  }
  return false;
}

int QRCodeGenerator::FindAndApplyOptimalMask(QRCodeModules& qrCode) {
  // TODO: IMPLEMENT
  ApplyMask(qrCode, 0);
  return 0;
}

void QRCodeGenerator::ApplyMask(QRCodeModules& qrCode, int mask) {
  assert(mask >= 0 && mask <= 7);

  for (int y = 0; y < qrCode.GetSize(); y++) {
    for (int x = 0; x < qrCode.GetSize(); x++) {
      if (IsReservedModule(qrCode.GetVersion(), x, y))
        continue;
      switch (mask) {
        case 0:
          if ((y + x) % 2 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        case 1:
          if (y % 2 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        case 2:
          if (x % 3 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        case 3:
          if ((y + x) % 3 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        case 4:
          if ((y / 2 + x / 3) % 2 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        case 5:
          if ((y * x) % 2 + (y * x) % 3 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        case 6:
          if ((y * x + (y * x) % 3) % 2 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        case 7:
          if ((y + x + (y * x) % 3) % 2 == 0)
            qrCode.SetModule(x, y, !qrCode.GetModule(x, y));
          break;
        default:
          break;
      }
    }
  }
}

void QRCodeGenerator::PlaceMetadata(QRCodeModules& qrCode, int chosenMask) {
  // Base information is the error correction (2 bits) and chosen mask (3 bits)
  uint16_t formatInformation = 0b01 << 13; // Hardcoded low error correction
  formatInformation |= chosenMask << 10;
  // Calculate 10 bits of Reed-Solomon error correction
  uint16_t formatGeneratorPolynomial = 0b10100110111;
  uint16_t formatDivisionWorking = formatInformation;
  for (int step = 14; step > 9; step--) {
    // skip leading zeroes
    if ((formatDivisionWorking & (1 << step)) == 0)
      continue;
    formatDivisionWorking ^= formatGeneratorPolynomial << (step - 10);
  }
  // Append error correction and XOR it with the required mask
  formatInformation |= formatDivisionWorking;
  formatInformation ^= 0b101010000010010;

  // Add format information to qr code
  // Around top left finder
  uint16_t mask = 1 << 14;
  for (int x = 0; x < 6; x++) {
    qrCode.SetModule(x, 8, formatInformation & mask);
    mask >>= 1;
  }
  qrCode.SetModule(7, 8, formatInformation & (1 << 8));
  qrCode.SetModule(8, 8, formatInformation & (1 << 7));
  qrCode.SetModule(8, 7, formatInformation & (1 << 6));
  mask = 1 << 5;
  for (int y = 5; y >= 0; y--) {
    qrCode.SetModule(8, y, formatInformation & mask);
    mask >>= 1;
  }
  // Around bottom left and top right finders
  mask = 1 << 14;
  for (int y = qrCode.GetSize() - 1; y >= qrCode.GetSize(); y--) {
    qrCode.SetModule(8, y, formatInformation & mask);
    mask >>= 1;
  }
  for (int x = qrCode.GetSize() - 8; x < qrCode.GetSize(); x++) {
    qrCode.SetModule(x, 8, formatInformation & mask);
    mask >>= 1;
  }
}

void QRCodeGenerator::PlaceVersionInfo(QRCodeModules& qrCode) {
  // Base information is the QR code version in 6 bits
  uint32_t versionInformation = qrCode.GetVersion() << 12;
  // Calculate 12 bits of Reed-Solomon error correction
  uint32_t versionGeneratorPolynomial = 0b1111100100101;
  uint32_t versionDivisionWorking = versionInformation;
  for (int step = 17; step > 11; step--) {
    if ((versionDivisionWorking & (1 << step)) == 0)
      continue;
    versionDivisionWorking ^= versionGeneratorPolynomial << (step - 12);
  }
  versionInformation |= versionDivisionWorking;

  // Add version information to the qr code
  // Mirrored across axis y=x, so can do both at once
  uint32_t mask = 1 << 17;
  for (int pos1 = 5; pos1 >= 0; pos1--) {
    for (int pos2 = qrCode.GetSize() - 9; pos2 > qrCode.GetSize() - 12; pos2--) {
      qrCode.SetModule(pos1, pos2, versionInformation & mask);
      qrCode.SetModule(pos2, pos1, versionInformation & mask);
      mask >>= 1;
    }
  }
}

QRCodeModules QRCodeGenerator::GenerateQRCode(const char* data, const uint16_t dataLen) {
  int version = FindMinFittingVersion(dataLen);
  if (version > 40 || version < 1) {
    return QRCodeModules(0);
  }

  std::unique_ptr<uint8_t[]> qrContents = GenerateQRContents(data, dataLen, version);
  QRCodeModules finalCode = StructureFinalCode(std::move(qrContents), version);
  return finalCode;
}