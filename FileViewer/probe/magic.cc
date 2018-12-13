/// file type with magic
#include "probe.hpp"

namespace probe {

details::Types identifybin(const mapview &mv) {
  if (mv.size() < 4) {
    return details::NONE;
  }
  switch (mv[0]) {
  case 0x00:
    break;
  case 0xDE:
    break;
  case 'B':
    break;
  case '!': // Archive
    break;
  case '\177': // ELF
    break;
  case 'M':
    if (mv.startswith("Microsoft C/C++ MSF 7.00\r\n")) {
      return details::PDB;
    }
    if (mv.startswith("MZ") && mv.size() >= 0x3c + 4) {
      // read32le
    }
    break;
  case 0x64: // x86-64 or ARM64 Windows.
    if (mv[1] == char(0x86) || mv[1] == char(0xaa)) {
      return details::COFF;
    }
    break;
  default:
    break;
  }
  return details::NONE;
}

details::Types mapview::identify() {
  auto i = identifybin(*this);
  if (i != details::NONE) {
    return i;
  }
  return details::NONE;
}

} // namespace probe