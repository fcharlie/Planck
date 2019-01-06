////////// MIME type
#include "inquisitive.hpp"

namespace inquisitive {
struct mime_value_t {
  types::Type t;
  const wchar_t *mime;
};
const wchar_t *inquisitive_mime(types::Type t) {
  // const mime_value_t  mimes[]={};
  return L"application/octet-stream";
}
} // namespace inquisitive