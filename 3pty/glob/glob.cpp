
#include <cassert>
#include <functional>
#include <glob/glob.h>
#include <iostream>
#include <map>
#include <regex>

namespace glob {

namespace {

bool string_replace(std::wstring &str, const std::wstring &from, const std::wstring &to) {
  std::size_t start_pos = str.find(from);
  if (start_pos == std::wstring::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::wstring translate(const std::wstring &pattern) {
  std::size_t i = 0, n = pattern.size();
  std::wstring result_string;

  while (i < n) {
    auto c = pattern[i];
    i += 1;
    if (c == '*') {
      result_string += L".*";
    } else if (c == '?') {
      result_string += L".";
    } else if (c == '[') {
      auto j = i;
      if (j < n && pattern[j] == '!') {
        j += 1;
      }
      if (j < n && pattern[j] == ']') {
        j += 1;
      }
      while (j < n && pattern[j] != ']') {
        j += 1;
      }
      if (j >= n) {
        result_string += L"\\[";
      } else {
        auto stuff = std::wstring(pattern.begin() + i, pattern.begin() + j);
        if (stuff.find(L"--") == std::wstring::npos) {
          string_replace(stuff, std::wstring{L"\\"}, std::wstring{LR"(\\)"});
        } else {
          std::vector<std::wstring> chunks;
          std::size_t k = 0;
          if (pattern[i] == '!') {
            k = i + 2;
          } else {
            k = i + 1;
          }

          while (true) {
            k = pattern.find(L"-", k, j);
            if (k == std::wstring::npos) {
              break;
            }
            chunks.push_back(std::wstring(pattern.begin() + i, pattern.begin() + k));
            i = k + 1;
            k = k + 3;
          }

          chunks.push_back(std::wstring(pattern.begin() + i, pattern.begin() + j));
          // Escape backslashes and hyphens for set difference (--).
          // Hyphens that create ranges shouldn't be escaped.
          bool first = true;
          for (auto &s : chunks) {
            string_replace(s, std::wstring{L"\\"}, std::wstring{LR"(\\)"});
            string_replace(s, std::wstring{L"-"}, std::wstring{LR"(\-)"});
            if (first) {
              stuff += s;
              first = false;
            } else {
              stuff += L"-" + s;
            }
          }
        }

        // Escape set operations (&&, ~~ and ||).
        std::wstring result;
        std::regex_replace(std::back_inserter(result),          // ressult
                           stuff.begin(), stuff.end(),          // string
                           std::wregex(std::wstring{LR"([&~|])"}), // pattern
                           std::wstring{LR"(\\\1)"});             // repl
        stuff = result;
        i = j + 1;
        if (stuff[0] == '!') {
          stuff = L"^" + std::wstring(stuff.begin() + 1, stuff.end());
        } else if (stuff[0] == '^' || stuff[0] == '[') {
          stuff = L"\\\\" + stuff;
        }
        result_string = result_string + L"[" + stuff + L"]";
      }
    } else {
      // SPECIAL_CHARS
      // closing ')', '}' and ']'
      // '-' (a range in character set)
      // '&', '~', (extended character set operations)
      // '#' (comment) and WHITESPACE (ignored) in verbose mode
      static std::wstring special_characters = L"()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
      static std::map<int, std::wstring> special_characters_map;
      if (special_characters_map.empty()) {
        for (auto &sc : special_characters) {
          special_characters_map.insert(
              std::make_pair(static_cast<int>(sc), std::wstring{L"\\"} + std::wstring(1, sc)));
        }
      }

      if (special_characters.find(c) != std::wstring::npos) {
        result_string += special_characters_map[static_cast<int>(c)];
      } else {
        result_string += c;
      }
    }
  }
  return std::wstring{L"(("} + result_string + std::wstring{LR"()|[\r\n])$)"};
}

std::wregex compile_pattern(const std::wstring &pattern) {
  return std::wregex(translate(pattern), std::wregex::ECMAScript);
}

bool fnmatch(const fs::path &name, const std::wstring &pattern) {
  return std::regex_match(name.wstring(), compile_pattern(pattern));
}

std::vector<fs::path> filter(const std::vector<fs::path> &names,
                             const std::wstring &pattern) {
  // std::cout << "Pattern: " << pattern << "\n";
  std::vector<fs::path> result;
  for (auto &name : names) {
    // std::cout << "Checking for " << name.wstring() << "\n";
    if (fnmatch(name, pattern)) {
      result.push_back(name);
    }
  }
  return result;
}

fs::path expand_tilde(fs::path path) {
  if (path.empty()) return path;

#ifdef _WIN32
    const char * home_variable = "USERNAME";
#else
    const char * home_variable = "USER";
#endif
    const char * home = std::getenv(home_variable);
    // Probably won't work if the home directory contains any non-ASCII characters
    std::wstring wide_home(home, home + strlen(home));
    
  if (home == nullptr) {
    throw std::invalid_argument("error: Unable to expand `~` - HOME environment variable not set.");
  }

  std::wstring s = path.wstring();
  if (s[0] == '~') {
    s = wide_home + s.substr(1, s.size() - 1);
    return fs::path(s);
  } else {
    return path;
  }
}

bool has_magic(const std::wstring &pathname) {
  static const auto magic_check = std::wregex(L"([*?[])");
  return std::regex_search(pathname, magic_check);
}

bool is_hidden(const std::wstring &pathname) { return pathname[0] == '.'; }

bool is_recursive(const std::wstring &pattern) { return pattern == L"**"; }

std::vector<fs::path> iter_directory(const fs::path &dirname, bool dironly) {
  std::vector<fs::path> result;

  auto current_directory = dirname;
  if (current_directory.empty()) {
    current_directory = fs::current_path();
  }

  if (fs::exists(current_directory)) {
    try {
      for (auto &entry : fs::directory_iterator(
              current_directory, fs::directory_options::follow_directory_symlink |
                                      fs::directory_options::skip_permission_denied)) {
        if (!dironly || entry.is_directory()) {
          if (dirname.is_absolute()) {
            result.push_back(entry.path());
          } else {
            result.push_back(fs::relative(entry.path()));
          }
        }
      }
    } catch (std::exception&) {
      // not a directory
      // do nothing
    }
  }

  return result;
}

// Recursively yields relative pathnames inside a literal directory.
std::vector<fs::path> rlistdir(const fs::path &dirname, bool dironly) {
  std::vector<fs::path> result;
  auto names = iter_directory(dirname, dironly);
  for (auto &x : names) {
    if (!is_hidden(x.wstring())) {
      result.push_back(x);
      for (auto &y : rlistdir(x, dironly)) {
        result.push_back(y);
      }
    }
  }
  return result;
}

// This helper function recursively yields relative pathnames inside a literal
// directory.
std::vector<fs::path> glob2(const fs::path &dirname, [[maybe_unused]] const fs::path &pattern,
                            bool dironly) {
  // std::cout << "In glob2\n";
  std::vector<fs::path> result{"."};
  assert(is_recursive(pattern.wstring()));
  for (auto &dir : rlistdir(dirname, dironly)) {
    result.push_back(dir);
  }
  return result;
}

// These 2 helper functions non-recursively glob inside a literal directory.
// They return a list of basenames.  _glob1 accepts a pattern while _glob0
// takes a literal basename (so it only has to check for its existence).

std::vector<fs::path> glob1(const fs::path &dirname, const fs::path &pattern,
                            bool dironly) {
  // std::cout << "In glob1\n";
  auto names = iter_directory(dirname, dironly);
  std::vector<fs::path> filtered_names;
  for (auto &n : names) {
    if (!is_hidden(n.wstring())) {
      filtered_names.push_back(n.filename());
      // if (n.is_relative()) {
      //   // std::cout << "Filtered (Relative): " << n << "\n";
      //   filtered_names.push_back(fs::relative(n));
      // } else {
      //   // std::cout << "Filtered (Absolute): " << n << "\n";
      //   filtered_names.push_back(n.filename());
      // }
    }
  }
  return filter(filtered_names, pattern.wstring());
}

std::vector<fs::path> glob0(const fs::path &dirname, const fs::path &basename,
                            bool /*dironly*/) {
  // std::cout << "In glob0\n";
  std::vector<fs::path> result;
  if (basename.empty()) {
    // 'q*x/' should match only directories.
    if (fs::is_directory(dirname)) {
      result = {basename};
    }
  } else {
    if (fs::exists(dirname / basename)) {
      result = {basename};
    }
  }
  return result;
}

std::vector<fs::path> glob(const fs::path &inpath, bool recursive = false,
                           bool dironly = false) {
  std::vector<fs::path> result;

  const auto pathname = inpath.wstring();
  auto path = fs::path(pathname);

  if (pathname[0] == '~') {
    // expand tilde
    path = expand_tilde(path);
  }

  auto dirname = path.parent_path();
  const auto basename = path.filename();

  if (!has_magic(pathname)) {
    assert(!dironly);
    if (!basename.empty()) {
      if (fs::exists(path)) {
        result.push_back(path);
      }
    } else {
      // Patterns ending with a slash should match only directories
      if (fs::is_directory(dirname)) {
        result.push_back(path);
      }
    }
    return result;
  }

  if (dirname.empty()) {
    if (recursive && is_recursive(basename.wstring())) {
      return glob2(dirname, basename, dironly);
    } else {
      return glob1(dirname, basename, dironly);
    }
  }

  std::vector<fs::path> dirs;
  if (dirname != fs::path(pathname) && has_magic(dirname.wstring())) {
    dirs = glob(dirname, recursive, true);
  } else {
    dirs = {dirname};
  }

  std::function<std::vector<fs::path>(const fs::path &, const fs::path &, bool)>
      glob_in_dir;
  if (has_magic(basename.wstring())) {
    if (recursive && is_recursive(basename.wstring())) {
      glob_in_dir = glob2;
    } else {
      glob_in_dir = glob1;
    }
  } else {
    glob_in_dir = glob0;
  }

  for (auto &d : dirs) {
    for (auto &name : glob_in_dir(d, basename, dironly)) {
      fs::path subresult = name;
      if (name.parent_path().empty()) {
        subresult = d / name;
      }
      result.push_back(subresult.lexically_normal());
    }
  }

  return result;
}

} // namespace end

std::vector<fs::path> glob(const std::wstring &pathname) {
  return glob(pathname, false);
}

std::vector<fs::path> rglob(const std::wstring &pathname) {
  return glob(pathname, true);
}

std::vector<fs::path> glob(const std::vector<std::wstring> &pathnames) {
  std::vector<fs::path> result;
  for (auto &pathname : pathnames) {
    for (auto &match : glob(pathname, false)) {
      result.push_back(std::move(match));
    }
  }
  return result;
}

std::vector<fs::path> rglob(const std::vector<std::wstring> &pathnames) {
  std::vector<fs::path> result;
  for (auto &pathname : pathnames) {
    for (auto &match : glob(pathname, true)) {
      result.push_back(std::move(match));
    }
  }
  return result;
}

std::vector<fs::path>
glob(const std::initializer_list<std::wstring> &pathnames) {
  return glob(std::vector<std::wstring>(pathnames));
}

std::vector<fs::path>
rglob(const std::initializer_list<std::wstring> &pathnames) {
  return rglob(std::vector<std::wstring>(pathnames));
}

} // namespace glob
