/// common
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include "probe.hpp"

probe::mapview::~mapview() {
  if (maped) {
    UnmapViewOfFile(FileMapHandle);
  }
  if (FileMapHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(FileMapHandle);
  }
  if (FileHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(FileHandle);
  }
}

#ifndef _M_X64
class FsDisableRedirection {
public:
  typedef BOOL WINAPI fntype_Wow64DisableWow64FsRedirection(PVOID *OldValue);
  typedef BOOL WINAPI fntype_Wow64RevertWow64FsRedirection(PVOID *OldValue);
  FsRedirection() {
    auto pfnWow64DisableWow64FsRedirection =
        (fntype_Wow64DisableWow64FsRedirection *)GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"),
            "Wow64DisableWow64FsRedirection");
    if (pfnWow64DisableWow64FsRedirection) {
      pfnWow64DisableWow64FsRedirection(&OldValue);
    }
  }
  ~FsRedirection() {
    auto pfnWow64RevertWow64FsRedirection =
        (fntype_Wow64RevertWow64FsRedirection *)GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"), "Wow64RevertWow64FsRedirection");
    if (pfnWow64RevertWow64FsRedirection) {
      pfnWow64RevertWow64FsRedirection(&OldValue);
    }
  }

private:
  PVOID OldValue = nullptr;
};
#endif

std::wstring FindExtension(std::wstring_view sv) {
  auto end = sv.data() + sv.size();
  auto it = end;
  for (; it != sv.data(); it--) {
    if (*it == 'L\\' || *it == L'/') {
      break;
    }
  }
  if (it == end) {
    return L"";
  }
  std::wstring_view fn(it + 1, end - it - 1);
  auto pos = fn.rfind(L'.');
  if (pos == std::string_view::npos) {
    return L"";
  }
  return std::wstring(fn.data() + pos + 1, fn.size() - pos - 1);
}

std::optional<std::wstring> probe::mapview::view(std::wstring_view sv) {
#ifndef _M_X64
  FsDisableRedirection fdr;
#endif
  FileHandle = CreateFileW(sv.data(), GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (FileHandle == INVALID_HANDLE_VALUE) {
    return std::make_optional<std::wstring>(L"CreateFileW error");
  }
  LARGE_INTEGER li;
  if (GetFileSizeEx(FileHandle, &li)) {
    size_ = li.QuadPart;
  }
  // File size.
  if (size_ <= 0) {
    return std::make_optional<std::wstring>(L"File size too small");
  }
  if (size_ > SIZE_MAX) {
    return std::make_optional<std::wstring>(
        L"File size too large, Please use x64");
  }
  FileMapHandle =
      CreateFileMappingW(FileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if (FileMapHandle == INVALID_HANDLE_VALUE) {
    return std::make_optional<std::wstring>(L"CreateFileMappingW error");
  }
  auto addr = ::MapViewOfFile(FileMapHandle, FILE_MAP_READ, 0, 0, 0);
  if (addr == nullptr) {
    return std::make_optional<std::wstring>(L"MapViewOfFile error");
  }
  data_ = reinterpret_cast<const char *>(addr);
  maped = true;
  extension = FindExtension(sv);
  return std::nullopt;
}
