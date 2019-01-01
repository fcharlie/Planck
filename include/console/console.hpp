///
#ifndef PLANCK_CONSOLE_HPP
#define PLANCK_CONSOLE_HPP
#include "adapter.hpp"
#include <string_view>

namespace planck {
// ChangePrintMode todo
inline bool ChangePrintMode(bool isstderr) {
  return details::adapter::instance().changeout(isstderr);
}

// check we use wide console output
inline bool UseWideConsole() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE || hOut == nullptr) {
    return false;
  }
  if (GetFileType(hOut) == FILE_TYPE_CHAR && GetConsoleOutputCP() == 65001) {
    return false;
  }
  return true;
}

std::wstring fromutf8(std::string_view sv) {
  auto sz =
      MultiByteToWideChar(CP_UTF8, 0, sv.data(), (int)sv.size(), nullptr, 0);
  std::wstring output;
  output.resize(sz);
  // C++17 string::data() has non const function
  MultiByteToWideChar(CP_UTF8, 0, sv.data(), (int)sv.size(), output.data(), sz);
  return output;
}

inline DWORD WriteWide(std::string_view msg) {
  auto w = fromutf8(msg);
  auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE || hOut == nullptr) {
    return 0;
  }
  DWORD dwWrite = 0;
  WriteConsoleW(hOut, w.data(), (DWORD)w.size(), &dwWrite, nullptr);
  return dwWrite;
}

inline bool VerboseEnable() {
  return details::adapter::instance().changelevel(details::kAdapterDEBUG) ==
         details::kAdapterDEBUG;
}

template <typename T> T Argument(T value) noexcept { return value; }
template <typename T>
T const *Argument(std::basic_string<T> const &value) noexcept {
  return value.c_str();
}
template <typename... Args>
int StringPrint(wchar_t *const buffer, size_t const bufferCount,
                wchar_t const *const format, Args const &... args) noexcept {
  int const result = swprintf(buffer, bufferCount, format, Argument(args)...);
  // ASSERT(-1 != result);
  return result;
}

template <typename... Args>
ssize_t PrintNone(const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return details::adapter::instance().adapterwrite(fc::White, buffer.data(),
                                                   size);
}

template <typename... Args>
ssize_t Print(int color, const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return details::adapter::instance().adapterwrite(color, buffer.data(), size);
}

template <typename... Args>
ssize_t verbose(const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return details::adapter::instance().adapterwritelevel(details::kAdapterDEBUG,
                                                        buffer.data(), size);
}

template <typename... Args> ssize_t info(const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return details::adapter::instance().adapterwritelevel(details::kAdapterINFO,
                                                        buffer.data(), size);
}

template <typename... Args> ssize_t warn(const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return details::adapter::instance().adapterwritelevel(details::kAdapterWARN,
                                                        buffer.data(), size);
}

template <typename... Args> ssize_t error(const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return details::adapter::instance().adapterwrite(details::kAdapterERROR,
                                                   buffer.data(), size);
}

template <typename... Args> ssize_t fatal(const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return details::adapter::instance().adapterwrite(details::kAdapterFATAL,
                                                   buffer.data(), size);
}

} // namespace planck
#include "adapter.ipp"
#endif