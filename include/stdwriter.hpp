/////////
#ifndef PLANCK_STDWRITER_HPP
#define PLANCK_STDWRITER_HPP
#include <cstdio>
#include <string_view>
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <Windows.h>
#endif

namespace planck {
enum console_mode {
  CONSOLE_FILE = 0, // File
  CONSOLE_TTY,      // Mintty like
  CONSOLE_CONHOST,  // Conhost
  CONSOLE_PTY       // Windows 10 PTY
};


inline bool EnableVTMode(HANDLE hFile) {
  DWORD dwMode = 0;
  if (!GetConsoleMode(hFile, &dwMode)) {
    return false;
  }
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(hFile, dwMode)) {
    return false;
  }
  return true;
}

inline int FileHandleMode(HANDLE hFile, bool &vt) {
  if (hFile == INVALID_HANDLE_VALUE) {
    return CONSOLE_FILE;
  }
  if (GetFileType(hFile) == FILE_TYPE_DISK) {
    return CONSOLE_FILE;
  }
  if (GetFileType(hFile) == FILE_TYPE_CHAR) {
    vt = EnableVTMode(hFile);
    return CONSOLE_CONHOST;
  }
  return CONSOLE_TTY;
}

class stdwriter_adapter {
public:
  stdwriter_adapter(const stdwriter_adapter &) = delete;
  stdwriter_adapter &operator=(const stdwriter_adapter &) = delete;
  stdwriter_adapter &instance() {
    static stdwriter_adapter instance_;
    return instance_;
  }

private:
  stdwriter_adapter() {
    //
  }
};
} // namespace planck

#endif