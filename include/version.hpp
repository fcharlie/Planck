///
#ifndef FILEVIEW_HPP
#define FILEVIEW_HPP
#if defined(__has_include)
#if __has_include(<baseversion.h>)
#include <baseversion.h>
#endif
#endif
#include <string>

#ifndef FILEVIEW_VERSION
#define FILEVIEW_VERSION L"1.0"
#endif

#ifndef FILEVIEW_HASH
#define FILEVIEW_HASH L"none"
#endif

#ifndef FILEVIEW_REFS
#define FILEVIEW_REFS L"none"
#endif

#ifndef FILEVIEW_APPNAME
#define FILEVIEW_APPNAME L"FileViewer-1.0.0"
#endif

namespace fileview {
//
inline std::wstring fileview_version() {
  // todo
  return FILEVIEW_APPNAME;
}
} // namespace fileview

#endif
