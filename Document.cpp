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
#include "Document.h"
#include "RuntimeError.h"
#include "WildcardPatterns.h"

#include <glob/glob.h>

#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

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
} // namespace

Document::Document()
{
}

Document::Document(const QString& path) : path_(path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
  {
    throw RuntimeError("Couldn't open file " + path + "for reading.");
  }

  QByteArray saveData = file.readAll();

  QJsonDocument jsonDoc(QJsonDocument::fromJson(saveData));
  loadFromJson(jsonDoc.object());
}

void Document::setLayout(const Layout& layout)
{
  if (layout != layout_)
  {
    layout_ = layout;
    modified_ = true;
    modificationStatusChanged();
  }
}

void Document::setPatterns(std::vector<QString> patterns)
{
  if (patterns != patterns_)
  {
    // TODO: Make this exception-safe. Probably the easiest way will be to move
    // the instance data (instances_ and magicExpressionMatches_) into a separate class.
    patterns_ = std::move(patterns);
    modified_ = true;
    regenerateInstances();
    modificationStatusChanged();
  }
}

void Document::regenerateInstances()
{
  std::size_t numMagicExpressions = 0;
  for (const QString& pattern : patterns_)
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
        throw RuntimeError("Any patterns with wildcard expressions must contain the same number of "
                           "wildcard expressions.");
      }
    }
  }

  std::vector<PatternMatches> matchesByPattern;
  std::transform(patterns_.begin(), patterns_.end(), std::back_inserter(matchesByPattern),
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

  instances_.clear();
  magicExpressionMatches_.clear();
  if (numMagicExpressions == 0)
  {
    std::vector<QString> instance;
    for (const PatternMatches& patternMatches : matchesByPattern)
    {
      if (patternMatches.matchingPaths.empty())
      {
        instance.push_back(QString());
      }
      else
      {
        instance.push_back(
          QString::fromStdWString(patternMatches.matchingPaths.front().path.wstring()));
      }
    }
    instances_.push_back(std::move(instance));
    magicExpressionMatches_.push_back({});
  }
  else
  {
    instances_.assign(magicExpressionMatchIndex.size(), std::vector<QString>(patterns_.size()));
    std::transform(magicExpressionMatchIndex.begin(), magicExpressionMatchIndex.end(),
                   std::back_inserter(magicExpressionMatches_),
                   [](const StringsToIndexMap::value_type& matchAndIndex)
                   {
                     std::vector<QString> match;
                     std::transform(matchAndIndex.first.begin(), matchAndIndex.first.end(),
                                    std::back_inserter(match), &QString::fromStdWString);
                     return match;
                   });
    for (size_t iPattern = 0; iPattern < patterns_.size(); ++iPattern)
    {
      const PatternMatches& patternMatches = matchesByPattern[iPattern];
      if (patternMatches.numMagicExpressions == 0)
      {
        if (!patternMatches.matchingPaths.empty())
        {
          const QString path =
            QString::fromStdWString(patternMatches.matchingPaths.front().path.wstring());
          for (std::vector<QString>& instance : instances_)
            instance[iPattern] = path;
        }
      }
      else
      {
        for (const MatchingPath& matchingPath : patternMatches.matchingPaths)
        {
          const std::size_t iInstance =
            magicExpressionMatchIndex.at(matchingPath.magicExpressionMatches);
          instances_[iInstance][iPattern] = QString::fromStdWString(matchingPath.path.wstring());
        }
      }
    }

    sortInstances();
  }
}

void Document::sortInstances()
{
  if (magicExpressionMatches_.size() != instances_.size())
    throw RuntimeError("Internal error: unexpected number of instances");

  const size_t numInstances = instances_.size();
  std::vector<size_t> indices(instances_.size());
  std::iota(indices.begin(), indices.end(), 0);

  QCollator collator;
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  collator.setNumericMode(true);
  auto lessThan = [&collator](const std::vector<QString>& va, const std::vector<QString>& vb)
  { return std::lexicographical_compare(va.begin(), va.end(), vb.begin(), vb.end(), collator); };
  std::sort(indices.begin(), indices.end(),
            [&](size_t a, size_t b)
            { return lessThan(magicExpressionMatches_[a], magicExpressionMatches_[b]); });

  std::vector<std::vector<QString>> sortedMagicExpressionMatches;
  std::vector<std::vector<QString>> sortedInstances;
  sortedMagicExpressionMatches.reserve(magicExpressionMatches_.size());
  sortedInstances.reserve(instances_.size());
  for (size_t i : indices)
  {
    sortedMagicExpressionMatches.push_back(std::move(magicExpressionMatches_[i]));
    sortedInstances.push_back(std::move(instances_[i]));
  }
  magicExpressionMatches_ = std::move(sortedMagicExpressionMatches);
  instances_ = std::move(sortedInstances);
}

QJsonObject Document::toJson() const
{
  QJsonObject json;

  QJsonObject jsonLayout;
  jsonLayout["rows"] = static_cast<int>(layout_.rows);
  jsonLayout["columns"] = static_cast<int>(layout_.columns);
  json["layout"] = jsonLayout;

  QJsonArray jsonPatterns;
  std::copy(patterns_.begin(), patterns_.end(), std::back_inserter(jsonPatterns));
  json["patterns"] = jsonPatterns;

  return json;
}

void Document::loadFromJson(const QJsonObject& json)
{
  QJsonObject jsonLayout = json["layout"].toObject();
  // TODO: (Graceful) error handling
  setLayout(Layout{static_cast<size_t>(jsonLayout["rows"].toInt()),
                   static_cast<size_t>(jsonLayout["columns"].toInt())});
  QJsonArray jsonPatterns = json["patterns"].toArray();
  std::vector<QString> patterns;
  std::transform(jsonPatterns.begin(), jsonPatterns.end(), std::back_inserter(patterns),
                 [](const QJsonValue& jsonPattern) { return jsonPattern.toString(); });
  setPatterns(std::move(patterns));

  modified_ = false;
  modificationStatusChanged();
}

void Document::save(const QString& path)
{
  QFile file(path);

  if (!file.open(QIODevice::WriteOnly))
  {
    throw RuntimeError("Couldn't open file " + path + " for writing.");
  }

  if (file.write(QJsonDocument(toJson()).toJson()) < 0)
  {
    throw RuntimeError("An error occurred while saving the document: " + file.errorString() + ".");
  }

  path_ = path;
  modified_ = false;
  modificationStatusChanged();
}