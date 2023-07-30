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
#include "ImageView.h"

#include <iostream>

ImageView::ImageView(QWidget* parent) : QGraphicsView(parent)
{
  setBackgroundRole(QPalette::Dark);
  setScene(&scene_);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void ImageView::loadImage(const QString& path)
{
  QPixmap pixmap;
  if (!path.isEmpty())
    pixmap.load(path);

  if (!item_)
  {
    item_ = new QGraphicsPixmapItem(pixmap);
    scene_.addItem(item_);
  }
  else
  {
    item_->setPixmap(pixmap);
  }
}

void ImageView::clear()
{
  loadImage(QString());
}

QRectF ImageView::imageRect() const
{
  if (item_)
    return item_->boundingRect();
  else
    return QRectF();
}

void ImageView::zoom(double relativeScale)
{
  auto guard = qScopeGuard([this] { transformChanged(transform()); });
  transformChanging();
  scale(relativeScale, relativeScale);
}

void ImageView::resetScale()
{
  auto guard = qScopeGuard([this] { transformChanged(transform()); });
  transformChanging();
  resetTransform();
}

void ImageView::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers() & Qt::CTRL)
  {
    zoom(std::pow(1.25, event->angleDelta().y() / 120.));
    event->accept();
  }
  else
  {
    QGraphicsView::wheelEvent(event);
  }
}
