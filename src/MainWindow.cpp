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

#include "AlbumEditorDialog.h"
#include "CancellationException.h"
#include "Constants.h"
#include "ContainerUtils.h"
#include "Document.h"
#include "MainWindow.h"
#include "PatternMatching.h"
#include "PatternMatchingProgressDialog.h"
#include "RuntimeError.h"
#include "Try.h"
#include "Version.h"
#include "ui_MainWindow.h"

#include <Qt>

namespace
{
QString instanceKeyToFileName(const QString& key)
{
  QString fileName = key;
  fileName.replace('/', "_");
  fileName.replace('\\', "_");
  return fileName;
}

bool validatePatterns(AlbumEditorDialog& dialog)
{
  if (allPatternsContainSameNumberOfMagicExpressionsOrNone(dialog.values()))
    return true;

  QMessageBox::warning(&dialog, "Warning",
                       "The number of wildcard patterns must be the same in all paths "
                       "containing any such patterns.");
  return false;
}
} // namespace

MainWindow::MainWindow(QWidget* parent, bool dontUseNativeDialogs)
  : QMainWindow(parent), ui_(std::make_unique<Ui::MainWindowClass>()),
    dontUseNativeDialogs_(dontUseNativeDialogs)
{
  ui_->setupUi(this);
  setWindowIcon(QIcon(":/icons/cameleon/Cameleon.ico"));

  populateLayoutSubmenu();
  initialiseRecentDocumentsSubmenu();

  instanceComboBox_ = new QComboBox(this);
  instanceComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  ui_->mainToolBar->addSeparator();
  ui_->mainToolBar->addWidget(instanceComboBox_);
  instanceComboBox_->installEventFilter(this);

  connect(instanceComboBox_, &QComboBox::currentIndexChanged, this,
          &MainWindow::onInstanceComboBox);
  connect(ui_->mainView, &MainView::mouseMovedOverImage, this, &MainWindow::onMouseMovedOverImage);
  connect(ui_->mainView, &MainView::mouseLeftImage, this, &MainWindow::onMouseLeftImage);

  QIcon::setThemeName("crystalsvg");

  ui_->actionNewAlbum->setIcon(QIcon::fromTheme("document-new"));
  ui_->actionOpenAlbum->setIcon(QIcon::fromTheme("document-open"));
  ui_->actionEditAlbum->setIcon(QIcon::fromTheme("document-edit"));
  ui_->actionRefreshAlbum->setIcon(QIcon::fromTheme("view-refresh"));
  ui_->actionSaveAlbum->setIcon(QIcon::fromTheme("document-save"));
  ui_->actionSaveAlbumAs->setIcon(QIcon::fromTheme("document-save-as"));

  ui_->actionFirstInstance->setIcon(QIcon::fromTheme("go-first"));
  ui_->actionPreviousInstance->setIcon(QIcon::fromTheme("go-previous"));
  ui_->actionNextInstance->setIcon(QIcon::fromTheme("go-next"));
  ui_->actionLastInstance->setIcon(QIcon::fromTheme("go-last"));

  ui_->actionBookmarkPage->setIcon(QIcon::fromTheme("bookmarks"));

  // Add a wide empty label to the status bar to force other labels to be right-aligned.
  statusBarMessageLabel_ = new QLabel(this);
  statusBar()->addWidget(statusBarMessageLabel_, 1 /*stretch*/);

  statusBarPixelLabel_ = new QLabel(this);
  const int minPixelLabelWidth =
    QFontMetrics(statusBarPixelLabel_->font())
      .boundingRect(statusBarPixelLabelText(QPoint(9999, 9999), QColor(255, 255, 255, 255)))
      .width();
  statusBarPixelLabel_->setMinimumWidth(minPixelLabelWidth);
  statusBarPixelLabel_->setText(QString());
  statusBarPixelLabel_->hide();

  statusBarInstanceLabel_ = new QLabel(this);
  const int minInstanceLabelWidth = QFontMetrics(statusBarInstanceLabel_->font())
                                      .boundingRect(statusBarInstanceLabelText(9999, 9999))
                                      .width();
  statusBarInstanceLabel_->setMinimumWidth(minInstanceLabelWidth);
  statusBarInstanceLabel_->setText(QString());
  statusBarInstanceLabel_->hide();

  updateDocumentDependentActions();
}

MainWindow::~MainWindow()
{
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == instanceComboBox_ && event->type() == QEvent::KeyPress)
  {
    // The key combinations Shift+Home, Shift+PgUp, Shift+PgDown and Shift+End trigger actions
    // navigating between bookmarked instances when these actions are enabled. When they are
    // disabled, we don't want these key combinations to be handled by the Instance combo box, as
    // this could confuse the user by activating a non-bookmarked instance.
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    auto key = keyEvent->key();
    if ((key == Qt::Key_Home || key == Qt::Key_PageUp || key == Qt::Key_PageDown ||
         key == Qt::Key_End) &&
        keyEvent->modifiers() & Qt::ShiftModifier)
    {
      return true;
    }
  }

  return QMainWindow::eventFilter(obj, event);
}

const MainView* MainWindow::mainView() const
{
  return ui_->mainView;
}

void MainWindow::processCommandLine()
{
  QStringList arguments = QCoreApplication::arguments();
  if (arguments.size() > 1)
    openDocument(arguments[1]);
}

void MainWindow::populateLayoutSubmenu()
{
  layoutMenu_ = new QMenu("&Layout", ui_->menuView);
  ui_->menuView->insertMenu(ui_->actionEditCaptions, layoutMenu_);
  ui_->menuView->insertSeparator(ui_->actionEditCaptions);
  layoutActionGroup_ = new QActionGroup(this);

  const size_t threshold = static_cast<size_t>(std::ceil(std::sqrt(MAX_NUM_PATTERNS)));
  for (size_t rows = 1; rows <= MAX_NUM_PATTERNS; ++rows)
  {
    size_t maxCols;
    if (rows <= threshold)
      maxCols = (MAX_NUM_PATTERNS + rows - 1) / rows; // round up
    else
      maxCols = MAX_NUM_PATTERNS / rows; // round down
    for (size_t cols = 1; cols <= maxCols; ++cols)
    {
      QAction* action = layoutMenu_->addAction(QString("%1 x %2").arg(rows).arg(cols));
      action->setCheckable(true);
      connect(action, &QAction::triggered, this, &MainWindow::onLayoutActionTriggered);
      layoutActionGroup_->addAction(action);
      layoutActions_[action] = Layout{rows, cols};
    }
  }
}

void MainWindow::initialiseRecentDocumentsSubmenu()
{
  recentDocumentsMenu_ = new QMenu("&Recent", ui_->menuFile);
  ui_->menuFile->insertMenu(ui_->actionQuit, recentDocumentsMenu_);
  ui_->menuFile->insertSeparator(ui_->actionQuit);

  QSettings settings;
  QStringList recentDocuments = settings.value("recentComparisons", QStringList()).toStringList();
  recentDocuments.resize(std::min<qsizetype>(recentDocuments.size(), MAX_NUM_RECENT_COMPARISONS));
  populateRecentDocumentsSubmenu(recentDocuments);
}

void MainWindow::prependToRecentDocuments(const QString& path)
{
  QSettings settings;
  QStringList recentDocuments = settings.value("recentComparisons", QStringList()).toStringList();
  recentDocuments.removeAll(path);
  recentDocuments.prepend(path);
  recentDocuments.resize(std::min<qsizetype>(recentDocuments.size(), MAX_NUM_RECENT_COMPARISONS));
  settings.setValue("recentComparisons", recentDocuments);
  populateRecentDocumentsSubmenu(recentDocuments);
}

void MainWindow::populateRecentDocumentsSubmenu(const QStringList& recentDocuments)
{
  recentDocumentsMenu_->clear();
  int i = 1;
  for (const QString& recentDocument : recentDocuments)
  {
    QAction* action = recentDocumentsMenu_->addAction(QString("&%1 %2").arg(i).arg(recentDocument));
    connect(action, &QAction::triggered, this, &MainWindow::onRecentDocumentActionTriggered);
    ++i;
  }
  recentDocumentsMenu_->setEnabled(!recentDocumentsMenu_->isEmpty());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (maybeSaveDocument())
  {
    doc_ = nullptr;
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void MainWindow::on_actionNewAlbum_triggered()
{
  if (!maybeSaveDocument())
  {
    return;
  }

  AlbumEditorDialog dialog(this, "recentPatterns");
  dialog.setWindowTitle("New Album");
  dialog.normalisePathSeparators(true);
  dialog.setValues({});
  dialog.setValidator(validatePatterns);
  setComboBoxPromptsToPatternExamples(dialog);
  if (dialog.exec() != QDialog::Accepted)
  {
    return;
  }

  std::unique_ptr<Document> newDoc = std::make_unique<Document>();
  newDoc->setLayout(defaultLayout(dialog.values().size()));

  PatternMatchingProgressDialog progressDialog(this);
  progressDialog.show();

  auto onFilesystemTraversalProgress = [&progressDialog]()
  { progressDialog.incrementProgressAndCheckForCancellation(); };

  if (!Try([&] { newDoc->setPatterns(dialog.values(), onFilesystemTraversalProgress); }))
    return;

  doc_ = std::move(newDoc);

  connectDocumentSignals();
  onDocumentPathChanged();
  onInstancesChanged();
}

void MainWindow::on_actionOpenAlbum_triggered()
{
  if (!maybeSaveDocument())
  {
    return;
  }

  QSettings settings;
  QString lastDir = settings.value("lastOpenDir", QString()).toString();
  QString path = QFileDialog::getOpenFileName(
    this, "Open Album", lastDir, "Albums (*.cml);;All files (*.*)", nullptr /*selectedFilter*/,
    dontUseNativeDialogs_ ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
  if (path.isEmpty())
  {
    return;
  }

  openDocument(path);
}

void MainWindow::openDocument(const QString& path)
{
  QSettings settings;
  settings.setValue("lastOpenDir", QFileInfo(path).dir().path());

  PatternMatchingProgressDialog progressDialog(this);
  progressDialog.show();

  auto onFilesystemTraversalProgress = [&progressDialog]()
  { progressDialog.incrementProgressAndCheckForCancellation(); };

  if (!Try([&] { doc_ = std::make_unique<Document>(path, onFilesystemTraversalProgress); }))
    return;

  connectDocumentSignals();
  onDocumentPathChanged();
  onInstancesChanged();
  if (!doc_->instances().empty())
    goToInstance(0);
}

void MainWindow::onRecentDocumentActionTriggered()
{
  if (!maybeSaveDocument())
    return;

  const QAction* action = dynamic_cast<QAction*>(sender());
  if (!action)
    return;

  // mid() removes the numerical prefix "&1 ", "&2 " etc.
  const QString path = action->text().mid(3);

  PatternMatchingProgressDialog progressDialog(this);
  progressDialog.show();

  auto onFilesystemTraversalProgress = [&progressDialog]()
  { progressDialog.incrementProgressAndCheckForCancellation(); };

  if (!Try([&] { doc_ = std::make_unique<Document>(path, onFilesystemTraversalProgress); }))
    return;

  connectDocumentSignals();
  onDocumentPathChanged();
  onInstancesChanged();
  if (!doc_->instances().empty())
    goToInstance(0);
}

void MainWindow::on_actionEditAlbum_triggered()
{
  AlbumEditorDialog dialog(this, "recentPatterns");
  dialog.setWindowTitle("Edit Album");
  dialog.normalisePathSeparators(true);
  dialog.setValues(doc_->patterns());
  dialog.setValidator(validatePatterns);
  setComboBoxPromptsToPatternExamples(dialog);
  if (dialog.exec() == QDialog::Accepted)
  {
    const std::optional<std::vector<QString>> previousInstanceKey = currentInstanceKey();

    const std::vector<QString> previousPatterns = doc_->patterns();
    const std::vector<QString> previousCaptionTemplates = doc_->captionTemplates();

    PatternMatchingProgressDialog progressDialog(this);
    progressDialog.show();

    auto onFilesystemTraversalProgress = [&progressDialog]()
    { progressDialog.incrementProgressAndCheckForCancellation(); };

    if (!Try([&] { doc_->setPatterns(dialog.values(), onFilesystemTraversalProgress); }))
      return;

    const size_t newNumPatterns = doc_->patterns().size();
    if (newNumPatterns != previousPatterns.size())
      doc_->setLayout(defaultLayout(newNumPatterns));

    doc_->setCaptionTemplates(
      updateCaptionTemplates(previousCaptionTemplates, previousPatterns, doc_->patterns()));

    onInstancesChanged();

    const int newInstance =
      previousInstanceKey ? findInstance(*doc_, *previousInstanceKey).value_or(0) : 0;
    if (!doc_->instances().empty())
      goToInstance(newInstance);
  }
}

std::optional<std::vector<QString>> MainWindow::currentInstanceKey() const
{
  if (instance_ < doc_->instances().size())
    return doc_->instances()[instance_].magicExpressionMatches;
  return std::nullopt;
}

void MainWindow::on_actionRefreshAlbum_triggered()
{
  const std::optional<std::vector<QString>> previousInstanceKey = currentInstanceKey();

  PatternMatchingProgressDialog progressDialog(this);
  progressDialog.show();

  auto onFilesystemTraversalProgress = [&progressDialog]()
  { progressDialog.incrementProgressAndCheckForCancellation(); };

  if (!Try([&] { doc_->regenerateInstances(onFilesystemTraversalProgress); }))
    return;

  onInstancesChanged();

  const int newInstance =
    previousInstanceKey ? findInstance(*doc_, *previousInstanceKey).value_or(0) : 0;
  if (!doc_->instances().empty())
    goToInstance(newInstance);
}

void MainWindow::on_actionSaveAlbum_triggered()
{
  saveDocument();
}

void MainWindow::on_actionSaveAlbumAs_triggered()
{
  saveDocumentAs();
}

void MainWindow::on_actionCloseAlbum_triggered()
{
  if (!maybeSaveDocument())
  {
    return;
  }

  doc_ = nullptr;
  onDocumentPathChanged();
  onInstancesChanged();
}

void MainWindow::on_actionQuit_triggered()
{
  qApp->closeAllWindows();
}

void MainWindow::on_actionZoomIn_triggered()
{
  ui_->mainView->zoom(1.25);
}

void MainWindow::on_actionZoomOut_triggered()
{
  ui_->mainView->zoom(1.0 / 1.25);
}

void MainWindow::on_actionZoom1to1_triggered()
{
  ui_->mainView->resetScale();
}

void MainWindow::on_actionSaveScreenshot_triggered()
{
  QSettings settings;
  QString lastDir = settings.value("lastSaveScreenshotDir", QString()).toString();
  QString proposedFileName = instanceKeyToFileName(doc_->instanceKey(instance_)) + ".png";
  QString path = QDir(lastDir).filePath(proposedFileName);

  path = QFileDialog::getSaveFileName(
    this, "Save Screenshot", path, "PNG images (*.png)", nullptr /*selectedFilter*/,
    dontUseNativeDialogs_ ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
  if (path.isEmpty())
    return;

  settings.setValue("lastSaveScreenshotDir", QFileInfo(path).dir().path());

  QPixmap pixmap = grab(toolBarAreaRect());
  QImage image = pixmap.toImage();
  if (!image.save(path))
  {
    QMessageBox::warning(this, "Save Screenshot",
                         QString("The screenshot could not be saved to %s.").arg(path));
  }
}

void MainWindow::on_actionSaveAllScreenshots_triggered()
{
  QSettings settings;
  QString lastDir = settings.value("lastSaveScreenshotDir", QString()).toString();
  QString dir = lastDir;

  dir = QFileDialog::getExistingDirectory(
    this, "Save All Screenshots", dir,
    QFileDialog::ShowDirsOnly |
      (dontUseNativeDialogs_ ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()));
  if (dir.isEmpty())
    return;

  settings.setValue("lastSaveScreenshotDir", dir);

  on_actionFirstInstance_triggered();
  qApp->processEvents();
  while (true)
  {
    QString path = dir + "\\" + instanceKeyToFileName(doc_->instanceKey(instance_)) + ".png";

    QPixmap pixmap = grab(toolBarAreaRect());
    QImage image = pixmap.toImage();
    if (!image.save(path))
    {
      QMessageBox::warning(this, "Save Screenshot",
                           QString("The screenshot could not be saved to %s.").arg(path));
      return;
    }

    if (instance_ + 1 == doc_->instances().size())
      return;

    on_actionNextInstance_triggered();
    qApp->processEvents();
  }
}

void MainWindow::on_actionEditCaptions_triggered()
{
  AlbumEditorDialog dialog(this, "recentCaptions");
  dialog.setWindowTitle("Edit Captions");
  dialog.setPrompt("Panel captions:");
  dialog.setNumberOfRows(doc_->captionTemplates().size());
  dialog.setFileDialogButtonsVisibility(false);
  dialog.setInfoLabelsVisibility(true);
  dialog.setInfoLabels(doc_->patterns());
  dialog.setSwapValuesButtonsVisibility(false);
  dialog.normalisePathSeparators(false);
  dialog.setValues(doc_->captionTemplates());
  dialog.setValidator(
    [numPatterns = doc_->patterns().size()](AlbumEditorDialog& dialog)
    {
      if (dialog.values().size() == numPatterns)
        return true;
      QMessageBox::warning(&dialog, "Warning",
                           "The number of captions must be the same as the number of panels.");
      return false;
    });

  if (dialog.exec() == QDialog::Accepted)
  {
    if (!Try([&] { doc_->setCaptionTemplates(dialog.values()); }))
      return;
    onCaptionTemplatesChanged();
  }
}

QRect MainWindow::toolBarAreaRect() const
{
  return QRect(menuBar()->geometry().bottomLeft(), statusBar()->geometry().topRight());
}

void MainWindow::on_actionFirstInstance_triggered()
{
  if (!doc_ || doc_->instances().empty())
    return;
  goToInstance(0);
}

void MainWindow::on_actionPreviousInstance_triggered()
{
  if (!doc_ || doc_->instances().empty() || instance_ == 0)
    return;
  goToInstance(instance_ - 1);
}

void MainWindow::on_actionNextInstance_triggered()
{
  if (!doc_ || doc_->instances().empty() || instance_ + 1 >= doc_->instances().size())
    return;
  goToInstance(instance_ + 1);
}

void MainWindow::on_actionLastInstance_triggered()
{
  if (!doc_ || doc_->instances().empty() || instance_ + 1 >= doc_->instances().size())
    return;
  goToInstance(doc_->instances().size() - 1);
}

void MainWindow::on_actionBookmarkPage_triggered(bool checked)
{
  if (!doc_ || doc_->instances().empty())
    return;
  if (checked)
    doc_->addBookmark(instance_);
  else
    doc_->removeBookmark(instance_);
  const QBrush brush = checked ? QBrush(BOOKMARK_COLOUR) : QBrush();
  instanceComboBox_->setItemData(instance_, brush, Qt::ForegroundRole);

  onBookmarksChanged();
}

void MainWindow::on_actionRemoveAllBookmarks_triggered()
{
  if (!doc_)
    return;

  if (QMessageBox::question(
        this, "Remove All Bookmarks",
        "Do you wish to remove all bookmarks? This operation cannot be undone.") !=
      QMessageBox::Yes)
    return;

  doc_->removeAllBookmarks();
  QBrush defaultBrush;
  for (size_t instance = 0; instance < doc_->instances().size(); ++instance)
    instanceComboBox_->setItemData(instance, defaultBrush, Qt::ForegroundRole);

  onBookmarksChanged();
}

void MainWindow::on_actionFirstBookmark_triggered()
{
  if (!doc_ || doc_->instances().empty() || doc_->bookmarks().empty())
    return;
  goToInstance(*doc_->bookmarks().begin());
}

void MainWindow::on_actionPreviousBookmark_triggered()
{
  if (!doc_ || doc_->instances().empty())
    return;

  auto it = std::upper_bound(doc_->bookmarks().rbegin(), doc_->bookmarks().rend(), instance_,
                             std::greater<size_t>());
  if (it != doc_->bookmarks().rend())
    goToInstance(*it);
}

void MainWindow::on_actionNextBookmark_triggered()
{
  if (!doc_ || doc_->instances().empty())
    return;

  auto it = std::upper_bound(doc_->bookmarks().begin(), doc_->bookmarks().end(), instance_);
  if (it != doc_->bookmarks().end())
    goToInstance(*it);
}

void MainWindow::on_actionLastBookmark_triggered()
{
  if (!doc_ || doc_->instances().empty() || doc_->bookmarks().empty())
    return;
  goToInstance(*doc_->bookmarks().rbegin());
}

void MainWindow::on_actionImportBookmarks_triggered()
{
  if (!doc_)
    return;

  QSettings settings;
  QString lastDir = settings.value("lastImportOrExportBookmarksDir", QString()).toString();

  QString fileName = QFileDialog::getOpenFileName(
    this, "Import Bookmarks", lastDir, "Text files (*.txt)", nullptr /*selectedFilter*/,
    dontUseNativeDialogs_ ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
  if (fileName.isEmpty())
    return;

  settings.setValue("lastImportOrExportBookmarksDir", QFileInfo(fileName).dir().path());
  QFile file(fileName);
  QString bookmarkKey;
  std::set<QString> bookmarkKeys;
  if (file.open(QFile::ReadOnly))
  {
    QTextStream stream(&file);
    while (stream.readLineInto(&bookmarkKey))
      bookmarkKeys.insert(bookmarkKey);
    size_t numImportedBookmarks = 0;
    const QBrush bookmarkBrush = QBrush(BOOKMARK_COLOUR);
    for (size_t i = 0; i < doc_->instances().size(); ++i)
    {
      if (contains(bookmarkKeys, doc_->instanceKey(i)) && !contains(doc_->bookmarks(), i))
      {
        doc_->addBookmark(i);
        instanceComboBox_->setItemData(i, bookmarkBrush, Qt::ForegroundRole);
        ++numImportedBookmarks;
      }
    }
    QMessageBox::information(this, "Import Bookmarks",
                             QString("%1 bookmarks have been imported.").arg(numImportedBookmarks),
                             QMessageBox::Ok);
    onBookmarksChanged();
  }
  else
  {
    QMessageBox::warning(this, "Warning",
                         QString("Could not open file '%1' for reading.").arg(fileName));
  }
}

void MainWindow::on_actionExportBookmarks_triggered()
{
  if (!doc_)
    return;

  QSettings settings;
  QString lastDir = settings.value("lastImportOrExportBookmarksDir", QString()).toString();

  QString fileName = QFileDialog::getSaveFileName(
    this, "Export Bookmarks", lastDir, "Text files (*.txt)", nullptr /*selectedFilter*/,
    dontUseNativeDialogs_ ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
  if (fileName.isEmpty())
    return;

  settings.setValue("lastImportOrExportBookmarksDir", QFileInfo(fileName).dir().path());
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Truncate))
  {
    QTextStream stream(&file);
    for (size_t i : doc_->bookmarks())
      stream << doc_->instanceKey(i) << "\n";
    QMessageBox::information(
      this, "Export Bookmarks",
      QString("%1 bookmarks have been exported.").arg(doc_->bookmarks().size()), QMessageBox::Ok);
  }
  else
  {
    QMessageBox::warning(this, "Warning",
                         QString("Could not open file '%1' for writing.").arg(fileName));
  }
}

void MainWindow::on_actionAboutCameleon_triggered()
{
  QMessageBox::about(
    this, "About Cam\u00E9l\u00E9on",
    "Cam\u00E9l\u00E9on " CAMELEON_VERSION ".<b></b><br><br>\n\n"
    "Copyright (C) 2023-2024 Wojciech \u015Amigaj.<br><br>\n\n"
    "This program is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation, either version 3 of the License, or "
    "(at your option) any later version.<br><br>\n\n"
    "Application icon by <a href=\"http://icons8.com/\">Icons8</a>.<br><br>\n\n"
    "This software uses the following open-source libraries:\n\n"
    "<ul>"
    "<li> Qt Toolkit. Copyright (C) 2017 The Qt Company Ltd. "
    "Contact: <a href=\"https://www.qt.io/licensing\">https://www.qt.io/licensing</a>. "
    "You may use, distribute and copy the Qt GUI Toolkit under the terms of "
    "GNU Lesser General Public License version 3, which supplements GNU General "
    "Public License Version 3.\n\n"
    "<li> glob for C++17. Copyright (C) 2019 Pranav.\n\n"
    "<li> KDE Crystal theme icons. Copyright (C) 2002 and following years KDE Artists. "
    "This library is free software; you can redistribute it and/or "
    "modify it under the terms of the GNU Lesser General Public "
    "License as published by the Free Software Foundation, "
    "version 2.1 of the License."
    "</ul>");
}

void MainWindow::onLayoutActionTriggered()
{
  QAction* senderAction = static_cast<QAction*>(sender());
  Layout layout = layoutActions_.at(senderAction);
  doc_->setLayout(layout);
  updateMainViewLayout();
}

void MainWindow::onDocumentModificationStatusChanged()
{
  updateDocumentModificationStatusDependentActions();
  setWindowModified(doc_ && doc_->modified());
}

void MainWindow::onDocumentPathChanged()
{
  QString appTitle = "Cam\u00E9l\u00E9on";
  QString title;
  if (doc_)
  {
    if (doc_->path().isEmpty())
      title = "Untitled.cml";
    else
      title = QFileInfo(doc_->path()).fileName();
    title += "[*] - " + appTitle;
  }
  else
  {
    title = appTitle;
  }
  setWindowTitle(title);
  setWindowModified(doc_ && doc_->modified());

  if (doc_ && !doc_->path().isEmpty())
    prependToRecentDocuments(doc_->path());
}

void MainWindow::onInstanceComboBox(int currentIndex)
{
  if (currentIndex >= 0)
    goToInstance(currentIndex);
}

void MainWindow::onMouseLeftImage()
{
  statusBarPixelLabel_->setText(QString());
}

void MainWindow::onMouseMovedOverImage(QPoint pixelCoords, QColor pixelColour)
{
  statusBarPixelLabel_->setText(statusBarPixelLabelText(pixelCoords, pixelColour));
}

void MainWindow::connectDocumentSignals()
{
  connect(doc_.get(), &Document::modificationStatusChanged, this,
          &MainWindow::onDocumentModificationStatusChanged);
}

void MainWindow::updateMainViewLayout()
{
  if (!doc_ || doc_->instances().empty())
  {
    ui_->mainView->setLayout(Layout{0, 0});
  }
  else
  {
    ui_->mainView->setLayout(doc_->layout());
  }
}

void MainWindow::updateLayoutSubmenu()
{
  if (doc_)
  {
    const size_t numPatterns = doc_->patterns().size();
    const Layout docLayout = doc_->layout();
    for (auto& [action, layout] : layoutActions_)
    {
      action->setEnabled(layout.panels() >= numPatterns);
      if (layout == docLayout)
      {
        action->setChecked(true);
      }
    }
  }
}

void MainWindow::populateInstanceComboBox()
{
  instanceComboBox_->clear();

  bool anyItemIsNonempty = false;
  if (doc_ != nullptr)
  {
    const size_t numInstances = doc_->instances().size();
    for (size_t instance = 0; instance < numInstances; ++instance)
    {
      QString item = doc_->instanceKey(instance);
      anyItemIsNonempty = anyItemIsNonempty || !item.isEmpty();
      const bool isBookmarked = contains(doc_->bookmarks(), instance);
      const QBrush brush = isBookmarked ? QBrush(BOOKMARK_COLOUR) : QBrush();
      instanceComboBox_->addItem(item);
      instanceComboBox_->setItemData(instanceComboBox_->count() - 1, brush, Qt::ForegroundRole);
    }
  }
  instanceComboBox_->setEnabled(anyItemIsNonempty);
}

void MainWindow::updateDocumentDependentUiElements()
{
  updateDocumentDependentActions();
  updateDocumentDependentWidgets();
}

void MainWindow::updateDocumentDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const bool hasInstances = isOpen && !doc_->instances().empty();
  const bool isModified = isOpen && doc_->modified();
  const bool hasPatterns = isOpen && !doc_->patterns().empty();
  ui_->actionEditAlbum->setEnabled(isOpen);
  ui_->actionRefreshAlbum->setEnabled(isOpen);
  ui_->actionSaveAlbum->setEnabled(isModified);
  ui_->actionSaveAlbumAs->setEnabled(isOpen);
  ui_->actionCloseAlbum->setEnabled(isOpen);
  ui_->actionZoomIn->setEnabled(hasInstances);
  ui_->actionZoomOut->setEnabled(hasInstances);
  ui_->actionZoom1to1->setEnabled(hasInstances);
  ui_->actionEditCaptions->setEnabled(hasPatterns);
  ui_->actionSaveScreenshot->setEnabled(hasInstances);

  layoutMenu_->setEnabled(hasInstances);

  updateDocumentModificationStatusDependentActions();
  updateInstanceDependentActions();
}

void MainWindow::updateDocumentDependentWidgets()
{
  const bool isOpen = doc_ != nullptr;
  if (isOpen && !statusBarPixelLabel_->isVisible())
  {
    statusBarPixelLabel_->show();
    statusBar()->addWidget(statusBarPixelLabel_);
  }
  else if (!isOpen && statusBarPixelLabel_->isVisible())
  {
    statusBarPixelLabel_->hide();
    statusBar()->removeWidget(statusBarPixelLabel_);
  }

  if (isOpen && !statusBarInstanceLabel_->isVisible())
  {
    statusBarInstanceLabel_->show();
    statusBar()->addWidget(statusBarInstanceLabel_);
  }
  else if (!isOpen && statusBarInstanceLabel_->isVisible())
  {
    statusBarInstanceLabel_->hide();
    statusBar()->removeWidget(statusBarInstanceLabel_);
  }

  updateInstanceDependentWidgets();
}

void MainWindow::updateDocumentModificationStatusDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const bool isModified = isOpen && doc_->modified();
  ui_->actionSaveAlbum->setEnabled(isModified);
}

void MainWindow::updateInstanceDependentUiElements()
{
  updateInstanceDependentActions();
  updateInstanceDependentWidgets();
}

void MainWindow::updateInstanceDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const int numInstances = isOpen ? doc_->instances().size() : 0;
  ui_->actionFirstInstance->setEnabled(numInstances > 0 && instance_ > 0);
  ui_->actionPreviousInstance->setEnabled(numInstances > 0 && instance_ > 0);
  ui_->actionNextInstance->setEnabled(numInstances > 0 && instance_ < numInstances - 1);
  ui_->actionLastInstance->setEnabled(numInstances > 0 && instance_ < numInstances - 1);
  updateBookmarkDependentActions();
}

void MainWindow::updateInstanceDependentWidgets()
{
  const bool isOpen = doc_ != nullptr;
  const int numInstances = isOpen ? doc_->instances().size() : 0;
  if (isOpen)
  {
    statusBarInstanceLabel_->setText(statusBarInstanceLabelText(instance_, numInstances));
  }
}

QString MainWindow::statusBarInstanceLabelText(int currentInstance, int numInstances)
{
  return QString("Page %1 of %2")
    .arg(std::min(currentInstance + 1, numInstances))
    .arg(numInstances);
}

QString MainWindow::statusBarPixelLabelText(const QPoint& pt, const QColor& colour)
{
  return QString("(X: %1, Y: %2)   (R: %3, G: %4, B: %5, A: %6)")
    .arg(pt.x())
    .arg(pt.y())
    .arg(colour.red())
    .arg(colour.green())
    .arg(colour.blue())
    .arg(colour.alpha());
}

void MainWindow::updateBookmarkDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const bool hasInstances = isOpen && !doc_->instances().empty();
  const bool hasBookmarks = isOpen && !doc_->bookmarks().empty();
  ui_->actionBookmarkPage->setEnabled(hasInstances);
  ui_->actionBookmarkPage->setChecked(isOpen && contains(doc_->bookmarks(), instance_));
  ui_->actionRemoveAllBookmarks->setEnabled(hasBookmarks);
  ui_->actionFirstBookmark->setEnabled(hasBookmarks && instance_ != *doc_->bookmarks().begin());
  ui_->actionPreviousBookmark->setEnabled(hasBookmarks && instance_ > *doc_->bookmarks().begin());
  ui_->actionNextBookmark->setEnabled(hasBookmarks && instance_ < *doc_->bookmarks().rbegin());
  ui_->actionLastBookmark->setEnabled(hasBookmarks && instance_ != *doc_->bookmarks().rbegin());
  ui_->actionImportBookmarks->setEnabled(isOpen && hasInstances);
  ui_->actionExportBookmarks->setEnabled(hasBookmarks);
}

void MainWindow::onInstancesChanged()
{
  updateMainViewLayout();
  updateLayoutSubmenu();
  populateInstanceComboBox();
  updateDocumentDependentUiElements();

  if (doc_ && doc_->instances().empty())
  {
    QMessageBox::information(this, "Information", "No pattern matches found.");
  }
}

void MainWindow::onActiveInstanceChanged()
{
  if (doc_ && !doc_->instances().empty())
  {
    Q_ASSERT(instance_ < doc_->instances().size());
    if (instance_ < doc_->instances().size())
    {
      instanceComboBox_->setCurrentIndex(instance_);
      ui_->mainView->setPaths(doc_->instances()[instance_].paths);
      ui_->mainView->setInstanceKey(doc_->instanceKey(instance_));
      ui_->mainView->setCaptions(doc_->captions(instance_));
    }
  }
  updateInstanceDependentUiElements();
}

void MainWindow::onCaptionTemplatesChanged()
{
  if (doc_ && !doc_->instances().empty())
  {
    Q_ASSERT(instance_ < doc_->instances().size());
    if (instance_ < doc_->instances().size())
    {
      ui_->mainView->setCaptions(doc_->captions(instance_));
    }
  }
}

void MainWindow::onBookmarksChanged()
{
  updateBookmarkDependentActions();
}

void MainWindow::goToInstance(int instance)
{
  if (!doc_ || instance < 0 || instance >= doc_->instances().size())
  {
    Q_ASSERT(false);
    return;
  }

  instance_ = instance;
  onActiveInstanceChanged();
}

bool MainWindow::maybeSaveDocument()
{
  if (!doc_ || !doc_->modified())
  {
    return true;
  }

  int ret = QMessageBox::warning(
    this, qApp->applicationName(), "The album has been modified. Do you want to save your changes?",
    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
  switch (ret)
  {
  case QMessageBox::Yes:
    return saveDocument();
  case QMessageBox::No:
    return true;
  default:
    return false;
  }
}

bool MainWindow::saveDocument()
{
  if (doc_->path().isEmpty())
  {
    return saveDocumentAs();
  }
  else
  {
    return saveDocument(doc_->path());
  }
}

bool MainWindow::saveDocumentAs()
{
  QSettings settings;

  QString path = doc_->path();
  if (path.isEmpty())
  {
    path = settings.value("lastSaveDir", QString()).toString();
  }
  path = QFileDialog::getSaveFileName(
    this, "Save Album", path, "Albums (*.cml)", nullptr /*selectedFilter*/,
    dontUseNativeDialogs_ ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
  if (path.isEmpty())
  {
    return false;
  }

  settings.setValue("lastSaveDir", QFileInfo(path).dir().path());

  return saveDocument(path);
}

bool MainWindow::saveDocument(const QString& path)
{
  return Try(
    [&]
    {
      doc_->save(path);
      onDocumentPathChanged();
    });
}
