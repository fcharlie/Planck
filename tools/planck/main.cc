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
  planck::PrintNone(L"%s", kUsage);
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
      planck::VerboseEnable();
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
  planck::VerboseEnable();
  if (argc < 2) {
    wprintf(L"%s usage file\n", argv[0]);
    return 1;
  }
  base::error_code ec;
  auto hlink = inquisitive::ResolveTarget(argv[1], ec);
  auto link = inquisitive::ResolveLinks(argv[1], ec);
  if (link) {
    wprintf(L"File %s hardlinks:\n", link->self.c_str());
    for (const auto &l : link->links) {
      wprintf(L"    %s\n", l.data());
    }
  }
  auto ir = inquisitive::inquisitive(argv[1], ec);
  if (ec) {
    planck::error(L"Error %s\n", ec.message);
    return 0;
  }
  if (ir) {
    if (ir->typeex() == inquisitive::types::PECOFF) {
      auto ps = inquisitive::inquisitive_pecoff(argv[1], ec);
      if (!ec && ps) {
        ir->add(L"Machine", ps->machine);
        ir->add(L"Subsystem", ps->subsystem);
        if (!ps->clrmsg.empty()) {
          ir->add(L"CLR", ps->clrmsg);
        }
        ir->add(L"Depends", ps->depends);
        if (!ps->delays.empty()) {
          ir->add(L"Delay Depends", ps->delays);
        }
      }
    }
    auto al = ir->alignlen() + 4;
    constexpr const size_t deslen = sizeof("Description") - 1;
    std::wstring space(al, L' ');
    planck::PrintNone(L"Description:%.*s%s\n", (int)(al - deslen - 1), space,
                      ir->description());
    for (const auto &v : ir->container()) {
      planck::PrintNone(L"%s:%.*s%s\n", v.name, (int)(al - v.name.size() - 1),
                        space, v.value);
    }
    for (const auto &m : ir->mcontainer()) {
      if (m.values.empty()) {
        continue;
      }
      planck::PrintNone(L"%s:%.*s%s\n", m.name, (int)(al - m.name.size() - 1),
                        space, m.values[0]);
      for (size_t i = 1; i < m.values.size(); i++) {
        planck::PrintNone(L"%s%s\n", space, m.values[i]);
      }
    }
    planck::PrintNone(L"\n");
  }
  return 0;
}