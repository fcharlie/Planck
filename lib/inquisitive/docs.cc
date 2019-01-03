//////////////
#include "details.hpp"
#include "inquisitive.hpp"

namespace inquisitive {
//
constexpr byte_t msdocMagic[] = {0xD0, 0xCF, 0x11, 0xE0,
                                 0xA1, 0xB1, 0x1A, 0xE1};
// http://www.openoffice.org/sc/compdocfileformat.pdf
// https://interoperability.blob.core.windows.net/files/MS-PPT/[MS-PPT].pdf

details::Types identify_docs(memview mv) {
  //
  return details::none;
}
} // namespace inquisitive
