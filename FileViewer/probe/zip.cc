//// ZIP family
// zip
// docx pptx xlsx...
// appx....
//#include "zlib.h"
#include <string_view>
#include "details.hpp"

namespace probe {
///
details::Types identify_zip_family(std::string_view mv) {
  // PK
  return details::none;
}
} // namespace probe