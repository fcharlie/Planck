//////////
#ifndef PLANCK_CONSOLE_ADAPTER_HPP
#define PLANCK_CONSOLE_ADAPTER_HPP
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <Windows.h>
#endif
#include <string>
#include <functional>

namespace planck {
using ssize_t = SSIZE_T;
namespace fc {
enum Color : WORD {
  Black = 0,
  DarkBlue = 1,
  DarkGreen = 2,
  DarkCyan = 3,
  DarkRed = 4,
  DarkMagenta = 5,
  DarkYellow = 6,
  Gray = 7,
  DarkGray = 8,
  Blue = 9,
  Green = 10,
  Cyan = 11,
  Red = 12,
  Magenta = 13,
  Yellow = 14,
  White = 15
};
}

namespace bc {
enum Color : WORD {
  Black = 0,
  DarkGray = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
  Blue = BACKGROUND_BLUE,
  Green = BACKGROUND_GREEN,
  Red = BACKGROUND_RED,
  Yellow = BACKGROUND_RED | BACKGROUND_GREEN,
  Magenta = BACKGROUND_RED | BACKGROUND_BLUE,
  Cyan = BACKGROUND_GREEN | BACKGROUND_BLUE,
  LightWhite = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED |
               BACKGROUND_INTENSITY,
  LightBlue = BACKGROUND_BLUE | BACKGROUND_INTENSITY,
  LightGreen = BACKGROUND_GREEN | BACKGROUND_INTENSITY,
  LightRed = BACKGROUND_RED | BACKGROUND_INTENSITY,
  LightYellow = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
  LightMagenta = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
  LightCyan = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
};
}

namespace details {
enum adaptermode_t : int {
  AdapterFile,
  AdapterConsole,
  AdapterConsoleTTY,
  AdapterTTY
};

enum adapterlevel_t : int {
  kAdapterDEBUG,
  kAdapterINFO,
  kAdapterWARN,
  kAdapterERROR,
  kAdapterFATAL
};

class adapter {
public:
  adapter(const adapter &) = delete;
  adapter &operator=(const adapter &) = delete;
  ~adapter() {
    //
  }
  static adapter &instance() {
    static adapter adapter_;
    return adapter_;
  }
  ssize_t adapterwritelevel(int level, const wchar_t *data, size_t len) {
    if (level < al) {
      return 0;
    }
    switch (level) {
    case kAdapterDEBUG:
      return adapterwrite(fc::Yellow, data, len);
    case kAdapterINFO:
      return adapterwrite(fc::Green, data, len);
    case kAdapterWARN:
      return adapterwrite(fc::DarkYellow, data, len);
    case kAdapterERROR:
    case kAdapterFATAL:
      return adapterwrite(fc::Red, data, len);
    default:
      break;
    }
    return 0;
  }
  ssize_t adapterwrite(int color, const wchar_t *data, size_t len) {
    switch (at) {
    case AdapterFile:
      return writefile(color, data, len);
    case AdapterConsole:
      return writeoldconsole(color, data, len);
    case AdapterConsoleTTY:
      return writeconsole(color, data, len);
    case AdapterTTY:
      return writetty(color, data, len);
    }
    return -1;
  }
  bool changeout(bool isstderr);
  bool changelevel(adapterlevel_t al_) {
    al = al_;
    return al;
  }

private:
  adapter();
  ssize_t writefile(int color, const wchar_t *data, size_t len);
  ssize_t writeoldconsole(int color, const wchar_t *data, size_t len);
  ssize_t writeconsole(int color, const wchar_t *data, size_t len);
  ssize_t writetty(int color, const wchar_t *data, size_t len);
  //
  int WriteConsoleInternal(const wchar_t *buffer, size_t len);
  adaptermode_t at{AdapterConsole};
  adapterlevel_t al{kAdapterINFO};
  HANDLE hConsole{nullptr};
  FILE *out{stdout};
};

} // namespace details
} // namespace planck

#endif