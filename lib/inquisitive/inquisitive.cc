/// common
#include "inquisitive.hpp"

namespace inquisitive {

class FileView {
public:
  FileView() = default;
  FileView(const FileView &) = delete;
  FileView &operator=(const FileView &) = delete;
  ~FileView() {
    if (data_ != nullptr) {
      HeapFree(GetProcessHeap(), 0, data_);
    }
  }
  bool initialize(std::wstring_view sv, std::size_t maxsize) {
    if (maxsize == 0) {
      SetLastError(ERROR_BAD_ARGUMENTS);
      return false;
    }
    data_ = reinterpret_cast<char *>(HeapAlloc(GetProcessHeap(), 0, maxsize));
    if (data_ == nullptr) {
      return false;
    }
    auto FileHandle =
        CreateFileW(sv.data(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (FileHandle == INVALID_HANDLE_VALUE) {
      return false;
    }
    LARGE_INTEGER li;
    if (GetFileSizeEx(FileHandle, &li) != TRUE || li.QuadPart == 0) {
      CloseHandle(FileHandle);
      return false;
    }
    DWORD dwSize = 0;
    if (ReadFile(FileHandle, data_, (DWORD)maxsize, &dwSize, nullptr) != TRUE) {
      CloseHandle(FileHandle);
      return false;
    }
    size_ = dwSize;
    CloseHandle(FileHandle);
    return true;
  }
  memview view(std::size_t pos = 0, std::size_t n = SIZE_MAX) {
    if (pos >= size_) {
      return memview(nullptr, 0);
    }
    auto begin = data_ + pos;
    auto sn = (std::min)(n, size_ - pos);
    return memview(data_ + pos, sn);
  }

private:
  char *data_{nullptr};
  size_t size_{0};
};

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

std::optional<inquisitive_result_t> inquisitive(std::wstring_view sv,
                                                base::error_code &ec) {
  auto extension = FindExtension(sv);
  FileView fv;
  if (!fv.initialize(sv, 32 * 1024)) {
    ec = base::make_system_error_code();
    return std::nullopt;
  }
  auto mv = fv.view();
  inquisitive_result_t ir;
  const inquisitive_handle_t handles[] = {
      // handles
      inquisitive_binobj,     inquisitive_fonts,
      inquisitive_zip_family, inquisitive_docs,
      inquisitive_images,     inquisitive_archives,
      inquisitive_media,      inquisitive_text,
      inquisitive_chardet
      //
  };
  for (auto h : handles) {
    if (h(mv, ir) == Found) {
      return std::make_optional<inquisitive_result_t>(std::move(ir));
    }
  }
  return std::nullopt;
}

} // namespace inquisitive
