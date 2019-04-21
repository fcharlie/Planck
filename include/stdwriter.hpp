/////////
#ifndef PLANCK_STDWRITER_HPP
#define PLANCK_STDWRITER_HPP
#include <cstdio>
#include <string_view>
#include "base.hpp"

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
  static stdwriter_adapter &instance() {
    static stdwriter_adapter instance_;
    return instance_;
  }
  int write(FILE *out, std::wstring_view msg) {
    int mode = (out == stderr ? stderrmode : stdoutmode);
    switch (mode) {
    case CONSOLE_TTY:
    case CONSOLE_FILE:
    case CONSOLE_PTY:
      break;
    case CONSOLE_CONHOST: {
      if (out == stderr) {
        return writeconsole(hStderr, stderrvt, msg);
      }
      return writeconsole(hStdout, stdoutvt, msg);
    }
    }
    auto umsg = base::ToNarrow(msg);
    return (int)fwrite(umsg.data(), 1, umsg.size(), out);
  }

private:
  HANDLE hStderr;
  HANDLE hStdout;
  int stderrmode{0};
  int stdoutmode{0};
  bool stderrvt{false};
  bool stdoutvt{false};
  stdwriter_adapter() {
    hStderr = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdout = GetStdHandle(STD_ERROR_HANDLE);
    stderrmode = FileHandleMode(hStderr, stderrvt);
    stdoutmode = FileHandleMode(hStdout, stdoutvt);
  }
  int writeconsole(HANDLE hFile, bool vt, std::wstring_view wsv) {
    if (vt) {
      DWORD dwWrite = 0;
      WriteConsoleW(hFile, wsv.data(), (DWORD)wsv.size(), &dwWrite, nullptr);
      return (int)dwWrite;
    }
    // unsupport now
    DWORD dwWrite = 0;
    WriteConsoleW(hFile, wsv.data(), (DWORD)wsv.size(), &dwWrite, nullptr);
    return (int)dwWrite;
  }
};

inline int console_write(FILE *out, std::wstring_view msg) {
  if (out != stdout && out != stderr) {
    return (int)fwrite(msg.data(), 1, msg.size(), out);
  }
  return stdwriter_adapter::instance().write(out, msg);
}

template <typename... Args>
int FPrintF(FILE *out, const wchar_t *fmt, Args... args) {
  //
  return 0;
}

} // namespace planck

#endif