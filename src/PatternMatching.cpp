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

#include "PatternMatching.h"
#include "PatternUtils.h"
#include "RuntimeError.h"

#include <glob/glob.h>

#include <regex>

bool operator==(const PatternMatch& a, const PatternMatch& b)
{
  return a.magicExpressionMatches == b.magicExpressionMatches && a.path == b.path;
}

bool operator!=(const PatternMatch& a, const PatternMatch& b)
{
  return !(a == b);
}

bool operator==(const PatternMatchingResult& a, const PatternMatchingResult& b)
{
  return a.numMagicExpressions == b.numMagicExpressions && a.patternMatches == b.patternMatches;
}

bool operator!=(const PatternMatchingResult& a, const PatternMatchingResult& b)
{
  return !(a == b);
}

PatternMatchingResult matchPattern(const QString& pattern,
                                   const std::function<void()>& onFilesystemTraversalProgress)
{
  PatternMatchingResult result;

  const QString nativePattern = QDir::toNativeSeparators(pattern);
  const std::wstring patternAsStdWString = nativePattern.toStdWString();

  const std::vector<glob::PathInfo> globResults =
    glob::rglob(patternAsStdWString, onFilesystemTraversalProgress);

  const std::wregex patternAsRegex(wildcardPatternToRegex(patternAsStdWString));
  result.numMagicExpressions = patternAsRegex.mark_count();
  for (const glob::PathInfo& info : globResults)
  {
    if (fs::is_directory(info.status))
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

bool allPatternsContainSameNumberOfMagicExpressionsOrNone(const std::vector<QString>& patterns)
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
        return false;
      }
    }
  }
  return true;
}

void checkAllPatternsContainSameNumberOfMagicExpressionsOrNone(const std::vector<QString>& patterns)
{
  if (!allPatternsContainSameNumberOfMagicExpressionsOrNone(patterns))
    throw RuntimeError("The number of wildcard patterns must be the same in all paths "
                       "containing any such patterns.");
}

std::vector<std::shared_ptr<PatternMatchingResult>>
matchPatterns(const std::vector<QString>& patterns,
              const std::function<void()>& onFilesystemTraversalProgress)
{
  std::vector<std::shared_ptr<PatternMatchingResult>> results;
  std::transform(patterns.begin(), patterns.end(), std::back_inserter(results),
                 [&onFilesystemTraversalProgress](const QString& pattern)
                 {
                   return std::make_shared<PatternMatchingResult>(
                     matchPattern(pattern, onFilesystemTraversalProgress));
                 });
  return results;
}

std::vector<std::shared_ptr<PatternMatchingResult>> matchPatternsReusingPreviousResults(
  const std::vector<QString>& patterns, const std::vector<QString>& previousPatterns,
  const std::vector<std::shared_ptr<PatternMatchingResult>>& previousResults,
  const std::function<void()>& onFilesystemTraversalProgress)
{
  std::vector<std::shared_ptr<PatternMatchingResult>> results;
  std::transform(patterns.begin(), patterns.end(), std::back_inserter(results),
                 [&](const QString& pattern)
                 {
                   if (auto previousPatternIt =
                         std::find(previousPatterns.begin(), previousPatterns.end(), pattern);
                       previousPatternIt != previousPatterns.end())
                   {
                     return *(previousResults.begin() +
                              (previousPatternIt - previousPatterns.begin()));
                   }
                   else
                   {
                     return std::make_shared<PatternMatchingResult>(
                       matchPattern(pattern, onFilesystemTraversalProgress));
                   }
                 });
  return results;
}
