///////// check binary object file format
#include <string_view>
#include "macho.cc"
#include "details.hpp"
#include "includes.hpp"

namespace inquisitive {

static constexpr const char ElfMagic[] = {0x7f, 'E', 'L', 'F', '\0'};
// The PE signature bytes that follows the DOS stub header.
static constexpr const char PEMagic[] = {'P', 'E', '\0', '\0'};

static constexpr const char BigObjMagic[] = {
    '\xc7', '\xa1', '\xba', '\xd1', '\xee', '\xba', '\xa9', '\x4b',
    '\xaf', '\x20', '\xfa', '\xf6', '\x6a', '\xa4', '\xdc', '\xb8',
};

static constexpr const char ClGlObjMagic[] = {
    '\x38', '\xfe', '\xb3', '\x0c', '\xa5', '\xd9', '\xab', '\x4d',
    '\xac', '\x9b', '\xd6', '\xb6', '\x22', '\x26', '\x53', '\xc2',
};

// The signature bytes that start a .res file.
static constexpr const char WinResMagic[] = {
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

details::Types identify_binexeobj_magic(std::string_view mv) {
  if (mv.size() < 4) {
    return details::none;
  }
  switch (mv[0]) {
  case 0x00:
    if (startswith(mv, "\0\0\xFF\xFF")) {
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
    if (startswith(mv, "\0asm")) {
      return details::wasm_object;
    }
    break;
  case 0xDE:
    if (startswith(mv, "\xDE\xC0\x17\x0B")) {
      return details::bitcode;
    }
    break;
  case 'B':
    if (startswith(mv, "BC\xC0\xDE")) {
      return details::archive;
    }
    break;
  case '!': // .a
    if (startswith(mv, "!<arch>\n") || startswith(mv, "!<thin>\n")) {
      return details::archive;
    }
    break;
  case '\177': // ELF
    if (startswith(mv, ElfMagic) && mv.size() >= 18) {
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
  case 0xCA:
    if (startswith(mv, "\xCA\xFE\xBA\xBE") ||
        startswith(mv, "\xCA\xFE\xBA\xBF")) {
      if (mv.size() >= 8 && mv[7] < 43) {
        return details::macho_universal_binary;
      }
    }
    break;
  case 0xFE:
  case 0xCE:
  case 0xCF: {
    uint16_t type = 0;
    if (startswith(mv, "\xFE\xED\xFA\xCE") ||
        startswith(mv, "\xFE\xED\xFA\xCF")) {
      /* Native endian */
      size_t minsize;
      if (mv[3] == char(0xCE))
        minsize = sizeof(mach_header);
      else
        minsize = sizeof(mach_header_64);
      if (mv.size() >= minsize)
        type = mv[12] << 24 | mv[13] << 12 | mv[14] << 8 | mv[15];
    } else if (startswith(mv, "\xCE\xFA\xED\xFE") ||
               startswith(mv, "\xCF\xFA\xED\xFE")) {
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
      return details::macho_object;
    case 2:
      return details::macho_executable;
    case 3:
      return details::macho_fixed_virtual_memory_shared_lib;
    case 4:
      return details::macho_core;
    case 5:
      return details::macho_preload_executable;
    case 6:
      return details::macho_dynamically_linked_shared_lib;
    case 7:
      return details::macho_dynamic_linker;
    case 8:
      return details::macho_bundle;
    case 9:
      return details::macho_dynamically_linked_shared_lib_stub;
    case 10:
      return details::macho_dsym_companion;
    case 11:
      return details::macho_kext_bundle;
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
      return details::coff_object;
    }
    [[fallthrough]];
  case 0x90: // PA-RISC Windows
  case 0x68: // mc68K Windows
    if (mv[1] == 0x02) {
      return details::coff_object;
    }
    break;
  case 'M':
    if (startswith(mv, "Microsoft C/C++ MSF 7.00\r\n")) {
      return details::pdb;
    }
    if (startswith(mv, "MZ") && mv.size() >= 0x3c + 4) {
      // read32le
      uint32_t off = readle<uint32_t>((void *)(mv.data() + 0x32));
      auto sv = mv.substr(off);
      if (startswith(sv, PEMagic)) {
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
} // namespace inquisitive
