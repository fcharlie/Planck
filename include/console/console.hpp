///
#ifndef PRIVEXEC_CONSOLE_HPP
#define PRIVEXEC_CONSOLE_HPP
#include "adapter.hpp"

namespace priv {
// ChangePrintMode todo
inline bool ChangePrintMode(bool isstderr) {
  return details::adapter::instance().changeout(isstderr);
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

} // namespace priv
#include "adapter.ipp"
#endif