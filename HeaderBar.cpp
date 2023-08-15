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
  ui.pixelLabel->setMinimumWidth(ui.pixelLabel->fontMetrics()
                                   .size(0, "(X: 0000, Y: 0000)   (R: 000, G: 000, B: 000, A: 000)")
                                   .width());
}

HeaderBar::~HeaderBar()
{
}

void HeaderBar::setLabel(const QString& label)
{
  ui.mainLabel->setText(label);
}

void HeaderBar::setPath(const QString& path)
{
  ui.pathLabel->setText(path);
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

void HeaderBar::clearLabel()
{
  ui.mainLabel->setText(QString());
}

void HeaderBar::clearPath()
{
  ui.pathLabel->setText(QString());
}

void HeaderBar::clearPixelProperties()
{
  ui.pixelLabel->setText(QString());
}
