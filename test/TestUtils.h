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

#include "MainWindow.h"

#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QTest>

inline MainWindow createMainWindowForTest()
{
  return MainWindow(nullptr /*parent*/, true /*dontUseNativeDialogs*/,
                    true /*dontPromptToRegisterFileType*/);
}

inline QByteArray readFile(const QString& filename)
{
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  return file.readAll();
}

template <typename T>
bool waitForActiveModalWidgetOfType(int timeout = 5000)
{
  return QTest::qWaitFor([] { return dynamic_cast<T*>(qApp->activeModalWidget()) != nullptr; });
}

inline void selectFile(QFileDialog* dlg, const QString& directory, const QString& fileName)
{
  dlg->setDirectory(directory);
  QLineEdit* lineEdit = dlg->findChild<QLineEdit*>();
  QVERIFY(lineEdit != nullptr);
  lineEdit->setText(fileName);
}