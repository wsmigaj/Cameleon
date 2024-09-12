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

#include "CameleonApplication.h"
#include "MainWindow.h"

#include <QFileOpenEvent>

bool CameleonApplication::event(QEvent* event)
{
  if (event->type() == QEvent::FileOpen)
  {
    QFileOpenEvent* openEvent = static_cast<QFileOpenEvent*>(event);
    const QUrl url = openEvent->url();
    if (url.isLocalFile())
    {
      for (QWidget* widget : QApplication::topLevelWidgets())
      {
        if (MainWindow* mainWindow = dynamic_cast<MainWindow*>(widget))
        {
          mainWindow->openDocument(url.toLocalFile());
          return true;
        }
      }
    }
  }

  return QApplication::event(event);
}