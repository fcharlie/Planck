////////////// FONT resolve
#include "inquisitive.hpp"

namespace inquisitive {

inline bool IsEot(base::MemView mv) {
  return mv.size() > 35 && mv[34] == 0x4C && mv[35] == 0x50 &&
         ((mv[8] == 0x02 && mv[9] == 0x00 && mv[10] == 0x01) ||
          (mv[8] == 0x01 && mv[9] == 0x00 && mv[10] == 0x00) ||
          (mv[8] == 0x02 && mv[9] == 0x00 && mv[10] == 0x02));
}

status_t inquisitive_fonts(base::MemView mv, inquisitive_result_t &ir) {
  //
  switch (mv[0]) {
  case 0x00:
    if (mv.size() > 4 && mv[1] == 0x01 && mv[2] == 0x00 && mv[3] == 0x00 && mv[4] == 0x00) {
      // https://en.wikipedia.org/wiki/TrueType
      ir.assign(L"TrueType Font", types::ttf);
      return Found;
    }
    break;
  case 0x4F:
    if (mv.size() > 4 && mv[1] == 0x54 && mv[2] == 0x54 && mv[3] == 0x4F && mv[4] == 0x00) {
      // https://en.wikipedia.org/wiki/OpenType
      ir.assign(L"OpenType Font", types::otf);
      return Found;
    }
    break;
  case 0x77:
    if (mv.size() <= 7) {
      break;
    }
    if (mv[1] == 0x4F && mv[2] == 0x46 && mv[3] == 0x46 && mv[4] == 0x00 && mv[5] == 0x01 &&
        mv[6] == 0x00 && mv[7] == 0x00) {
      // https://en.wikipedia.org/wiki/Web_Open_Font_Format
      ir.assign(L"Web Open Font Format", types::woff);
      return Found;
    }
    if (mv[1] == 0x4F && mv[2] == 0x46 && mv[3] == 0x32 && mv[4] == 0x00 && mv[5] == 0x01 &&
        mv[6] == 0x00 && mv[7] == 0x00) {
      // https://en.wikipedia.org/wiki/Web_Open_Font_Format
      ir.assign(L"Web Open Font Format 2.0", types::woff2);
      return Found;
    }
    break;
  default:
    break;
  }
  if (IsEot(mv)) {
    ir.assign(L"Embedded OpenType (EOT) fonts", types::eot);
    return Found;
  }
  return None;
}
} // namespace inquisitive