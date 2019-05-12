///
#include <string_view>
#include <clocale>
#include <cstdio>

void dofmt(std::wstring_view fmt) {
  do {
    auto pos = fmt.find(L'%');
    if (pos == std::wstring_view::npos) {
      wprintf(L"END format [%.*s]\n", (int)fmt.size(), fmt.data());
      return;
    }
    auto s = fmt.substr(0, pos);
    wprintf(L"END format [%.*s]\n", (int)s.size(), s.data());
    fmt.remove_prefix(pos + 1);
    /// --parse format

  } while (!fmt.empty());
}

int wmain(int argc, wchar_t const *argv[]) {
  /* code */
  _wsetlocale(LC_ALL, L"");
  dofmt(L"no format string");
  dofmt(L"format %d----%s---%%-");
  return 0;
}