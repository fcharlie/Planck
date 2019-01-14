#pragma once
#include <cstring>
#include <functional>
#include <string_view>
#include <vector>
#include "charconvex.hpp"
#include "errorcode.hpp"

namespace planck {
class ParseArgv {
public:
  enum HasArgs {
    required_argument, /// -i 11 or -i=xx
    no_argument,
    optional_argument /// -s --long --long=xx
  };
  struct option {
    const wchar_t *name;
    HasArgs has_args;
    int val;
  };
  using ArgumentCallback =
      std::function<bool(int, const wchar_t *optarg, const wchar_t *raw)>;
  ParseArgv(int argc, wchar_t *const *argv) : argc_(argc), argv_(argv) {}
  ParseArgv(const ParseArgv &) = delete;
  ParseArgv &operator=(const ParseArgv &) = delete;

  base::error_code ParseArgument(const std::vector<option> &opts,
                                 const ArgumentCallback &callback) {
    if (argc_ == 0 || argv_ == nullptr) {
      return base::make_error_code(L"invalid argument input");
    };
    index = 1;
    for (; index < argc_; index++) {
      std::wstring_view arg = argv_[index];
      if (arg[0] != '-') {
        uargs.push_back(arg);
        continue;
      }
      auto ec = ParseInternal(arg, opts, callback);
      if (ec) {
        return ec;
      }
    }
    return base::error_code();
  }
  const std::vector<std::wstring_view> &UnresolvedArgs() const { return uargs; }

private:
  int argc_;
  wchar_t *const *argv_;
  std::vector<std::wstring_view> uargs;
  int index{0};
  base::error_code ParseInternal(std::wstring_view arg,
                                 const std::vector<option> &opts,
                                 const ArgumentCallback &callback) {
    /*
    -x ; -x value -Xvalue
    --xy;--xy=value;--xy value
    */
    if (arg.size() < 2) {
      return base::make_error_code(L"Invalid argument");
    }
    int ch = -1;
    HasArgs ha = optional_argument;
    const wchar_t *optarg = nullptr;

    if (arg[1] == '-') {
      /// parse long
      /// --name value; --name=value
      std::wstring_view name;
      auto pos = arg.find('=');
      if (pos != std::wstring_view::npos) {
        if (pos + 1 >= arg.size()) {
          return base::make_error_code(
              std::wstring(L"Incorrect argument: ").append(arg));
        }
        name = arg.substr(2, pos - 2);
        optarg = arg.data() + pos + 1;
      } else {
        name = arg.substr(2);
      }
      for (auto &o : opts) {
        if (name.compare(o.name) == 0) {
          ch = o.val;
          ha = o.has_args;
          break;
        }
      }
    } else {
      /// parse short
      ch = arg[1];

      /// -x=xxx
      if (arg.size() == 3 && arg[2] == '=') {
        return base::make_error_code(
            std::wstring(L"Incorrect argument: ").append(arg));
      }
      if (arg.size() > 3) {
        if (arg[2] == '=') {
          optarg = arg.data() + 3;
        } else {
          optarg = arg.data() + 2;
        }
      }
      for (auto &o : opts) {
        if (o.val == ch) {
          ha = o.has_args;
          break;
        }
      }
    }

    if (optarg != nullptr && ha == no_argument) {
      return base::make_error_code(
          std::wstring(L"Unacceptable input: ").append(arg));
    }
    if (optarg == nullptr && ha == required_argument) {
      if (index + 1 >= argc_) {
        return base::make_error_code(
            std::wstring(L"Option name cannot be empty: ").append(arg));
      }
      optarg = argv_[index + 1];
      index++;
    }
    if (callback(ch, optarg, arg.data())) {
      return base::error_code();
    }
    return base::make_error_code(L"skipped");
  }
};

} // namespace planck