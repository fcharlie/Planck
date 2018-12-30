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
#include "details.hpp"

namespace inquisitive {


template <class IntegerT>
[[nodiscard]] inline bool Integer_append_chars(const IntegerT _Raw_value,
                                               const int _Base,
                                               std::wstring &wstr) noexcept {
  using _Unsigned = std::make_unsigned_t<IntegerT>;
  _Unsigned _Value = static_cast<_Unsigned>(_Raw_value);
  if constexpr (std::is_signed_v<IntegerT>) {
    if (_Raw_value < 0) {
      wstr.push_back('-');
      _Value = static_cast<_Unsigned>(0 - _Value);
    }
  }

  constexpr size_t _Buff_size =
      sizeof(_Unsigned) * CHAR_BIT; // enough for base 2
  wchar_t _Buff[_Buff_size];
  wchar_t *const _Buff_end = _Buff + _Buff_size;
  wchar_t *_RNext = _Buff_end;

  static constexpr wchar_t _Digits[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b',
      'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
  static_assert(std::size(_Digits) == 36);

  switch (_Base) {
  case 10: { // Derived from _UIntegral_to_buff()
    constexpr bool _Use_chunks = sizeof(_Unsigned) > sizeof(size_t);

    if constexpr (_Use_chunks) { // For 64-bit numbers on 32-bit platforms,
                                 // work in chunks to avoid 64-bit divisions.
      while (_Value > 0xFFFF'FFFFU) {
        unsigned long _Chunk =
            static_cast<unsigned long>(_Value % 1'000'000'000);
        _Value = static_cast<_Unsigned>(_Value / 1'000'000'000);

        for (int _Idx = 0; _Idx != 9; ++_Idx) {
          *--_RNext = static_cast<char>('0' + _Chunk % 10);
          _Chunk /= 10;
        }
      }
    }

    using _Truncated =
        std::conditional_t<_Use_chunks, unsigned long, _Unsigned>;

    _Truncated _Trunc = static_cast<_Truncated>(_Value);

    do {
      *--_RNext = static_cast<wchar_t>('0' + _Trunc % 10);
      _Trunc /= 10;
    } while (_Trunc != 0);
    break;
  }

  case 2:
    do {
      *--_RNext = static_cast<wchar_t>('0' + (_Value & 0b1));
      _Value >>= 1;
    } while (_Value != 0);
    break;

  case 4:
    do {
      *--_RNext = static_cast<wchar_t>('0' + (_Value & 0b11));
      _Value >>= 2;
    } while (_Value != 0);
    break;

  case 8:
    do {
      *--_RNext = static_cast<wchar_t>('0' + (_Value & 0b111));
      _Value >>= 3;
    } while (_Value != 0);
    break;

  case 16:
    do {
      *--_RNext = _Digits[_Value & 0b1111];
      _Value >>= 4;
    } while (_Value != 0);
    break;

  case 32:
    do {
      *--_RNext = _Digits[_Value & 0b11111];
      _Value >>= 5;
    } while (_Value != 0);
    break;

  default:
    do {
      *--_RNext = _Digits[_Value % _Base];
      _Value = static_cast<_Unsigned>(_Value / _Base);
    } while (_Value != 0);
    break;
  }
  const ptrdiff_t _Digits_written = _Buff_end - _RNext;
  wstr.append(_RNext, _Digits_written);
  return true;
}

template <class IntegerT>
[[nodiscard]] inline std::wstring
Integer_to_chars(const IntegerT _Raw_value,
                 const int _Base) noexcept // strengthened
{
  std::wstring wr;
  Integer_append_chars(_Raw_value, _Base, wr);
  return wr;
}

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
                                                std::error_code &ec);
std::optional<pe_minutiae_t> inquisitive_pecoff(std::wstring_view sv,
                                                std::error_code &ec);
std::optional<elf_minutiae_t> inquisitive_elf(std::wstring_view sv,
                                              std::error_code &ec);
} // namespace inquisitive

#endif
