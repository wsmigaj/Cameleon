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

  void setPath(const QString& path);
  void setPixmap(const QPixmap& pixmap);
  void setMessage(const QString& msg);
  void clear();

  void setCaption(const QString& caption);

  HeaderBar* headerBar() { return headerBar_; }
  const HeaderBar* headerBar() const { return headerBar_; }

  ImageWidget* imageWidget() { return imageWidget_; }
  const ImageWidget* imageWidget() const { return imageWidget_; }

private slots:
  void onMouseMovedOverImage(QPoint pixelCoords, QColor pixelColour);
  void onMouseLeftImage();

private:
  HeaderBar* headerBar_ = nullptr;
  ImageWidget* imageWidget_ = nullptr;
  QLabel* placeholderLabel_ = nullptr;
};
