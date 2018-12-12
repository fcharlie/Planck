//////
#ifndef PROBE_DETAILS_HPP
#define PROBE_DETAILS_HPP
#pragma once
#include <string_view>

namespace probe {
namespace details {
enum Types {
  ANSI,
  UTF8,
  UTF8BOM,
  UTF16LE,
  UTF16BE,
  ///
  EPUB,
  PE,  // EXE DLL
  ELF, //
  CAB,
  MSI,
  ZIP,
  SevenZIP,
  RAR,
  TAR,
  BZ2,
  XZ,
  DEB,
  AR,
  RPM,
  SQLITE,
  //
  JPG,
  JP2,
  PNG,
  GIF,
  WEBP,
  CR2,
  TIF,
  BMP,
  JXR,
  PSD,
  ICON,
  //
  DOC,
  DOCX,
  XLS,
  XLSX,
  PPT,
  PPTX
};
}
} // namespace probe

#endif