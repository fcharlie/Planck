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

struct BigObjHeader {
  enum : uint16_t { MinBigObjectVersion = 2 };

  uint16_t Sig1; ///< Must be IMAGE_FILE_MACHINE_UNKNOWN (0).
  uint16_t Sig2; ///< Must be 0xFFFF.
  uint16_t Version;
  uint16_t Machine;
  uint32_t TimeDateStamp;
  uint8_t UUID[16];
  uint32_t unused1;
  uint32_t unused2;
  uint32_t unused3;
  uint32_t unused4;
  uint32_t NumberOfSections;
  uint32_t PointerToSymbolTable;
  uint32_t NumberOfSymbols;
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

static const char ElfMagic[] = {0x7f, 'E', 'L', 'F', '\0'};

details::Types identifybin(const mapview &mv) {
  if (mv.size() < 4) {
    return details::none;
  }
  switch (mv[0]) {
  case 0x00:
    if (mv.startswith("\0\0\xFF\xFF")) {
      size_t minsize = offsetof(BigObjHeader, UUID) + sizeof(BigObjMagic);
      if (mv.size() < minsize) {
        return details::coff_import_library;
      }
      const char *start = mv.data() + offsetof(BigObjHeader, UUID);
      if (memcmp(start, BigObjMagic, sizeof(BigObjMagic)) == 0) {
        return details::coff_object;
      }
      if (memcmp(start, ClGlObjMagic, sizeof(ClGlObjMagic)) == 0) {
        return details::coff_cl_gl_object;
      }
      return details::coff_import_library;
    }
    if (mv.size() >= sizeof(WinResMagic) &&
        memcmp(mv.data(), WinResMagic, sizeof(WinResMagic)) == 0) {
      return details::windows_resource;
    }
    if (mv[1] == 0) {
      return details::coff_object;
    }
    if (mv.startswith("\0asm")) {
      return details::wasm_object;
    }
    break;
  case 0xDE:
    if (mv.startswith("\xDE\xC0\x17\x0B")) {
      return details::bitcode;
    }
    break;
  case 'B':
    if (mv.startswith("BC\xC0\xDE")) {
      return details::archive;
    }
    break;
  case '!': // .a
    if (mv.startswith("!<arch>\n") || mv.startswith("!<thin>\n")) {
      return details::archive;
    }
    break;
  case '\177': // ELF
    if (mv.startswith(ElfMagic) && mv.size() >= 18) {
      bool Data2MSB = (mv[5] == 2);
      unsigned high = Data2MSB ? 16 : 17;
      unsigned low = Data2MSB ? 17 : 16;
      if (mv[high] == 0) {
        switch (mv[low]) {
        default:
          return details::elf;
        case 1:
          return details::elf_relocatable;
        case 2:
          return details::elf_executable;
        case 3:
          return details::elf_shared_object;
        case 4:
          return details::elf_core;
        }
      }
      return details::elf;
    }
    break;
  case 'M':
    if (mv.startswith("Microsoft C/C++ MSF 7.00\r\n")) {
      return details::pdb;
    }
    if (mv.startswith("MZ") && mv.size() >= 0x3c + 4) {
      // read32le
      uint32_t off = llvm::support::endian::read32le(mv.data() + 0x32);
      if (mv.indexswith(off, PEMagic)) {
        return details::pecoff_executable;
      }
    }
    break;
  case 0x64: // x86-64 or ARM64 Windows.
    if (mv[1] == char(0x86) || mv[1] == char(0xaa)) {
      return details::coff_object;
    }
    break;
  default:
    break;
  }
  return details::none;
}

details::Types identitytext(const mapview &mv) {
  constexpr const char UTF8Magic[] = {0xEF, 0xBB, 0xBF};
  constexpr const char UTF16LEMagic[] = {0xFF, 0xFE};
  constexpr const char UTF16BEMagic[] = {0xFE, 0xFF};
  if (mv.startswith(UTF8Magic)) {
    return details::utf8bom;
  }
  if (mv.startswith(UTF16LEMagic)) {
    return details::utf16le;
  }
  if (mv.startswith(UTF16BEMagic)) {
    return details::utf16be;
  }
  return details::ascii;
}

details::Types mapview::identify() {
  auto i = identifybin(*this);
  if (i != details::none) {
    return i;
  }
  return identitytext(*this);
}

} // namespace probe