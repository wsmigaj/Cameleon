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

void HeaderBar::setLabel(const QString& label)
{
  ui.mainLabel->setText(label);
}

void HeaderBar::setPath(const QString& path)
{
  ui.pathLabel->setText(path);
}

void HeaderBar::setCoordinates(const QPoint& pt)
{
  ui.coordLabel->setText(QString("(%1, %2)").arg(pt.x()).arg(pt.y()));
}

void HeaderBar::setColour(const QColor& colour)
{
  ui.colourLabel->setText(QString("(%1, %2, %3, %4)")
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

void HeaderBar::clearCoordinates()
{
  ui.coordLabel->setText(QString());
}

void HeaderBar::clearColour()
{
  ui.colourLabel->setText(QString());
}
