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

details::Types identify_font(const mapview &mv) {
  switch (mv[0]) {
  case 0x00:
    if (mv.size() > 4 && mv[1] == 0x01 && mv[2] == 0x00 && mv[3] == 0x00 &&
        mv[4] == 0x00) {
      return details::ttf;
    }
    break;
  case 0x4F:
    if (mv.size() > 4 && mv[1] == 0x54 && mv[2] == 0x54 && mv[3] == 0x4F &&
        mv[4] == 0x00) {
      return details::otf;
    }
    break;
  case 0x77:
    if (mv.size() <= 7) {
      break;
    }
    if (mv[1] == 0x4F && mv[2] == 0x46 && mv[3] == 0x46 && mv[4] == 0x00 &&
        mv[5] == 0x01 && mv[6] == 0x00 && mv[7] == 0x00) {
      return details::woff;
    }
    if (mv[1] == 0x4F && mv[2] == 0x46 && mv[3] == 0x32 && mv[4] == 0x00 &&
        mv[5] == 0x01 && mv[6] == 0x00 && mv[7] == 0x00) {
      return details::woff2;
    }
    break;
  default:
    break;
  }
  return details::none;
}

details::Types mapview::identify() {
  if (size_ == 0) {
    return details::none;
  }
  auto t = identify_binexeobj_magic(std::string_view(data_, size_));
  if (t != details::none) {
    return t;
  }
  /// File is font.
  t = identify_font(*this);
  if (t != details::none) {
    return t;
  }
  return identity_text(*this);
}

} // namespace probe