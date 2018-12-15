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

// Memview
bool PEImageLookup() {
  //
  return false;
}