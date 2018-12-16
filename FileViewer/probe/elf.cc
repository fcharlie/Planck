/// ELF details
#include "elf.h"
#include "memview.hpp"
#include "probe_fwd.hpp"

//  Executable and Linkable Format ELF
// Thanks musl libc
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://hg.icculus.org/icculus/fatelf/raw-file/tip/docs/fatelf-specification.txt

namespace probe {
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
  if (mv[SELFMAG] == ELFCLASS64) {
    return elfimagelookup32(mv, em);
  }
  auto h = mv.cast<Elf64_Ehdr>(0);
  if (h == nullptr) {
    return false;
  }
  return true;
}
} // namespace probe