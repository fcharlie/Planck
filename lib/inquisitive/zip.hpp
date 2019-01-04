//// ZIP
#ifndef INQUISITIVE_ZIP_HPP
#define INQUISITIVE_ZIP_HPP
#include <cstdint>
#include <endian.hpp>
// https://en.wikipedia.org/wiki/Zip_(file_format)

namespace inquisitive {
// LE
#pragma pack(2)
struct zip_file_header_t {
  uint8_t magic[4]; //{'P','K','',''} // 0x04034b50 LE
  uint16_t version;
  uint16_t bitflag;
  uint16_t method;
  uint16_t mtime;
  uint16_t mdate;
  uint32_t crc32;
  uint32_t compressedsize;
  uint32_t uncompressedsize;
  uint16_t namelen;
  uint16_t fieldlength;
};
#pragma pack()
} // namespace inquisitive

#endif
