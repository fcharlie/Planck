/// file type with magic
#include "probe.hpp"

namespace probe {
details::Types identify_binexeobj_magic(std::string_view mv); /// binexeobj.cc
details::Types identity_text(const mapview &mv) {
  switch (mv[0]) {
  case 0xEF: // UTF8 BOM 0xEF 0xBB 0xBF
    if (mv.size() >= 3 && mv[1] == 0xBB && mv[2] == 0xBF) {
      return details::utf8bom;
    }
    break;
  case 0xFF: // UTF16LE 0xFF 0xFE
    if (mv.size() >= 2 && mv[1] == 0xFE) {
      return details::utf16le;
    }
    break;
  case 0xFE: // UTF16BE 0xFE 0xFF
    if (mv.size() >= 2 && mv[1] == 0xFF) {
      return details::utf16be;
    }
    break;
  default:
    break;
  }
  /// TODO use chardet
  return details::ascii;
}

details::Types mapview::identify() {
  if (size_ == 0) {
    return details::none;
  }
  auto i = identify_binexeobj_magic(std::string_view(data_, size_));
  if (i != details::none) {
    return i;
  }
  return identity_text(*this);
}

} // namespace probe