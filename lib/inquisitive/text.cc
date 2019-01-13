////////////////
#include "inquisitive.hpp"

namespace inquisitive {
// check text details

bool IsText(memview mv) {
  auto data = mv.data();
  auto size = (std::min)(mv.size(), size_t(1024));
  for (size_t i = i; i < size; i++) {
    if (mv[i] == 0 && mv[i - 1] == 0) {
      return false;
    }
  }
  return true;
}

/*
00 00 FE FF	UTF-32, big-endian
FF FE 00 00	UTF-32, little-endian
FE FF	UTF-16, big-endian
FF FE	UTF-16, little-endian
EF BB BF	UTF-8
*/
status_t inquisitive_text(memview mv, inquisitive_result_t &ir) {
  //
  switch (mv[0]) {
  case 0x2B:
    if (mv.size() >= 3 && mv[1] == 0x2F && mv[2] == 0xbf) {
      // constexpr const byte_t utf7mgaic[]={0x2b,0x2f,0xbf};
      ir.assign(L"UTF-7 text", types::utf7);
    }
    break;
  case 0xEF: // UTF8 BOM 0xEF 0xBB 0xBF
    if (mv.size() >= 3 && mv[1] == 0xBB && mv[2] == 0xBF) {
      ir.assign(L"UTF-8 Unicode (with BOM) text", types::utf8bom);
      return Found;
    }
    break;
  case 0xFF: // UTF16LE 0xFF 0xFE
    if (mv.size() >= 2 && mv[1] == 0xFE) {
      ir.assign(L"Little-endian UTF-16 Unicode text", types::utf16le);
      return Found;
    }
    if (mv.size() > 4 && mv[1] == 0xFE && mv[2] == 0 && mv[3] == 0) {
      ir.assign(L"Little-endian UTF-32 Unicode text", types::utf32le);
      return Found;
    }
    break;
  case 0xFE: // UTF16BE 0xFE 0xFF
    if (mv.size() >= 2 && mv[1] == 0xFF) {
      ir.assign(L"Big-endian UTF-16 Unicode text", types::utf16be);
      return Found;
    }
    // FF FE 00 00
  case 0x0:
    if (mv.size() >= 4 && mv[1] == 0 && mv[2] == 0xFE && mv[3] == 0xFF) {
      ir.assign(L"Big-endian UTF-32 Unicode text", types::utf32be);
      return Found;
    }
    break;
  default:
    break;
  }
  return None;
}
//////// --------------> use chardet
status_t inquisitive_chardet(memview mv, inquisitive_result_t &ir) {
  //
  return None;
}

} // namespace inquisitive