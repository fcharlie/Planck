/// PE details

// PE32+ executable (console) x86-64, for MS Windows
// PE32 executable (DLL) (console) Intel 80386 Mono/.Net assembly, for MS
// Windows PE32 executable (console) Intel 80386, for MS Windows file command
// not support check arm and arm64

/*
https://github.com/chromium/chromium/blob/master/base/win/pe_image.cc
bool PEImage::ImageRVAToOnDiskOffset(uintptr_t rva,
                                     DWORD* on_disk_offset) const {
  LPVOID address = RVAToAddr(rva);
  return ImageAddrToOnDiskOffset(address, on_disk_offset);
}

bool PEImage::ImageAddrToOnDiskOffset(LPVOID address,
                                      DWORD* on_disk_offset) const {
  if (NULL == address)
    return false;

  // Get the section that this address belongs to.
  PIMAGE_SECTION_HEADER section_header = GetImageSectionFromAddr(address);
  if (NULL == section_header)
    return false;

  // Don't follow the virtual RVAToAddr, use the one on the base.
  DWORD offset_within_section =
      static_cast<DWORD>(reinterpret_cast<uintptr_t>(address)) -
      static_cast<DWORD>(reinterpret_cast<uintptr_t>(
          PEImage::RVAToAddr(section_header->VirtualAddress)));

  *on_disk_offset = section_header->PointerToRawData + offset_within_section;
  return true;
}

PVOID PEImage::RVAToAddr(uintptr_t rva) const {
  if (rva == 0)
    return NULL;

  return reinterpret_cast<char*>(module_) + rva;
}
*/

#include "memview.hpp"

// Memview
namespace probe {
/*
https://github.com/llvm-mirror/llvm/blob/master/lib/Object/COFFObjectFile.cpp#L451
uint64_t COFFObjectFile::getImageBase() const {
  if (PE32Header)
    return PE32Header->ImageBase;
  else if (PE32PlusHeader)
    return PE32PlusHeader->ImageBase;
  // This actually comes up in practice.
  return 0;
}
// Returns the file offset for the given VA.
std::error_code COFFObjectFile::getVaPtr(uint64_t Addr, uintptr_t &Res) const {
  uint64_t ImageBase = getImageBase();
  uint64_t Rva = Addr - ImageBase;
  assert(Rva <= UINT32_MAX);
  return getRvaPtr((uint32_t)Rva, Res);
}

// Returns the file offset for the given RVA.
std::error_code COFFObjectFile::getRvaPtr(uint32_t Addr, uintptr_t &Res) const {
  for (const SectionRef &S : sections()) {
    const coff_section *Section = getCOFFSection(S);
    uint32_t SectionStart = Section->VirtualAddress;
    uint32_t SectionEnd = Section->VirtualAddress + Section->VirtualSize;
    if (SectionStart <= Addr && Addr < SectionEnd) {
      uint32_t Offset = Addr - SectionStart;
      Res = uintptr_t(base()) + Section->PointerToRawData + Offset;
      return std::error_code();
    }
  }
  return object_error::parse_failed;
}
*/

bool peimagelookup(memview mv) {
  //auto h=mv.cast<llvm::COFF::header>(0);
  return false;
}
} // namespace probe