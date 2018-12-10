///
#include <string>
#include "resolve.hpp"
#include "console/console.hpp"
#pragma comment(lib, "Pathcch")

int wmain(int argc, wchar_t **argv) {
  //
  priv::VerboseEnable();
  if (argc < 2) {
    wprintf(L"%s usage file\n", argv[0]);
    return 1;
  }
  auto hlink = viewer::ResolveTarget(argv[1]);
  auto link = viewer::ResolveLinks(argv[1]);
  if (link) {
    wprintf(L"File %s hardlinks:\n", link->self.c_str());
    for (const auto &l : link->links) {
      wprintf(L"    %s\n", l.data());
    }
  }
  return 0;
}