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
#include "MainView.h"

#include "Document.h"
#include "HeaderBar.h"
#include "ImageView.h"
#include "ImageWidget.h"

MainView::MainView(QWidget* parent) : QWidget(parent)
{
  mainLayout_ = new QGridLayout(this);
}

MainView::~MainView()
{
}

void MainView::setPaths(std::vector<QString> paths)
{
  paths.resize(layout_.panels());
  paths_ = std::move(paths);
  reloadImages();
}

void MainView::clearPaths()
{
  setPaths({});
}

void MainView::reloadImages()
{
  QRectF unitedRect;
  for (size_t i = 0; i < paths_.size() && i < imageViews_.size(); ++i)
  {
    imageViews_[i]->loadImage(paths_[i]);
    unitedRect = unitedRect.united(imageViews_[i]->imageWidget()->imageRect());
  }
  for (size_t i = paths_.size(); i < imageViews_.size(); ++i)
  {
    imageViews_[i]->clear();
  }

  for (ImageView* imageView : imageViews_)
  {
    imageView->imageWidget()->setSceneRect(unitedRect);
  }
}

void MainView::setLayout(const Layout& layout)
{
  if (layout == layout_)
  {
    return;
  }

  layout_ = layout;

  // TODO: Make this exception-safe

  for (ImageView* view : imageViews_)
  {
    mainLayout_->removeWidget(view);
  }

  const int oldNumViews = imageViews_.size();
  const int numViews = layout.panels();

  for (int i = oldNumViews - 1; i >= numViews; --i)
  {
    imageViews_[i]->deleteLater();
    imageViews_.pop_back();
  }
  for (int i = imageViews_.size(); i < numViews; ++i)
  {
    ImageView* newView = new ImageView(this);
    ImageWidget* newImageWidget = newView->imageWidget();
    connect(newImageWidget->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            &MainView::onImageWidgetHorizontalScrollBarValueChanged);
    connect(newImageWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            &MainView::onImageWidgetVerticalScrollBarValueChanged);
    connect(newImageWidget, &ImageWidget::transformChanging, this,
            &MainView::onImageWidgetTransformChanging);
    connect(newImageWidget, &ImageWidget::transformChanged, this,
            &MainView::onImageWidgetTransformChanged);
    newImageWidget->setDragMode(QGraphicsView::ScrollHandDrag);
    newImageWidget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    newView->headerBar()->setLabel(QString(char('A' + i)));
    imageViews_.push_back(newView);

    // TODO: set the transform and scroll bar position of the new view to match those
    // of the existing views.
  }

  for (int row = 0, index = 0; row < layout.rows; ++row)
    for (int column = 0; column < layout.columns; ++column, ++index)
      mainLayout_->addWidget(imageViews_[index], row, column);

  for (int row = 0; row < layout.rows; ++row)
    mainLayout_->setRowStretch(row, 1);
  for (int row = layout.rows; row < mainLayout_->rowCount(); ++row)
    mainLayout_->setRowStretch(row, 0);
  for (int column = 0; column < layout.columns; ++column)
    mainLayout_->setColumnStretch(column, 1);
  for (int column = 0; column < mainLayout_->columnCount(); ++column)
    mainLayout_->setColumnStretch(column, 0);
}

//void MainView::onInstancesChanged()
//{
//  // In fact, if the patterns change, the most intuitive behaviour would be to:
//  // - update the layout (without reloading any images)
//  // - if a non-empty instance was being displayed, identify the new current instance most similar to that instance; otherwise use the instance index 0
//  // - set the current instance index to that instance and reload the images
//  onLayoutChanged();
//  int newInstanceIndex = 0;
//  if (doc_ && !std::all_of(currentInstance_.begin(), currentInstance_.end(), [](const QString &s) { return s.isEmpty(); }))
//  {
//    newInstanceIndex = doc_->closestInstanceIndex(currentInstance_);
//  }
//  setCurrentInstanceIndex(newInstanceIndex, ForceReload::YES);
//}

void MainView::zoom(double relativeScale)
{
  if (!imageViews_.empty())
  {
    imageViews_.front()->imageWidget()->zoom(relativeScale);
  }
}

void MainView::resetScale()
{
  if (!imageViews_.empty())
  {
    imageViews_.front()->imageWidget()->resetScale();
  }
}

void MainView::onImageWidgetTransformChanging()
{
  ++numOngoingTransformUpdates_;
}

void MainView::onImageWidgetTransformChanged(QTransform transform)
{
  ImageWidget* senderWidget = static_cast<ImageWidget*>(sender());
  for (ImageView* receiverView : imageViews_)
  {
    ImageWidget* receiverWidget = receiverView->imageWidget();
    if (receiverWidget != senderWidget)
    {
      {
        auto guard =
          qScopeGuard([receiverWidget, origAnchor = receiverWidget->transformationAnchor()]
                      { receiverWidget->setTransformationAnchor(origAnchor); });
        receiverWidget->setTransformationAnchor(QGraphicsView::NoAnchor);
        receiverWidget->setTransform(transform);
      }
      receiverWidget->horizontalScrollBar()->setValue(senderWidget->horizontalScrollBar()->value());
      receiverWidget->verticalScrollBar()->setValue(senderWidget->verticalScrollBar()->value());
    }
  }
  --numOngoingTransformUpdates_;
}

void MainView::onImageWidgetHorizontalScrollBarValueChanged(int value)
{
  if (numOngoingTransformUpdates_ > 0)
    return;

  for (ImageView* imageView : imageViews_)
  {
    QScrollBar* scrollBar = imageView->imageWidget()->horizontalScrollBar();
    if (scrollBar != sender())
      scrollBar->setValue(value);
  }
}

void MainView::onImageWidgetVerticalScrollBarValueChanged(int value)
{
  if (numOngoingTransformUpdates_ > 0)
    return;

  for (ImageView* imageView : imageViews_)
  {
    QScrollBar* scrollBar = imageView->imageWidget()->verticalScrollBar();
    if (scrollBar != sender())
      scrollBar->setValue(value);
  }
}
