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

#include "Layout.h"

#include <QWidget>

class Document;
class ImageView;

// enum class ForceReload {NO, YES};

class MainView : public QWidget
{
  Q_OBJECT

public:
  MainView(QWidget* parent);
  MainView(const MainView&) = delete;
  MainView& operator=(const MainView&) = delete;
  ~MainView() override;

  const Layout& layout() const { return layout_; }
  void setLayout(const Layout& layout);

  const std::vector<QString>& paths() const { return paths_; }
  void setPaths(std::vector<QString> paths);
  void clearPaths();

  void reloadImages();

  void zoom(double relativeScale);
  void resetScale();

private slots:
  void onImageViewHorizontalScrollBarValueChanged(int value);
  void onImageViewVerticalScrollBarValueChanged(int value);
  void onImageViewTransformChanging();
  void onImageViewTransformChanged(QTransform transform);

private:
  QGridLayout* mainLayout_;
  std::vector<ImageView*> imageViews_;

  Layout layout_ = Layout{0, 0};
  std::vector<QString> paths_;

  int numOngoingTransformUpdates_ = 0;
};
