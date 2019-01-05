/// https://lowlevelbits.org/parsing-mach-o-files/
#include "inquisitive.hpp"
#include "macho.hpp"

// Some docs
// 1. Mach-O loader for Linux
// https://github.com/shinh/maloader
//
// 2. PARSING MACH-O FILES
// https://lowlevelbits.org/parsing-mach-o-files/

/* hack until arm64 headers are worked out */
#ifndef CPU_TYPE_ARM64
#define CPU_TYPE_ARM64 (CPU_TYPE_ARM | CPU_ARCH_ABI64)
#endif /* !CPU_TYPE_ARM64 */

namespace inquisitive {
class macho_memview {
public:
private:
};

std::optional<macho_minutiae_t> inquisitive_macho(std::wstring_view sv,
                                                  base::error_code &ec) {
  //
  return std::nullopt;
}

} // namespace inquisitive

// No more plans to parse Mach-O file details