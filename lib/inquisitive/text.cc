////////////////
#include "inquisitive.hpp"

namespace inquisitive {
// check text details
status_t inquisitive_text(memview mv, inquisitive_result_t &ir) {
  //
  switch (mv[0]) {
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
    break;
  case 0xFE: // UTF16BE 0xFE 0xFF
    if (mv.size() >= 2 && mv[1] == 0xFF) {
      ir.assign(L"Big-endian UTF-16 Unicode text", types::utf16be);
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