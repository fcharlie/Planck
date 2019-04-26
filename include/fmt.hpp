// Impl StrFormat
#ifndef PLANCK_FMT_HPP
#define PLANCK_FMT_HPP
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <string_view>
// Thanks
// https://github.com/chromium/chromium/blob/master/base/strings/safe_sprintf.cc
// https://github.com/chromium/chromium/blob/master/base/strings/safe_sprintf.h

namespace wink {
namespace internal {
struct Arg {
  enum Type {
    INTEGER,
    UINTEGER,
    STRING, // C-style string and string_view
    POINTER
  };
  // Any integer-like value.
  Arg(signed char c) : type(INTEGER) {
    integer.i = c;
    integer.width = sizeof(char);
  }
  Arg(unsigned char c) : type(UINTEGER) {
    integer.i = c;
    integer.width = sizeof(char);
  }
  Arg(signed short j) : type(INTEGER) {
    integer.i = j;
    integer.width = sizeof(short);
  }
  Arg(unsigned short j) : type(UINTEGER) {
    integer.i = j;
    integer.width = sizeof(short);
  }
  Arg(signed int j) : type(INTEGER) {
    integer.i = j;
    integer.width = sizeof(int);
  }
  Arg(unsigned int j) : type(UINTEGER) {
    integer.i = j;
    integer.width = sizeof(int);
  }
  Arg(signed long j) : type(INTEGER) {
    integer.i = j;
    integer.width = sizeof(long);
  }
  Arg(unsigned long j) : type(UINTEGER) {
    integer.i = j;
    integer.width = sizeof(long);
  }
  Arg(signed long long j) : type(INTEGER) {
    integer.i = j;
    integer.width = sizeof(long long);
  }
  Arg(unsigned long long j) : type(UINTEGER) {
    integer.i = j;
    integer.width = sizeof(long long);
  }

  // A C-style text string. and string_view
  Arg(std::wstring_view sv) : type(STRING) {
    stringview.data = sv.data();
    stringview.len = sv.size();
  }

  // Any pointer value that can be cast to a "void*".
  template <class T> Arg(T *p) : ptr((void *)p), type(POINTER) {}
  union {
    struct {
      int64_t i;
      unsigned char width;
    } integer;
    struct {
      double f;
      unsigned char width;
    }floating;
    struct {
      const wchar_t *data;
      size_t len;
    } stringview;
    const void *ptr;
  };
  const enum Type type;
};
ssize_t StrFormatInternal(wchar_t *buf, size_t sz, const wchar_t *fmt,
                          const Arg *args, size_t max_args);
} // namespace internal
template <typename... Args>
ssize_t StrFormat(wchar_t *buf, size_t N, const wchar_t *fmt, Args... args) {
  const internal::Arg arg_array[] = {args...};
  return internal::StrFormatInternal(buf, N, fmt, arg_array, sizeof...(args));
}

template <size_t N, typename... Args>
ssize_t StrFormat(wchar_t (&buf)[N], const wchar_t *fmt, Args... args) {
  // Use Arg() object to record type information and then copy arguments to an
  // array to make it easier to iterate over them.
  const internal::Arg arg_array[] = {args...};
  return internal::StrFormatInternal(buf, N, fmt, arg_array, sizeof...(args));
}
// Fast-path when we don't actually need to substitute any arguments.
ssize_t StrFormat(wchar_t *buf, size_t N, const wchar_t *fmt);
template <size_t N> inline ssize_t StrFormat(wchar_t (&buf)[N], const wchar_t *fmt) {
  return StrFormat(buf, N, fmt);
}

} // namespace wink

#endif
