////
#ifndef FILEVIEWER_INQUISTIVE_HPP
#define FILEVIEWER_INQUISTIVE_HPP
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
#include "details.hpp"

namespace inquisitive {
constexpr const int einident = 16;
using planck::memview;
namespace endina {
enum endian_t : unsigned { None, LittleEndian, BigEndian };
}
struct elf_minutiae_t {
  uint8_t ident[einident];
  bool is64bit{false}; /// 64 Bit
  endina::endian_t endian;
  std::wstring machine;
  std::wstring osabi;
  std::wstring etype;
  std::wstring rpath;             // RPATH or some
  std::vector<std::wstring> deps; /// require so
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
  std::vector<std::wstring> deps; /// DLL required
  pe_version_t osver;
  pe_version_t linkver;
  pe_version_t imagever;
  bool isdll;
};

namespace types {
enum Types {
  NONE,
  PECOFF, /// if return PECOFF todo dump PE details
  ZIP,
  ELF,
};
}

struct inquisitive_result_t {
  details::Types basetype;
  types::Types type;
  std::wstring details;
};

std::wstring fromutf8(std::string_view text);
std::optional<inquisitive_result_t> inquisitive(std::wstring_view sv,
                                                std::error_code &ec);
std::optional<pe_minutiae_t> inquisitive_pecoff(std::wstring_view sv,
                                                std::error_code &ec);
std::optional<elf_minutiae_t> inquisitive_elf(std::wstring_view sv,
                                              std::error_code &ec);
} // namespace inquisitive

#endif
