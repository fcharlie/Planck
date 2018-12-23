////////////////
#ifndef PROBE_FWD_HPP
#define PROBE_FWD_HPP
#include <string>
#include <map>
#include <string_view>
#include <system_error>
#include <optional>
#include <vector>
#include "memview.hpp"

namespace probe {
constexpr const int einident = 16;
namespace endina {
enum endian_t : unsigned { None, LittleEndian, BigEndian };
}
struct elf_minutiae_t {
  uint8_t ident[einident];
  bool is64bit; /// 64 Bit
  endina::endian_t endian;
  std::wstring machine;
  std::wstring rpath;             // RPATH or some
  std::vector<std::wstring> deps; /// require so
};

struct pe_version_t {
  uint16_t major;
  uint16_t minor;
};

struct pe_minutiae_t {
  std::wstring machine;
  std::wstring subsystem;
  std::wstring clrmsg;
  std::vector<std::wstring> characteristics;
  std::vector<std::wstring> deps; /// DLL required
  pe_version_t osver;
  pe_version_t linkver;
  pe_version_t imagever;
  bool isdll;
};

std::optional<pe_minutiae_t> PortableExecutableDump(memview mv,
                                                    std::error_code &ec);
std::wstring fromutf8(std::string_view text);
} // namespace probe

#endif