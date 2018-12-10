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
#include <variant>

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

enum file_type_t : unsigned long {
  HardLink, /// If no reparse point, file always hardlink
  SymbolicLink,
  MountPoint,
  WimImage,
  Wof,
  Wcifs,
  AppExecLink,
  AFUnix, /// Unix domain socket
  OneDrive,
  Placeholder,
  StorageSync,
  ProjFS
};
/*
static LPCWSTR AppExecLinkParts[] =
{
    L"AppPackageID",
    L"AppUserModelID",
    L"TargetPath",
};
*/

inline std::wstring hexencode(const char *data, size_t len) {
  const wchar_t hex[] = L"0123456789abcdef";
  std::wstring hs(len * 2, L'0');
  wchar_t *buf = &hs[0];
  auto end = data + len;
  for (auto it = data; it != end; it++) {
    unsigned int val = *data;
    *buf++ = hex[val >> 4];
    *buf++ = hex[val & 0xf];
  }
  return hs;
}

struct appexeclink_t {
  std::wstring pkid;
  std::wstring appuserid;
  std::wstring target;
};
struct reparse_wim_t {
  std::wstring guid;
  std::wstring hash;
};

struct reparse_wof_t {
  ULONG wofversion;
  ULONG wofprovider;
  ULONG version;
  ULONG algorithm;
};

struct reparse_wcifs_t {
  ULONG Version; // Expected to be 1 by wcifs.sys
  ULONG Reserved;
  //GUID LookupGuid;      // GUID used for lookup in wcifs!WcLookupLayer
  //USHORT WciNameLength; // Length of the WCI subname, in bytes
  std::wstring LookupGuid;
  std::wstring WciName;
};

struct reparse_hsm_t {
  USHORT Flags;  // Flags (0x8000 = not compressed)
  USHORT Length; // Length of the data (uncompressed)
  std::vector<std::byte> data;
};
using av_internal_t = std::variant<appexeclink_t, reparse_wim_t, reparse_wof_t,
                                   reparse_wcifs_t, reparse_hsm_t>;
struct file_target_t {
  std::wstring path;
  av_internal_t av;
  unsigned long type{HardLink};
};

struct file_links_t {
  std::wstring self;
  std::vector<std::wstring> links;
};

std::optional<file_target_t> ResolveTarget(std::wstring_view sv);
std::optional<file_links_t> ResolveLinks(std::wstring_view sv);

} // namespace viewer

#endif