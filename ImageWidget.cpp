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
#include "ImageWidget.h"

#include <iostream>

ImageWidget::ImageWidget(QWidget* parent) : QGraphicsView(parent)
{
  setBackgroundRole(QPalette::Dark);
  setScene(&scene_);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  scene_.installEventFilter(this);
}

void ImageWidget::loadImage(const QString& path)
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
  image_ = pixmap.toImage();
}

void ImageWidget::clear()
{
  loadImage(QString());
}

QRectF ImageWidget::imageRect() const
{
  if (item_)
    return item_->boundingRect();
  else
    return QRectF();
}

void ImageWidget::zoom(double relativeScale)
{
  auto guard = qScopeGuard([this] { transformChanged(transform()); });
  transformChanging();
  scale(relativeScale, relativeScale);
}

void ImageWidget::resetScale()
{
  auto guard = qScopeGuard([this] { transformChanged(transform()); });
  transformChanging();
  resetTransform();
}

void ImageWidget::wheelEvent(QWheelEvent* event)
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

bool ImageWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == &scene_ && item_ && !item_->pixmap().isNull())
  {
    if (event->type() == QEvent::GraphicsSceneMouseMove)
    {
      QGraphicsSceneMouseEvent* mouseSceneEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
      if (item_->boundingRect().contains(mouseSceneEvent->scenePos()))
      {
        const QPoint point = mouseSceneEvent->scenePos().toPoint();
        emit mouseMovedOverImage(point, image_.pixelColor(point));
      }
    }
    else if (event->type() == QEvent::GraphicsSceneLeave)
    {
      QGraphicsSceneMouseEvent* mouseSceneEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
      emit mouseLeftImage();
    }
  }
  return QGraphicsView::eventFilter(watched, event);
}
