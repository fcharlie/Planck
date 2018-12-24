//////////
#ifndef MEMVIEW_HPP
#define MEMVIEW_HPP

#include <cstdint>
#include <cstring>
#include <climits>
#include <algorithm>
#include <iterator>
#include <limits>

namespace inquisitive {
class memview {
public:
  static constexpr std::size_t npos = SIZE_MAX;
  memview() = default;
  memview(const char *d, std::size_t l) : data_(d), size_(l) {}
  memview(const memview &other) {
    data_ = other.data_;
    size_ = other.size_;
  }
  bool startswith(memview mv) {
    if (mv.size_ > size_) {
      return false;
    }
    return (memcmp(data_, mv.data_, mv.size_) == 0);
  }
  bool indexswith(std::size_t offset, std::string_view sv) const {
    if (offset > size_) {
      return false;
    }
    return memcmp(data_ + offset, sv.data(), sv.size()) == 0;
  }
  memview submv(std::size_t pos, std::size_t n = npos) {
    return memview(data_ + pos, (std::min)(n, size_ - pos));
  }
  std::size_t size() const { return size_; }
  const char *data() const { return data_; }
  unsigned char operator[](const std::size_t off) const {
    if (off >= size_) {
      return UCHAR_MAX;
    }
    return (unsigned char)data_[off];
  }
  template <typename T> const T *cast(size_t off) {
    if (off + sizeof(T) >= size_) {
      return nullptr;
    }
    return reinterpret_cast<const T *>(data_ + off);
  }

private:
  const char *data_{nullptr};
  std::size_t size_{0};
};
} // namespace inquisitive

#endif
