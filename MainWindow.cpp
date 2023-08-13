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
#include "Settings.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
  ui_.setupUi(this);
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
  if (dialog.exec() != QDialog::Accepted)
  {
    return;
  }

  doc_ = std::make_unique<Document>();
  doc_->setLayout(Settings::defaultLayout(dialog.patterns().size()));
  doc_->setPatterns(dialog.patterns());

  connectDocumentSignals();
  updateMainView();
  updateDocumentDependentActions();
}

void MainWindow::on_actionOpenComparison_triggered()
{
  if (!maybeSaveDocument())
  {
    return;
  }

  QString path = QFileDialog::getOpenFileName(this, "Open Comparison", QString(),
                                              "Comparisons (*.cml)|All files (*.*)");
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
  updateMainView();
  updateDocumentDependentActions();
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

    doc_->setPatterns(dialog.patterns());

    if (doc_->instances().empty())
    {
      ui_.mainView->setLayout(Layout{0, 0});
      QMessageBox::information(this, "Information", "No pattern matches found");
    }
    else
    {
      ui_.mainView->setLayout(doc_->layout());
      // TODO: Replace this with navigateToInstance(...). Add a function updating the enable status navigation buttons.
      ui_.mainView->setPaths(doc_->instances().front());
    }
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
  updateMainView();
  updateDocumentDependentActions();
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
}

void MainWindow::on_actionPreviousInstance_triggered()
{
}

void MainWindow::on_actionNextInstance_triggered()
{
}

void MainWindow::on_actionLastInstance_triggered()
{
}

void MainWindow::onDocumentModificationStatusChanged()
{
  updateDocumentModificationStatusDependentActions();
}

void MainWindow::connectDocumentSignals()
{
  connect(doc_.get(), &Document::modificationStatusChanged, this,
          &MainWindow::onDocumentModificationStatusChanged);
}

void MainWindow::updateMainView()
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
    ui_.mainView->setPaths(doc_->instances().front());
  }
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
}

void MainWindow::updateDocumentModificationStatusDependentActions()
{
  const bool isOpen = doc_ != nullptr;
  const bool isModified = isOpen && doc_->modified();
  ui_.actionSaveComparison->setEnabled(isModified);
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
