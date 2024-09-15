
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

std::map<int, std::wstring> make_special_characters_map(const std::wstring& special_characters)
{
  std::map<int, std::wstring> special_characters_map;
  for (auto& sc : special_characters)
  {
    special_characters_map.insert(
      std::make_pair(static_cast<int>(sc), std::wstring{L"\\"} + std::wstring(1, sc)));
  }
  return special_characters_map;
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
      static const std::wstring special_characters = L"()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
      static const std::map<int, std::wstring> special_characters_map = make_special_characters_map(special_characters);

      if (special_characters.find(c) != std::wstring::npos) {
        result_string += special_characters_map.at(static_cast<int>(c));
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

/// Returns true if the input path matche the glob pattern
bool fnmatch(const fs::path &name, const std::wstring &pattern) {
  return std::regex_match(name.wstring(), compile_pattern(pattern));
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

std::vector<PathInfo> iter_directory(const fs::path &dirname, bool dironly,
                                     const std::function<void()> &onFilesystemTraversalProgress) {
  std::vector<PathInfo> result;

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
            result.emplace_back(entry.path(), entry.status());
          } else {
            result.emplace_back(fs::relative(entry.path()), entry.status());
          }
        }
        onFilesystemTraversalProgress();
      }
    } catch (std::exception&) {
      // not a directory
      // do nothing
    }
  }

  return result;
}

// Recursively yields relative pathnames inside a literal directory.
std::vector<PathInfo> rlistdir(const fs::path &dirname, bool dironly,
                               const std::function<void()> &onFilesystemTraversalProgress) {
  std::vector<PathInfo> result;
  auto infos = iter_directory(dirname, dironly, onFilesystemTraversalProgress);
  for (auto &x : infos) {
    if (!is_hidden(x.path.wstring())) {
      result.push_back(x);
      for (auto &y : rlistdir(x.path, dironly, onFilesystemTraversalProgress)) {
        result.push_back(y);
      }
    }
  }
  return result;
}

// This helper function recursively yields relative pathnames inside a literal
// directory.
std::vector<PathInfo> glob2(const PathInfo &dirinfo, [[maybe_unused]] const fs::path &pattern,
                            bool dironly,
                            const std::function<void()> &onFilesystemTraversalProgress) {
  // std::cout << "In glob2\n";
  std::vector<PathInfo> result{{".", dirinfo.status}};
  assert(is_recursive(pattern.wstring()));
  for (auto &dir : rlistdir(dirinfo.path, dironly, onFilesystemTraversalProgress)) {
    result.push_back(dir);
  }
  return result;
}

// These 2 helper functions non-recursively glob inside a literal directory.
// They return a list of basenames.  _glob1 accepts a pattern while _glob0
// takes a literal basename (so it only has to check for its existence).

std::vector<PathInfo> glob1(const PathInfo &dirinfo, const fs::path &pattern,
                            bool dironly,
                            const std::function<void()> &onFilesystemTraversalProgress) {
  // std::cout << "In glob1\n";
  auto infos = iter_directory(dirinfo.path, dironly, onFilesystemTraversalProgress);
  std::vector<PathInfo> result;
  for (auto &info : infos) {
    if (!is_hidden(info.path.wstring())) {
      if (fnmatch(info.path.filename(), pattern.wstring()))
        result.push_back(info);
    }
  }
  return result;
}

std::vector<PathInfo> glob0(const PathInfo &dirinfo, const fs::path &basename,
                            bool /*dironly*/, 
                            const std::function<void()> &onFilesystemTraversalProgress) {
  // std::cout << "In glob0\n";
  std::vector<PathInfo> result;
  if (basename.empty()) {
    // 'q*x/' should match only directories.
    if (fs::is_directory(dirinfo.status)) {
      result = {{basename, dirinfo.status}};
    }
  } else {
    if (fs::file_status status = fs::status(dirinfo.path / basename); fs::exists(status)) {
      result = {{basename, status}};
    }
  }
  onFilesystemTraversalProgress();  
  return result;
}

std::vector<PathInfo> glob(const fs::path &inpath, 
                           const std::function<void()> &onFilesystemTraversalProgress,
                           bool recursive = false,
                           bool dironly = false) {
  std::vector<PathInfo> result;

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
      if (fs::file_status status = fs::status(path); fs::exists(status)) {
        result.emplace_back(path, status);
      }
    } else {
      // Patterns ending with a slash should match only directories
      if (fs::file_status status = fs::status(dirname); fs::is_directory(dirname)) {
        result.emplace_back(path, status);
      }
    }
    return result;
  }

  if (dirname.empty()) {
    PathInfo dirinfo{dirname, fs::status(dirname)};
    if (recursive && is_recursive(basename.wstring())) {
      return glob2(dirinfo, basename, dironly, onFilesystemTraversalProgress);
    } else {
      return glob1(dirinfo, basename, dironly, onFilesystemTraversalProgress);
    }
  }

  std::vector<PathInfo> dirinfos;
  if (dirname != fs::path(pathname) && has_magic(dirname.wstring())) {
    dirinfos = glob(dirname, onFilesystemTraversalProgress, recursive, true);
  } else {
    dirinfos = {{dirname, fs::status(dirname)}};
  }

  std::function<std::vector<PathInfo>(const PathInfo &, const fs::path &, bool, const std::function<void()> &)>
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

  for (auto &dirinfo : dirinfos) {
    for (auto &info : glob_in_dir(dirinfo, basename, dironly, onFilesystemTraversalProgress)) {
      PathInfo subresult = info;
      if (info.path.parent_path().empty()) {
        subresult.path = dirinfo.path / info.path;
        subresult.status = fs::status(subresult.path);
      }
      subresult.path = subresult.path.lexically_normal();
      result.push_back(std::move(subresult));
    }
  }

  return result;
}

} // namespace end

std::vector<PathInfo> glob(const std::wstring &pathname, 
                           const std::function<void()> &onFilesystemTraversalProgress) {
  return glob(pathname, onFilesystemTraversalProgress, false);
}

std::vector<PathInfo> rglob(const std::wstring &pathname,
                            const std::function<void()> &onFilesystemTraversalProgress) {
  return glob(pathname, onFilesystemTraversalProgress, true);
}

std::vector<PathInfo> glob(const std::vector<std::wstring> &pathnames,
                           const std::function<void()> &onFilesystemTraversalProgress) {
  std::vector<PathInfo> result;
  for (auto &pathname : pathnames) {
    for (auto &match : glob(pathname, onFilesystemTraversalProgress, false)) {
      result.push_back(std::move(match));
    }
  }
  return result;
}

std::vector<PathInfo> rglob(const std::vector<std::wstring> &pathnames,
                            const std::function<void()> &onFilesystemTraversalProgress) {
  std::vector<PathInfo> result;
  for (auto &pathname : pathnames) {
    for (auto &match : glob(pathname, onFilesystemTraversalProgress, true)) {
      result.push_back(std::move(match));
    }
  }
  return result;
}

std::vector<PathInfo>
glob(const std::initializer_list<std::wstring> &pathnames,
     const std::function<void()> &onFilesystemTraversalProgress) {
  return glob(std::vector<std::wstring>(pathnames), onFilesystemTraversalProgress);
}

std::vector<PathInfo>
rglob(const std::initializer_list<std::wstring> &pathnames,
      const std::function<void()> &onFilesystemTraversalProgress) {
  return rglob(std::vector<std::wstring>(pathnames), onFilesystemTraversalProgress);
}

} // namespace glob
