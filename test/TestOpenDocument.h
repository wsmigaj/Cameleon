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

class TestOpenDocument : public QObject
{
  Q_OBJECT
private slots:
  void openDocument_cancelOpen();
  void openDocument_okOpen();
  void openDocument_openDocument_cancelOpen();
  void openDocument_openDocument_okOpen();
  void openDocument_modifyDocument_openDocument_cancelSave();
  void openDocument_modifyDocument_openDocument_okSave_cancelOpen();
  void openDocument_modifyDocument_openDocument_okSave_errorOnSave();
  void openDocument_modifyDocument_openDocument_okSave_okOpen();
  void openDocument_modifyDocument_openDocument_doNotSave_cancelOpen();
  void openDocument_modifyDocument_openDocument_doNotSave_okOpen();
};
