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
#include "HeaderBar.h"
#include "ImageView.h"
#include "ImageWidget.h"

ImageView::ImageView(QWidget* parent) : QWidget(parent)
{
  headerBar_ = new HeaderBar(this);
  imageWidget_ = new ImageWidget(this);

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(headerBar_);
  layout->addWidget(imageWidget_, 1);

  connect(imageWidget_, &ImageWidget::mouseMovedOverImage, this, &ImageView::onMouseMovedOverImage);
  connect(imageWidget_, &ImageWidget::mouseLeftImage, this, &ImageView::onMouseLeftImage);
}

ImageView::~ImageView()
{
}

void ImageView::loadImage(const QString& path)
{
  headerBar_->setPath(path);
  headerBar_->clearColour();
  headerBar_->clearCoordinates();
  imageWidget_->loadImage(path);
}

void ImageView::clear()
{
  headerBar_->clearLabel();
  headerBar_->clearPath();
  headerBar_->clearColour();
  headerBar_->clearCoordinates();
  imageWidget_->loadImage(QString());
}

void ImageView::onMouseMovedOverImage(QPoint pixelCoords, QColor pixelColour)
{
  headerBar_->setColour(pixelColour);
  headerBar_->setCoordinates(pixelCoords);
}

void ImageView::onMouseLeftImage()
{
  headerBar_->clearColour();
  headerBar_->clearCoordinates();
}
