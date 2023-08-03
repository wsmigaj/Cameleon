// This file is part of Caméléon.
//
// Copyright (C) 2023 Wojciech Śmigaj
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

#include "stdafx.h"
#include "WildcardPatterns.h"

#include <filesystem>
#include <regex>

bool replaceFirstMatch(std::string& str, const std::string& from, const std::string& to)
{
  std::size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::size_t replaceAllMatches(std::string& str, const std::string& from, const std::string& to)
{
  std::size_t num_replacements = 0;
  std::size_t start_pos = str.find(from);
  while (start_pos != std::string::npos)
  {
    str.replace(start_pos, from.length(), to);
    ++num_replacements;
    start_pos += to.length();
    start_pos = str.find(from, start_pos);
  }
  return num_replacements;
}

std::string wildcardPatternToRegex(const std::string& pattern)
{
  static const std::string separator =
    std::filesystem::path::preferred_separator == '\\' ? std::string{R"(\\)"} : std::string{"/"};
  static const std::string any_character_except_separator = "[^" + separator + "]";

  std::size_t i = 0, n = pattern.size();
  std::string result_string;

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
        result_string += "(.*)";
      }
      else
      {
        // single asterisk
        result_string += "(" + any_character_except_separator + "*)";
      }
    }
    else if (c == '?')
    {
      result_string += "(" + any_character_except_separator + ")";
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
        result_string += "\\[";
      }
      else
      {
        auto stuff = std::string(pattern.begin() + i, pattern.begin() + j);
        if (stuff.find("--") == std::string::npos)
        {
          // NOTE(wsmigaj): Directory separators within character classes won't work.
          replaceFirstMatch(stuff, std::string{"\\"}, std::string{R"(\\)"});
        }
        else
        {
          std::vector<std::string> chunks;
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
            k = pattern.find("-", k, j);
            if (k == std::string::npos)
            {
              break;
            }
            chunks.push_back(std::string(pattern.begin() + i, pattern.begin() + k));
            i = k + 1;
            k = k + 3;
          }

          chunks.push_back(std::string(pattern.begin() + i, pattern.begin() + j));
          // Escape backslashes and hyphens for set difference (--).
          // Hyphens that create ranges shouldn't be escaped.
          bool first = true;
          for (auto& s : chunks)
          {
            replaceFirstMatch(s, std::string{"\\"}, std::string{R"(\\)"});
            replaceFirstMatch(s, std::string{"-"}, std::string{R"(\-)"});
            if (first)
            {
              stuff += s;
              first = false;
            }
            else
            {
              stuff += "-" + s;
            }
          }
        }

        // Escape set operations (&&, ~~ and ||).
        std::string result;
        std::regex_replace(std::back_inserter(result),          // ressult
                           stuff.begin(), stuff.end(),          // string
                           std::regex(std::string{R"([&~|])"}), // pattern
                           std::string{R"(\\\1)"});             // repl
        stuff = result;
        i = j + 1;
        if (stuff[0] == '!')
        {
          stuff = "^" + std::string(stuff.begin() + 1, stuff.end());
        }
        else if (stuff[0] == '^' || stuff[0] == '[')
        {
          stuff = "\\\\" + stuff;
        }
        result_string = result_string + "([" + stuff + "])";
      }
    }
    else
    {
      // SPECIAL_CHARS
      // closing ')', '}' and ']'
      // '-' (a range in character set)
      // '&', '~', (extended character set operations)
      // '#' (comment) and WHITESPACE (ignored) in verbose mode
      static std::string special_characters = "()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
      static std::map<int, std::string> special_characters_map;
      if (special_characters_map.empty())
      {
        for (auto& sc : special_characters)
        {
          special_characters_map.insert(
            std::make_pair(static_cast<int>(sc), std::string{"\\"} + std::string(1, sc)));
        }
      }

      if (special_characters.find(c) != std::string::npos)
      {
        result_string += special_characters_map[static_cast<int>(c)];
      }
      else
      {
        result_string += c;
      }
    }
  }
  return std::string{"(?:(?:"} + result_string + std::string{R"()|[\r\n])$)"};
}