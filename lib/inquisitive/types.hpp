///////////
#ifndef INQUISITIVE_INDEXS_HPP
#define INQUISITIVE_INDEXS_HPP

namespace inquisitive {
//
namespace types {
enum Type {
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
  /// archive
  epub,
  zip,
  tar,
  rar,
  gz,
  bz2,
  p7z,
  xz,
  pdf,
  swf,
  rtf,
  eot,
  ps,
  sqlite,
  nes,
  crx,
  deb,
  lz,
  rpm,
  cab,
  msi,
  dmg,
  xar,
  wim,
  z,
  // image
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
  // docs
  doc,
  docx,
  xls,
  xlsx,
  ppt,
  pptx,
  // font
  woff,
  woff2,
  ttf,
  otf,
  // Media
  midi,
  mp3,
  m4a,
  ogg,
  flac,
  wav,
  amr,
  aac,
  mp4,
  m4v,
  mkv,
  webm,
  mov,
  avi,
  wmv,
  mpeg,
  flv,
  // support git 
  gitpack,
  gitpkindex,
  gitmidx
};

enum TypeEx {
  NONE,
  PECOFF, /// if return PECOFF todo dump PE details
  ELF,
  MACHO,
  ZIP
};
} // namespace types
} // namespace inquisitive

#endif