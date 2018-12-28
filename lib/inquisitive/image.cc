///////////////
#include "inquisitive.hpp"
#include "includes.hpp"
#include "details.hpp"

namespace inquisitive {

// struct psd_header_t {
//   uint8_t sig[4];
//   uint16_t ver;
//   uint8_t reserved[6];
//   uint16_t alpha;
//   uint32_t height;
//   uint32_t width;
//   uint16_t depth;
//   uint16_t cm;
// };
// https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_19840

details::Types identify_image(memview mv) {
  constexpr const byte_t icoMagic[] = {0x00, 0x00, 0x01, 0x00};
  constexpr const byte_t jpegMagic[] = {0xFF, 0xD8, 0xFF};
  constexpr const byte_t jpeg2000Magic[] = {0x0,  0x0, 0x0, 0xC,  0x6A, 0x50,
                                            0x20, 0xD, 0xA, 0x87, 0xA,  0x0};
  constexpr const byte_t pngMagic[] = {0x89, 0x50, 0x4E, 0x47};
  constexpr const byte_t gifMagic[] = {0x47, 0x49, 0x46};
  constexpr const byte_t webpMagic[] = {0x57, 0x45, 0x42, 0x50};
  constexpr const byte_t psdMagic[] = {0x38, 0x42, 0x50, 0x53};
  constexpr const size_t psdhlen = 4 + 2 + 6 + 2 + 4 + 4 + 2 + 2;
  switch (mv[0]) {
  case 0x0:
    if (mv.startswith(icoMagic)) {
      return details::ico;
    }
    if (mv.startswith(jpeg2000Magic)) {
      return details::jp2;
    }
    break;
  case 0x38:
    if (mv.startswith(psdMagic) && mv.size() > psdhlen) {
      // Version: always equal to 1.
      auto ver = readbe<uint16_t>((void *)(mv.data() + 4));
      if (ver == 1) {
        return details::psd;
      }
    }
    break;
  case 0x42:
    if (mv.size() > 2 && mv[1] == 0x4D) {
      return details::bmp;
    }
    break;
  case 0x47:
    if (mv.startswith(gifMagic)) {
      constexpr size_t gmlen = sizeof(gifMagic);
      if (mv.size() > gmlen + 3 && mv[gmlen] == '8' &&
          (mv[gmlen + 1] == '7' || mv[gmlen + 1] == '9') &&
          mv[gmlen + 2] == 'a') {
        return details::gif;
      }
    }
    break;
  case 0x49:
    if (mv.size() > 9 && mv[1] == 0x49 && mv[2] == 0x2A && mv[3] == 0x0 &&
        mv[8] == 0x43 && mv[9] == 0x52) {
      return details::cr2;
    }
    if (mv.size() > 3 && mv[1] == 0x49 && mv[2] == 0x2A && mv[3] == 0x0) {
      return details::tif;
    }
    if (mv.size() > 2 && mv[1] == 0x49 && mv[2] == 0xBC) {
      return details::jxr;
    }
    break;
  case 0x4D:
    if (mv.size() > 9 && mv[0] == 0x4D && mv[1] == 0x4D && mv[2] == 0x0 &&
        mv[3] == 0x2A && mv[8] == 0x43 && mv[9] == 0x52) {
      return details::cr2;
    }
    if (mv.size() > 3 && mv[1] == 0x4D && mv[2] == 0x0 && mv[3] == 0x2A) {
      return details::tif;
    }
    break;
  case 0x57:
    if (mv.startswith(webpMagic)) {
      return details::webp;
    }
    break;
  case 0x89:
    if (mv.startswith(pngMagic)) {
      return details::png;
    }
    break;
  case 0xFF:
    if (mv.startswith(jpegMagic)) {
      return details::jpg;
    }
    break;
  default:
    break;
  }
  return details::none;
}
} // namespace inquisitive
