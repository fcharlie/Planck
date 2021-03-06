//////////////
#include "inquisitive.hpp"
#include "docs.hpp"

namespace inquisitive {

// RTF format
// https://en.wikipedia.org/wiki/Rich_Text_Format
/*{\rtf1*/
status_t inquisitive_rtfinternal(base::MemView mv, inquisitive_result_t &ir) {
  constexpr byte_t rtfMagic[] = {0x7B, 0x5C, 0x72, 0x74, 0x66};
  if (!mv.StartsWith(rtfMagic) || mv.size() < 6) {
    return None;
  }
  std::wstring name(L"Rich Text Format data, version ");

  for (size_t i = 5; i < mv.size(); i++) {
    auto ch = mv[i];
    if (ch == '\\' || ch == '\n') {
      break;
    }
    /// version is alpha number
    name.push_back(ch);
    ir.assign(name, types::rtf);
    return Found;
  }
  return None;
}

// http://www.openoffice.org/sc/compdocfileformat.pdf
// https://interoperability.blob.core.windows.net/files/MS-PPT/[MS-PPT].pdf

status_t inquisitive_docs(base::MemView mv, inquisitive_result_t &ir) {
  constexpr const byte_t msofficeMagic[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
  constexpr const byte_t pptMagic[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1,
                                       0x1A, 0xE1, 0x00, 0x00, 0x00, 0x00};
  constexpr const byte_t wordMagic[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1, 0x00};
  constexpr const byte_t xlsMagic[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1, 0x00};
  if (inquisitive_rtfinternal(mv, ir) == Found) {
    return Found;
  }
  constexpr const auto olesize = sizeof(oleheader_t);
  if (mv.StartsWith(msofficeMagic) || mv.size() < 512) {
    return None;
  }
  auto oh = mv.cast<oleheader_t>(0);
  // PowerPoint Document

  return None;
}
} // namespace inquisitive
