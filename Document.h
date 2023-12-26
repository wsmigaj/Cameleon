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

#pragma once

#include "Instance.h"
#include "Layout.h"

#include <QString>

#include <vector>

class PatternMatchingResult;

class Document : public QObject
{
  Q_OBJECT

public:
  Document();
  explicit Document(
    const QString& path, const std::function<void()>& onFilesystemTraversalProgress = []() {});
  Document(const Document&) = delete;
  Document(Document&&) = default;
  Document& operator=(const Document&) = delete;
  Document& operator=(Document&&) = default;
  ~Document() override;

  const QString& path() const { return path_; }

  Layout layout() const { return layout_; }
  void setLayout(const Layout& layout);

  const std::vector<QString>& patterns() const { return patterns_; }
  void setPatterns(
    std::vector<QString> patterns,
    const std::function<void()>& onFilesystemTraversalProgress = []() {});

  const std::vector<QString>& captionTemplates() const { return captionTemplates_; }
  void setCaptionTemplates(std::vector<QString> captionTemplates);

  std::vector<QString> captions(size_t instance) const;

  bool modified() const { return modified_; }

  void regenerateInstances(const std::function<void()>& onFilesystemTraversalProgress = []() {});

  const std::vector<Instance>& instances() const { return instances_; }

  QJsonObject toJson() const;
  void loadFromJson(
    const QJsonObject& json, const std::function<void()>& onFilesystemTraversalProgress = []() {});

  void save(const QString& path);

signals:
  void modificationStatusChanged();

private:
  QString path_;
  Layout layout_ = Layout{0, 0};
  std::vector<QString> patterns_;
  std::vector<QString> captionTemplates_;

  bool modified_ = false;
  std::vector<std::shared_ptr<PatternMatchingResult>> patternMatchingResults_;
  std::vector<Instance> instances_;
};

std::optional<int> findInstance(const Document& doc, const std::vector<QString>& key);
