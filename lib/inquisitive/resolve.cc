///
#include "resolve.hpp"
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include <winioctl.h>
#include <pathcch.h>
#include <memory>
#include <vector>
#include <charconv>
#include <ShlObj.h>
#include "console/console.hpp"

std::wstring guidencode(const GUID &guid) {
  wchar_t wbuf[64];
  swprintf_s(wbuf, L"{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}",
             guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
             guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],
             guid.Data4[6], guid.Data4[7]);
  return std::wstring(wbuf);
}

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
  auto FileHandle = CreateFileW(
      sv.data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr, OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
  if (FileHandle == INVALID_HANDLE_VALUE) {
    return std::nullopt;
  }
  BYTE mxbuf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = {0};
  auto rebuf = reinterpret_cast<PREPARSE_DATA_BUFFER>(mxbuf);
  DWORD dwlen = 0;
  if (DeviceIoControl(FileHandle, FSCTL_GET_REPARSE_POINT, nullptr, 0, rebuf,
                      MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwlen,
                      nullptr) != TRUE) {
    CloseHandle(FileHandle);
    return std::nullopt;
  }
  CloseHandle(FileHandle);

  file_target_t file;
  switch (rebuf->ReparseTag) {
  case IO_REPARSE_TAG_SYMLINK: {
    file.type = SymbolicLink;
    auto wstr =
        rebuf->SymbolicLinkReparseBuffer.PathBuffer +
        (rebuf->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR));
    auto wlen =
        rebuf->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
    if (wlen >= 4 && wstr[0] == L'\\' && wstr[1] == L'?' && wstr[2] == L'?' &&
        wstr[3] == L'\\') {
      /* Starts with \??\ */
      if (wlen >= 6 &&
          ((wstr[4] >= L'A' && wstr[4] <= L'Z') ||
           (wstr[4] >= L'a' && wstr[4] <= L'z')) &&
          wstr[5] == L':' && (wlen == 6 || wstr[6] == L'\\')) {
        /* \??\<drive>:\ */
        wstr += 4;
        wlen -= 4;

      } else if (wlen >= 8 && (wstr[4] == L'U' || wstr[4] == L'u') &&
                 (wstr[5] == L'N' || wstr[5] == L'n') &&
                 (wstr[6] == L'C' || wstr[6] == L'c') && wstr[7] == L'\\') {
        /* \??\UNC\<server>\<share>\ - make sure the final path looks like */
        /* \\<server>\<share>\ */
        wstr += 6;
        wstr[0] = L'\\';
        wlen -= 6;
      }
    }
    file.path.assign(wstr, wlen);
  } break;
  case IO_REPARSE_TAG_MOUNT_POINT: {
    file.type = MountPoint;
    auto wstr =
        rebuf->MountPointReparseBuffer.PathBuffer +
        (rebuf->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR));
    auto wlen =
        rebuf->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
    /* Only treat junctions that look like \??\<drive>:\ as symlink. */
    /* Junctions can also be used as mount points, like \??\Volume{<guid>}, */
    /* but that's confusing for programs since they wouldn't be able to */
    /* actually understand such a path when returned by uv_readlink(). */
    /* UNC paths are never valid for junctions so we don't care about them. */
    if (!(wlen >= 6 && wstr[0] == L'\\' && wstr[1] == L'?' && wstr[2] == L'?' &&
          wstr[3] == L'\\' &&
          ((wstr[4] >= L'A' && wstr[4] <= L'Z') ||
           (wstr[4] >= L'a' && wstr[4] <= L'z')) &&
          wstr[5] == L':' && (wlen == 6 || wstr[6] == L'\\'))) {
      return std::nullopt;
    }

    /* Remove leading \??\ */
    wstr += 4;
    wlen -= 4;
    file.path.assign(wstr, wlen);
  } break;
  case IO_REPARSE_TAG_APPEXECLINK: {
    // L"AppExec link";
    file.type = AppExecLink;
    if (rebuf->AppExecLinkReparseBuffer.StringCount >= 3) {
      LPWSTR szString = (LPWSTR)rebuf->AppExecLinkReparseBuffer.StringList;
      std::vector<LPWSTR> strv;
      for (ULONG i = 0; i < rebuf->AppExecLinkReparseBuffer.StringCount; i++) {
        strv.push_back(szString);
        szString += wcslen(szString) + 1;
      }
      appexeclink_t alink{strv[0], strv[1], strv[2]};
      file.av = alink;
      file.path = strv[2];
      // to get value auto x=std::get<appexeclink_t>(file.av);
    }
  } break;
  case IO_REPARSE_TAG_AF_UNIX:
    // L"Unix domain socket";
    file.type = AFUnix;
    file.path = sv;
    break;
  case IO_REPARSE_TAG_ONEDRIVE:
    // L"OneDrive file";
    file.type = OneDrive;
    file.path = sv;
    break;
  case IO_REPARSE_TAG_FILE_PLACEHOLDER:
    // L"Placeholder file";
    file.type = Placeholder;
    file.path = sv;

    break;
  case IO_REPARSE_TAG_STORAGE_SYNC:
    // L"Storage sync file";
    file.type = StorageSync;
    file.path = sv;
    break;
  case IO_REPARSE_TAG_PROJFS:
    // L"Projected File";
    file.type = ProjFS;
    file.path = sv;
    break;
  case IO_REPARSE_TAG_WIM: {
    file.type = WimImage;
    file.path = sv;
    reparse_wim_t wim;
    wim.guid = guidencode(rebuf->WimImageReparseBuffer.ImageGuid);
    wim.hash = hexencode(reinterpret_cast<const char *>(
                             rebuf->WimImageReparseBuffer.ImagePathHash),
                         sizeof(rebuf->WimImageReparseBuffer.ImagePathHash));
    file.av = wim;
  } break;
  case IO_REPARSE_TAG_WOF: {
    // wof.sys Windows Overlay File System Filter Driver
    file.type = Wof;
    file.path = sv;
    reparse_wof_t wof;
    wof.algorithm = rebuf->WofReparseBuffer.FileInfo_Algorithm;
    wof.version = rebuf->WofReparseBuffer.FileInfo_Version;
    wof.wofprovider = rebuf->WofReparseBuffer.Wof_Provider;
    wof.wofversion = rebuf->WofReparseBuffer.Wof_Version;
    file.av = wof;
  } break;
  case IO_REPARSE_TAG_WCI: {
    // wcifs.sys Windows Container Isolation FS Filter Driver
    file.type = Wcifs;
    file.path = sv;
    reparse_wcifs_t wci;
    wci.WciName.assign(rebuf->WcifsReparseBuffer.WciName,
                       rebuf->WcifsReparseBuffer.WciNameLength);
    wci.Version = rebuf->WcifsReparseBuffer.Version;
    wci.Reserved = rebuf->WcifsReparseBuffer.Reserved;
    wci.LookupGuid = guidencode(rebuf->WcifsReparseBuffer.LookupGuid);
    file.av = wci;
  } break;
  case IO_REPARSE_TAG_HSM:
    break;
  default:
    break;
  }
  if (file.type == HardLink) {
    return std::nullopt;
  }
  return std::make_optional<file_target_t>(file);
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
  auto FileHandle = CreateFileW(self.data(),           // file to open
                                GENERIC_READ,          // open for reading
                                FILE_SHARE_READ,       // share for reading
                                NULL,                  // default security
                                OPEN_EXISTING,         // existing file only
                                FILE_ATTRIBUTE_NORMAL, // normal file
                                NULL);
  if (FileHandle == INVALID_HANDLE_VALUE) {
    return std::nullopt;
  }
  BY_HANDLE_FILE_INFORMATION bi;
  if (GetFileInformationByHandle(FileHandle, &bi) != TRUE) {
    CloseHandle(FileHandle);
    return std::nullopt;
  }
  CloseHandle(FileHandle);
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
    s.append(linkPath.get(), dwlen - 1);
    // priv::verbose(L"Find: %s %zu %zu\n", s, s.size(), self.size());
    if (!HardLinkEqual(s, self)) {
      link.links.push_back(s);
    }
    dwlen = PATHCCH_MAX_CCH;
  } while (FindNextFileNameW(hFind, &dwlen, linkPath.get()));
  FindClose(hFind);
  link.self = self;
  return std::make_optional<file_links_t>(link);
}

// Shell link flags
// Thanks:
// https://github.com/reactos/reactos/blob/bfcbda227f99c1b59e8ed71f5e0f59f793d496a1/sdk/include/reactos/undocshell.h#L800
enum : DWORD {
  SldfNone = 0x00000000,
  HasLinkTargetIDList = 0x00000001,
  HasLinkInfo = 0x00000002,
  HasName = 0x00000004,
  HasRelativePath = 0x00000008,
  HasWorkingDir = 0x00000010,
  HasArguments = 0x00000020,
  HasIconLocation = 0x00000040,
  IsUnicode = 0x00000080,
  ForceNoLinkInfo = 0x00000100,
  HasExpString = 0x00000200,
  RunInSeparateProcess = 0x00000400,
  Unused1 = 0x00000800,
  HasDrawinID = 0x00001000,
  RunAsUser = 0x00002000,
  HasExpIcon = 0x00004000,
  NoPidlAlias = 0x00008000,
  Unused2 = 0x00010000,
  RunWithShimLayer = 0x00020000,
  ForceNoLinkTrack = 0x00040000,
  EnableTargetMetadata = 0x00080000,
  DisableLinkPathTarcking = 0x00100000,
  DisableKnownFolderTarcking = 0x00200000,
  DisableKnownFolderAlia = 0x00400000,
  AllowLinkToLink = 0x00800000,
  UnaliasOnSave = 0x01000000,
  PreferEnvironmentPath = 0x02000000,
  KeepLocalIDListForUNCTarget = 0x04000000,
  PersistVolumeIDRelative = 0x08000000,
  SldfInvalid = 0x0ffff7ff,
  Reserved = 0x80000000
};

#pragma pack(1)
/*
SHELL_LINK = SHELL_LINK_HEADER [LINKTARGET_IDLIST] [LINKINFO]
 [STRING_DATA] *EXTRA_DATA
*/
struct shell_link_t {
  DWORD dwSize;
  BYTE uuid[16];
  DWORD linkflags;
  DWORD fileattr;
  FILETIME createtime;
  FILETIME accesstime;
  FILETIME writetime;
  LONG filesize;
  DWORD iconindex;
  DWORD showcommand;
  WORD hotkey;
  WORD reserved1;
  DWORD reserved2;
  DWORD reserved3;
};

struct link_target_idlist_t {
  WORD idlistsize;
  // IDLIST
};

#pragma pack()

class PbShellLink {
public:
  static constexpr auto nullfile_t = INVALID_HANDLE_VALUE;
  static inline void Close(HANDLE hFile) {
    if (hFile != nullfile_t) {
      CloseHandle(hFile);
    }
  }
  PbShellLink() = default;
  PbShellLink(const PbShellLink &) = delete;
  PbShellLink &operator=(const PbShellLink &) = delete;
  ~PbShellLink() {
    if (baseAddr != nullptr) {
      UnmapViewOfFile(baseAddr);
    }
    Close(FileMap);
    Close(FileHandle);
  }
  // https://msdn.microsoft.com/en-us/library/dd871305.aspx
  std::optional<std::wstring> Target(std::wstring_view sv) {
    if ((FileHandle = CreateFileW(sv.data(), GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                  nullptr)) == nullfile_t) {
      return std::nullopt;
    }
    LARGE_INTEGER li;
    if (GetFileSizeEx(FileHandle, &li) != TRUE || li.QuadPart < 0x4E) {
      return std::nullopt;
    }
    if ((FileMap = CreateFileMappingW(FileHandle, nullptr, PAGE_READONLY, 0, 0,
                                      nullptr)) == nullfile_t) {
      return std::nullopt;
    }
    baseAddr = MapViewOfFile(FileMap, FILE_MAP_READ, 0, 0, 0);
    if (baseAddr == nullptr) {
      return std::nullopt;
    }
    auto data = reinterpret_cast<UCHAR *>(baseAddr);
    if (*(DWORD *)(data + 0x00) != 0x0000004C) {
      // LNK file header invalid
      return std::nullopt;
    }
    if (*(DWORD *)(data + 0x04) != 0x00021401 ||
        *(DWORD *)(data + 0x08) != 0x00000000 ||
        *(DWORD *)(data + 0x0C) != 0x000000C0 ||
        *(DWORD *)(data + 0x10) != 0x46000000) {
      // shell link GUID invalid
      return std::nullopt;
    }
    // check for presence of shell item ID list
    if ((*(BYTE *)(data + 0x14) & 0x01) == 0) {
      // shell item ID list is not present
      return std::nullopt;
    }
    WCHAR target[4096] = {0};
    // convert shell item identifier list to file system path
    // the shell item ID list starts always at offset 0x4E after the LNK
    // file header and the WORD for the length of the item ID list
    // see the reference "The Windows Shortcut File Format" by "Jesse Hager"
    if (SHGetPathFromIDListEx((LPCITEMIDLIST)(data + 0x4E), target,
                              ARRAYSIZE(target), GPFIDL_UNCPRINTER) == FALSE) {
      return std::nullopt;
    }
    return std::make_optional<std::wstring>(target);
  }

private:
  HANDLE FileHandle{nullfile_t};
  HANDLE FileMap{nullfile_t};
  LPVOID baseAddr{nullptr};
};

std::optional<std::wstring> ResolveShortcut(std::wstring_view sv) {
  PbShellLink slink;
  return slink.Target(sv);
}
} // namespace viewer