#ifndef PLANCK_ERRORCODE_HPP
#define PLANCK_ERRORCODE_HPP
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

namespace base {
// final_act
// https://github.com/Microsoft/GSL/blob/ebe7ebfd855a95eb93783164ffb342dbd85cbc27/include/gsl/gsl_util#L85-L89
template <class F> class final_act {
public:
  explicit final_act(F f) noexcept : f_(std::move(f)), invoke_(true) {}

  final_act(final_act &&other) noexcept
      : f_(std::move(other.f_)), invoke_(other.invoke_) {
    other.invoke_ = false;
  }

  final_act(const final_act &) = delete;
  final_act &operator=(const final_act &) = delete;

  ~final_act() noexcept {
    if (invoke_)
      f_();
  }

private:
  F f_;
  bool invoke_;
};

// finally() - convenience function to generate a final_act
template <class F> inline final_act<F> finally(const F &f) noexcept {
  return final_act<F>(f);
}

template <class F> inline final_act<F> finally(F &&f) noexcept {
  return final_act<F>(std::forward<F>(f));
}
struct error_code {
  std::wstring message;
  long code{NO_ERROR};
  explicit operator bool() const noexcept { return code != NO_ERROR; }
};
inline error_code make_error_code(int val, std::wstring_view msg) {
  return error_code{std::wstring(msg), val};
}
inline error_code make_error_code(std::wstring_view msg) {
  return error_code{std::wstring(msg), -1};
}
inline std::wstring system_error_dump(DWORD ec) {
  LPWSTR buf = nullptr;
  auto rl = FormatMessageW(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, ec,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&buf, 0, nullptr);
  if (rl == 0) {
    return L"FormatMessageW error";
  }
  std::wstring msg(buf, rl);
  LocalFree(buf);
  return msg;
}

inline error_code make_system_error_code() {
  error_code ec;
  ec.code = GetLastError();
  ec.message = system_error_dump(ec.code);
  return ec;
}
} // namespace base

#endif