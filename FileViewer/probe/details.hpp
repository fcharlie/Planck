//////
#ifndef PROBE_DETAILS_HPP
#define PROBE_DETAILS_HPP
#pragma once
#include <string_view>

namespace probe {
namespace details {
enum Types {
  none,
  /// TEXT
  ascii,
  utf8,
  utf8bom,
  utf16le,
  utf16be,
  ///
  bitcode,                               ///< Bitcode file
  archive,                               ///< ar style archive file
  elf,                                   ///< ELF Unknown type
  elf_relocatable,                       ///< ELF Relocatable object file
  elf_executable,                        ///< ELF Executable image
  elf_shared_object,                     ///< ELF dynamically linked shared lib
  elf_core,                              ///< ELF core image
  macho_object,                          ///< Mach-O Object file
  macho_executable,                      ///< Mach-O Executable
  macho_fixed_virtual_memory_shared_lib, ///< Mach-O Shared Lib, FVM
  macho_core,                            ///< Mach-O Core File
  macho_preload_executable,              ///< Mach-O Preloaded Executable
  macho_dynamically_linked_shared_lib,   ///< Mach-O dynlinked shared lib
  macho_dynamic_linker,                  ///< The Mach-O dynamic linker
  macho_bundle,                          ///< Mach-O Bundle file
  macho_dynamically_linked_shared_lib_stub, ///< Mach-O Shared lib stub
  macho_dsym_companion,                     ///< Mach-O dSYM companion file
  macho_kext_bundle,                        ///< Mach-O kext bundle file
  macho_universal_binary,                   ///< Mach-O universal binary
  coff_cl_gl_object,   ///< Microsoft cl.exe's intermediate code file
  coff_object,         ///< COFF object file
  coff_import_library, ///< COFF import library
  pecoff_executable,   ///< PECOFF executable file
  windows_resource,    ///< Windows compiled resource file (.res)
  wasm_object,         ///< WebAssembly Object file
  pdb,                 ///< Windows PDB debug info file
  ///
  epub,
  cab,
  msi,
  zip,
  p7z,
  rar,
  tar,
  baz,
  xz,
  deb,
  ar,
  rpm,
  sqlite,
  //
  jpg,
  jp2,
  png,
  gif,
  webp,
  cr2,
  tif,
  bmp,
  jxr,
  psd,
  ico,
  //
  doc,
  docx,
  xls,
  xlsx,
  ppt,
  pptx
};
}
} // namespace probe

#endif