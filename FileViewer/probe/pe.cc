/// PE details

// PE32+ executable (console) x86-64, for MS Windows
// PE32 executable (DLL) (console) Intel 80386 Mono/.Net assembly, for MS
// Windows PE32 executable (console) Intel 80386, for MS Windows file command
// not support check arm and arm64

#include <system_error>
#include <string_view>
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include "probe_fwd.hpp"

#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif

#ifndef PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64
#define PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64 13
#endif
#ifndef PROCESSOR_ARCHITECTURE_IA32_ON_ARM64
#define PROCESSOR_ARCHITECTURE_IA32_ON_ARM64 14
#endif

#ifndef IMAGE_FILE_MACHINE_TARGET_HOST
#define IMAGE_FILE_MACHINE_TARGET_HOST                                         \
  0x0001 // Useful for indicating we want to interact with the host and not a
         // WoW guest.
#endif

/// #define PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64   13
#ifndef IMAGE_FILE_MACHINE_ARM64
//// IMAGE_FILE_MACHINE_ARM64 is Windows
#define IMAGE_FILE_MACHINE_ARM64 0xAA64 // ARM64 Little-Endian
#endif

#ifndef IMAGE_FILE_MACHINE_CHPE_X86
#define IMAGE_FILE_MACHINE_CHPE_X86 0x3A64 /// defined in ntimage.h
#endif

#ifndef IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG
#define IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG 17 // XBOX Code Catalog
#endif

namespace probe {

struct key_value_t {
  uint32_t index;
  const wchar_t *value;
};

std::wstring Machine(uint32_t index) {
  // https://wbenny.github.io/2018/11/04/wow64-internals.html
  const key_value_t machines[] = {
      {IMAGE_FILE_MACHINE_UNKNOWN, L"UNKNOWN"},
      {IMAGE_FILE_MACHINE_TARGET_HOST, L"WoW Gest"},
      {IMAGE_FILE_MACHINE_I386, L"Intel 386"},
      {IMAGE_FILE_MACHINE_R3000, L"MIPS little-endian, 0x160 big-endian"},
      {IMAGE_FILE_MACHINE_R4000, L"MIPS little-endian"},
      {IMAGE_FILE_MACHINE_R10000, L"MIPS little-endian"},
      {IMAGE_FILE_MACHINE_WCEMIPSV2, L"MIPS little-endian WCE v2"},
      {IMAGE_FILE_MACHINE_ALPHA, L"Alpha_AXP"},
      {IMAGE_FILE_MACHINE_SH3, L"SH3 little-endian"},
      {IMAGE_FILE_MACHINE_SH3DSP, L"SH3 DSP"},
      {IMAGE_FILE_MACHINE_SH3E, L"SH3E little-endian"},
      {IMAGE_FILE_MACHINE_SH4, L"SH4 little-endian"},
      {IMAGE_FILE_MACHINE_SH5, L"SH5"},
      {IMAGE_FILE_MACHINE_ARM, L"ARM Little-Endian"},
      {IMAGE_FILE_MACHINE_THUMB, L"ARM Thumb/Thumb-2 Little-Endian"},
      {IMAGE_FILE_MACHINE_ARMNT, L"ARM Thumb-2 Little-Endian"},
      {IMAGE_FILE_MACHINE_AM33, L"TAM33BD"},
      {IMAGE_FILE_MACHINE_POWERPC, L"IBM PowerPC Little-Endian"},
      {IMAGE_FILE_MACHINE_POWERPCFP, L"IBM PowerPC  (FP support)"},
      {IMAGE_FILE_MACHINE_IA64, L"Intel Itanium"},
      {IMAGE_FILE_MACHINE_MIPS16, L"MIPS"},
      {IMAGE_FILE_MACHINE_ALPHA64, L"ALPHA64"},
      {IMAGE_FILE_MACHINE_MIPSFPU, L"MIPS"},
      {IMAGE_FILE_MACHINE_MIPSFPU16, L"MIPS"},
      {IMAGE_FILE_MACHINE_TRICORE, L"Infineon"},
      {IMAGE_FILE_MACHINE_CEF, L"IMAGE_FILE_MACHINE_CEF"},
      {IMAGE_FILE_MACHINE_EBC, L"EFI Byte Code"},
      {IMAGE_FILE_MACHINE_AMD64, L"AMD64 (K8)"},
      {IMAGE_FILE_MACHINE_M32R, L"M32R little-endian"},
      {IMAGE_FILE_MACHINE_ARM64, L"ARM64 Little-Endian"},
      {IMAGE_FILE_MACHINE_CEE, L"IMAGE_FILE_MACHINE_CEE"},
      {IMAGE_FILE_MACHINE_CHPE_X86, L"Hybrid PE"}
      //
  };
  for (const auto &kv : machines) {
    if (kv.index == index) {
      return kv.value;
    }
  }
  return L"UNKNOWN";
}

std::wstring Characteristics(uint32_t index) {
  const key_value_t cs[] = {
      {IMAGE_FILE_RELOCS_STRIPPED, L"Relocation info stripped"},
      // Relocation info stripped from file.
      {IMAGE_FILE_EXECUTABLE_IMAGE, L"Executable File"},
      // File is executable  (i.e. no unresolved external references).
      {IMAGE_FILE_LINE_NUMS_STRIPPED, L"PE line numbers stripped"},
      // Line nunbers stripped from file.
      {IMAGE_FILE_LOCAL_SYMS_STRIPPED, L"Symbol stripped"},
      // Local symbols stripped from file.
      {IMAGE_FILE_AGGRESIVE_WS_TRIM, L"Aggressively trim the working set"},
      // Aggressively trim working set
      {IMAGE_FILE_LARGE_ADDRESS_AWARE,
       L"The application can handle addresses larger than 2 GB."},
      // App can handle >2gb addresses
      {IMAGE_FILE_BYTES_REVERSED_LO, L"obsolete"},
      // Bytes of machine word are reversed.
      {IMAGE_FILE_32BIT_MACHINE, L"Support 32-bit words"},
      // 32 bit word machine.
      {IMAGE_FILE_DEBUG_STRIPPED, L"Debug info stripped"},
      // Debugging info stripped from file in .DBG file
      {IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP, L"Image In Removeable disk"},
      // If Image is on removable media, copy and run from the swap file.
      {IMAGE_FILE_NET_RUN_FROM_SWAP, L"Image In Net"},
      // If Image is on Net, copy and run from the swap file.
      {IMAGE_FILE_SYSTEM, L"System File"},
      // System File.
      {IMAGE_FILE_DLL, L"Dynamic Link Library"},
      // File is a DLL.
      {IMAGE_FILE_UP_SYSTEM_ONLY,
       L"The file should be run only on a uniprocessor computer."},
      // File should only be run on a UP machine
      {IMAGE_FILE_BYTES_REVERSED_HI, L"obsolete"}
      // Bytes of machine word are reversed.
  };
  for (const auto &kv : cs) {
    if (kv.index == index) {
      return kv.value;
    }
  }
  return L"UNKNOWN";
}

std::wstring Subsystem(uint32_t index) {
  const key_value_t subs[] = {
      {IMAGE_SUBSYSTEM_UNKNOWN, L"UNKNOWN"},
      {IMAGE_SUBSYSTEM_NATIVE, L"Native"}, // not require subsystem
      {IMAGE_SUBSYSTEM_WINDOWS_GUI, L"Windows GUI"},
      {IMAGE_SUBSYSTEM_WINDOWS_CUI, L"Windows CUI"},
      {IMAGE_SUBSYSTEM_OS2_CUI, L"OS/2  CUI"},
      {IMAGE_SUBSYSTEM_POSIX_CUI, L"Posix character subsystem"},
      {IMAGE_SUBSYSTEM_NATIVE_WINDOWS, L"Native Win9x driver"},
      {IMAGE_SUBSYSTEM_WINDOWS_CE_GUI, L"Windows CE subsystem"},
      {IMAGE_SUBSYSTEM_EFI_APPLICATION, L"EFI Application"},
      {IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER, L"EFI Boot Service Driver"},
      {IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER, L"EFI Runtime Driver"},
      {IMAGE_SUBSYSTEM_EFI_ROM, L"EFI ROM"},
      {IMAGE_SUBSYSTEM_XBOX, L"Xbox system"},
      {IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION, L"Windows Boot Application"},
      {IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG, L"XBOX Code Catalog"}
      //
  };
  for (const auto &kv : subs) {
    if (kv.index == index) {
      return kv.value;
    }
  }
  return L"UNKNOWN";
}



std::optional<pe_minutiae_t> PortableExecutableDump(std::wstring_view sv,
                                                    std::error_code &ec) {
  //
  return std::nullopt;
}

} // namespace probe