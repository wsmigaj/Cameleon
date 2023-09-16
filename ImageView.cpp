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
  imageWidget_->hide();
  placeholderLabel_ = new QLabel(this);
  placeholderLabel_->setAlignment(Qt::AlignCenter);
  placeholderLabel_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  placeholderLabel_->show();

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(headerBar_);
  layout->addWidget(imageWidget_, 1);
  layout->addWidget(placeholderLabel_, 1);

  connect(imageWidget_, &ImageWidget::mouseMovedOverImage, this, &ImageView::onMouseMovedOverImage);
  connect(imageWidget_, &ImageWidget::mouseLeftImage, this, &ImageView::onMouseLeftImage);
}

ImageView::~ImageView()
{
}

void ImageView::loadImage(const QString& path)
{
  headerBar_->clearPixelProperties();
  const bool imageLoadedSuccessfully = !path.isEmpty() && imageWidget_->loadImage(path);
  imageWidget_->setVisible(imageLoadedSuccessfully);
  if (!imageLoadedSuccessfully)
  {
    QString msg;
    if (path.isEmpty())
    {
      msg = "No matching file.";
    }
    else
    {
      QFileInfo info(path);
      if (info.exists())
      {
        if (info.isDir())
          msg = "Path points to a directory.";
        else
          msg = "Image failed to load.";
      }
      else
      {
        msg = "File does not exist.";
      }
    }
    placeholderLabel_->setText(msg);
  }
  placeholderLabel_->setVisible(!imageLoadedSuccessfully);
}

void ImageView::setCaption(const QString& caption)
{
  headerBar_->setCaption(caption);
}

void ImageView::clear()
{
  headerBar_->clearId();
  headerBar_->clearCaption();
  headerBar_->clearPixelProperties();
  imageWidget_->loadImage(QString());
  imageWidget_->hide();
  placeholderLabel_->setText(QString("No matching file."));
  placeholderLabel_->show();
}

void ImageView::onMouseMovedOverImage(QPoint pixelCoords, QColor pixelColour)
{
  headerBar_->setPixelProperties(pixelCoords, pixelColour);
}

void ImageView::onMouseLeftImage()
{
  headerBar_->clearPixelProperties();
}
