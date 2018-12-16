////
#ifndef FILEVIEWER_PROBE_HPP
#define FILEVIEWER_PROBE_HPP
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
#include "details.hpp"

namespace probe {
constexpr size_t mappedsizelimit = 4 * 1024 * 1024;
// 4M limit 4M mapped. check is ok
inline size_t mapviewsize(std::int64_t l) {
  return l > mappedsizelimit ? mappedsizelimit : (size_t)l;
}
class mapview {
public:
  mapview() = default;
  mapview(const mapview &) = delete;
  mapview &operator=(const mapview &) = delete;
  ~mapview();
  // open file and map view. so optional false is ok, true is error reason.
  std::optional<std::wstring> view(std::wstring_view sv);
  std::size_t size() const { return size_; }
  const char *data() const { return data_; }
  unsigned char operator[](const std::size_t off) const {
    if (off >= size_) {
      return 255;
    }
    return (unsigned char)data_[off];
  }
  bool startswith(const char *prefix, size_t pl) const {
    if (pl >= size_) {
      return false;
    }
    return memcmp(data_, prefix, pl) == 0;
  }
  bool startswith(std::string_view sv) const {
    return startswith(sv.data(), sv.size());
  }
  bool indexswith(std::size_t offset, std::string_view sv) const {
    if (offset > size_) {
      return false;
    }
    return memcmp(data_ + offset, sv.data(), sv.size()) == 0;
  }
  details::Types identify();

private:
  HANDLE FileHandle{INVALID_HANDLE_VALUE};
  HANDLE FileMapHandle{INVALID_HANDLE_VALUE};
  const char *data_{nullptr};
  std::size_t size_{0};
  std::wstring extension;
  bool maped{false};
};

} // namespace probe

#endif