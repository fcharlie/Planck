//// archive format
// BZ 7z Rar!
#include <string_view>
#include <optional>
#include "details.hpp"

namespace probe {
constexpr const unsigned n7zSignatureSize = 6;
constexpr const byte_t n7zMagic[n7zSignatureSize] = {'7',  'z',  0xBC,
                                                     0xAF, 0x27, 0x1C};
constexpr const byte_t rarMagic[] = {0x52, 0x61, 0x72,
                                     0x21, 0x14, 0x7}; // sv[6]==0x0 or 0x1
// https://github.com/h2non/filetype/blob/master/matchers/archive.go
constexpr const byte_t pdfMagic[] = {0x25, 0x50, 0x44, 0x46};
constexpr const byte_t swfMagic1[] = {0x43, 0x57, 0x53};
constexpr const byte_t swfMagic2[] = {0x46, 0x57, 0x53};
constexpr const byte_t rtfMagic[] = {0x7B, 0x5C, 0x72, 0x74, 0x66};
constexpr const byte_t msiMagic[] = {0x53, 0x5A, 0x44, 0x44, 0x88,
                                     0xF0, 0x27, 0x33, 0x41};
constexpr const byte_t vmdMagic[] = {'K', 'D', 'M', 'V'};
constexpr const byte_t rpmMagic[] = {0xED, 0xAB, 0xEE, 0xDB}; // size>96
constexpr const byte_t comMagic[] = {0xD0, 0xCF, 0x11, 0xE0,
                                     0xA1, 0xB1, 0x1A, 0xE1};
//
details::Types identify_archive(std::string_view mv) {
  //
  if (startswithB(mv, rpmMagic)) {
    return details::rpm;
  }
  return details::none;
}

// EPUB file
inline bool epub(const byte_t *buf, size_t size) {
  return size > 57 && buf[0] == 0x50 && buf[1] == 0x4B && buf[2] == 0x3 &&
         buf[3] == 0x4 && buf[30] == 0x6D && buf[31] == 0x69 &&
         buf[32] == 0x6D && buf[33] == 0x65 && buf[34] == 0x74 &&
         buf[35] == 0x79 && buf[36] == 0x70 && buf[37] == 0x65 &&
         buf[38] == 0x61 && buf[39] == 0x70 && buf[40] == 0x70 &&
         buf[41] == 0x6C && buf[42] == 0x69 && buf[43] == 0x63 &&
         buf[44] == 0x61 && buf[45] == 0x74 && buf[46] == 0x69 &&
         buf[47] == 0x6F && buf[48] == 0x6E && buf[49] == 0x2F &&
         buf[50] == 0x65 && buf[51] == 0x70 && buf[52] == 0x75 &&
         buf[53] == 0x62 && buf[54] == 0x2B && buf[55] == 0x7A &&
         buf[56] == 0x69 && buf[57] == 0x70;
}

/*
https://github.com/mcmilk/7-Zip-zstd/blob/master/CPP/7zip/Archive/7z/7zHeader.h
const UInt32 k_Copy = 0;
const UInt32 k_Delta = 3;

const UInt32 k_LZMA2 = 0x21;

const UInt32 k_SWAP2 = 0x20302;
const UInt32 k_SWAP4 = 0x20304;

const UInt32 k_LZMA  = 0x30101;
const UInt32 k_PPMD  = 0x30401;

const UInt32 k_Deflate = 0x40108;
const UInt32 k_BZip2   = 0x40202;

const UInt32 k_BCJ   = 0x3030103;
const UInt32 k_BCJ2  = 0x303011B;
const UInt32 k_PPC   = 0x3030205;
const UInt32 k_IA64  = 0x3030401;
const UInt32 k_ARM   = 0x3030501;
const UInt32 k_ARMT  = 0x3030701;
const UInt32 k_SPARC = 0x3030805;

const UInt32 k_LZHAM = 0x4F71001;
const UInt32 k_ZSTD  = 0x4F71101;
const UInt32 k_BROTLI= 0x4F71102;
const UInt32 k_LZ4   = 0x4F71104;
const UInt32 k_LZ5   = 0x4F71105;
const UInt32 k_LIZARD= 0x4F71106;
*/

// check 7z method.
std::optional<std::string_view> n7zmethod(std::string_view sv) {
  //
  return std::nullopt;
}

// https://rarlab.com/technote.htm rar
} // namespace probe