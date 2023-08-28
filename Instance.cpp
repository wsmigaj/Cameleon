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
#include "Instance.h"
#include "RuntimeError.h"
#include "WildcardPatterns.h"

#include <glob/glob.h>

#include <regex>

namespace
{
struct MatchingPath
{
  std::filesystem::path path;
  std::vector<std::wstring> magicExpressionMatches;
};

struct PatternMatches
{
  size_t numMagicExpressions = 0;
  std::vector<MatchingPath> matchingPaths;
};

PatternMatches matchWildcardPattern(const std::wstring& pattern)
{
  PatternMatches matches;

  const std::vector<std::filesystem::path> globResults = glob::rglob(pattern);

  const std::wregex patternAsRegex(wildcardPatternToRegex(pattern));
  matches.numMagicExpressions = patternAsRegex.mark_count();
  for (const std::filesystem::path& path : globResults)
  {
    const std::wstring pathAsString = path.wstring();
    std::wsmatch match;
    std::vector<std::wstring> magicExpressionMatches;
    if (std::regex_match(pathAsString, match, patternAsRegex))
    {
      if (match.size() != matches.numMagicExpressions + 1)
      {
        throw RuntimeError(QString::fromStdWString(L"Internal error: the path '" + pathAsString +
                                                   L"' did not match all magic expressions"));
      }
      for (std::size_t i = 1; i < match.size(); ++i)
      {
        magicExpressionMatches.push_back(match[i].str());
      }
    }
    else
    {
      throw RuntimeError(
        QString::fromStdWString(L"Internal error: the path '" + pathAsString +
                                L"' unexpectedly did not match a regular expression."));
    }
    matches.matchingPaths.push_back(MatchingPath{path, std::move(magicExpressionMatches)});
  }

  return matches;
}

void sortInstances(std::vector<Instance>& instances)
{
  const size_t numInstances = instances.size();

  QCollator collator;
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  collator.setNumericMode(true);
  auto lessThan = [&collator](const Instance& va, const Instance& vb)
  {
    return std::lexicographical_compare(
      va.magicExpressionMatches.begin(), va.magicExpressionMatches.end(),
      vb.magicExpressionMatches.begin(), vb.magicExpressionMatches.end(), collator);
  };
  std::sort(instances.begin(), instances.end(), lessThan);
}
} // namespace

std::vector<Instance> findInstances(const std::vector<QString>& patterns)
{
  std::size_t numMagicExpressions = 0;
  for (const QString& pattern : patterns)
  {
    const std::wregex patternAsRegex(wildcardPatternToRegex(pattern.toStdWString()));
    const std::size_t markCount = patternAsRegex.mark_count();
    if (markCount > 0)
    {
      if (numMagicExpressions == 0)
      {
        numMagicExpressions = markCount;
      }
      else if (markCount != numMagicExpressions)
      {
        throw RuntimeError("The number of wildcard expressions must be the same in all patterns "
                           "containing any such expressions.");
      }
    }
  }

  std::vector<PatternMatches> matchesByPattern;
  std::transform(patterns.begin(), patterns.end(), std::back_inserter(matchesByPattern),
                 [](const QString& pattern)
                 { return matchWildcardPattern(pattern.toStdWString()); });

  using StringsToIndexMap = std::map<std::vector<std::wstring>, std::size_t>;
  StringsToIndexMap magicExpressionMatchIndex;
  for (const PatternMatches& patternMatches : matchesByPattern)
  {
    if (patternMatches.numMagicExpressions != 0)
    {
      for (const MatchingPath& path : patternMatches.matchingPaths)
      {
        magicExpressionMatchIndex[path.magicExpressionMatches] = 0;
      }
    }
  }

  {
    std::size_t i = 0;
    for (auto& kv : magicExpressionMatchIndex)
    {
      kv.second = i++;
    }
  }

  std::vector<Instance> instances;
  if (numMagicExpressions == 0)
  {
    std::vector<QString> paths;
    for (const PatternMatches& patternMatches : matchesByPattern)
    {
      if (patternMatches.matchingPaths.empty())
      {
        paths.push_back(QString());
      }
      else
      {
        paths.push_back(
          QString::fromStdWString(patternMatches.matchingPaths.front().path.wstring()));
      }
    }
    instances.push_back(Instance{paths, {}});
  }
  else
  {
    const size_t numPatterns = patterns.size();
    std::transform(magicExpressionMatchIndex.begin(), magicExpressionMatchIndex.end(),
                   std::back_inserter(instances),
                   [numPatterns](const StringsToIndexMap::value_type& matchAndIndex)
                   {
                     std::vector<QString> matches;
                     std::transform(matchAndIndex.first.begin(), matchAndIndex.first.end(),
                                    std::back_inserter(matches), &QString::fromStdWString);
                     return Instance{std::vector<QString>(numPatterns), matches};
                   });
    for (size_t iPattern = 0; iPattern < numPatterns; ++iPattern)
    {
      const PatternMatches& patternMatches = matchesByPattern[iPattern];
      if (patternMatches.numMagicExpressions == 0)
      {
        if (!patternMatches.matchingPaths.empty())
        {
          const QString path =
            QString::fromStdWString(patternMatches.matchingPaths.front().path.wstring());
          for (Instance& instance : instances)
            instance.paths[iPattern] = path;
        }
      }
      else
      {
        for (const MatchingPath& matchingPath : patternMatches.matchingPaths)
        {
          const std::size_t iInstance =
            magicExpressionMatchIndex.at(matchingPath.magicExpressionMatches);
          instances[iInstance].paths[iPattern] =
            QString::fromStdWString(matchingPath.path.wstring());
        }
      }
    }

    sortInstances(instances);
  }

  return instances;
}
