///
#include <string>
#include <string_view>
#include "resolve.hpp"
#include "console/console.hpp"
#include "inquisitive.hpp"
#pragma comment(lib, "Pathcch")

struct AppArgv {
  std::vector<std::wstring_view> files;
  bool verbose{false};
  void push_back(std::wstring_view sv) { files.push_back(sv); }
  auto empty() const { return files.empty(); }
  auto begin() const { return files.begin(); }
  auto end() const { return files.end(); }
  auto size() const { return files.size(); }
  auto operator[](size_t off) { return files[off]; }
};

template <typename... Args> bool IsSameArg(std::wstring_view sv, Args... args) {
  std::initializer_list<std::wstring_view> svl = {args...};
  for (const auto s : svl) {
    if (s == sv) {
      return true;
    }
  }
  return false;
}

void Usage() {
  constexpr const auto kUsage = LR"(FileView)";
  priv::PrintNone(L"%s", kUsage);
}

bool ParseArgv(int argc, wchar_t **argv, AppArgv &av) {
  for (int i = 1; i < argc; i++) {
    auto arg = argv[i];
    if (arg[0] != L'-') {
      av.push_back(arg);
      continue;
    }
    if (IsSameArg(arg, L"-V", L"--verbose")) {
      av.verbose = true;
      priv::VerboseEnable();
      continue;
    }
    if (IsSameArg(arg, L"-v", L"--version")) {
      printf("1.0\n");
      exit(0);
    }
    if (IsSameArg(arg, L"-h", L"--help", L"-?")) {
      Usage();
      exit(0);
    }
  }
  if (av.empty()) {
    //
    return false;
  }
  return true;
}

std::optional<std::wstring> FileTarget(std::wstring_view sv, bool verbose) {
  //
  return std::nullopt;
}

int wmain(int argc, wchar_t **argv) {

  //
  priv::VerboseEnable();
  // auto text = u8"\x4F60\x597D";
  // auto wt = probe::convert(text);
  // priv::PrintNone(L"Text: %s\n", wt);
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