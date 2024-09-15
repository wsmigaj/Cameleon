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

#pragma once

#include <QtWidgets/QMainWindow>

class Document;
class Layout;
class MainView;

namespace Ui
{
class MainWindowClass;
}

class QAction;
class QActionGroup;
class QComboBox;
class QGridLayout;
class QLabel;
class QMenu;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr, bool dontUseNativeDialogs = false,
                      bool dontPromptToRegisterFileType = false);
  ~MainWindow() override;

  bool eventFilter(QObject* obj, QEvent* event) override;

  const Document* document() const { return doc_.get(); }
  int instance() const { return instance_; }

  const MainView* mainView() const;

  void openDocument(const QString& path);

public slots:
  void processCommandLine();

protected:
  void closeEvent(QCloseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

private slots:
  void on_actionNewAlbum_triggered();
  void on_actionOpenAlbum_triggered();
  void on_actionEditAlbum_triggered();
  void on_actionRefreshAlbum_triggered();
  void on_actionUseRelativePathsInSavedAlbum_triggered(bool checked);
  void on_actionSaveAlbum_triggered();
  void on_actionSaveAlbumAs_triggered();
  void on_actionCloseAlbum_triggered();
  void on_actionQuit_triggered();

  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_actionZoom1to1_triggered();
  void on_actionSaveScreenshot_triggered();
  void on_actionSaveAllScreenshots_triggered();
  void on_actionEditCaptions_triggered();

  void on_actionFirstInstance_triggered();
  void on_actionPreviousInstance_triggered();
  void on_actionNextInstance_triggered();
  void on_actionLastInstance_triggered();

  void on_actionBookmarkPage_triggered(bool checked);
  void on_actionRemoveAllBookmarks_triggered();
  void on_actionFirstBookmark_triggered();
  void on_actionPreviousBookmark_triggered();
  void on_actionNextBookmark_triggered();
  void on_actionLastBookmark_triggered();
  void on_actionImportBookmarks_triggered();
  void on_actionExportBookmarks_triggered();

  void on_actionRegisterFileType_triggered();
  void on_actionUnregisterFileType_triggered();

  void on_actionTutorial_triggered();
  void on_actionAboutCameleon_triggered();

  void onLayoutActionTriggered();
  void onRecentDocumentActionTriggered();

  void onDocumentModificationStatusChanged();
  void onInstanceComboBox(int currentIndex);

  void onMouseLeftImage();
  void onMouseMovedOverImage(QPoint pixelCoords, QColor pixelColour);

private:
  void connectDocumentSignals();

  void populateInstanceComboBox();

  void initialiseRecentDocumentsSubmenu();
  void prependToRecentDocuments(const QString& path);
  void populateRecentDocumentsSubmenu(const QStringList& recentAlbums);

  void updateMainViewLayout();
  void populateLayoutSubmenu();
  void updateLayoutSubmenu();

  void updateDocumentDependentUiElements();
  void updateDocumentDependentActions();
  void updateDocumentDependentWidgets();
  void updateDocumentModificationStatusDependentActions();
  void updateInstanceDependentUiElements();
  void updateInstanceDependentActions();
  void updateInstanceDependentWidgets();
  void updateBookmarkDependentActions();

  void onDocumentPathChanged();
  void onInstancesChanged();
  void onActiveInstanceChanged();
  void onCaptionTemplatesChanged();
  void onBookmarksChanged();

  std::optional<std::vector<QString>> currentInstanceKey() const;
  void goToInstance(int instance);

  bool maybeSaveDocument();
  bool saveDocument();
  bool saveDocumentAs();
  bool saveDocument(const QString& path);

  static bool isFileTypeRegistered();
  void maybePromptToRegisterFileType();

  QRect toolBarAreaRect() const;

  static QString statusBarInstanceLabelText(int currentInstance, int numInstances);
  static QString statusBarPixelLabelText(const QPoint& pt, const QColor& colour);

private:
  static const size_t MAX_NUM_RECENT_COMPARISONS = 9;
  static const auto BOOKMARK_COLOUR = Qt::blue;

private:
  std::unique_ptr<Ui::MainWindowClass> ui_;
  bool dontUseNativeDialogs_;
  QComboBox* instanceComboBox_ = nullptr;
  QMenu* layoutMenu_ = nullptr;
  QActionGroup* layoutActionGroup_ = nullptr;
  std::map<QAction*, Layout> layoutActions_;
  QMenu* recentDocumentsMenu_ = nullptr;

  QLabel* statusBarMessageLabel_ = nullptr;
  QLabel* statusBarInstanceLabel_ = nullptr;
  QLabel* statusBarPixelLabel_ = nullptr;

  std::unique_ptr<Document> doc_;
  int instance_ = 0;
};
