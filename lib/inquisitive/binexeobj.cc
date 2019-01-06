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

static constexpr const byte_t debMagic[] = {
    0x21, 0x3C, 0x61, 0x72, 0x63, 0x68, 0x3E, 0x0A, 0x64, 0x65, 0x62,
    0x69, 0x61, 0x6E, 0x2D, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79};

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

status_t inquisitive_binobj(memview mv, inquisitive_result_t &ir) {
  if (mv.size() < 4) {
    return None;
  }
  switch (mv[0]) {
  case 0x00:
    if (mv.startswith("\0\0\xFF\xFF", 4)) {
      size_t minsize = offsetof(BigObjHeader, UUID) + sizeof(BigObjMagic);
      if (mv.size() < minsize) {
        ir.Assign(L"COFF import library", types::coff_import_library);
        return Found;
      }
      const char *start = mv.data() + offsetof(BigObjHeader, UUID);
      if (memcmp(start, BigObjMagic, sizeof(BigObjMagic)) == 0) {
        ir.Assign(L"COFF object", types::coff_object);
        return Found;
      }
      if (memcmp(start, ClGlObjMagic, sizeof(ClGlObjMagic)) == 0) {
        ir.Assign(L"Microsoft cl.exe's intermediate code file",
                  types::coff_cl_gl_object);
        return Found;
      }
      ir.Assign(L"COFF import library", types::coff_import_library);
      return Found;
    }
    if (mv.size() >= sizeof(WinResMagic) &&
        memcmp(mv.data(), WinResMagic, sizeof(WinResMagic)) == 0) {
      ir.Assign(L"Windows compiled resource file (.res)",
                types::windows_resource);
      return Found;
    }
    if (mv[1] == 0) {
      ir.Assign(L"COFF object", types::coff_object);
      return Found;
    }
    if (mv.startswith("\0asm", 4)) {
      ir.Assign(L"WebAssembly Object file", types::wasm_object);
      return Found;
    }
    break;
  case 0xDE:
    if (mv.startswith("\xDE\xC0\x17\x0B", 4)) {
      ir.Assign(L"LLVM IR bitcode", types::bitcode);
      return Found;
    }
    break;
  case 'B':
    if (mv.startswith("BC\xC0\xDE", 3)) {
      ir.Assign(L"LLVM IR bitcode", types::bitcode);
      return Found;
    }
    break;
  case '!': // .a
    if (mv.startswith("!<arch>\n") && !mv.startswith(debMagic) ||
        mv.startswith("!<thin>\n")) {
      // Skip DEB package
      ir.Assign(L"ar style archive file", types::archive);
      return Found;
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
          ir.Assign(L"ELF Unknown type", types::elf, types::ELF);
          return Found;
        case 1:
          ir.Assign(L"ELF Relocatable object file", types::elf_relocatable,
                    types::ELF);
          return Found;
        case 2:
          ir.Assign(L"ELF Executable image", types::elf_executable, types::ELF);
          return Found;
        case 3:
          ir.Assign(L"ELF dynamically linked shared lib",
                    types::elf_shared_object, types::ELF);
          return Found;
        case 4:
          ir.Assign(L"ELF core image", types::elf_core, types::ELF);
          return Found;
        }
      }
      ir.Assign(L"ELF Unknown type", types::elf, types::ELF);
      return Found;
    }
    break;
  case 0xCA:
    if (mv.startswith("\xCA\xFE\xBA\xBE") ||
        mv.startswith("\xCA\xFE\xBA\xBF")) {
      if (mv.size() >= 8 && mv[7] < 43) {
        ir.Assign(L"Mach-O universal binary", types::macho_universal_binary,
                  types::MACHO);
        return Found;
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
      if (mv[3] == char(0xCE)) {
        minsize = sizeof(mach_header);
      } else {
        minsize = sizeof(mach_header_64);
      }
      if (mv.size() >= minsize)
        type = mv[12] << 24 | mv[13] << 12 | mv[14] << 8 | mv[15];
    } else if (mv.startswith("\xCE\xFA\xED\xFE") ||
               mv.startswith("\xCF\xFA\xED\xFE")) {
      /* Reverse endian */
      size_t minsize;
      if (mv[0] == char(0xCE)) {
        minsize = sizeof(mach_header);
      } else {
        minsize = sizeof(mach_header_64);
      }
      if (mv.size() >= minsize) {
        type = mv[15] << 24 | mv[14] << 12 | mv[13] << 8 | mv[12];
      }
    }
    switch (type) {
    default:
      break;
    case 1:
      ir.Assign(L"Mach-O Object file", types::macho_object, types::MACHO);
      return Found;
    case 2:
      ir.Assign(L"Mach-O Executable", types::macho_executable, types::MACHO);
      return Found;
    case 3:
      ir.Assign(L"Mach-O Shared Lib, FVM",
                types::macho_fixed_virtual_memory_shared_lib, types::MACHO);
      return Found;
    case 4:
      ir.Assign(L"Mach-O Core File", types::macho_core, types::MACHO);
      return Found;
    case 5:
      ir.Assign(L"Mach-O Preloaded Executable", types::macho_preload_executable,
                types::MACHO);
      return Found;
    case 6:
      ir.Assign(L"Mach-O dynlinked shared lib",
                types::macho_dynamically_linked_shared_lib, types::MACHO);
      return Found;
    case 7:
      ir.Assign(L"The Mach-O dynamic linker", types::macho_dynamic_linker,
                types::MACHO);
      return Found;
    case 8:
      ir.Assign(L"Mach-O Bundle file", types::macho_bundle, types::MACHO);
      return Found;
    case 9:
      ir.Assign(L"Mach-O Shared lib stub",
                types::macho_dynamically_linked_shared_lib_stub, types::MACHO);
      return Found;
    case 10:
      ir.Assign(L"Mach-O dSYM companion file", types::macho_dsym_companion,
                types::MACHO);
      return Found;
    case 11:
      ir.Assign(L"Mach-O kext bundle file", types::macho_kext_bundle,
                types::MACHO);
      return Found;
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
      ir.Assign(L"COFF object", types::coff_object);
      return Found;
    }
    [[fallthrough]];
  case 0x90: // PA-RISC Windows
  case 0x68: // mc68K Windows
    if (mv[1] == 0x02) {
      ir.Assign(L"COFF object", types::coff_object);
      return Found;
    }
    break;
  case 'M':
    if (mv.startswith("Microsoft C/C++ MSF 7.00\r\n")) {
      ir.Assign(L"Windows PDB debug info file", types::pdb);
      return Found;
    }
    if (mv.startswith("MZ") && mv.size() >= 0x3c + 4) {
      // read32le
      uint32_t off = planck::readle<uint32_t>(mv.data() + 0x32);
      auto sv = mv.submv(off);
      if (mv.startswith(PEMagic)) {
        ir.Assign(L"PECOFF executable file", types::pecoff_executable,
                  types::PECOFF);
        return Found;
      }
    }
    break;
  case 0x64: // x86-64 or ARM64 Windows.
    if (mv[1] == char(0x86) || mv[1] == char(0xaa)) {
      ir.Assign(L"COFF object", types::coff_object);
      return Found;
    }
    break;
  default:
    break;
  }
  return None;
}

} // namespace inquisitive
