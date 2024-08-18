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

#pragma once

#include <QWidget>

namespace Ui
{
class HeaderBarClass;
}

class HeaderBar : public QWidget
{
  Q_OBJECT

public:
  HeaderBar(QWidget* parent = nullptr);
  ~HeaderBar() override;

  QString caption() const;

  void setId(const QString& id);
  void setCaption(const QString& caption);
  void setPixelProperties(const QPoint& pt, const QColor& colour);

  void clearId();
  void clearCaption();
  void clearPixelProperties();

private:
  std::unique_ptr<Ui::HeaderBarClass> ui_;
};
