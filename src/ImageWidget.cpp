// This file is part of Caméléon.
//
// Copyright (C) 2023-2024 Wojciech Śmigaj
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

#include "ImageWidget.h"

#include <iostream>

ImageWidget::ImageWidget(QWidget* parent) : QGraphicsView(parent)
{
  setBackgroundRole(QPalette::Dark);
  setScene(&scene_);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  scene_.installEventFilter(this);

  createActions();
}

ImageWidget::~ImageWidget()
{
  scene_.removeEventFilter(this);
}

void ImageWidget::createActions()
{
  copyImageAction_ = new QAction("Copy &Image", this);
  connect(copyImageAction_, &QAction::triggered, this, &ImageWidget::onCopyImage);
  copyFullPathAction_ = new QAction("Copy Full &Path", this);
  connect(copyFullPathAction_, &QAction::triggered, this, &ImageWidget::onCopyFullPath);
  copyFileNameAction_ = new QAction("Copy File &Name", this);
  connect(copyFileNameAction_, &QAction::triggered, this, &ImageWidget::onCopyFileName);
  copyInstanceKeyAction_ = new QAction("Copy Page &Title", this);
  connect(copyInstanceKeyAction_, &QAction::triggered, this, &ImageWidget::onCopyInstanceKey);
  openInExplorerAction_ = new QAction("&Open In Explorer", this);
  connect(openInExplorerAction_, &QAction::triggered, this, &ImageWidget::onOpenInExplorer);
}

void ImageWidget::setPath(const QString& path)
{
  path_ = path;
}

void ImageWidget::setInstanceKey(const QString& instanceKey)
{
  instanceKey_ = instanceKey;
}

void ImageWidget::setPixmap(const QPixmap& pixmap)
{
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
  setPath(QString());
  setPixmap(QPixmap());
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
        QPointF pointF = mouseSceneEvent->scenePos();
        pointF.setX(std::floor(pointF.x()));
        pointF.setY(std::floor(pointF.y()));
        const QPoint point = pointF.toPoint();
        emit mouseMovedOverImage(point, image_.pixelColor(point));
      }
      else
      {
        emit mouseLeftImage();
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

void ImageWidget::contextMenuEvent(QContextMenuEvent* event)
{
  if (!image_.isNull())
  {
    QMenu menu(this);
    menu.addAction(copyImageAction_);
    menu.addAction(copyFullPathAction_);
    menu.addAction(copyFileNameAction_);
    menu.addAction(copyInstanceKeyAction_);
#ifdef Q_OS_WIN
    menu.addSeparator();
    menu.addAction(openInExplorerAction_);
#endif
    menu.exec(event->globalPos());
  }
}

void ImageWidget::onCopyImage()
{
  if (!image_.isNull())
  {
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setImage(image_);
  }
}

void ImageWidget::onCopyInstanceKey()
{
  QClipboard* clipboard = QGuiApplication::clipboard();
  clipboard->setText(instanceKey_);
}

void ImageWidget::onCopyFullPath()
{
  if (!path_.isEmpty())
  {
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(path_);
  }
}

void ImageWidget::onCopyFileName()
{
  if (!path_.isEmpty())
  {
    QFileInfo fileInfo(path_);
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(fileInfo.fileName());
  }
}

void ImageWidget::onOpenInExplorer()
{
  if (!path_.isEmpty())
  {
    QStringList params;
    params += QLatin1String("/select,");
    params += QDir::toNativeSeparators(path_);
    QProcess::startDetached("explorer.exe", params);
  }
}
