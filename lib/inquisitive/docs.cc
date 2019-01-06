//////////////
#include "inquisitive.hpp"

namespace inquisitive {

// RTF format
// https://en.wikipedia.org/wiki/Rich_Text_Format
/*{\rtf1*/
status_t inquisitive_rtfinternal(memview mv, inquisitive_result_t &ir) {
  constexpr byte_t rtfMagic[] = {0x7B, 0x5C, 0x72, 0x74, 0x66};
  if (!mv.startswith(rtfMagic) || mv.size() < 6) {
    return None;
  }
  ir.Assign(L"Rich Text Format data, version ", types::rtf);
  for (size_t i = 5; i < mv.size(); i++) {
    auto ch = mv[i];
    if (ch == '\\' || ch == '\n') {
      break;
    }
    /// version is alpha number
    ir.name.push_back(ch);
  }
  return None;
}

//
constexpr byte_t msdocMagic[] = {0xD0, 0xCF, 0x11, 0xE0,
                                 0xA1, 0xB1, 0x1A, 0xE1};
// http://www.openoffice.org/sc/compdocfileformat.pdf
// https://interoperability.blob.core.windows.net/files/MS-PPT/[MS-PPT].pdf

status_t inquisitive_docs(memview mv, inquisitive_result_t &ir) {
  //
  return None;
}
} // namespace inquisitive
