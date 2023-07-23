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

#include "ImageView.h"
#include "ui_MainWindow.h"

#include <QtWidgets/QMainWindow>

class Document;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

private slots:
  void on_actionNewComparison_triggered();
  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_actionZoom1to1_triggered();
  void onImageViewHorizontalScrollBarValueChanged(int value);
  void onImageViewVerticalScrollBarValueChanged(int value);
  void onImageViewTransformChanging();
  void onImageViewTransformChanged(QTransform transform);

private:
  void updateLayout(int numRows, int numColumns);
  void updateImageViews();

private:
  Ui::MainWindowClass ui_;
  std::unique_ptr<Document> doc_;

  QGridLayout* mainLayout_;
  std::vector<ImageView*> imageViews_;
  int numOngoingTransformUpdates_ = 0;
};
