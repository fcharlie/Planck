///
#include <string>
#include "resolve.hpp"
#include "console/console.hpp"
#include "probe/probe_fwd.hpp"
#pragma comment(lib, "Pathcch")

int wmain(int argc, wchar_t **argv) {
  //
  // auto text = u8"\x4F60\x597D";
  // auto wt = probe::convert(text);
  priv::VerboseEnable();
  priv::PrintNone(L"Text: %s\n", wt);
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