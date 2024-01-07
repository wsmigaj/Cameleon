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

#include "Instance.h"
#include "PatternMatching.h"
#include "RuntimeError.h"

namespace
{
using StringsToIndexMap = std::map<std::vector<std::wstring>, std::size_t>;

std::size_t numberOfMagicExpressions(
  const std::vector<std::shared_ptr<PatternMatchingResult>>& patternMatchingResults)
{
  if (patternMatchingResults.empty())
    return 0;

  if (std::any_of(patternMatchingResults.begin(), patternMatchingResults.end(),
                  [&patternMatchingResults](const std::shared_ptr<PatternMatchingResult>& result) {
                    return result->numMagicExpressions !=
                           patternMatchingResults.front()->numMagicExpressions;
                  }))
    throw RuntimeError("The number of wildcard expressions must be the same in all patterns "
                       "containing any such expressions.");

  return patternMatchingResults.front()->numMagicExpressions;
}

StringsToIndexMap enumerateUniqueMagicExpressionMatches(
  const std::vector<std::shared_ptr<PatternMatchingResult>>& results)
{
  StringsToIndexMap uniqueMagicExpressionMatchesIndex;
  for (const std::shared_ptr<PatternMatchingResult>& result : results)
  {
    if (result->numMagicExpressions != 0)
    {
      for (const PatternMatch& patternMatch : result->patternMatches)
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

std::vector<Instance>
createInstances(std::size_t numMagicExpressions,
                const std::vector<std::shared_ptr<PatternMatchingResult>>& results,
                const StringsToIndexMap& uniqueMagicExpressionMatchesIndex)
{
  std::vector<Instance> instances;
  if (numMagicExpressions == 0)
  {
    std::vector<QString> paths;
    for (const std::shared_ptr<PatternMatchingResult>& result : results)
    {
      if (result->patternMatches.empty())
      {
        paths.push_back(QString());
      }
      else
      {
        paths.push_back(QString::fromStdWString(result->patternMatches.front().path.wstring()));
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
      const std::shared_ptr<PatternMatchingResult>& result = results[iPattern];
      if (result->numMagicExpressions == 0)
      {
        if (!result->patternMatches.empty())
        {
          const QString path =
            QString::fromStdWString(result->patternMatches.front().path.wstring());
          for (Instance& instance : instances)
            instance.paths[iPattern] = path;
        }
      }
      else
      {
        for (const PatternMatch& patternMatch : result->patternMatches)
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

std::vector<Instance>
findInstances(const std::vector<std::shared_ptr<PatternMatchingResult>>& patternMatchingResults)
{
  const std::size_t numMagicExpressions = numberOfMagicExpressions(patternMatchingResults);

  // Assign an index to each unique set of magic expression matches.
  const StringsToIndexMap uniqueMagicExpressionMatchesIndex =
    enumerateUniqueMagicExpressionMatches(patternMatchingResults);

  return createInstances(numMagicExpressions, patternMatchingResults,
                         uniqueMagicExpressionMatchesIndex);
}