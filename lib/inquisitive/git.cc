//////// GIT pack index and other files.
#include <endian.hpp>
#include "inquisitive.hpp"

namespace inquisitive {
// todo resolve git index pack and midx files
#pragma pack(1)
struct git_pack_header_t {
  uint8_t signature[4]; /// P A C K
  uint32_t version;     // BE
  uint32_t objsize;     // BE
};

struct git_index_header_t {
  uint8_t siganture[4];
  uint32_t version;
  uint32_t fanout[256];
};

struct git_index3_header_t {
  uint8_t signature[4]; // 0xFF, 0x74, 0x4F, 0x63
  uint32_t version;
  uint32_t hslength;
  uint32_t packobjects;
  uint32_t objectformats; // 2
};

struct git_midx_header_t {
  uint8_t siganture[4]; // M I D X
  uint8_t version;
  uint8_t oidversion; // 1 SHA1
  uint8_t chunks;
  uint8_t basemidxsize; // 0
  uint32_t packfiles;   //
};
#pragma pack()
// https://github.com/git/git/blob/master/Documentation/technical/pack-format.txt
status_t inquisitive_gitbinary(memview mv, inquisitive_result_t &ir) {
  constexpr const byte_t packMagic[] = {'P', 'A', 'C', 'K'};
  constexpr const byte_t midxMagic[] = {'M', 'I', 'D', 'X'};
  constexpr const byte_t indexMagic[] = {0xFF, 0x74, 0x4F, 0x63};
  if (mv.startswith(packMagic)) {
    auto hd = mv.cast<git_pack_header_t>(0);
    if (hd == nullptr) {
      return None;
    }
    wchar_t buf[64];
    _snwprintf_s(
        buf, ArrayLength(buf), L"Git pack file, version %d, objects %d",
        planck::resolvebe(hd->version), planck::resolvebe(hd->objsize));
    ir.assign(buf, types::gitpack);
    return Found;
  }
  if (mv.startswith(indexMagic)) {
    auto hd = mv.cast<git_index_header_t>(0);
    if (hd == nullptr) {
      return None;
    }
    wchar_t buf[128];
    auto ver = planck::resolvebe(hd->version);
    switch (ver) {
    case 2:
      _snwprintf_s(buf, ArrayLength(buf),
                   L"Git pack indexs file, version %d, total objects %d", ver,
                   planck::resolvebe(hd->fanout[255]));
      break;
    case 3: {
      auto hd3 = mv.cast<git_index3_header_t>(0);
      _snwprintf_s(buf, ArrayLength(buf),
                   L"Git pack indexs file, version %d, total objects %d", ver,
                   planck::resolvebe(hd3->packobjects));
    } break;
    default:
      _snwprintf_s(buf, ArrayLength(buf), L"Git pack indexs file, version %d",
                   ver);
      break;
    };

    ir.assign(buf, types::gitpkindex);
    return Found;
  }
  if (mv.startswith(midxMagic)) {
    auto hd = mv.cast<git_midx_header_t>(0);
    if (hd == nullptr) {
      return None;
    }
    wchar_t buf[128];
    _snwprintf_s(buf, ArrayLength(buf),
                 L"Git multi-pack-index, version %d, oid version %d, chunks "
                 L"%d, pack files %d",
                 (int)hd->version, int(hd->oidversion), int(hd->chunks),
                 planck::resolvebe(hd->packfiles));
    ir.assign(buf, types::gitpack);
    return Found;
  }

  return None;
}

} // namespace inquisitive
