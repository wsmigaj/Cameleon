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
} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
  ui_.setupUi(this);

  instanceComboBox_ = new QComboBox(this);
  instanceComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  ui_.mainToolBar->addWidget(instanceComboBox_);

  connect(instanceComboBox_, &QComboBox::currentIndexChanged, this,
          &MainWindow::onInstanceComboBox);

  QIcon::setThemeName("tango");

  ui_.actionNewComparison->setIcon(QIcon::fromTheme("document-new"));
  ui_.actionOpenComparison->setIcon(QIcon::fromTheme("document-open"));
  ui_.actionEditComparison->setIcon(QIcon::fromTheme("document-properties"));
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

  QString path = QFileDialog::getOpenFileName(this, "Open Comparison", QString(),
                                              "Comparisons (*.cml);;All files (*.*)");
  if (path.isEmpty())
  {
    return;
  }

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

    doc_->setLayout(Settings::defaultLayout(dialog.patterns().size()));
    doc_->setPatterns(dialog.patterns());
    onInstancesChanged();
    goToInstance(0);
  }
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
  if (!doc_)
  {
    ui_.mainView->setLayout(Layout{0, 0});
  }
  else if (doc_->instances().empty())
  {
    ui_.mainView->setLayout(Layout{0, 0});
    QMessageBox::information(this, "Information", "No pattern matches found");
  }
  else
  {
    ui_.mainView->setLayout(doc_->layout());
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
  ui_.actionSaveComparison->setEnabled(isModified);
  ui_.actionSaveComparisonAs->setEnabled(isOpen);
  ui_.actionCloseComparison->setEnabled(isOpen);
  ui_.actionZoomIn->setEnabled(hasInstances);
  ui_.actionZoomOut->setEnabled(hasInstances);
  ui_.actionZoom1to1->setEnabled(hasInstances);

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
  populateInstanceComboBox();
  updateDocumentDependentActions();
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
  QString path =
    QFileDialog::getSaveFileName(this, "Save Comparison", doc_->path(), "Comparisons (*.cml)");
  if (path.isEmpty())
  {
    return false;
  }

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
