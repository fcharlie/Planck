//////////
#ifndef MEMVIEW_HPP
#define MEMVIEW_HPP

#include <cstdint>
#include <algorithm>
#include <iterator>
#include <limits>

namespace probe {
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
    //
    return false;
  }
  memview submv(std::size_t pos, std::size_t n = npos) {
    return memview(data_ + pos, (std::min)(n, size_ - pos));
  }

private:
  const char *data_{nullptr};
  std::size_t size_{0};
};
} // namespace probe

#endif