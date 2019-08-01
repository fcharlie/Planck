///////////////
#include "inquisitive.hpp"

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

status_t inquisitive_images(base::MemView mv, inquisitive_result_t &ir) {
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
    if (mv.StartsWith(icoMagic)) {
      ir.assign(L"ICO file format (.ico)", types::ico);
      return Found;
    }
    if (mv.StartsWith(jpeg2000Magic)) {
      ir.assign(L"JPEG 2000 Image", types::jp2);
      return Found;
    }
    break;
  case 0x38:
    if (mv.StartsWith(psdMagic) && mv.size() > psdhlen) {
      // Version: always equal to 1.
      auto ver = bela::readbe<uint16_t>((void *)(mv.data() + 4));
      if (ver == 1) {
        ir.assign(L"Photoshop document file extension", types::psd);
        return Found;
      }
    }
    break;
  case 0x42:
    if (mv.size() > 2 && mv[1] == 0x4D) {
      ir.assign(L"Bitmap image file format (.bmp)", types::bmp);
      return Found;
    }
    break;
  case 0x47:
    if (mv.StartsWith(gifMagic)) {
      constexpr size_t gmlen = sizeof(gifMagic);
      if (mv.size() > gmlen + 3 && mv[gmlen] == '8' &&
          (mv[gmlen + 1] == '7' || mv[gmlen + 1] == '9') &&
          mv[gmlen + 2] == 'a') {
        ir.assign(L"Graphics Interchange Format (.gif)", types::gif);
        return Found;
      }
    }
    break;
  case 0x49:
    if (mv.size() > 9 && mv[1] == 0x49 && mv[2] == 0x2A && mv[3] == 0x0 &&
        mv[8] == 0x43 && mv[9] == 0x52) {
      ir.assign(L"Canon 5D Mark IV CR2", types::cr2);
      return Found;
    }
    if (mv.size() > 3 && mv[1] == 0x49 && mv[2] == 0x2A && mv[3] == 0x0) {
      ir.assign(L"Tagged Image File Format (.tif)", types::tif);
      return Found;
    }
    if (mv.size() > 2 && mv[1] == 0x49 && mv[2] == 0xBC) {
      ir.assign(L"JPEG extended range", types::jxr);
      return Found;
    }
    break;
  case 0x4D:
    if (mv.size() > 9 && mv[0] == 0x4D && mv[1] == 0x4D && mv[2] == 0x0 &&
        mv[3] == 0x2A && mv[8] == 0x43 && mv[9] == 0x52) {
      ir.assign(L"Canon 5D Mark IV CR2", types::cr2);
      return Found;
    }
    if (mv.size() > 3 && mv[1] == 0x4D && mv[2] == 0x0 && mv[3] == 0x2A) {
      ir.assign(L"Tagged Image File Format (.tif)", types::tif);
      return Found;
    }
    break;
  case 0x57:
    if (mv.StartsWith(webpMagic)) {
      ir.assign(L"WebP Image", types::webp);
      return Found;
    }
    break;
  case 0x89:
    if (mv.StartsWith(pngMagic)) {
      ir.assign(L"Portable Network Graphics (.png)", types::png);
      return Found;
    }
    break;
  case 0xFF:
    if (mv.StartsWith(jpegMagic)) {
      ir.assign(L"JPEG Image", types::jpg);
      return Found;
    }
    break;
  default:
    break;
  }
  return None;
}

} // namespace inquisitive
