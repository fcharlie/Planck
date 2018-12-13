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

namespace probe {
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
  bool startswith(const char *prefix, int64_t pl) {
    if (pl >= size_) {
      return false;
    }
    return memcmp(data_, prefix, pl) == 0;
  }

private:
  HANDLE FileHandle{INVALID_HANDLE_VALUE};
  HANDLE FileMapHandle{INVALID_HANDLE_VALUE};
  const char *data_{nullptr};
  std::int64_t size_{0};
  bool maped{false};
};

} // namespace probe

#endif