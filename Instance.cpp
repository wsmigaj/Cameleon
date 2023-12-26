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
struct PatternMatch
{
  std::filesystem::path path;
  std::vector<std::wstring> magicExpressionMatches;
};

struct PatternMatchingResult
{
  size_t numMagicExpressions = 0;
  std::vector<PatternMatch> patternMatches;
};

using StringsToIndexMap = std::map<std::vector<std::wstring>, std::size_t>;

size_t countMagicExpressions(const std::vector<QString>& patterns)
{
  size_t numMagicExpressions = 0;
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
  return numMagicExpressions;
}

PatternMatchingResult
matchWildcardPattern(const std::wstring& pattern,
                     const std::function<void()>& onFilesystemTraversalProgress)
{
  PatternMatchingResult result;

  const std::vector<glob::PathInfo> globResults =
    glob::rglob(pattern, onFilesystemTraversalProgress);

  const std::wregex patternAsRegex(wildcardPatternToRegex(pattern));
  result.numMagicExpressions = patternAsRegex.mark_count();
  for (const glob::PathInfo& info : globResults)
  {
    if (std::filesystem::is_directory(info.status))
      continue;

    const std::wstring path = info.path.wstring();
    std::wsmatch match;
    std::vector<std::wstring> magicExpressionMatches;
    if (std::regex_match(path, match, patternAsRegex))
    {
      if (match.size() != result.numMagicExpressions + 1)
      {
        throw RuntimeError(QString::fromStdWString(L"Internal error: the path '" + path +
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
        QString::fromStdWString(L"Internal error: the path '" + path +
                                L"' unexpectedly did not match a regular expression."));
    }
    result.patternMatches.push_back(PatternMatch{path, std::move(magicExpressionMatches)});
  }

  return result;
}

StringsToIndexMap
enumerateUniqueMagicExpressionMatches(const std::vector<PatternMatchingResult>& results)
{
  StringsToIndexMap uniqueMagicExpressionMatchesIndex;
  for (const PatternMatchingResult& result : results)
  {
    if (result.numMagicExpressions != 0)
    {
      for (const PatternMatch& patternMatch : result.patternMatches)
      {
        uniqueMagicExpressionMatchesIndex[patternMatch.magicExpressionMatches] = 0;
      }
    }
  }

  {
    std::size_t i = 0;
    for (auto& [magicExpressionMatches, index] : uniqueMagicExpressionMatchesIndex)
    {
      index = i++;
    }
  }
  return uniqueMagicExpressionMatchesIndex;
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

std::vector<Instance> createInstances(std::size_t numMagicExpressions,
                                      const std::vector<PatternMatchingResult>& results,
                                      const StringsToIndexMap& uniqueMagicExpressionMatchesIndex)
{
  std::vector<Instance> instances;
  if (numMagicExpressions == 0)
  {
    std::vector<QString> paths;
    for (const PatternMatchingResult& result : results)
    {
      if (result.patternMatches.empty())
      {
        paths.push_back(QString());
      }
      else
      {
        paths.push_back(QString::fromStdWString(result.patternMatches.front().path.wstring()));
      }
    }
    instances.push_back(Instance{paths, {}});
  }
  else
  {
    const size_t numPatterns = results.size();
    std::transform(uniqueMagicExpressionMatchesIndex.begin(),
                   uniqueMagicExpressionMatchesIndex.end(), std::back_inserter(instances),
                   [numPatterns](const StringsToIndexMap::value_type& matchAndIndex)
                   {
                     std::vector<QString> matches;
                     std::transform(matchAndIndex.first.begin(), matchAndIndex.first.end(),
                                    std::back_inserter(matches), &QString::fromStdWString);
                     return Instance{std::vector<QString>(numPatterns), matches};
                   });
    for (size_t iPattern = 0; iPattern < numPatterns; ++iPattern)
    {
      const PatternMatchingResult& result = results[iPattern];
      if (result.numMagicExpressions == 0)
      {
        if (!result.patternMatches.empty())
        {
          const QString path =
            QString::fromStdWString(result.patternMatches.front().path.wstring());
          for (Instance& instance : instances)
            instance.paths[iPattern] = path;
        }
      }
      else
      {
        for (const PatternMatch& patternMatch : result.patternMatches)
        {
          const std::size_t iInstance =
            uniqueMagicExpressionMatchesIndex.at(patternMatch.magicExpressionMatches);
          instances[iInstance].paths[iPattern] =
            QString::fromStdWString(patternMatch.path.wstring());
        }
      }
    }
    sortInstances(instances);
  }
  return instances;
}
} // namespace

std::vector<Instance> findInstances(const std::vector<QString>& patterns,
                                    const std::function<void()>& onFilesystemTraversalProgress)
{
  const std::size_t numMagicExpressions = countMagicExpressions(patterns);

  std::vector<PatternMatchingResult> results;
  std::transform(
    patterns.begin(), patterns.end(), std::back_inserter(results),
    [&onFilesystemTraversalProgress](const QString& pattern)
    { return matchWildcardPattern(pattern.toStdWString(), onFilesystemTraversalProgress); });

  // Assign an index to each unique set of magic expression matches.
  const StringsToIndexMap uniqueMagicExpressionMatchesIndex =
    enumerateUniqueMagicExpressionMatches(results);

  return createInstances(numMagicExpressions, results, uniqueMagicExpressionMatchesIndex);
}