/// common
#include "inquisitive.hpp"

std::wstring FindExtension(std::wstring_view sv) {
  auto end = sv.data() + sv.size();
  auto it = end;
  for (; it != sv.data(); it--) {
    if (*it == 'L\\' || *it == L'/') {
      break;
    }
  }
  if (it == end) {
    return L"";
  }
  std::wstring_view fn(it + 1, end - it - 1);
  auto pos = fn.rfind(L'.');
  if (pos == std::string_view::npos) {
    return L"";
  }
  return std::wstring(fn.data() + pos + 1, fn.size() - pos - 1);
}

namespace inquisitive {
namespace details {

const wchar_t *typenames(Types t) {
  switch (t) {
  case none:
    break;
  case utf8:
    return L"UTF-8 Unicode text";
  case utf8bom:
    return L"UTF-8 Unicode (with BOM) text";
  case utf16le:
    return L"Little-endian UTF-16 Unicode text";
  case utf16be:
    return L"Big-endian UTF-16 Unicode text";
  case bitcode:
    return L"LLVM IR bitcode";
  case archive:
    return L"ar style archive file"; /// Static library
  case elf:
    return L"ELF Unknown type";
  case elf_relocatable:
    return L"ELF Relocatable object file";
  case elf_executable:
    return L"ELF Executable image";
  case elf_shared_object:
    return L"ELF dynamically linked shared lib";
  case elf_core:
    return L"ELF core image";
  case macho_object:
    return L"Mach-O Object file";
  case macho_executable:
    return L"Mach-O Executable";
  case macho_fixed_virtual_memory_shared_lib:
    return L"Mach-O Shared Lib, FVM";
  case macho_core:
    return L"Mach-O Core File";
  case macho_preload_executable:
    return L"Mach-O Preloaded Executable";
  case macho_dynamically_linked_shared_lib:
    return L"Mach-O dynlinked shared lib";
  case macho_dynamic_linker:
    return L"The Mach-O dynamic linker";
  case macho_bundle:
    return L"Mach-O Bundle file";
  case macho_dynamically_linked_shared_lib_stub:
    return L"Mach-O Shared lib stub";
  case macho_dsym_companion:
    return L"Mach-O dSYM companion file";
  case macho_kext_bundle:
    return L"Mach-O kext bundle file";
  case macho_universal_binary:
    return L"Mach-O universal binary";
  case coff_cl_gl_object:
    return L"Microsoft cl.exe's intermediate code file";
  case coff_object:
    return L"COFF object file";
  case coff_import_library:
    return L"COFF import library";
  case pecoff_executable:
    return L"PECOFF executable file";
  case windows_resource:
    return L"Windows compiled resource file (.res)";
  case wasm_object:
    return L"WebAssembly Object file";
  case pdb:
    return L"Windows PDB debug info file";
  case epub:
    return L"EPUB document";
  case zip:
    return L"Zip Archive";
  case tar:
    return L"Tarball Archive";
  case rar:
    return L"Roshal Archive (rar)";
  case gz:
    return L"gz compressed data";
  case bz2:
    return L"bz2 compressed data";
  case p7z:
    return L"7-Zip Archive";
  case xz:
    return L"XZ Archive";
  case pdf:
    return L"PDF document";
  case swf:
    return L"Adobe Flash file format";
  case rtf:
    return L"Rich Text Format data";
  case eot:
    return L"Embedded OpenType";
  case ps:
    return L"Postscript";
  case sqlite:
    return L"SQLite3";
  case nes:
    return L"application/x-nintendo-nes-rom";
  case crx:
    return L"Google Chrome Extension";
  case deb:
    return L"Debian packages";
  case lz:
    return L"lzip Achive";
  case rpm:
    return L"RPM Package Manager";
  case cab:
    return L"Cabinet (CAB) Microsoft Windows installation archive format";
  case msi:
    return L"Windows Installer packages";
  case jpg:
    return L"JPEG Image";
  case jp2:
    return L"JPEG 2000 Image";
  case png:
    return L"Portable Network Graphics (.png)";
  case gif:
    return L"Graphics Interchange Format (.gif)";
  case webp:
    return L"WebP Image";
  case cr2:
    return L"Canon 5D Mark IV CR2";
  case tif:
    return L"Tagged Image File Format (.tif)";
  case bmp:
    return L"Bitmap image file format (.bmp)";
  case jxr:
    return L"JPEG extended range";
  case psd:
    return L"Photoshop document file extension";
  case ico:
    return L"ICO file format (.ico)";
  case doc:
    return L"Microsoft Word (.doc)";
  case docx:
    return L"Microsoft Word (.docx)";
  case xls:
    return L"Microsoft Excel (.xls)";
  case xlsx:
    return L"Microsoft Excel (.xlsx)";
  case ppt:
    return L"Microsoft PowerPoint (.ppt)";
  case pptx:
    return L"Microsoft PowerPoint (.pptx)";
  default:
    break;
  }
  return L"ASCII Text";
}
} // namespace details

details::Types identify_binexeobj_magic(std::string_view mv); /// binexeobj.cc
details::Types identify_image(std::string_view mv);
details::Types identity_text(memview mv) {
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

details::Types identify_font(memview mv) {
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

std::optional<inquisitive_result_t> inquisitive(std::wstring_view sv,
                                                std::error_code &ec) {
  auto extension = FindExtension(sv);
  return std::nullopt;
}
} // namespace inquisitive