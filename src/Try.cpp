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

#include "Try.h"
#include "CancellationException.h"

namespace
{
QMainWindow* mainWindow()
{
  for (QWidget* w : qApp->topLevelWidgets())
    if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
      return mainWin;
  return nullptr;
}
} // namespace

bool Try(const std::function<void()>& f)
{
  try
  {
    f();
    return true;
  }
  catch (CancellationException&)
  {
    return false;
  }
  catch (std::exception& ex)
  {
    QMessageBox::warning(mainWindow(), "Warning", ex.what());
    return false;
  }
  catch (...)
  {
    QMessageBox::warning(mainWindow(), "Warning", "An unidentified problem occurred.");
    return false;
  }
}