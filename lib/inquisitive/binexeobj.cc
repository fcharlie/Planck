///////// check binary object file format
#include <string_view>
#include "macho.hpp"
#include "inquisitive.hpp"

namespace inquisitive {

static constexpr const unsigned char ElfMagic[] = {0x7f, 'E', 'L', 'F'};
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

status_t inquisitive_binobj(base::MemView mv, inquisitive_result_t &ir) {
  if (mv.size() < 4) {
    return None;
  }
  constexpr std::string_view wasmobj{"\0asm", 4};
  constexpr std::string_view irobj{"\xDE\xC0\x17\x0B", 4};
  constexpr std::string_view irobj2{"\xBC\xC0\xDE", 3};
  constexpr std::string_view bobj{"\0\0\xFF\xFF", 4};
  switch (mv[0]) {
  case 0x00:
    if (mv.StartsWith(bobj)) {
      size_t minsize = offsetof(BigObjHeader, UUID) + sizeof(BigObjMagic);
      if (mv.size() < minsize) {
        ir.assign(L"COFF import library", types::coff_import_library);
        return Found;
      }
      const char *start = reinterpret_cast<const char *>(mv.data()) +
                          offsetof(BigObjHeader, UUID);
      if (memcmp(start, BigObjMagic, sizeof(BigObjMagic)) == 0) {
        ir.assign(L"COFF object", types::coff_object);
        return Found;
      }
      if (memcmp(start, ClGlObjMagic, sizeof(ClGlObjMagic)) == 0) {
        ir.assign(L"Microsoft cl.exe's intermediate code file",
                  types::coff_cl_gl_object);
        return Found;
      }
      ir.assign(L"COFF import library", types::coff_import_library);
      return Found;
    }
    if (mv.size() >= sizeof(WinResMagic) &&
        memcmp(mv.data(), WinResMagic, sizeof(WinResMagic)) == 0) {
      ir.assign(L"Windows compiled resource file (.res)",
                types::windows_resource);
      return Found;
    }
    // if (mv[1] == 0) {
    //   ir.assign(L"COFF object", types::coff_object);
    //   return Found;
    // }
    if (mv.StartsWith(wasmobj)) {
      ir.assign(L"WebAssembly Object file", types::wasm_object);
      return Found;
    }
    break;
  case 0xDE:
    if (mv.StartsWith(irobj)) {
      ir.assign(L"LLVM IR bitcode", types::bitcode);
      return Found;
    }
    break;
  case 'B':
    if (mv.StartsWith(irobj2)) {
      ir.assign(L"LLVM IR bitcode", types::bitcode);
      return Found;
    }
    break;
  case '!': // .a
    if (mv.StartsWith("!<arch>\n") && !mv.StartsWith(debMagic) ||
        mv.StartsWith("!<thin>\n")) {
      // Skip DEB package
      ir.assign(L"ar style archive file", types::archive);
      return Found;
    }
    break;
  case '\177': // ELF
    if (mv.StartsWith(ElfMagic) && mv.size() >= 18) {
      bool Data2MSB = (mv[5] == 2);
      unsigned high = Data2MSB ? 16 : 17;
      unsigned low = Data2MSB ? 17 : 16;
      if (mv[high] == 0) {
        switch (mv[low]) {
        default:
          ir.assign(L"ELF Unknown type", types::elf, types::ELF);
          return Found;
        case 1:
          ir.assign(L"ELF Relocatable object file", types::elf_relocatable,
                    types::ELF);
          return Found;
        case 2:
          ir.assign(L"ELF Executable image", types::elf_executable, types::ELF);
          return Found;
        case 3:
          ir.assign(L"ELF dynamically linked shared lib",
                    types::elf_shared_object, types::ELF);
          return Found;
        case 4:
          ir.assign(L"ELF core image", types::elf_core, types::ELF);
          return Found;
        }
      }
      ir.assign(L"ELF Unknown type", types::elf, types::ELF);
      return Found;
    }
    break;
  case 0xCA:
    if (mv.StartsWith("\xCA\xFE\xBA\xBE") ||
        mv.StartsWith("\xCA\xFE\xBA\xBF")) {
      if (mv.size() >= 8 && mv[7] < 43) {
        ir.assign(L"Mach-O universal binary", types::macho_universal_binary,
                  types::MACHO);
        return Found;
      }
    }
    break;
  case 0xFE:
  case 0xCE:
  case 0xCF: {
    uint16_t type = 0;
    if (mv.StartsWith("\xFE\xED\xFA\xCE") ||
        mv.StartsWith("\xFE\xED\xFA\xCF")) {
      /* Native endian */
      size_t minsize;
      if (mv[3] == 0xCE) {
        minsize = sizeof(mach_header);
      } else {
        minsize = sizeof(mach_header_64);
      }
      if (mv.size() >= minsize)
        type = mv[12] << 24 | mv[13] << 12 | mv[14] << 8 | mv[15];
    } else if (mv.StartsWith("\xCE\xFA\xED\xFE") ||
               mv.StartsWith("\xCF\xFA\xED\xFE")) {
      /* Reverse endian */
      size_t minsize;
      if (mv[0] == 0xCE) {
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
      ir.assign(L"Mach-O Object file", types::macho_object, types::MACHO);
      return Found;
    case 2:
      ir.assign(L"Mach-O Executable", types::macho_executable, types::MACHO);
      return Found;
    case 3:
      ir.assign(L"Mach-O Shared Lib, FVM",
                types::macho_fixed_virtual_memory_shared_lib, types::MACHO);
      return Found;
    case 4:
      ir.assign(L"Mach-O Core File", types::macho_core, types::MACHO);
      return Found;
    case 5:
      ir.assign(L"Mach-O Preloaded Executable", types::macho_preload_executable,
                types::MACHO);
      return Found;
    case 6:
      ir.assign(L"Mach-O dynlinked shared lib",
                types::macho_dynamically_linked_shared_lib, types::MACHO);
      return Found;
    case 7:
      ir.assign(L"The Mach-O dynamic linker", types::macho_dynamic_linker,
                types::MACHO);
      return Found;
    case 8:
      ir.assign(L"Mach-O Bundle file", types::macho_bundle, types::MACHO);
      return Found;
    case 9:
      ir.assign(L"Mach-O Shared lib stub",
                types::macho_dynamically_linked_shared_lib_stub, types::MACHO);
      return Found;
    case 10:
      ir.assign(L"Mach-O dSYM companion file", types::macho_dsym_companion,
                types::MACHO);
      return Found;
    case 11:
      ir.assign(L"Mach-O kext bundle file", types::macho_kext_bundle,
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
      ir.assign(L"COFF object", types::coff_object);
      return Found;
    }
    [[fallthrough]];
  case 0x90: // PA-RISC Windows
  case 0x68: // mc68K Windows
    if (mv[1] == 0x02) {
      ir.assign(L"COFF object", types::coff_object);
      return Found;
    }
    break;
  case 'M':
    if (mv.StartsWith("Microsoft C/C++ MSF 7.00\r\n")) {
      ir.assign(L"Windows PDB debug info file", types::pdb);
      return Found;
    }
    if (mv.StartsWith("MZ") && mv.size() >= 0x3c + 4) {
      // read32le
      uint32_t off = bela::readle<uint32_t>(mv.data() + 0x3c);
      auto sv = mv.submv(off);
      if (sv.StartsWith(PEMagic)) {
        ir.assign(L"PE executable file", types::pecoff_executable,
                  types::PECOFF);
        return Found;
      }
    }
    break;
  case 0x64: // x86-64 or ARM64 Windows.
    if (mv[1] == 0x86 || mv[1] == 0xaa) {
      ir.assign(L"COFF object", types::coff_object);
      return Found;
    }
    break;
  default:
    break;
  }
  return None;
}

} // namespace inquisitive
