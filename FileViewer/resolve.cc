///
#include "resolve.hpp"
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include <pathcch.h>
#include <memory>
#include "console/console.hpp"

/*
#define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)
#define IO_REPARSE_TAG_HSM                      (0xC0000004L)
#define IO_REPARSE_TAG_HSM2                     (0x80000006L)
#define IO_REPARSE_TAG_SIS                      (0x80000007L)
#define IO_REPARSE_TAG_WIM                      (0x80000008L)
#define IO_REPARSE_TAG_CSV                      (0x80000009L)
#define IO_REPARSE_TAG_DFS                      (0x8000000AL)
#define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)
#define IO_REPARSE_TAG_DFSR                     (0x80000012L)
#define IO_REPARSE_TAG_DEDUP                    (0x80000013L)
#define IO_REPARSE_TAG_NFS                      (0x80000014L)
#define IO_REPARSE_TAG_FILE_PLACEHOLDER         (0x80000015L)
#define IO_REPARSE_TAG_WOF                      (0x80000017L)
#define IO_REPARSE_TAG_WCI                      (0x80000018L)
#define IO_REPARSE_TAG_WCI_1                    (0x90001018L)
#define IO_REPARSE_TAG_GLOBAL_REPARSE           (0xA0000019L)
#define IO_REPARSE_TAG_CLOUD                    (0x9000001AL)
#define IO_REPARSE_TAG_CLOUD_1                  (0x9000101AL)
#define IO_REPARSE_TAG_CLOUD_2                  (0x9000201AL)
#define IO_REPARSE_TAG_CLOUD_3                  (0x9000301AL)
#define IO_REPARSE_TAG_CLOUD_4                  (0x9000401AL)
#define IO_REPARSE_TAG_CLOUD_5                  (0x9000501AL)
#define IO_REPARSE_TAG_CLOUD_6                  (0x9000601AL)
#define IO_REPARSE_TAG_CLOUD_7                  (0x9000701AL)
#define IO_REPARSE_TAG_CLOUD_8                  (0x9000801AL)
#define IO_REPARSE_TAG_CLOUD_9                  (0x9000901AL)
#define IO_REPARSE_TAG_CLOUD_A                  (0x9000A01AL)
#define IO_REPARSE_TAG_CLOUD_B                  (0x9000B01AL)
#define IO_REPARSE_TAG_CLOUD_C                  (0x9000C01AL)
#define IO_REPARSE_TAG_CLOUD_D                  (0x9000D01AL)
#define IO_REPARSE_TAG_CLOUD_E                  (0x9000E01AL)
#define IO_REPARSE_TAG_CLOUD_F                  (0x9000F01AL)
#define IO_REPARSE_TAG_CLOUD_MASK               (0x0000F000L)
#define IO_REPARSE_TAG_APPEXECLINK              (0x8000001BL)
#define IO_REPARSE_TAG_PROJFS                   (0x9000001CL)
#define IO_REPARSE_TAG_STORAGE_SYNC             (0x8000001EL)
#define IO_REPARSE_TAG_WCI_TOMBSTONE            (0xA000001FL)
#define IO_REPARSE_TAG_UNHANDLED                (0x80000020L)
#define IO_REPARSE_TAG_ONEDRIVE                 (0x80000021L)
#define IO_REPARSE_TAG_PROJFS_TOMBSTONE         (0xA0000022L)
#define IO_REPARSE_TAG_AF_UNIX                  (0x80000023L)
/// Windows Linux Subsystem
#define IO_REPARSE_TAG_LX_FIFO                  (0x80000024L)
#define IO_REPARSE_TAG_LX_CHR                   (0x80000025L)
#define IO_REPARSE_TAG_LX_BLK                   (0x80000026L)
*/

#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK (0x8000001BL)
#endif

// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/ntifs/ns-ntifs-_reparse_data_buffer

#define SYMLINK_FLAG_RELATIVE                                                  \
  0x00000001 // If set then this is a relative symlink.
#define SYMLINK_DIRECTORY                                                      \
  0x80000000 // If set then this is a directory symlink. This is not persisted
             // on disk and is programmatically set by file system.
#define SYMLINK_FILE                                                           \
  0x40000000 // If set then this is a file symlink. This is not persisted on
             // disk and is programmatically set by file system.

#define SYMLINK_RESERVED_MASK                                                  \
  0xF0000000 // We reserve the high nibble for internal use

typedef struct _REPARSE_DATA_BUFFER {
  ULONG ReparseTag;         // Reparse tag type
  USHORT ReparseDataLength; // Length of the reparse data
  USHORT Reserved;          // Used internally by NTFS to store remaining length

  union {
    // Structure for IO_REPARSE_TAG_SYMLINK
    // Handled by nt!IoCompleteRequest
    struct {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      ULONG Flags;
      WCHAR PathBuffer[1];
    } SymbolicLinkReparseBuffer;

    // Structure for IO_REPARSE_TAG_MOUNT_POINT
    // Handled by nt!IoCompleteRequest
    struct {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      WCHAR PathBuffer[1];
    } MountPointReparseBuffer;

    // Structure for IO_REPARSE_TAG_WIM
    // Handled by wimmount!FPOpenReparseTarget->wimserv.dll
    // (wimsrv!ImageExtract)
    struct {
      GUID ImageGuid;           // GUID of the mounted VIM image
      BYTE ImagePathHash[0x14]; // Hash of the path to the file within the image
    } WimImageReparseBuffer;

    // Structure for IO_REPARSE_TAG_WOF
    // Handled by FSCTL_GET_EXTERNAL_BACKING, FSCTL_SET_EXTERNAL_BACKING in NTFS
    // (Windows 10+)
    struct {
      //-- WOF_EXTERNAL_INFO --------------------
      ULONG Wof_Version;  // Should be 1 (WOF_CURRENT_VERSION)
      ULONG Wof_Provider; // Should be 2 (WOF_PROVIDER_FILE)

      //-- FILE_PROVIDER_EXTERNAL_INFO_V1 --------------------
      ULONG FileInfo_Version; // Should be 1 (FILE_PROVIDER_CURRENT_VERSION)
      ULONG
      FileInfo_Algorithm; // Usually 0 (FILE_PROVIDER_COMPRESSION_XPRESS4K)
    } WofReparseBuffer;

    // Structure for IO_REPARSE_TAG_APPEXECLINK
    struct {
      ULONG StringCount;   // Number of the strings in the StringList, separated
                           // by '\0'
      WCHAR StringList[1]; // Multistring (strings separated by '\0', terminated
                           // by '\0\0')
    } AppExecLinkReparseBuffer;

    // Structure for IO_REPARSE_TAG_WCI (0x80000018)
    struct {
      ULONG Version; // Expected to be 1 by wcifs.sys
      ULONG Reserved;
      GUID LookupGuid;      // GUID used for lookup in wcifs!WcLookupLayer
      USHORT WciNameLength; // Length of the WCI subname, in bytes
      WCHAR WciName[1];     // The WCI subname (not zero terminated)
    } WcifsReparseBuffer;

    // Handled by cldflt.sys!HsmpRpReadBuffer
    struct {
      USHORT Flags;    // Flags (0x8000 = not compressed)
      USHORT Length;   // Length of the data (uncompressed)
      BYTE RawData[1]; // To be RtlDecompressBuffer-ed
    } HsmReparseBufferRaw;

    // Dummy structure
    struct {
      UCHAR DataBuffer[1];
    } GenericReparseBuffer;
  } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_SIZE                                        \
  FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)

namespace viewer {

HMODULE KrModule() {
  static HMODULE hModule = GetModuleHandleW(L"kernel32.dll");
  if (hModule == nullptr) {
    OutputDebugStringW(L"GetModuleHandleA failed");
  }
  return hModule;
}

#ifndef _M_X64
class FsRedirection {
public:
  typedef BOOL WINAPI fntype_Wow64DisableWow64FsRedirection(PVOID *OldValue);
  typedef BOOL WINAPI fntype_Wow64RevertWow64FsRedirection(PVOID *OldValue);
  FsRedirection() {
    auto hModule = KrModule();
    auto pfnWow64DisableWow64FsRedirection =
        (fntype_Wow64DisableWow64FsRedirection *)GetProcAddress(
            hModule, "Wow64DisableWow64FsRedirection");
    if (pfnWow64DisableWow64FsRedirection) {
      pfnWow64DisableWow64FsRedirection(&OldValue);
    }
  }
  ~FsRedirection() {
    auto hModule = KrModule();
    auto pfnWow64RevertWow64FsRedirection =
        (fntype_Wow64RevertWow64FsRedirection *)GetProcAddress(
            hModule, "Wow64RevertWow64FsRedirection");
    if (pfnWow64RevertWow64FsRedirection) {
      pfnWow64RevertWow64FsRedirection(&OldValue);
    }
  }

private:
  PVOID OldValue = nullptr;
};
#endif

bool PathCanonicalizeEx(std::wstring_view sv, std::wstring &path) {
  LPWSTR lpPart;
  if (sv.size() > 4 && sv[0] == '\\' && sv[1] == '\\' && sv[3] == '\\') {
    sv.remove_prefix(4);
  }
  auto N = GetFullPathNameW(sv.data(), 0, nullptr, nullptr);
  if (N == 0) {
    return false;
  }
  path.resize(N + 1);
  N = GetFullPathNameW(sv.data(), N + 1, &path[0], &lpPart);
  if (N == 0) {
    return false;
  }
  path.resize(N);
  if (path.size() > 2 && (path.back() == L'\\' || path.back() == L'/')) {
    path.pop_back();
  }
  return true;
}

std::optional<file_target_t> ResolveTarget(std::wstring_view sv) {
#ifndef _M_X64
  FsRedirection fsr;
#endif
  return std::nullopt;
}

inline bool HardLinkEqual(std::wstring_view lh, std::wstring_view rh) {
  if (lh.size() != rh.size()) {
    return false;
  }
  return _wcsnicmp(lh.data(), rh.data(), rh.size()) == 0;
}

// File hardlinks.
std::optional<file_links_t> ResolveLinks(std::wstring_view sv) {
#ifndef _M_X64
  FsRedirection fsr;
#endif
  std::wstring self;
  if (!PathCanonicalizeEx(sv, self)) {
    auto ec = error_code::lasterror();
    priv::error(L"GetFullPathNameW: [%s] %s\n", sv.data(), ec.message.c_str());
    return std::nullopt;
  }
  priv::verbose(L"FullPath: %s\n", self.data());
  auto hFile = CreateFileW(self.data(),           // file to open
                           GENERIC_READ,          // open for reading
                           FILE_SHARE_READ,       // share for reading
                           NULL,                  // default security
                           OPEN_EXISTING,         // existing file only
                           FILE_ATTRIBUTE_NORMAL, // normal file
                           NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    return std::nullopt;
  }
  BY_HANDLE_FILE_INFORMATION bi;
  if (GetFileInformationByHandle(hFile, &bi) != TRUE) {
    CloseHandle(hFile);
    return std::nullopt;
  }
  CloseHandle(hFile);
  priv::info(L"File: %s links: %d\n", self.data(), bi.nNumberOfLinks);
  LARGE_INTEGER li;
  li.HighPart = bi.nFileIndexHigh;
  li.LowPart = bi.nFileIndexLow;
  priv::info(L"File Index: 0x%016x\n", li.QuadPart);
  if (bi.nNumberOfLinks <= 1) {
    /// on other hardlinks
    return std::nullopt;
  }
  auto linkPath = std::make_unique<wchar_t[]>(PATHCCH_MAX_CCH);
  DWORD dwlen = PATHCCH_MAX_CCH;
  auto hFind = FindFirstFileNameW(self.c_str(), 0, &dwlen, linkPath.get());
  if (hFind == INVALID_HANDLE_VALUE) {
    auto ec = error_code::lasterror();
    priv::Print(priv::fc::Red, L"unable find file links: [%s] (%d)%s %08x\n",
                self, ec.code, ec.message, (intptr_t)self.c_str());
    return std::nullopt;
  }
  file_links_t link;

  do {
    auto s = self.substr(0, 2);
    s.append(linkPath.get(),dwlen-1);
    //priv::verbose(L"Find: %s %zu %zu\n", s, s.size(), self.size());
    if (!HardLinkEqual(s, self)) {
      link.links.push_back(s);
    }
    dwlen = PATHCCH_MAX_CCH;
  } while (FindNextFileNameW(hFind, &dwlen, linkPath.get()));
  FindClose(hFind);
  link.self = self;
  return std::make_optional<file_links_t>(link);
}

} // namespace viewer