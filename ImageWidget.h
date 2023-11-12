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

#pragma once
#include <qgraphicsview.h>

class ImageWidget : public QGraphicsView
{
  Q_OBJECT

public:
  explicit ImageWidget(QWidget* parent = nullptr);
  ~ImageWidget() override;

  void setPath(const QString& path);
  void setPixmap(const QPixmap& pixmap);
  void clear();

  QRectF imageRect() const;

  void zoom(double relativeScale);
  void resetScale();

  bool eventFilter(QObject* watched, QEvent* event) override;

signals:
  void transformChanging();
  void transformChanged(QTransform transform);
  void mouseMovedOverImage(QPoint pixelCoords, QColor pixelColour);
  void mouseLeftImage();

protected:
  void wheelEvent(QWheelEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
  void onCopyImage();
  void onCopyFullPath();
  void onCopyFileName();
  void onOpenInExplorer();

private:
  void createActions();

private:
  QAction* copyImageAction_;
  QAction* copyFullPathAction_;
  QAction* copyFileNameAction_;
  QAction* openInExplorerAction_;

  QGraphicsScene scene_;
  QGraphicsPixmapItem* item_ = nullptr;
  QString path_;
  QImage image_;
};
