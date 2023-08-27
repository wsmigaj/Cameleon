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

protected:
  void closeEvent(QCloseEvent* event) override;

private slots:
  void on_actionNewComparison_triggered();
  void on_actionOpenComparison_triggered();
  void on_actionEditComparison_triggered();
  void on_actionRefreshComparison_triggered();
  void on_actionSaveComparison_triggered();
  void on_actionSaveComparisonAs_triggered();
  void on_actionCloseComparison_triggered();
  void on_actionQuit_triggered();
  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_actionZoom1to1_triggered();
  void on_actionSaveScreenshot_triggered();
  void on_actionFirstInstance_triggered();
  void on_actionPreviousInstance_triggered();
  void on_actionNextInstance_triggered();
  void on_actionLastInstance_triggered();
  void onLayoutActionTriggered();

  void onDocumentModificationStatusChanged();
  void onInstanceComboBox(int currentIndex);

private:
  void connectDocumentSignals();

  void populateLayoutSubmenu();
  void populateInstanceComboBox();

  void updateMainViewLayout();
  void updateLayoutSubmenu();

  void updateDocumentDependentActions();
  void updateDocumentModificationStatusDependentActions();
  void updateInstanceDependentActions();

  void onDocumentPathChanged();
  void onInstancesChanged();
  void onActiveInstanceChanged();

  void goToInstance(int instance);

  bool maybeSaveDocument();
  bool saveDocument();
  bool saveDocumentAs();
  bool saveDocument(const QString& path);

  QRect toolBarAreaRect() const;

private:
  Ui::MainWindowClass ui_;
  QComboBox* instanceComboBox_ = nullptr;
  QMenu* layoutMenu_ = nullptr;
  QActionGroup* layoutActionGroup_ = nullptr;
  std::map<QAction*, Layout> layoutActions_;
  QGridLayout* mainLayout_ = nullptr;
  std::unique_ptr<Document> doc_;
  int instance_ = 0;
};
