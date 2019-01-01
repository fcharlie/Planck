////////////////////////
#ifndef PLANCKUTILS_HPP
#define PLANCKUTILS_HPP
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <charconv.hpp>
#include <console/console.hpp>

namespace planck {

struct AttributesTable {
  std::wstring name;
  std::wstring value;
};

struct AttributesMultiTable {
  std::wstring name;
  std::vector<std::wstring> values;
};
struct AttributesTables {
  std::vector<AttributesTable> ats;
  std::vector<AttributesMultiTable> amts;
  std::size_t mnlen{0};
  bool Empty() const { return ats.empty() && amts.empty(); }
  AttributesTables &Clear() {
    mnlen = 0;
    ats.clear();
    amts.clear();
    return *this;
  }
  AttributesTables &Append(std::wstring_view name, std::wstring &&value) {
    mnlen = (std::max)(mnlen, name.size());
    ats.emplace_back(AttributesTable{std::wstring(name), std::move(value)});
    return *this;
  }
  AttributesTables &Append(std::wstring_view name, std::wstring_view value) {
    mnlen = (std::max)(mnlen, name.size());
    ats.emplace_back(AttributesTable{std::wstring(name), std::wstring(value)});
    return *this;
  }
  AttributesTables &Append(std::wstring_view name,
                           const std::vector<std::wstring> &value) {
    mnlen = (std::max)(mnlen, name.size());
    AttributesMultiTable amt;
    amt.name = name;
    amts.push_back(amt);
    return *this;
  }
  bool DumpWrite(FILE *file) {
    if (file == nullptr) {
      return false;
    }
    if ((file == stdout || file == stderr) && planck::UseWideConsole()) {
      /// console
    }
    return true;
  }
  /// Format gen
  bool AttributesWrite() {
    //
    return true;
  }
};

} // namespace planck

#endif