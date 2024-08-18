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

#include "HeaderBar.h"
#include "ui_HeaderBar.h"

HeaderBar::HeaderBar(QWidget* parent) : QWidget(parent), ui_(std::make_unique<Ui::HeaderBarClass>())
{
  ui_->setupUi(this);
}

HeaderBar::~HeaderBar()
{
}

QString HeaderBar::caption() const
{
  return ui_->captionLabel->text();
}

void HeaderBar::setId(const QString& id)
{
  ui_->idLabel->setText(id);
}

void HeaderBar::setCaption(const QString& caption)
{
  ui_->captionLabel->setText(caption);
}

void HeaderBar::setPixelProperties(const QPoint& pt, const QColor& colour)
{
  ui_->pixelLabel->setText(QString("(X: %1, Y: %2)   (R: %3, G: %4, B: %5, A: %6)")
                             .arg(pt.x())
                             .arg(pt.y())
                             .arg(colour.red())
                             .arg(colour.green())
                             .arg(colour.blue())
                             .arg(colour.alpha()));
}

void HeaderBar::clearId()
{
  ui_->idLabel->setText(QString());
}

void HeaderBar::clearCaption()
{
  ui_->captionLabel->setText(QString());
}

void HeaderBar::clearPixelProperties()
{
  ui_->pixelLabel->setText(QString());
}
