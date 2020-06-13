///
#ifndef PLANCK_HPP
#define PLANCK_HPP
#if defined(__has_include)
#if __has_include(<baseversion.h>)
#include <baseversion.h>
#endif
#endif
#include <string>

#ifndef PLANCK_VERSION
#define PLANCK_VERSION L"1.0"
#endif

#ifndef PLANCK_HASH
#define PLANCK_HASH L"none"
#endif

#ifndef PLANCK_REFS
#define PLANCK_REFS L"none"
#endif

#ifndef PLANCK_APPNAME
#define PLANCK_APPNAME L"Planck-1.0.0"
#endif

namespace planck {
//
inline std::wstring planck_version() {
  // todo
  return PLANCK_APPNAME;
}
} // namespace planck

#endif
