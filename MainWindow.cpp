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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
  ui_.setupUi(this);
  mainLayout_ = new QGridLayout(ui_.mainView);
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_actionNewComparison_triggered()
{
  //   QMessageBox::information(this, "Info", "New comparison clicked");
  ComparisonDialog dialog(this);
  dialog.setWindowTitle("New Comparison");
  if (dialog.exec() == QDialog::Accepted)
  {
    // For now, we'll always reset to the first case in the sequence.
    // Later we might restore the case shown previously if certain conditions are met.

    doc_ = std::make_unique<Document>();
    doc_->setLayout(Layout{2, 2});
    doc_->setPatterns(dialog.patterns());

    // Let's assume the central widget is a MainVie.
    // view_ = new MainView(this);

    if (doc_->instances().empty())
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