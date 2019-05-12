//////
#include "fmt_fwd.hpp"

namespace base {
namespace format_internal {
class buffer {
public:
  buffer(wchar_t *data_, size_t cap_) : data(data_), cap(cap_) {}
  buffer(const buffer &) = delete;
  ~buffer() {
    if (len < cap) {
      data[len] = 0;
    }
  }
  void push_back(wchar_t ch) {
    if (len < cap) {
      data[len++] = ch;
    } else {
      overflow = true;
    }
  }
  buffer &append(std::wstring_view s) {
    if (len + s.size() < cap) {
      memcpy(data + len, s.data(), s.size() * 2);
      len += s.size();
    } else {
      overflow = true;
    }
    return *this;
  }
  bool IsOverflow() const { return overflow; }
  size_t length() const { return len; }

private:
  wchar_t *data{nullptr};
  size_t len{0};
  size_t cap{0};
  bool overflow{false};
};

template <typename T> class Writer {
public:
  Writer(T &t_) : t(t) {}
  Writer(const Writer &) = delete;
  Writer &operator=(const Writer &) = delete;
  void Append(std::wstring_view sv, size_t pad) {
    constexpr std::wstring_view padzero =
        L"000000000000000000000000000000000000000000000000";
    if (sv.size() < pad) {
      t.append(padzero.substr(0, pad - sv.size()));
    }
    t.append(sv);
  }
  void Out(wchar_t ch) { t.push_back(ch); }
  bool Pad(wchar_t pad, size_t padding, size_t len);
  bool Integer(bool sign, bool upcase, int64_t i, int base, wchar_t pad,
               size_t padding, const wchar_t *prefix); // 0x00000F
  bool Float(bool sign, bool upcase, long double ld, int base, wchar_t pad,
             size_t padding, const wchar_t *prefix); /// 0.4f
  bool Overflow();

private:
  T &t;
};

template <> inline bool Writer<std::wstring>::Overflow() { return false; }
template <> inline bool Writer<buffer>::Overflow() { return t.IsOverflow(); }

using StringWriter = Writer<std::wstring>;
using BufferWriter = Writer<buffer>;

template <typename T>
bool StrFormatInternal(Writer<T> &w, std::wstring_view fmt,
                       const FormatArg *args, size_t max_args) {
  do {
    auto pos = fmt.find(L'%');
    if (pos == std::wstring_view::npos) {
      w.Append(fmt);
      return !w.Overflow();
    }
    w.Append(fmt.substr(0, pos));
    fmt.remove_prefix(pos + 1);
    /// --parse format

  } while (!fmt.empty());

  return true;
}

std::wstring StrFormatInternal(std::wstring_view fmt, const FormatArg *args,
                               size_t max_args) {
  std::wstring s;
  StringWriter sw(s);
  StrFormatInternal(sw, fmt, args, max_args);
  return s;
}

ssize_t StrFormatInternal(wchar_t *buf, size_t buflen, std::wstring_view fmt,
                          const FormatArg *args, size_t max_args) {
  buffer buffer_(buf, buflen);
  BufferWriter bw(buffer_);
  StrFormatInternal(bw, fmt, args, max_args);
  return static_cast<ssize_t>(buffer_.length());
}
} // namespace format_internal

ssize_t StrFormat(wchar_t *buf, size_t N, const wchar_t *fmt) {
  format_internal::buffer buffer_(buf, N);
  std::wstring s;
  const wchar_t *src = fmt;
  for (; *src; ++src) {
    buffer_.push_back(*src);
    if (src[0] == '%' && src[1] == '%') {
      ++src;
    }
  }
  return buffer_.IsOverflow() ? -1 : static_cast<ssize_t>(buffer_.length());
}

std::wstring StrFormat(const wchar_t *fmt) {
  std::wstring s;
  const wchar_t *src = fmt;
  for (; *src; ++src) {
    s.push_back(*src);
    if (src[0] == '%' && src[1] == '%') {
      ++src;
    }
  }
  return s;
}

} // namespace base