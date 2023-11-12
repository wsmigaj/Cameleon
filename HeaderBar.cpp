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

HeaderBar::HeaderBar(QWidget* parent) : QWidget(parent)
{
  ui.setupUi(this);
}

HeaderBar::~HeaderBar()
{
}

void HeaderBar::setId(const QString& id)
{
  ui.idLabel->setText(id);
}

void HeaderBar::setCaption(const QString& caption)
{
  ui.captionLabel->setText(caption);
}

void HeaderBar::setPixelProperties(const QPoint& pt, const QColor& colour)
{
  ui.pixelLabel->setText(QString("(X: %1, Y: %2)   (R: %3, G: %4, B: %5, A: %6)")
                           .arg(pt.x())
                           .arg(pt.y())
                           .arg(colour.red())
                           .arg(colour.green())
                           .arg(colour.blue())
                           .arg(colour.alpha()));
}

void HeaderBar::clearId()
{
  ui.idLabel->setText(QString());
}

void HeaderBar::clearCaption()
{
  ui.captionLabel->setText(QString());
}

void HeaderBar::clearPixelProperties()
{
  ui.pixelLabel->setText(QString());
}
