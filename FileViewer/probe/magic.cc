/// file type with magic
#include "probe.hpp"
#include "includes.hpp"

namespace probe {
// The PE signature bytes that follows the DOS stub header.
static const char PEMagic[] = {'P', 'E', '\0', '\0'};

static const char BigObjMagic[] = {
    '\xc7', '\xa1', '\xba', '\xd1', '\xee', '\xba', '\xa9', '\x4b',
    '\xaf', '\x20', '\xfa', '\xf6', '\x6a', '\xa4', '\xdc', '\xb8',
};

static const char ClGlObjMagic[] = {
    '\x38', '\xfe', '\xb3', '\x0c', '\xa5', '\xd9', '\xab', '\x4d',
    '\xac', '\x9b', '\xd6', '\xb6', '\x22', '\x26', '\x53', '\xc2',
};

// The signature bytes that start a .res file.
static const char WinResMagic[] = {
    '\x00', '\x00', '\x00', '\x00', '\x20', '\x00', '\x00', '\x00',
    '\xff', '\xff', '\x00', '\x00', '\xff', '\xff', '\x00', '\x00',
};

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
      uint32_t off = llvm::support::endian::read32le(mv.data() + 0x32);
      if (mv.indexswith(off, PEMagic)) {
        return details::PE;
      }
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