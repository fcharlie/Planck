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
#include "types.hpp"

namespace inquisitive {
using byte_t = unsigned char;
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

struct inquisitive_attribute_t {
  std::wstring name;
  std::wstring value;
  inquisitive_attribute_t() = default;
  inquisitive_attribute_t(std::wstring_view n, std::wstring_view v)
      : name(n), value(v) {
    //
  }
};

struct inquisitive_result_t {
  std::wstring name;
  std::vector<inquisitive_attribute_t> attrs;
  types::Type type{types::none};
  types::TypeEx typeex{types::NONE};
  inquisitive_result_t() = default;
  inquisitive_result_t(std::wstring_view dv, types::Type t0 = types::none,
                       types::TypeEx t1 = types::NONE) {
    Assign(dv, t0, t1);
  }
  inquisitive_result_t &Assign(std::wstring_view dv,
                               types::Type t0 = types::none,
                               types::TypeEx t1 = types::NONE) {
    name.assign(dv);
    type = t0;
    typeex = t1;
    return *this;
  }
  inquisitive_result_t &Add(std::wstring_view name, std::wstring_view value) {
    attrs.emplace_back(name, value);
    return *this;
  }
};

std::wstring fromutf8(std::string_view text);

typedef enum inquisitive_status_e : int {
  None = 0,
  Found, ///
  Break
} status_t;

// ---> todo resolve
status_t inquisitive_binobj(memview mv, inquisitive_result_t &ir);
status_t inquisitive_images(memview mv, inquisitive_result_t &ir);
status_t inquisitive_docs(memview mv, inquisitive_result_t &ir);
status_t inquisitive_archives(memview mv, inquisitive_result_t &ir);
status_t inquisitive_zip_family(memview mv, inquisitive_result_t &ir);
status_t inquisitive_fonts(memview mv, inquisitive_result_t &ir);
status_t inquisitive_text(memview mv, inquisitive_result_t &ir);
status_t inquisitive_media(memview mv, inquisitive_result_t &ir);
status_t inquisitive_chardet(memview mv, inquisitive_result_t &ir);

std::optional<inquisitive_result_t> inquisitive(std::wstring_view sv,
                                                base::error_code &ec);

std::optional<pe_minutiae_t> inquisitive_pecoff(std::wstring_view sv,
                                                base::error_code &ec);
std::optional<elf_minutiae_t> inquisitive_elf(std::wstring_view sv,
                                              base::error_code &ec);
} // namespace inquisitive

#endif
