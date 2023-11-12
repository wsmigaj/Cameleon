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
#include "CancellationException.h"
#include "ComparisonDialog.h"
#include "Constants.h"
#include "Document.h"
#include "MainWindow.h"
#include "RuntimeError.h"
#include "Try.h"
#include "PatternMatchingProgressDialog.h"

namespace
{
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

QString instanceKeyToFileName(const std::vector<QString>& keys)
{
  QString fileName = join(keys, "...");
  fileName.replace('/', "_");
  fileName.replace('\\', "_");
  return fileName;
}
} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
  ui_.setupUi(this);

  populateLayoutSubmenu();
  initialiseRecentComparisonsSubmenu();

  instanceComboBox_ = new QComboBox(this);
  instanceComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  ui_.mainToolBar->addWidget(instanceComboBox_);

  connect(instanceComboBox_, &QComboBox::currentIndexChanged, this,
          &MainWindow::onInstanceComboBox);

  QIcon::setThemeName("crystalsvg");

  ui_.actionNewComparison->setIcon(QIcon::fromTheme("document-new"));
  ui_.actionOpenComparison->setIcon(QIcon::fromTheme("document-open"));
  ui_.actionEditComparison->setIcon(QIcon::fromTheme("document-edit"));
  ui_.actionRefreshComparison->setIcon(QIcon::fromTheme("view-refresh"));
  ui_.actionSaveComparison->setIcon(QIcon::fromTheme("document-save"));
  ui_.actionSaveComparisonAs->setIcon(QIcon::fromTheme("document-save-as"));

  ui_.actionFirstInstance->setIcon(QIcon::fromTheme("go-first"));
  ui_.actionPreviousInstance->setIcon(QIcon::fromTheme("go-previous"));
  ui_.actionNextInstance->setIcon(QIcon::fromTheme("go-next"));
  ui_.actionLastInstance->setIcon(QIcon::fromTheme("go-last"));

  mainLayout_ = new QGridLayout(ui_.mainView);
  updateDocumentDependentActions();
}

MainWindow::~MainWindow()
{
}

void MainWindow::populateLayoutSubmenu()
{
  layoutMenu_ = new QMenu("&Layout", ui_.menuView);
  ui_.menuView->insertMenu(ui_.actionEditCaptions, layoutMenu_);
  ui_.menuView->insertSeparator(ui_.actionEditCaptions);
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

void MainWindow::initialiseRecentComparisonsSubmenu()
{
  recentComparisonsMenu_ = new QMenu("&Recent Comparisons", ui_.menuFile);
  ui_.menuFile->insertMenu(ui_.actionQuit, recentComparisonsMenu_);
  ui_.menuFile->insertSeparator(ui_.actionQuit);

  QSettings settings;
  QStringList recentComparisons = settings.value("recentComparisons", QStringList()).toStringList();
  recentComparisons.resize(
    std::min<qsizetype>(recentComparisons.size(), MAX_NUM_RECENT_COMPARISONS));
  populateRecentComparisonsSubmenu(recentComparisons);
}

void MainWindow::prependToRecentComparisons(const QString& path)
{
  QSettings settings;
  QStringList recentComparisons = settings.value("recentComparisons", QStringList()).toStringList();
  recentComparisons.removeAll(path);
  recentComparisons.prepend(path);
  recentComparisons.resize(
    std::min<qsizetype>(recentComparisons.size(), MAX_NUM_RECENT_COMPARISONS));
  settings.setValue("recentComparisons", recentComparisons);
  populateRecentComparisonsSubmenu(recentComparisons);
}

void MainWindow::populateRecentComparisonsSubmenu(const QStringList& recentComparisons)
{
  recentComparisonsMenu_->clear();
  int i = 1;
  for (const QString& recentComparison : recentComparisons)
  {
    QAction* action =
      recentComparisonsMenu_->addAction(QString("&%1 %2").arg(i).arg(recentComparison));
    connect(action, &QAction::triggered, this, &MainWindow::onRecentComparisonActionTriggered);
    ++i;
  }
  recentComparisonsMenu_->setEnabled(!recentComparisonsMenu_->isEmpty());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (maybeSaveDocument())
  {
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void MainWindow::on_actionNewComparison_triggered()
{
  if (!maybeSaveDocument())
  {
    return;
  }

  ComparisonDialog dialog(this, "recentPatterns");
  dialog.setWindowTitle("New Comparison");
  dialog.normalisePathSeparators(true);
  dialog.setValues({});
  if (dialog.exec() != QDialog::Accepted)
  {
    return;
  }

  doc_ = std::make_unique<Document>();
  doc_->setLayout(defaultLayout(dialog.values().size()));

  PatternMatchingProgressDialog progressDialog(this);
  progressDialog.show();

  auto onFilesystemTraversalProgress = [&progressDialog]()
  { progressDialog.incrementProgressAndCheckForCancellation(); };

  doc_->setPatterns(dialog.values(), onFilesystemTraversalProgress);

  connectDocumentSignals();
  onDocumentPathChanged();
  onInstancesChanged();
}

void MainWindow::on_actionOpenComparison_triggered()
{
  if (!maybeSaveDocument())
  {
    return;
  }

  QSettings settings;
  QString lastDir = settings.value("lastOpenDir", QString()).toString();
  QString path = QFileDialog::getOpenFileName(this, "Open Comparison", lastDir,
                                              "Comparisons (*.cml);;All files (*.*)");
  if (path.isEmpty())
  {
    return;
  }

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
  goToInstance(0);
}

void MainWindow::onRecentComparisonActionTriggered()
{
  if (!maybeSaveDocument())
    return;

  const QAction* action = dynamic_cast<QAction*>(sender());
  if (!action)
    return;

  // mid() removes the numerical prefix "&1 ", "&2 " etc.
  const QString path = action->text().mid(3);

  if (!Try([&] { doc_ = std::make_unique<Document>(path); }))
    return;

  connectDocumentSignals();
  onDocumentPathChanged();
  onInstancesChanged();
  goToInstance(0);
}

void MainWindow::on_actionEditComparison_triggered()
{
  ComparisonDialog dialog(this, "recentPatterns");
  dialog.setWindowTitle("Edit Comparison");
  dialog.normalisePathSeparators(true);
  dialog.setValues(doc_->patterns());
  if (dialog.exec() == QDialog::Accepted)
  {
    const std::optional<std::vector<QString>> previousInstanceKey = currentInstanceKey();

    const size_t previousNumPatterns = doc_->patterns().size();

    PatternMatchingProgressDialog progressDialog(this);
    progressDialog.show();

    auto onFilesystemTraversalProgress = [&progressDialog]()
    { progressDialog.incrementProgressAndCheckForCancellation(); };

    if (!Try([&] { doc_->setPatterns(dialog.values(), onFilesystemTraversalProgress); }))
      return;

    const size_t currentNumPatterns = doc_->patterns().size();
    if (currentNumPatterns != previousNumPatterns)
      doc_->setLayout(defaultLayout(currentNumPatterns));

    onInstancesChanged();

    const int newInstance =
      previousInstanceKey ? findInstance(*doc_, *previousInstanceKey).value_or(0) : 0;
    goToInstance(newInstance);
  }
}

std::optional<std::vector<QString>> MainWindow::currentInstanceKey() const
{
  if (instance_ < doc_->instances().size())
    return doc_->instances()[instance_].magicExpressionMatches;
  return std::nullopt;
}

void MainWindow::on_actionRefreshComparison_triggered()
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
  goToInstance(newInstance);
}

void MainWindow::on_actionSaveComparison_triggered()
{
  saveDocument();
}

void MainWindow::on_actionSaveComparisonAs_triggered()
{
  saveDocumentAs();
}

void MainWindow::on_actionCloseComparison_triggered()
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
  ui_.mainView->zoom(1.25);
}

void MainWindow::on_actionZoomOut_triggered()
{
  ui_.mainView->zoom(1.0 / 1.25);
}

void MainWindow::on_actionZoom1to1_triggered()
{
  ui_.mainView->resetScale();
}

void MainWindow::on_actionSaveScreenshot_triggered()
{
  QSettings settings;
  QString lastDir = settings.value("lastSaveScreenshotDir", QString()).toString();
  QString proposedFileName =
    instanceKeyToFileName(doc_->instances()[instance_].magicExpressionMatches) + ".png";
  QString path = QDir(lastDir).filePath(proposedFileName);

  path = QFileDialog::getSaveFileName(this, "Save Screenshot", path, "PNG images (*.png)");
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

  dir = QFileDialog::getExistingDirectory(this, "Save All Screenshots", dir);
  if (dir.isEmpty())
    return;

  settings.setValue("lastSaveScreenshotDir", dir);

  on_actionFirstInstance_triggered();
  qApp->processEvents();
  while (true)
  {
    QString path = dir + "\\" +
                   instanceKeyToFileName(doc_->instances()[instance_].magicExpressionMatches) +
                   ".png";

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
  ComparisonDialog dialog(this, "recentCaptions");
  dialog.setWindowTitle("Edit Captions");
  dialog.setPrompt("");
  dialog.setNumberOfRows(doc_->captionTemplates().size());
  dialog.setFileDialogButtonsVisibility(false);
  dialog.setSwapValuesButtonsVisibility(false);
  dialog.normalisePathSeparators(false);
  dialog.setValues(doc_->captionTemplates());
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
    title += " - " + appTitle;
  }
  else
  {
    title = appTitle;
  }
  setWindowTitle(title);

  if (doc_ && !doc_->path().isEmpty())
    prependToRecentComparisons(doc_->path());
}

void MainWindow::onInstanceComboBox(int currentIndex)
{
  if (currentIndex >= 0)
    goToInstance(currentIndex);
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
    ui_.mainView->setLayout(Layout{0, 0});
  }
  else
  {
    ui_.mainView->setLayout(doc_->layout());
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
    for (const Instance& instance : doc_->instances())
    {
      QString item = join(instance.magicExpressionMatches, "...");
      anyItemIsNonempty = anyItemIsNonempty || !item.isEmpty();
      instanceComboBox_->addItem(std::move(item));
    }
  }
  instanceComboBox_->setEnabled(anyItemIsNonempty);
}

void MainWindow::updateDocumentDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const bool hasInstances = isOpen && !doc_->instances().empty();
  const bool isModified = isOpen && doc_->modified();
  const bool hasPatterns = isOpen && !doc_->patterns().empty();
  ui_.actionEditComparison->setEnabled(isOpen);
  ui_.actionRefreshComparison->setEnabled(isOpen);
  ui_.actionSaveComparison->setEnabled(isModified);
  ui_.actionSaveComparisonAs->setEnabled(isOpen);
  ui_.actionCloseComparison->setEnabled(isOpen);
  ui_.actionZoomIn->setEnabled(hasInstances);
  ui_.actionZoomOut->setEnabled(hasInstances);
  ui_.actionZoom1to1->setEnabled(hasInstances);
  ui_.actionEditCaptions->setEnabled(hasPatterns);
  ui_.actionSaveScreenshot->setEnabled(hasInstances);

  layoutMenu_->setEnabled(hasInstances);

  updateDocumentModificationStatusDependentActions();
  updateInstanceDependentActions();
}

void MainWindow::updateDocumentModificationStatusDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const bool isModified = isOpen && doc_->modified();
  ui_.actionSaveComparison->setEnabled(isModified);
}

void MainWindow::updateInstanceDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const int numInstances = isOpen ? doc_->instances().size() : 0;
  ui_.actionFirstInstance->setEnabled(numInstances > 0 && instance_ > 0);
  ui_.actionPreviousInstance->setEnabled(numInstances > 0 && instance_ > 0);
  ui_.actionNextInstance->setEnabled(numInstances > 0 && instance_ < numInstances - 1);
  ui_.actionLastInstance->setEnabled(numInstances > 0 && instance_ < numInstances - 1);
}

void MainWindow::onInstancesChanged()
{
  updateMainViewLayout();
  updateLayoutSubmenu();
  populateInstanceComboBox();
  updateDocumentDependentActions();

  if (doc_ && doc_->instances().empty())
  {
    QMessageBox::information(this, "Information", "No pattern matches found");
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
      ui_.mainView->setPaths(doc_->instances()[instance_].paths);
      ui_.mainView->setCaptions(doc_->captions(instance_));
    }
  }
  updateInstanceDependentActions();
}

void MainWindow::onCaptionTemplatesChanged()
{
  if (doc_ && !doc_->instances().empty())
  {
    Q_ASSERT(instance_ < doc_->instances().size());
    if (instance_ < doc_->instances().size())
    {
      ui_.mainView->setCaptions(doc_->captions(instance_));
    }
  }
}

void MainWindow::goToInstance(int instance)
{
  instance_ = instance;
  onActiveInstanceChanged();
}

bool MainWindow::maybeSaveDocument()
{
  if (!doc_ || !doc_->modified())
  {
    return true;
  }

  int ret =
    QMessageBox::warning(this, qApp->applicationName(),
                         "The comparison has been modified. Do you want to save your changes?",
                         QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                         QMessageBox::Cancel | QMessageBox::Escape);
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
  path = QFileDialog::getSaveFileName(this, "Save Comparison", path, "Comparisons (*.cml)");
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
