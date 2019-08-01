/// common
#include "inquisitive.hpp"

namespace inquisitive {

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
                                                bela::error_code &ec) {
  auto extension = FindExtension(sv);
  bela::MapView mmv;
  if (!mmv.MappingView(sv, ec, 2, 32 * 1024)) {
    ec = bela::make_system_error_code();
    return std::nullopt;
  }
  auto mv = mmv.subview();
  inquisitive_result_t ir;
  const inquisitive_handle_t handles[] = {
      // handles
      inquisitive_binobj, inquisitive_fonts,     inquisitive_zip_family,
      inquisitive_docs,   inquisitive_images,    inquisitive_archives,
      inquisitive_media,  inquisitive_gitbinary, inquisitive_shlink,
      inquisitive_text,   inquisitive_chardet
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
