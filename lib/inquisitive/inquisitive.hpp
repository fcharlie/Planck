////
#ifndef FILEVIEWER_INQUISITIVE_HPP
#define FILEVIEWER_INQUISITIVE_HPP
#pragma once
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <system_error>
#include <mapview.hpp>
#include <charconv.hpp>
#include <errorcode.hpp>
#include "details.hpp"

namespace inquisitive {


constexpr const int einident = 16;
using planck::memview;
namespace endian {
enum endian_t : unsigned { None, LittleEndian, BigEndian };
}
struct elf_minutiae_t {
  std::wstring machine;
  std::wstring osabi;
  std::wstring etype;
  std::wstring rpath;                // RPATH or some
  std::wstring rupath;               // RUPATH
  std::wstring soname;               // SONAME
  std::vector<std::wstring> depends; /// require so
  int version;
  endian::endian_t endian;
  bool bit64{false}; /// 64 Bit
};

struct pe_version_t {
  uint16_t major{0};
  uint16_t minor{0};
};

struct pe_minutiae_t {
  std::wstring machine;
  std::wstring subsystem;
  std::wstring clrmsg;
  std::vector<std::wstring> characteristics;
  std::vector<std::wstring> depends; /// DLL required
  pe_version_t osver;
  pe_version_t linkver;
  pe_version_t imagever;
  bool isdll;
};

namespace types {
enum Types {
  NONE,
  PECOFF, /// if return PECOFF todo dump PE details
  ELF,
  MACHO,
  ZIP

};
}

struct inquisitive_result_t {
  details::Types basetype;
  types::Types type;
  std::wstring details;
};

std::wstring fromutf8(std::string_view text);
std::optional<inquisitive_result_t> inquisitive(std::wstring_view sv,
                                                base::error_code &ec);
std::optional<pe_minutiae_t> inquisitive_pecoff(std::wstring_view sv,
                                                base::error_code &ec);
std::optional<elf_minutiae_t> inquisitive_elf(std::wstring_view sv,
                                              base::error_code &ec);
} // namespace inquisitive

#endif
