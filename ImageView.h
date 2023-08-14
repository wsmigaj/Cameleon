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

class HeaderBar;
class ImageWidget;

class ImageView : public QWidget
{
  Q_OBJECT

public:
  explicit ImageView(QWidget* parent = nullptr);
  ~ImageView() override;

  void setLabel(const QString& label);
  void clearLabel();

  /// If `path` is empty, the image is cleared.
  void loadImage(const QString& path);
  void clear();

  void setCoordinates(const QPoint& pt);
  void clearCoordinates();

  void setColour(const QColor& colour);
  void clearColour();

  QRectF imageRect() const;

  void zoom(double relativeScale);
  void resetScale();

signals:
  void transformChanging();
  void transformChanged(QTransform transform);

private slots:
  void onImageWidgetTransformChanging();
  void onImageWidgetTransformChanged(QTransform transform);

private:
  HeaderBar* headerBar_ = nullptr;
  ImageWidget* imageWidget_ = nullptr;
};
