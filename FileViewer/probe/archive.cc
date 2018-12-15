//// archive format
// BZ 7z Rar!
#include <string_view>
#include "details.hpp"

namespace probe {
using byte_t = unsigned char;
constexpr const unsigned n7zSignatureSize = 6;
constexpr const byte_t n7zMagic[n7zSignatureSize] = {'7',  'z',  0xBC,
                                                     0xAF, 0x27, 0x1C};
// https://github.com/h2non/filetype/blob/master/matchers/archive.go
constexpr const byte_t pdfMagic[] = {0x25, 0x50, 0x44, 0x46};
constexpr const byte_t swfMagic1[] = {0x43, 0x57, 0x53};
constexpr const byte_t swfMagic2[] = {0x46, 0x57, 0x53};
constexpr const byte_t rtfMagic[] = {0x7B, 0x5C, 0x72, 0x74, 0x66};

// https://rarlab.com/technote.htm rar
} // namespace probe