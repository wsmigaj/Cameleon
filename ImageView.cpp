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
  layout->addWidget(headerBar_);
  layout->addWidget(imageWidget_, 1);

  connect(imageWidget_, &ImageWidget::transformChanging, this,
          &ImageView::onImageWidgetTransformChanging);
  connect(imageWidget_, &ImageWidget::transformChanged, this,
          &ImageView::onImageWidgetTransformChanged);
}

ImageView::~ImageView()
{
}

void ImageView::setLabel(const QString& label)
{
  headerBar_->setLabel(label);
}

void ImageView::clearLabel()
{
  headerBar_->clearLabel();
}

void ImageView::setCoordinates(const QPoint& pt)
{
  headerBar_->setCoordinates(pt);
}

void ImageView::clearCoordinates()
{
  headerBar_->clearCoordinates();
}

void ImageView::setColour(const QColor& colour)
{
  headerBar_->setColour(colour);
}

void ImageView::clearColour()
{
  headerBar_->clearColour();
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

QRectF ImageView::imageRect() const
{
  return imageWidget_->imageRect();
}

void ImageView::zoom(double relativeScale)
{
  imageWidget_->zoom(relativeScale);
}

void ImageView::resetScale()
{
  imageWidget_->resetScale();
}

void ImageView::onImageWidgetTransformChanging()
{
  emit transformChanging();
}

void ImageView::onImageWidgetTransformChanged(QTransform transform)
{
  emit transformChanged(transform);
}
