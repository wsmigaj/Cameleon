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

Document::Document()
{
}

Document::Document(const QString& path, const std::function<void()>& onFilesystemTraversalProgress)
  : path_(path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
  {
    throw RuntimeError("Couldn't open file " + path + " for reading.");
  }

  QByteArray saveData = file.readAll();

  QJsonDocument jsonDoc(QJsonDocument::fromJson(saveData));
  loadFromJson(jsonDoc.object(), onFilesystemTraversalProgress);
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
    instances_ = findInstances(patterns, onFilesystemTraversalProgress);
    patterns_ = std::move(patterns);
    captions_.resize(patterns_.size(), "%f");
    modified_ = true;
    modificationStatusChanged();
  }
}

void Document::setCaptions(std::vector<QString> captions)
{
  if (captions.size() != patterns_.size())
    throw RuntimeError("The number of captions must match the number of patterns.");

  if (captions != captions_)
  {
    captions_ = std::move(captions);
  }
}

void Document::regenerateInstances(const std::function<void()>& onFilesystemTraversalProgress)
{
  instances_ = findInstances(patterns_, onFilesystemTraversalProgress);
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

void Document::loadFromJson(const QJsonObject& json,
                            const std::function<void()>& onFilesystemTraversalProgress)
{
  QJsonObject jsonLayout = json["layout"].toObject();
  // TODO: (Graceful) error handling
  setLayout(Layout{static_cast<size_t>(jsonLayout["rows"].toInt()),
                   static_cast<size_t>(jsonLayout["columns"].toInt())});
  QJsonArray jsonPatterns = json["patterns"].toArray();
  std::vector<QString> patterns;
  std::transform(jsonPatterns.begin(), jsonPatterns.end(), std::back_inserter(patterns),
                 [](const QJsonValue& jsonPattern) { return jsonPattern.toString(); });
  setPatterns(std::move(patterns), onFilesystemTraversalProgress);

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
