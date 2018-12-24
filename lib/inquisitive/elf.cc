/// ELF details
#include <elf.h>
#include "inquisitive.hpp"

//  Executable and Linkable Format ELF
// Thanks musl libc
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://hg.icculus.org/icculus/fatelf/raw-file/tip/docs/fatelf-specification.txt

namespace inquisitive {
// Lookup rpath goto https://github.com/fcharlie/cmchrpath
struct StringEntry {
  std::string Value;
  size_t Position;
  size_t Size;
  int IndexInSection;
};

bool elfimagelookup32(memview mv, elf_minutiae_t &em) {
  auto h = mv.cast<Elf32_Ehdr>(0);
  if (h == nullptr) {
    return false;
  }
  return true;
}

bool elfimagelookup(memview mv, elf_minutiae_t &em) {
  if (mv.size() < sizeof(Elf32_Ehdr)) {
    // NOT elf
    return false;
  }
  memcmp(mv.data(), em.ident, einident);
  switch (mv[EI_DATA]) {
  case 0:
    em.endian = endina::None;
  case 1:
    em.endian = endina::LittleEndian;
  case 2:
    em.endian = endina::BigEndian;
  default:
    em.endian = endina::None;
    break;
  }
  if (mv[SELFMAG] != ELFCLASS64) {
    em.is64bit = false;
    return elfimagelookup32(mv, em);
  }
  em.is64bit = true;
  auto h = mv.cast<Elf64_Ehdr>(0);
  if (h == nullptr) {
    return false;
  }
  return true;
}
} // namespace inquisitive
