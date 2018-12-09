///
#ifndef FILEVIEWER_RESOLVE_HPP
#define FILEVIEWER_RESOLVE_HPP
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

namespace viewer {
struct error_code {
  long code{S_OK};
  bool operator()() { return code == S_OK; }
  std::wstring message;
  static error_code lasterror() {
    error_code ec;
    ec.code = GetLastError();
    LPWSTR buf = nullptr;
    auto rl = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr,
        ec.code, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&buf, 0,
        nullptr);
    if (rl == 0) {
      ec.message = L"unknown error";
      return ec;
    }
    ec.message.assign(buf, rl);
    LocalFree(buf);
    return ec;
  }
};
struct file_target_t {
  std::wstring type;
  std::wstring target;
};

struct file_links_t {
  std::wstring self;
  std::vector<std::wstring> links;
};

std::optional<file_target_t> ResolveTarget(std::wstring_view sv);
std::optional<file_links_t> ResolveLinks(std::wstring_view sv);

} // namespace viewer

#endif