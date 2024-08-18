// This file is part of Caméléon.
//
// Copyright (C) 2024 Wojciech Śmigaj
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

#include <QObject>

class TestOpenAlbum : public QObject
{
  Q_OBJECT
private slots:
  void open_cancelOpen();
  void open_okOpen();
  void open_open_cancelOpen();
  void open_open_okOpen();
  void open_modify_open_cancelSave();
  void open_modify_open_okSave_cancelOpen();
  void open_modify_open_okSave_errorOnSave();
  void open_modify_open_okSave_okOpen();
  void open_modify_open_doNotSave_cancelOpen();
  void open_modify_open_doNotSave_okOpen();
};
