///////// check binary object file format
#include <string_view>
#include <endian.hpp>
#include "macho.hpp"
#include "inquisitive.hpp"

namespace inquisitive {

static constexpr const unsigned char ElfMagic[] = {0x7f, 'E', 'L', 'F', '\0'};
// The PE signature bytes that follows the DOS stub header.
static constexpr const char PEMagic[] = {'P', 'E', '\0', '\0'};

static constexpr const unsigned char BigObjMagic[] = {
    '\xc7', '\xa1', '\xba', '\xd1', '\xee', '\xba', '\xa9', '\x4b',
    '\xaf', '\x20', '\xfa', '\xf6', '\x6a', '\xa4', '\xdc', '\xb8',
};

static constexpr const unsigned char ClGlObjMagic[] = {
    '\x38', '\xfe', '\xb3', '\x0c', '\xa5', '\xd9', '\xab', '\x4d',
    '\xac', '\x9b', '\xd6', '\xb6', '\x22', '\x26', '\x53', '\xc2',
};

// The signature bytes that start a .res file.
static constexpr const unsigned char WinResMagic[] = {
    '\x00', '\x00', '\x00', '\x00', '\x20', '\x00', '\x00', '\x00',
    '\xff', '\xff', '\x00', '\x00', '\xff', '\xff', '\x00', '\x00',
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

types::Type identify_binexeobj_magic(memview mv) {
  if (mv.size() < 4) {
    return types::none;
  }
  switch (mv[0]) {
  case 0x00:
    if (mv.startswith("\0\0\xFF\xFF", 4)) {
      size_t minsize = offsetof(BigObjHeader, UUID) + sizeof(BigObjMagic);
      if (mv.size() < minsize) {
        return types::coff_import_library;
      }
      const char *start = mv.data() + offsetof(BigObjHeader, UUID);
      if (memcmp(start, BigObjMagic, sizeof(BigObjMagic)) == 0) {
        return types::coff_object;
      }
      if (memcmp(start, ClGlObjMagic, sizeof(ClGlObjMagic)) == 0) {
        return types::coff_cl_gl_object;
      }
      return types::coff_import_library;
    }
    if (mv.size() >= sizeof(WinResMagic) &&
        memcmp(mv.data(), WinResMagic, sizeof(WinResMagic)) == 0) {
      return types::windows_resource;
    }
    if (mv[1] == 0) {
      return types::coff_object;
    }
    if (mv.startswith("\0asm", 4)) {
      return types::wasm_object;
    }
    break;
  case 0xDE:
    if (mv.startswith("\xDE\xC0\x17\x0B", 4)) {
      return types::bitcode;
    }
    break;
  case 'B':
    if (mv.startswith("BC\xC0\xDE", 3)) {
      return types::archive;
    }
    break;
  case '!': // .a
    if (mv.startswith("!<arch>\n") || mv.startswith("!<thin>\n")) {
      return types::archive;
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
          return types::elf;
        case 1:
          return types::elf_relocatable;
        case 2:
          return types::elf_executable;
        case 3:
          return types::elf_shared_object;
        case 4:
          return types::elf_core;
        }
      }
      return types::elf;
    }
    break;
  case 0xCA:
    if (mv.startswith("\xCA\xFE\xBA\xBE") ||
        mv.startswith("\xCA\xFE\xBA\xBF")) {
      if (mv.size() >= 8 && mv[7] < 43) {
        return types::macho_universal_binary;
      }
    }
    break;
  case 0xFE:
  case 0xCE:
  case 0xCF: {
    uint16_t type = 0;
    if (mv.startswith("\xFE\xED\xFA\xCE") ||
        mv.startswith("\xFE\xED\xFA\xCF")) {
      /* Native endian */
      size_t minsize;
      if (mv[3] == char(0xCE))
        minsize = sizeof(mach_header);
      else
        minsize = sizeof(mach_header_64);
      if (mv.size() >= minsize)
        type = mv[12] << 24 | mv[13] << 12 | mv[14] << 8 | mv[15];
    } else if (mv.startswith("\xCE\xFA\xED\xFE") ||
               mv.startswith("\xCF\xFA\xED\xFE")) {
      /* Reverse endian */
      size_t minsize;
      if (mv[0] == char(0xCE))
        minsize = sizeof(mach_header);
      else
        minsize = sizeof(mach_header_64);
      if (mv.size() >= minsize)
        type = mv[15] << 24 | mv[14] << 12 | mv[13] << 8 | mv[12];
    }
    switch (type) {
    default:
      break;
    case 1:
      return types::macho_object;
    case 2:
      return types::macho_executable;
    case 3:
      return types::macho_fixed_virtual_memory_shared_lib;
    case 4:
      return types::macho_core;
    case 5:
      return types::macho_preload_executable;
    case 6:
      return types::macho_dynamically_linked_shared_lib;
    case 7:
      return types::macho_dynamic_linker;
    case 8:
      return types::macho_bundle;
    case 9:
      return types::macho_dynamically_linked_shared_lib_stub;
    case 10:
      return types::macho_dsym_companion;
    case 11:
      return types::macho_kext_bundle;
    }
    break;
  }
  case 0xF0: // PowerPC Windows
  case 0x83: // Alpha 32-bit
  case 0x84: // Alpha 64-bit
  case 0x66: // MPS R4000 Windows
  case 0x50: // mc68K
  case 0x4c: // 80386 Windows
  case 0xc4: // ARMNT Windows
    if (mv[1] == 0x01) {
      return types::coff_object;
    }
    [[fallthrough]];
  case 0x90: // PA-RISC Windows
  case 0x68: // mc68K Windows
    if (mv[1] == 0x02) {
      return types::coff_object;
    }
    break;
  case 'M':
    if (mv.startswith("Microsoft C/C++ MSF 7.00\r\n")) {
      return types::pdb;
    }
    if (mv.startswith("MZ") && mv.size() >= 0x3c + 4) {
      // read32le
      uint32_t off = planck::readle<uint32_t>(mv.data() + 0x32);
      auto sv = mv.submv(off);
      if (mv.startswith(PEMagic)) {
        return types::pecoff_executable;
      }
    }
    break;
  case 0x64: // x86-64 or ARM64 Windows.
    if (mv[1] == char(0x86) || mv[1] == char(0xaa)) {
      return types::coff_object;
    }
    break;
  default:
    break;
  }
  return types::none;
}
} // namespace inquisitive
