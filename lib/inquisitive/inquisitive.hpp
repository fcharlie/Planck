////
#ifndef PLANCK_INQUISITIVE_HPP
#define PLANCK_INQUISITIVE_HPP
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
#include <algorithm>
#include <system_error>
#include <mapview.hpp>
#include <bela/base.hpp>
#include "types.hpp"

namespace bela {
inline bela::error_code make_error_code(const bela::AlphaNum &a) {
  return bela::error_code{std::wstring(a.Piece()), 1};
}
} // namespace base

namespace inquisitive {
using byte_t = unsigned char;
constexpr const int einident = 16;
using planck::memview;

/*
 * Compute the length of an array with constant length.  (Use of this method
 * with a non-array pointer will not compile.)
 *
 * Beware of the implicit trailing '\0' when using this with string constants.
 */
template <typename T, size_t N> constexpr size_t ArrayLength(T (&aArr)[N]) {
  return N;
}

template <typename T, size_t N> constexpr T *ArrayEnd(T (&aArr)[N]) {
  return aArr + ArrayLength(aArr);
}

/**
 * std::equal has subpar ergonomics.
 */

template <typename T, typename U, size_t N>
bool ArrayEqual(const T (&a)[N], const U (&b)[N]) {
  return std::equal(a, a + N, b);
}

template <typename T, typename U>
bool ArrayEqual(const T *const a, const U *const b, const size_t n) {
  return std::equal(a, a + n, b);
}

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
  std::vector<std::wstring> delays;  //
  pe_version_t osver;
  pe_version_t linkver;
  pe_version_t imagever;
  bool isdll;
};

struct macho_minutiae_t {
  std::wstring machine;
  std::wstring mtype; /// Mach-O type
  bool isfat{false};
  bool is64abi{false};
};

struct inquisitive_attribute_t {
  std::wstring name;
  std::wstring value;
  inquisitive_attribute_t() = default;
  inquisitive_attribute_t(const std::wstring_view &n,
                          const std::wstring_view &v)
      : name(n), value(v) {}
  inquisitive_attribute_t(std::wstring &&n, std::wstring &&v)
      : name(std::move(n)), value(std::move(v)) {}
};

struct inquisitive_mattribute_t {
  std::wstring name;
  std::vector<std::wstring> values;
  inquisitive_mattribute_t() = default;
  inquisitive_mattribute_t(const std::wstring_view &n,
                           const std::vector<std::wstring> &v)
      : name(n), values(v) {}
  inquisitive_mattribute_t(std::wstring &&n, std::vector<std::wstring> &&v)
      : name(std::move(n)), values(std::move(v)) {}
};

class inquisitive_result {
private:
  std::wstring name;
  std::vector<inquisitive_attribute_t> attrs;
  std::vector<inquisitive_mattribute_t> mattrs;
  std::size_t mnlen{deslen}; // description
  types::Type t{types::none};
  types::TypeEx e{types::NONE};
  void move_from(inquisitive_result &&other) {
    name.assign(std::move(other.name));
    attrs = std::move(other.attrs);
    mattrs = std::move(other.mattrs);
    mnlen = other.mnlen;
    t = other.t;
    e = other.e;
  }
  void copy_from(const inquisitive_result &other) {
    name = other.name;
    attrs = other.attrs;
    mattrs = other.mattrs;
    mnlen = other.mnlen;
    t = other.t;
    e = other.e;
  }

public:
  using mcontainer_t = std::vector<inquisitive_mattribute_t>;
  using container_t = std::vector<inquisitive_attribute_t>;
  static constexpr size_t deslen = sizeof("Description") - 1;
  inquisitive_result() = default;
  inquisitive_result(std::wstring_view dv, types::Type t0 = types::none,
                     types::TypeEx t1 = types::NONE) {
    assign(dv, t0, t1);
  }
  inquisitive_result(inquisitive_result &&other) {
    move_from(std::move(other));
  }

  inquisitive_result &operator=(inquisitive_result &&other) {
    move_from(std::move(other));
    return *this;
  }

  inquisitive_result(const inquisitive_result &other) { copy_from(other); }
  inquisitive_result &operator=(const inquisitive_result &other) {
    copy_from(other);
    return *this;
  }

  void clear() {
    name.clear();
    attrs.clear();
    mattrs.clear();
    mnlen = deslen;
    t = types::none;
    e = types::NONE;
  }

  inquisitive_result &assign(const std::wstring_view &dv,
                             types::Type t0 = types::none,
                             types::TypeEx t1 = types::NONE) {
    name.assign(dv);
    t = t0;
    e = t1;
    return *this;
  }

  inquisitive_result &assign(std::wstring &&dv, types::Type t0 = types::none,
                             types::TypeEx t1 = types::NONE) {
    name.assign(std::move(dv));
    t = t0;
    e = t1;
    return *this;
  }

  inquisitive_result &add(const std::wstring_view &name,
                          const std::wstring_view &value) {
    mnlen = (std::max)(mnlen, name.size());
    attrs.emplace_back(name, value);
    return *this;
  }

  inquisitive_result &add(std::wstring &&name, std::wstring &&value) {
    mnlen = (std::max)(mnlen, name.size());
    attrs.emplace_back(std::move(name), std::move(value));
    return *this;
  }
  inquisitive_result &add(std::wstring &&name, uint64_t value) {
    mnlen = (std::max)(mnlen, name.size());
    auto sv = bela::AlphaNum(value);
    attrs.emplace_back(std::move(name), std::wstring(sv.Piece()));
    return *this;
  }

  inquisitive_result &add(std::wstring &&name,
                          std::vector<std::wstring> &&values) {
    mnlen = (std::max)(mnlen, name.size());
    mattrs.emplace_back(std::move(name), std::move(values));
    return *this;
  }

  inquisitive_result &add(const std::wstring_view &name,
                          const std::vector<std::wstring> &values) {
    mnlen = (std::max)(mnlen, name.size());
    mattrs.emplace_back(name, values);
    return *this;
  }

  const std::wstring &description() const { return name; }
  size_t alignlen() const { return mnlen; }
  types::TypeEx typeex() const { return e; }
  types::Type type() const { return t; }
  const container_t &container() const { return attrs; }
  const mcontainer_t &mcontainer() const { return mattrs; }
};

using inquisitive_result_t = inquisitive_result;

typedef enum inquisitive_status_e : int {
  None = 0,
  Found, ///
  Break
} status_t;

// ---> todo resolve
typedef status_t (*inquisitive_handle_t)(memview mv, inquisitive_result_t &ir);
status_t inquisitive_binobj(memview mv, inquisitive_result_t &ir);
status_t inquisitive_fonts(memview mv, inquisitive_result_t &ir);
status_t inquisitive_zip_family(memview mv, inquisitive_result_t &ir);
status_t inquisitive_docs(memview mv, inquisitive_result_t &ir);
status_t inquisitive_images(memview mv, inquisitive_result_t &ir);
status_t inquisitive_archives(memview mv, inquisitive_result_t &ir);
status_t inquisitive_media(memview mv, inquisitive_result_t &ir);
// EX
status_t inquisitive_gitbinary(memview mv, inquisitive_result_t &ir);
status_t inquisitive_shlink(memview mv, inquisitive_result_t &ir);
/////////// ---
status_t inquisitive_text(memview mv, inquisitive_result_t &ir);
status_t inquisitive_chardet(memview mv, inquisitive_result_t &ir);

std::optional<inquisitive_result_t> inquisitive(std::wstring_view sv,
                                                bela::error_code &ec);

std::optional<pe_minutiae_t> inquisitive_pecoff(std::wstring_view sv,
                                                bela::error_code &ec);
std::optional<elf_minutiae_t> inquisitive_elf(std::wstring_view sv,
                                              bela::error_code &ec);
std::optional<macho_minutiae_t> inquisitive_macho(std::wstring_view sv,
                                                  bela::error_code &ec);
} // namespace inquisitive

#endif
