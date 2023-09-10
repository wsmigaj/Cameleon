
#pragma once
#include <string>
#include <vector>

#ifdef GLOB_USE_GHC_FILESYSTEM
#include <ghc/filesystem.hpp>
#else
#include <filesystem>
#endif

namespace glob {

#ifdef GLOB_USE_GHC_FILESYSTEM
namespace fs = ghc::filesystem;
#else
namespace fs = std::filesystem;
#endif

/// \param pathname string containing a path specification
/// \return vector of paths that match the pathname
///
/// Pathnames can be absolute (/usr/src/Foo/Makefile) or relative (../../Tools/*/*.gif)
/// Pathnames can contain shell-style wildcards
/// Broken symlinks are included in the results (as in the shell)
std::vector<fs::path> glob(const std::wstring &pathname, 
                           const std::function<void()> &onFilesystemTraversalProgress = [](){});

/// \param pathnames string containing a path specification
/// \return vector of paths that match the pathname
///
/// Globs recursively.
/// The pattern “**” will match any files and zero or more directories, subdirectories and
/// symbolic links to directories.
std::vector<fs::path> rglob(const std::wstring &pathname, 
                            const std::function<void()> &onFilesystemTraversalProgress = [](){});

/// Runs `glob` against each pathname in `pathnames` and accumulates the results
std::vector<fs::path> glob(const std::vector<std::wstring> &pathnames, 
                           const std::function<void()> &onFilesystemTraversalProgress = [](){});

/// Runs `rglob` against each pathname in `pathnames` and accumulates the results
std::vector<fs::path> rglob(const std::vector<std::wstring> &pathnames, 
                            const std::function<void()> &onFilesystemTraversalProgress = [](){});

/// Initializer list overload for convenience
std::vector<fs::path> glob(const std::initializer_list<std::wstring> &pathnames, 
                           const std::function<void()> &onFilesystemTraversalProgress = [](){});

/// Initializer list overload for convenience
std::vector<fs::path> rglob(const std::initializer_list<std::wstring> &pathnames, 
                            const std::function<void()> &onFilesystemTraversalProgress = [](){});

/// Returns true if the input path matche the glob pattern
  bool fnmatch(const fs::path &name, const std::wstring &pattern);

} // namespace glob
