//// archive format
// BZ 7z Rar!
#include <string_view>
#include <optional>
#include <endian.hpp>
#include "inquisitive.hpp"

namespace inquisitive {
// 7z details:
// https://github.com/mcmilk/7-Zip-zstd/blob/master/CPP/7zip/Archive/7z/7zHeader.h
constexpr const unsigned k7zSignatureSize = 6;
#pragma pack(1)
struct p7z_header_t {
  byte_t signature[k7zSignatureSize];
  byte_t major; /// 7z major version default is 0
  byte_t minor;
};
#pragma pack()

status_t inquisitive_7zinternal(memview mv, inquisitive_result_t &ir) {
  constexpr const byte_t k7zSignature[k7zSignatureSize] = {'7',  'z',  0xBC,
                                                           0xAF, 0x27, 0x1C};
  constexpr const byte_t k7zFinishSignature[k7zSignatureSize] = {
      '7', 'z', 0xBC, 0xAF, 0x27, 0x1C + 1};
  auto hd = mv.cast<p7z_header_t>(0);
  if (hd == nullptr) {
    return None;
  }
  if (memcmp(hd->signature, k7zSignature, k7zSignatureSize) != 0) {
    return None;
  }
  wchar_t buf[64];
  _snwprintf_s(buf, 64, L"7-zip archive data, version %d.%d", (int)hd->major,
               (int)hd->minor);
  ir.Assign(buf, types::p7z);
  return Found;
}

// RAR archive
// https://www.rarlab.com/technote.htm
status_t inquisitive_rarinternal(memview mv, inquisitive_result_t &ir) {
  /*RAR 5.0 signature consists of 8 bytes: 0x52 0x61 0x72 0x21 0x1A 0x07 0x01
   * 0x00. You need to search for this signature in supposed archive from
   * beginning and up to maximum SFX module size. Just for comparison this is
   * RAR 4.x 7 byte length signature: 0x52 0x61 0x72 0x21 0x1A 0x07 0x00.*/
  constexpr const byte_t rarSignature[] = {0x52, 0x61, 0x72, 0x21,
                                           0x1A, 0x07, 0x01, 0x00};
  constexpr const byte_t rar4Signature[] = {0x52, 0x61, 0x72, 0x21,
                                            0x1A, 0x07, 0x00};
  if (mv.startswith(rarSignature)) {
    ir.Assign(L"Roshal Archive (rar), version 5", types::rar);
    return Found;
  }
  if (mv.startswith(rar4Signature)) {
    ir.Assign(L"Roshal Archive (rar), version 4", types::rar);
    return Found;
  }
  return None;
}

///
#pragma pack(1)
struct xar_header {
  uint32_t magic; // xar!
  uint16_t size;  // BE
  uint16_t version;
  uint64_t toc_length_compressed;
  uint64_t toc_length_uncompressed;
  uint32_t cksum_alg;
  /* A nul-terminated, zero-padded to multiple of 4, message digest name
   * appears here if cksum_alg is 3 which must not be empty ("") or "none".
   */
};
#pragma pack()

status_t inquisitive_xarinternal(memview mv, inquisitive_result_t &ir) {
  // https://github.com/mackyle/xar/wiki/xarformat
  constexpr const byte_t xarSignature[] = {'x', 'a', 'r', '!'};
  if (!mv.startswith(xarSignature)) {
    return None;
  }
  auto xhd = mv.cast<xar_header>(0);
  if (xhd == nullptr || planck::resolvebe(xhd->size) < 28) {
    return None;
  }
  auto ver = planck::resolvebe(xhd->version);
  wchar_t buf[64];
  _snwprintf_s(buf, 64, L"eXtensible ARchive format, version %d", (int)ver);
  ir.Assign(buf, types::xar);
  return Found;
}

// OS X .dmg
// https://en.wikipedia.org/wiki/Apple_Disk_Image
#pragma pack(1)
struct uuid_t {
  uint8_t uuid[16];
};
struct apple_disk_image_header {
  uint8_t Signature[4];
  uint32_t Version;
  uint32_t HeaderSize;
  uint32_t Flags;
  uint64_t RunningDataForkOffset;
  uint64_t DataForkOffset;
  uint64_t DataForkLength;
  uint64_t RsrcForkOffset;
  uint64_t RsrcForkLength;
  uint32_t SegmentNumber;
  uint32_t SegmentCount;
  uuid_t SegmentID;
  uint32_t DataChecksumType;
  uint32_t DataChecksumSize;
  uint32_t DataChecksum[32];
  uint64_t XMLOffset;
  uint64_t XMLLength;
  uint8_t Reserved1[120];
  uint32_t ChecksumType;
  uint32_t ChecksumSize;
  uint32_t Checksum[32];
  uint32_t ImageVariant;
  uint64_t SectorCount;
  uint32_t reserved2;
  uint32_t reserved3;
  uint32_t reserved4;
};
#pragma pack()

status_t inquisitive_dmginternal(memview mv, inquisitive_result_t &ir) {
  constexpr const byte_t dmgSignature[] = {'k', 'o', 'l', 'y'};
  if (!mv.startswith(dmgSignature)) {
    return None;
  }
  auto hd = mv.cast<apple_disk_image_header>(0);
  constexpr auto hsize = sizeof(apple_disk_image_header);
  if (hd == nullptr || planck::resolvebe(hd->HeaderSize) != hsize) {
    return None;
  }
  auto ver = planck::resolvebe(hd->Version);
  wchar_t buf[64];
  _snwprintf_s(buf, 64, L"Apple Disk Image, version %d", (int)ver);
  ir.Assign(buf, types::dmg);
  return Found;
}

// PDF file format

status_t inquisitive_pdfinternal(memview mv, inquisitive_result_t &ir) {
  // https://www.adobe.com/content/dam/acom/en/devnet/acrobat/pdfs/pdf_reference_1-7.pdf
  // %PDF-1.7
  constexpr const byte_t pdfMagic[] = {0x25, 0x50, 0x44, 0x46, '-'};
  if (!mv.startswith(pdfMagic) || mv.size() < 8) {
    return None;
  }
  auto ch = reinterpret_cast<const char *>(memchr(mv.data(), '\n', mv.size()));
  if (ch == nullptr) {
    // Forged PDF file
    return None;
  }

  std::wstring buf(L"Portable Document Format (PDF), version ");
  for (auto it = mv.data() + 5; it < ch; it++) {
    buf.push_back(*it); /// PDF Use ASCII string
  }
  ir.Assign(buf, types::pdf);
  return None;
}

// https://github.com/h2non/filetype/blob/master/matchers/archive.go
constexpr const byte_t swfMagic1[] = {0x43, 0x57, 0x53};
constexpr const byte_t swfMagic2[] = {0x46, 0x57, 0x53};
constexpr const byte_t rtfMagic[] = {0x7B, 0x5C, 0x72, 0x74, 0x66};
constexpr const byte_t msiMagic[] = {0x53, 0x5A, 0x44, 0x44, 0x88,
                                     0xF0, 0x27, 0x33, 0x41};
// constexpr const byte_t vmdMagic[] = {'K', 'D', 'M', 'V'};
constexpr const byte_t rpmMagic[] = {0xED, 0xAB, 0xEE, 0xDB}; // size>96
constexpr const byte_t comMagic[] = {0xD0, 0xCF, 0x11, 0xE0,
                                     0xA1, 0xB1, 0x1A, 0xE1};
constexpr const byte_t cabMagic[] = {'M', 'S', 'C', 'F', 0, 0, 0, 0};

// EPUB file
inline bool epub(const byte_t *buf, size_t size) {
  return size > 57 && buf[0] == 0x50 && buf[1] == 0x4B && buf[2] == 0x3 &&
         buf[3] == 0x4 && buf[30] == 0x6D && buf[31] == 0x69 &&
         buf[32] == 0x6D && buf[33] == 0x65 && buf[34] == 0x74 &&
         buf[35] == 0x79 && buf[36] == 0x70 && buf[37] == 0x65 &&
         buf[38] == 0x61 && buf[39] == 0x70 && buf[40] == 0x70 &&
         buf[41] == 0x6C && buf[42] == 0x69 && buf[43] == 0x63 &&
         buf[44] == 0x61 && buf[45] == 0x74 && buf[46] == 0x69 &&
         buf[47] == 0x6F && buf[48] == 0x6E && buf[49] == 0x2F &&
         buf[50] == 0x65 && buf[51] == 0x70 && buf[52] == 0x75 &&
         buf[53] == 0x62 && buf[54] == 0x2B && buf[55] == 0x7A &&
         buf[56] == 0x69 && buf[57] == 0x70;
}

status_t inquisitive_archives(memview mv, inquisitive_result_t &ir) {
  if (inquisitive_7zinternal(mv, ir) == Found) {
    return Found;
  }
  if (inquisitive_rarinternal(mv, ir) == Found) {
    return Found;
  }
  if (inquisitive_xarinternal(mv, ir) == Found) {
    return Found;
  }
  if (inquisitive_dmginternal(mv, ir) == Found) {
    return Found;
  }
  if (inquisitive_pdfinternal(mv, ir) == Found) {
    return Found;
  }

  if (mv.startswith(rpmMagic)) {
    ir.Assign(L"RPM Package Manager", types::rpm);
    return Found;
  }

  if (mv.startswith(swfMagic1) || mv.startswith(swfMagic2)) {
    ir.Assign(L"Adobe Flash file format", types::swf);
    return Found;
  }
  if (mv.startswith(rtfMagic)) {
    ir.Assign(L"Rich Text Format data", types::rtf);
    return Found;
  }
  if (mv.startswith(msiMagic)) {
    ir.Assign(L"Windows Installer packages", types::msi);
    return Found;
  }
  if (epub((const byte_t *)mv.data(), mv.size())) {
    ir.Assign(L"EPUB document", types::epub);
    return Found;
  }
  return None;
}
// https://rarlab.com/technote.htm rar
} // namespace inquisitive
