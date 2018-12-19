////////////////
#ifndef PROBE_FWD_HPP
#define PROBE_FWD_HPP
#include <string>
#include <map>
#include <string_view>
#include <system_error>
#include <optional>

namespace probe {
constexpr const int einident = 16;
struct elf_minutiae_t {
  uint8_t ident[einident];
};

struct pe_version_t {
  uint16_t major;
  uint16_t minor;
};

struct pe_minutiae_t {
  std::wstring machine;
  std::wstring subsystem;
  std::wstring characteristics;
  std::wstring clrmsg;
  pe_version_t osver;
  pe_version_t linkver;
  pe_version_t imagever;
};

std::optional<pe_minutiae_t> PortableExecutableDump(std::wstring_view sv,
                                                    std::error_code &ec);
std::wstring fromutf8(std::string_view text);
} // namespace probe

#endif