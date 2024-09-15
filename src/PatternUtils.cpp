// This file is part of Caméléon.
//
// Copyright (C) 2023-2024 Wojciech Śmigaj
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "PatternUtils.h"

#include "ghc/fs_std_fwd.hpp"

#include <regex>

namespace
{
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
} // namespace

bool replaceFirstMatch(std::wstring& str, const std::wstring& from, const std::wstring& to)
{
  std::size_t start_pos = str.find(from);
  if (start_pos == std::wstring::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::size_t replaceAllMatches(std::wstring& str, const std::wstring& from, const std::wstring& to)
{
  std::size_t num_replacements = 0;
  std::size_t start_pos = str.find(from);
  while (start_pos != std::wstring::npos)
  {
    str.replace(start_pos, from.length(), to);
    ++num_replacements;
    start_pos += to.length();
    start_pos = str.find(from, start_pos);
  }
  return num_replacements;
}

std::wstring wildcardPatternToRegex(const std::wstring& pattern)
{
  static const std::wstring separator =
    fs::path::preferred_separator == '\\' ? std::wstring{LR"(\\)"} : std::wstring{L"/"};
  static const std::wstring any_character_except_separator = L"[^" + separator + L"]";

  std::size_t i = 0, n = pattern.size();
  std::wstring result_string;

  while (i < n)
  {
    auto c = pattern[i];
    i += 1;
    if (c == '*')
    {
      if (i < n && pattern[i] == '*')
      {
        // multiple consecutive asterisks
        i += 1;
        while (i < n && pattern[i] == '*')
        {
          i += 1;
        }
        result_string += L"(.*)";
      }
      else
      {
        // single asterisk
        result_string += L"(" + any_character_except_separator + L"*)";
      }
    }
    else if (c == '?')
    {
      result_string += L"(" + any_character_except_separator + L")";
    }
    else if (c == '[')
    {
      auto j = i;
      if (j < n && pattern[j] == '!')
      {
        j += 1;
      }
      if (j < n && pattern[j] == ']')
      {
        j += 1;
      }
      while (j < n && pattern[j] != ']')
      {
        j += 1;
      }
      if (j >= n)
      {
        result_string += L"\\[";
      }
      else
      {
        auto stuff = std::wstring(pattern.begin() + i, pattern.begin() + j);
        if (stuff.find(L"--") == std::wstring::npos)
        {
          // NOTE(wsmigaj): Directory separators within character classes won't work.
          replaceFirstMatch(stuff, std::wstring{L"\\"}, std::wstring{LR"(\\)"});
        }
        else
        {
          std::vector<std::wstring> chunks;
          std::size_t k = 0;
          if (pattern[i] == '!')
          {
            k = i + 2;
          }
          else
          {
            k = i + 1;
          }

          while (true)
          {
            k = pattern.find(L"-", k, j);
            if (k == std::wstring::npos)
            {
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
          for (auto& s : chunks)
          {
            replaceFirstMatch(s, std::wstring{L"\\"}, std::wstring{LR"(\\)"});
            replaceFirstMatch(s, std::wstring{L"-"}, std::wstring{LR"(\-)"});
            if (first)
            {
              stuff += s;
              first = false;
            }
            else
            {
              stuff += L"-" + s;
            }
          }
        }

        // Escape set operations (&&, ~~ and ||).
        std::wstring result;
        std::regex_replace(std::back_inserter(result),             // ressult
                           stuff.begin(), stuff.end(),             // string
                           std::wregex(std::wstring{LR"([&~|])"}), // pattern
                           std::wstring{LR"(\\\1)"});              // repl
        stuff = result;
        i = j + 1;
        if (stuff[0] == '!')
        {
          stuff = L"^" + std::wstring(stuff.begin() + 1, stuff.end());
        }
        else if (stuff[0] == '^' || stuff[0] == '[')
        {
          stuff = L"\\\\" + stuff;
        }
        result_string = result_string + L"([" + stuff + L"])";
      }
    }
    else
    {
      // SPECIAL_CHARS
      // closing ')', '}' and ']'
      // '-' (a range in character set)
      // '&', '~', (extended character set operations)
      // '#' (comment) and WHITESPACE (ignored) in verbose mode
      static const std::wstring special_characters = L"()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
      static const std::map<int, std::wstring> special_characters_map =
        make_special_characters_map(special_characters);

      if (special_characters.find(c) != std::wstring::npos)
      {
        result_string += special_characters_map.at(static_cast<int>(c));
      }
      else
      {
        result_string += c;
      }
    }
  }
  return std::wstring{L"(?:(?:"} + result_string + std::wstring{LR"()|[\r\n])$)"};
}