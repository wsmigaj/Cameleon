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
  mainLayout_ = new QGridLayout(ui_.centralWidget);
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
    doc_->setPatterns(dialog.patterns(), 2, 2);
    updateLayout(doc_->numRows(), doc_->numColumns());
    // TODO: Handle the case of no matching patterns.
    if (doc_->numCases() == 0)
    {
      QMessageBox::information(this, "Information", "No images found");
    }
    updateImageViews();
  }
}

void MainWindow::on_actionZoomIn_triggered()
{
  if (!imageViews_.empty())
  {
    imageViews_.front()->zoom(1.25);
  }
}

void MainWindow::on_actionZoomOut_triggered()
{
  if (!imageViews_.empty())
  {
    imageViews_.front()->zoom(1.0 / 1.25);
  }
}

void MainWindow::on_actionZoom1to1_triggered()
{
  if (!imageViews_.empty())
  {
    imageViews_.front()->resetScale();
  }
}

void MainWindow::updateLayout(int numRows, int numColumns)
{
  if (numRows == mainLayout_->rowCount() && numColumns == mainLayout_->columnCount())
  {
    return;
  }

  for (ImageView* view : imageViews_)
  {
    mainLayout_->removeWidget(view);
  }

  const int oldNumViews = imageViews_.size();
  const int numViews = numRows * numColumns;

  for (int i = oldNumViews - 1; i >= numViews; --i)
  {
    imageViews_[i]->deleteLater();
    imageViews_.pop_back();
  }
  for (int i = imageViews_.size(); i < numViews; ++i)
  {
    ImageView* newView = new ImageView(this);
    connect(newView->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            &MainWindow::onImageViewHorizontalScrollBarValueChanged);
    connect(newView->verticalScrollBar(), &QScrollBar::valueChanged, this,
            &MainWindow::onImageViewVerticalScrollBarValueChanged);
    connect(newView, &ImageView::transformChanging, this,
            &MainWindow::onImageViewTransformChanging);
    connect(newView, &ImageView::transformChanged, this, &MainWindow::onImageViewTransformChanged);
    newView->setDragMode(QGraphicsView::ScrollHandDrag);
    newView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    imageViews_.push_back(newView);
  }

  for (int row = 0, index = 0; row < numRows; ++row)
    for (int column = 0; column < numColumns; ++column, ++index)
      mainLayout_->addWidget(imageViews_[index], row, column);

  for (int row = 0; row < numRows; ++row)
    mainLayout_->setRowStretch(row, 1);
  for (int column = 0; column < numColumns; ++column)
    mainLayout_->setColumnStretch(column, 1);

  Q_ASSERT(mainLayout_->rowCount() == numRows);
  Q_ASSERT(mainLayout_->columnCount() == numColumns);
}

void MainWindow::updateImageViews()
{
  const std::vector<QString> paths = doc_->currentCase();
  Q_ASSERT(paths.size() == imageViews_.size());

  QRectF unitedRect;
  for (size_t i = 0; i < paths.size(); ++i)
  {
    imageViews_[i]->loadImage(paths[i]);
    unitedRect = unitedRect.united(imageViews_[i]->imageRect());
  }

  for (size_t i = 0; i < paths.size(); ++i)
  {
    imageViews_[i]->setSceneRect(unitedRect);
  }
}

void MainWindow::onImageViewTransformChanging()
{
  ++numOngoingTransformUpdates_;
}

void MainWindow::onImageViewTransformChanged(QTransform transform)
{
  ImageView* senderView = static_cast<ImageView*>(sender());
  for (ImageView* imageView : imageViews_)
  {
    if (imageView != senderView)
    {
      {
        auto guard = qScopeGuard([imageView, origAnchor = imageView->transformationAnchor()]
                                 { imageView->setTransformationAnchor(origAnchor); });
        imageView->setTransformationAnchor(QGraphicsView::NoAnchor);
        imageView->setTransform(transform);
      }
      imageView->horizontalScrollBar()->setValue(senderView->horizontalScrollBar()->value());
      imageView->verticalScrollBar()->setValue(senderView->verticalScrollBar()->value());
    }
  }
  --numOngoingTransformUpdates_;
}

void MainWindow::onImageViewHorizontalScrollBarValueChanged(int value)
{
  if (numOngoingTransformUpdates_ > 0)
    return;

  for (ImageView* imageView : imageViews_)
  {
    QScrollBar* scrollBar = imageView->horizontalScrollBar();
    if (scrollBar != sender())
      scrollBar->setValue(value);
  }
}

void MainWindow::onImageViewVerticalScrollBarValueChanged(int value)
{
  if (numOngoingTransformUpdates_ > 0)
    return;

  for (ImageView* imageView : imageViews_)
  {
    QScrollBar* scrollBar = imageView->verticalScrollBar();
    if (scrollBar != sender())
      scrollBar->setValue(value);
  }
}

// TODO:
// 1. Zoom from menu  -> transforms
