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

#include "Document.h"
#include "Constants.h"
#include "ContainerUtils.h"
#include "PatternMatching.h"
#include "RuntimeError.h"

namespace
{
const char* DEFAULT_CAPTION_TEMPLATE = "%p";

QString join(const std::vector<QString>& strings, const QString& sep = QString())
{
  QString result;
  if (!strings.empty())
    result = strings.front();
  for (size_t i = 1; i < strings.size(); ++i)
  {
    result += sep;
    result += strings[i];
  }
  return result;
}

std::vector<QString> jsonStringArrayToStringVector(const QJsonArray& jsonArray)
{
  std::vector<QString> stringVector;
  std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(stringVector),
                 [](const QJsonValue& x) { return x.toString(); });
  return stringVector;
}

QJsonArray stringVectorToJsonStringArray(const std::vector<QString>& stringVector)
{
  QJsonArray jsonArray;
  std::copy(stringVector.begin(), stringVector.end(), std::back_inserter(jsonArray));
  return jsonArray;
}
} // namespace

Document::Document()
{
}

Document::~Document() = default;

Document::Document(const QString& path, const std::function<void()>& onFilesystemTraversalProgress)
  : path_(path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
  {
    throw RuntimeError("Could not open file " + path + " for reading.");
  }

  QByteArray saveData = file.readAll();

  QJsonParseError error;
  QJsonDocument jsonDoc(QJsonDocument::fromJson(saveData, &error));
  if (jsonDoc.isNull())
  {
    throw RuntimeError("Could not parse file " + path + ": " + error.errorString() + ".");
  }
  initialiseFromJson(jsonDoc.object(), onFilesystemTraversalProgress);
  modified_ = false;
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

void Document::setPatterns(std::vector<QString> patterns,
                           const std::function<void()>& onFilesystemTraversalProgress)
{
  if (patterns != patterns_)
  {
    checkAllPatternsContainSameNumberOfMagicExpressionsOrNone(patterns);
    std::vector<std::shared_ptr<PatternMatchingResult>> patternMatchingResults;
    if (patternMatchingResults_.size() == patterns_.size())
    {
      patternMatchingResults = matchPatternsReusingPreviousResults(
        patterns, patterns_, patternMatchingResults_, onFilesystemTraversalProgress);
    }
    else
    {
      patternMatchingResults = matchPatterns(patterns, onFilesystemTraversalProgress);
    }

    const std::set<std::vector<QString>> oldBookmarkKeys = bookmarkKeys();
    std::vector<Instance> newInstances = findInstances(patternMatchingResults);
    std::set<size_t> newBookmarks = findInstanceIndices(newInstances, oldBookmarkKeys);

    instances_ = std::move(newInstances);
    bookmarks_ = std::move(newBookmarks);
    patternMatchingResults_ = std::move(patternMatchingResults);
    patterns_ = std::move(patterns);
    captionTemplates_.resize(patterns_.size(), DEFAULT_CAPTION_TEMPLATE);
    modified_ = true;
    modificationStatusChanged();
  }
}

void Document::setCaptionTemplates(std::vector<QString> captionTemplates)
{
  if (captionTemplates.size() != patterns_.size())
    throw RuntimeError("The number of captions must be the same as the number of panels.");

  if (captionTemplates != captionTemplates_)
  {
    captionTemplates_ = std::move(captionTemplates);
    modified_ = true;
    modificationStatusChanged();
  }
}

std::vector<QString> Document::captions(size_t instanceIndex) const
{
  if (instanceIndex >= instances_.size())
    throw RuntimeError("Invalid page index");
  const Instance& instance = instances_[instanceIndex];

  std::vector<QString> result = captionTemplates_;
  for (size_t i = 0; i < captionTemplates_.size(); ++i)
  {
    if (instance.paths[i].isEmpty())
      result[i] = QString();
    else
      result[i].replace(DEFAULT_CAPTION_TEMPLATE, instance.paths[i]);
  }
  return result;
}

std::set<std::vector<QString>> Document::bookmarkKeys() const
{
  std::set<std::vector<QString>> keys;
  std::transform(bookmarks_.begin(), bookmarks_.end(), std::inserter(keys, keys.end()),
                 [this](size_t index) { return instances_[index].magicExpressionMatches; });
  return keys;
}

void Document::addBookmark(size_t instanceIndex)
{
  if (instanceIndex >= instances_.size())
    throw RuntimeError("Invalid page index");

  if (contains(bookmarks_, instanceIndex))
    return; // Bookmark already exists

  bookmarks_.insert(instanceIndex);
  modified_ = true;
  modificationStatusChanged();
}

void Document::removeBookmark(size_t instanceIndex)
{
  if (instanceIndex >= instances_.size())
    throw RuntimeError("Invalid page index");

  auto it = bookmarks_.find(instanceIndex);
  if (it == bookmarks_.end())
    return; // Bookmark does not exist.

  bookmarks_.erase(it);
  modified_ = true;
  modificationStatusChanged();
}

void Document::toggleBookmark(size_t instanceIndex)
{
  if (instanceIndex >= instances_.size())
    throw RuntimeError("Invalid page index");

  auto it = bookmarks_.find(instanceIndex);
  if (it != bookmarks_.end())
    bookmarks_.erase(it);
  else
    bookmarks_.insert(instanceIndex);

  modified_ = true;
  modificationStatusChanged();
}

void Document::removeAllBookmarks()
{
  if (bookmarks_.empty())
    return;

  bookmarks_.clear();
  modified_ = true;
  modificationStatusChanged();
}

QString Document::instanceKey(size_t instanceIndex) const
{
  if (instanceIndex >= instances_.size())
    throw RuntimeError("Invalid page index");
  const Instance& instance = instances_[instanceIndex];
  return join(instance.magicExpressionMatches, "...");
}

void Document::regenerateInstances(const std::function<void()>& onFilesystemTraversalProgress)
{
  // This check may not be strictly necessary but better safe than sorry.
  checkAllPatternsContainSameNumberOfMagicExpressionsOrNone(patterns_);
  std::vector<std::shared_ptr<PatternMatchingResult>> patternMatchingResults =
    matchPatterns(patterns_, onFilesystemTraversalProgress);

  const std::set<std::vector<QString>> oldBookmarkKeys = bookmarkKeys();
  std::vector<Instance> newInstances = findInstances(patternMatchingResults);
  std::set<size_t> newBookmarks = findInstanceIndices(newInstances, oldBookmarkKeys);

  instances_ = std::move(newInstances);
  bookmarks_ = std::move(newBookmarks);
  patternMatchingResults_ = std::move(patternMatchingResults);
}

QJsonObject Document::toJson() const
{
  QJsonObject json;

  json["version"] = 1;
  {
    QJsonObject jsonLayout;
    jsonLayout["rows"] = static_cast<int>(layout_.rows);
    jsonLayout["columns"] = static_cast<int>(layout_.columns);
    json["layout"] = jsonLayout;
  }

  json["patterns"] = stringVectorToJsonStringArray(patterns_);
  json["captionTemplates"] = stringVectorToJsonStringArray(captionTemplates_);

  {
    QJsonArray jsonBookmarks;
    for (size_t i : bookmarks_)
      jsonBookmarks.push_back(stringVectorToJsonStringArray(instances_[i].magicExpressionMatches));
    json["bookmarks"] = jsonBookmarks;
  }

  return json;
}

void Document::initialiseFromJson(const QJsonObject& json,
                                  const std::function<void()>& onFilesystemTraversalProgress)
{
  {
    QJsonArray jsonPatterns = json["patterns"].toArray();
    std::vector<QString> patterns;
    std::transform(jsonPatterns.begin(), jsonPatterns.end(), std::back_inserter(patterns),
                   [](const QJsonValue& jsonPattern) { return jsonPattern.toString(); });
    if (patterns.size() > MAX_NUM_PATTERNS)
      patterns.resize(MAX_NUM_PATTERNS);
    setPatterns(std::move(patterns), onFilesystemTraversalProgress);
  }
  {
    QJsonObject jsonLayout = json["layout"].toObject();
    Layout layout{static_cast<size_t>(jsonLayout["rows"].toInt()),
                  static_cast<size_t>(jsonLayout["columns"].toInt())};
    if (layout.panels() < patterns_.size())
      layout = defaultLayout(patterns_.size());
    setLayout(layout);
  }
  if (json.contains("captionTemplates"))
  {
    QJsonArray jsonCaptionTemplates = json["captionTemplates"].toArray();
    std::vector<QString> captionTemplates;
    std::transform(jsonCaptionTemplates.begin(), jsonCaptionTemplates.end(),
                   std::back_inserter(captionTemplates),
                   [](const QJsonValue& jsonCaptionTemplate)
                   { return jsonCaptionTemplate.toString(); });
    captionTemplates.resize(patterns_.size(), DEFAULT_CAPTION_TEMPLATE);
    setCaptionTemplates(std::move(captionTemplates));
  }
  else
  {
    setCaptionTemplates(std::vector<QString>(patterns().size(), DEFAULT_CAPTION_TEMPLATE));
  }
  if (json.contains("bookmarks"))
  {
    QJsonArray jsonBookmarks = json["bookmarks"].toArray();
    std::set<std::vector<QString>> bookmarkKeys;
    std::transform(jsonBookmarks.begin(), jsonBookmarks.end(),
                   std::inserter(bookmarkKeys, bookmarkKeys.end()),
                   [](const QJsonValue& jsonBookmark)
                   { return jsonStringArrayToStringVector(jsonBookmark.toArray()); });
    bookmarks_ = findInstanceIndices(instances_, bookmarkKeys);
  }
}

void Document::save(const QString& path)
{
  QFile file(path);

  if (!file.open(QIODevice::WriteOnly))
  {
    throw RuntimeError("Could not open file " + path + " for writing.");
  }

  if (file.write(QJsonDocument(toJson()).toJson()) < 0)
  {
    throw RuntimeError("An error occurred while saving the document: " + file.errorString() + ".");
  }

  modified_ = false;
  path_ = path;
  modificationStatusChanged();
}

std::optional<int> findInstance(const Document& doc, const std::vector<QString>& key)
{
  auto matchingInstanceIt = std::find_if(doc.instances().begin(), doc.instances().end(),
                                         [&key](const Instance& instance)
                                         { return instance.magicExpressionMatches == key; });

  if (matchingInstanceIt == doc.instances().end())
    return std::nullopt;
  else
    return matchingInstanceIt - doc.instances().begin();
}

std::set<size_t> findInstanceIndices(const std::vector<Instance>& instances,
                                     const std::set<std::vector<QString>>& keys)
{
  std::set<size_t> result;
  if (!keys.empty())
    for (size_t i = 0; i < instances.size(); ++i)
      if (contains(keys, instances[i].magicExpressionMatches))
        result.insert(i);
  return result;
}

std::vector<QString> updateCaptionTemplates(const std::vector<QString>& previousCaptionTemplates,
                                            const std::vector<QString>& previousPatterns,
                                            const std::vector<QString>& newPatterns)
{
  const size_t previousNumPatterns = previousPatterns.size();
  const size_t newNumPatterns = newPatterns.size();
  if (previousCaptionTemplates.size() != previousNumPatterns)
  {
    throw RuntimeError("Internal error: mismatch between the number of patterns and the number of "
                       "caption templates");
  }

  std::vector<QString> newCaptionTemplates(newNumPatterns);
  std::vector<bool> isPreviousCaptionTemplateReused(previousNumPatterns, false);
  std::vector<bool> isNewCaptionTemplateReady(newNumPatterns, false);

  for (size_t patternIndex = 0; patternIndex < std::min(newNumPatterns, previousNumPatterns);
       ++patternIndex)
  {
    if (newPatterns[patternIndex] == previousPatterns[patternIndex])
    {
      newCaptionTemplates[patternIndex] = previousCaptionTemplates[patternIndex];
      isPreviousCaptionTemplateReused[patternIndex] = true;
      isNewCaptionTemplateReady[patternIndex] = true;
    }
  }

  for (size_t newPatternIndex = 0; newPatternIndex < newNumPatterns; ++newPatternIndex)
  {
    if (isNewCaptionTemplateReady[newPatternIndex])
      continue;

    for (size_t previousPatternIndex = 0; previousPatternIndex < previousNumPatterns;
         ++previousPatternIndex)
    {
      if (!isPreviousCaptionTemplateReused[previousPatternIndex] &&
          previousPatterns[previousPatternIndex] == newPatterns[newPatternIndex])
      {
        newCaptionTemplates[newPatternIndex] = previousCaptionTemplates[previousPatternIndex];
        isPreviousCaptionTemplateReused[previousPatternIndex] = true;
        isNewCaptionTemplateReady[newPatternIndex] = true;
        break;
      }
    }
  }

  for (size_t patternIndex = 0; patternIndex < newNumPatterns; ++patternIndex)
  {
    if (isNewCaptionTemplateReady[patternIndex])
      continue;

    if (patternIndex < previousNumPatterns && !isPreviousCaptionTemplateReused[patternIndex])
      newCaptionTemplates[patternIndex] = previousCaptionTemplates[patternIndex];
    else
      newCaptionTemplates[patternIndex] = DEFAULT_CAPTION_TEMPLATE;
  }

  return newCaptionTemplates;
}