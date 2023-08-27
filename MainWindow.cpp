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
#include "ComparisonDialog.h"
#include "Constants.h"
#include "Document.h"
#include "MainWindow.h"
#include "RuntimeError.h"
#include "Settings.h"

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

  instanceComboBox_ = new QComboBox(this);
  instanceComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  ui_.mainToolBar->addWidget(instanceComboBox_);

  connect(instanceComboBox_, &QComboBox::currentIndexChanged, this,
          &MainWindow::onInstanceComboBox);

  QIcon::setThemeName("tango");

  ui_.actionNewComparison->setIcon(QIcon::fromTheme("document-new"));
  ui_.actionOpenComparison->setIcon(QIcon::fromTheme("document-open"));
  ui_.actionEditComparison->setIcon(QIcon::fromTheme("document-properties"));
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
  ui_.menuView->insertMenu(ui_.actionSaveScreenshot, layoutMenu_);
  ui_.menuView->insertSeparator(ui_.actionSaveScreenshot);
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

  ComparisonDialog dialog(this);
  dialog.setWindowTitle("New Comparison");
  dialog.setPatterns({});
  if (dialog.exec() != QDialog::Accepted)
  {
    return;
  }

  doc_ = std::make_unique<Document>();
  doc_->setLayout(Settings::defaultLayout(dialog.patterns().size()));
  doc_->setPatterns(dialog.patterns());

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

  std::unique_ptr<Document> doc;
  try
  {
    doc = std::make_unique<Document>(path);
  }
  catch (std::exception& ex)
  {
    QMessageBox::warning(this, "Warning", ex.what());
    return;
  }
  catch (...)
  {
    QMessageBox::warning(this, "Warning", "An unidentified problem occurred.");
    return;
  }

  doc_ = std::move(doc);
  connectDocumentSignals();
  onDocumentPathChanged();
  onInstancesChanged();
  goToInstance(0);
}

void MainWindow::on_actionEditComparison_triggered()
{
  ComparisonDialog dialog(this);
  dialog.setWindowTitle("Edit Comparison");
  dialog.setPatterns(doc_->patterns());
  if (dialog.exec() == QDialog::Accepted)
  {
    // For now, we'll always reset to the first case in the sequence.
    // Later we might restore the case shown previously if certain conditions are met.

    if (dialog.patterns().size() != doc_->patterns().size())
      doc_->setLayout(Settings::defaultLayout(dialog.patterns().size()));
    doc_->setPatterns(dialog.patterns());
    onInstancesChanged();
    goToInstance(0);
  }
}

void MainWindow::on_actionRefreshComparison_triggered()
{
  // For now, we'll always reset to the first case in the sequence.
  // Later we might restore the case shown previously if certain conditions are met.

  doc_->regenerateInstances();
  onInstancesChanged();
  goToInstance(0);
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
  QString proposedFileName = instanceKeyToFileName(doc_->instanceKeys()[instance_]) + ".png";
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
    for (const std::vector<QString>& matches : doc_->instanceKeys())
    {
      QString item = join(matches, "...");
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
  ui_.actionEditComparison->setEnabled(isOpen);
  ui_.actionRefreshComparison->setEnabled(isOpen);
  ui_.actionSaveComparison->setEnabled(isModified);
  ui_.actionSaveComparisonAs->setEnabled(isOpen);
  ui_.actionCloseComparison->setEnabled(isOpen);
  ui_.actionZoomIn->setEnabled(hasInstances);
  ui_.actionZoomOut->setEnabled(hasInstances);
  ui_.actionZoom1to1->setEnabled(hasInstances);
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
    if (instance_ < doc_->instances().size())
    {
      instanceComboBox_->setCurrentIndex(instance_);
      ui_.mainView->setPaths(doc_->instances()[instance_]);
    }
    else
    {
      throw RuntimeError("Internal error: invalid instance index");
    }
  }
  updateInstanceDependentActions();
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
  try
  {
    doc_->save(path);
    onDocumentPathChanged();
    return true;
  }
  catch (std::exception& ex)
  {
    QMessageBox::warning(this, "Warning", ex.what());
  }
  catch (...)
  {
    QMessageBox::warning(this, "Warning", "An unidentified problem occurred.");
  }
  return false;
}
