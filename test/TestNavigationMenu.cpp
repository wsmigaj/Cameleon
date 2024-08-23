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

#include "TestNavigationMenu.h"
#include "AlbumEditorDialog.h"
#include "Document.h"
#include "MainWindow.h"
#include "TestDataDir.h"
#include "TestUtils.h"

#include <QAction>
#include <QAbstractButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTest>
#include <QTimer>

QTEST_MAIN(TestNavigationMenu)

void TestNavigationMenu::stateBeforeAlbumOpening()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* firstInstanceAction = w.findChild<QAction*>("actionFirstInstance");
  QVERIFY(firstInstanceAction != nullptr);
  QAction* previousInstanceAction = w.findChild<QAction*>("actionPreviousInstance");
  QVERIFY(previousInstanceAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);
  QAction* lastInstanceAction = w.findChild<QAction*>("actionLastInstance");
  QVERIFY(lastInstanceAction != nullptr);

  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(!nextInstanceAction->isEnabled());
  QVERIFY(!lastInstanceAction->isEnabled());
}

void TestNavigationMenu::navigationInAlbumWith0Pages()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* firstInstanceAction = w.findChild<QAction*>("actionFirstInstance");
  QVERIFY(firstInstanceAction != nullptr);
  QAction* previousInstanceAction = w.findChild<QAction*>("actionPreviousInstance");
  QVERIFY(previousInstanceAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);
  QAction* lastInstanceAction = w.findChild<QAction*>("actionLastInstance");
  QVERIFY(lastInstanceAction != nullptr);

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "no-matches.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);

                       QTimer::singleShot(
                         0,
                         [asyncSuccess]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                           QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                           QVERIFY(*asyncSuccess = (box->text() == "No pattern matches found."));
                           QTest::keyClick(box, Qt::Key_Enter);
                         });
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(!nextInstanceAction->isEnabled());
  QVERIFY(!lastInstanceAction->isEnabled());
}

void TestNavigationMenu::navigationInAlbumWith1Page()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* firstInstanceAction = w.findChild<QAction*>("actionFirstInstance");
  QVERIFY(firstInstanceAction != nullptr);
  QAction* previousInstanceAction = w.findChild<QAction*>("actionPreviousInstance");
  QVERIFY(previousInstanceAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);
  QAction* lastInstanceAction = w.findChild<QAction*>("actionLastInstance");
  QVERIFY(lastInstanceAction != nullptr);

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "red-checkerboard.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.instance() == 0);
  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(!nextInstanceAction->isEnabled());
  QVERIFY(!lastInstanceAction->isEnabled());
}

void TestNavigationMenu::navigationInAlbumWith2Pages()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* firstInstanceAction = w.findChild<QAction*>("actionFirstInstance");
  QVERIFY(firstInstanceAction != nullptr);
  QAction* previousInstanceAction = w.findChild<QAction*>("actionPreviousInstance");
  QVERIFY(previousInstanceAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);
  QAction* lastInstanceAction = w.findChild<QAction*>("actionLastInstance");
  QVERIFY(lastInstanceAction != nullptr);

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "green-checkerboards.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.instance() == 0);
  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(nextInstanceAction->isEnabled());
  QVERIFY(lastInstanceAction->isEnabled());

  nextInstanceAction->trigger();
  QVERIFY(w.instance() == 1);
  QVERIFY(firstInstanceAction->isEnabled());
  QVERIFY(previousInstanceAction->isEnabled());
  QVERIFY(!nextInstanceAction->isEnabled());
  QVERIFY(!lastInstanceAction->isEnabled());

  previousInstanceAction->trigger();
  QVERIFY(w.instance() == 0);
  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(nextInstanceAction->isEnabled());
  QVERIFY(lastInstanceAction->isEnabled());

  lastInstanceAction->trigger();
  QVERIFY(w.instance() == 1);
  QVERIFY(firstInstanceAction->isEnabled());
  QVERIFY(previousInstanceAction->isEnabled());
  QVERIFY(!nextInstanceAction->isEnabled());
  QVERIFY(!lastInstanceAction->isEnabled());

  firstInstanceAction->trigger();
  QVERIFY(w.instance() == 0);
  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(nextInstanceAction->isEnabled());
  QVERIFY(lastInstanceAction->isEnabled());
}

void TestNavigationMenu::navigationInAlbumWith5Pages()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* firstInstanceAction = w.findChild<QAction*>("actionFirstInstance");
  QVERIFY(firstInstanceAction != nullptr);
  QAction* previousInstanceAction = w.findChild<QAction*>("actionPreviousInstance");
  QVERIFY(previousInstanceAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);
  QAction* lastInstanceAction = w.findChild<QAction*>("actionLastInstance");
  QVERIFY(lastInstanceAction != nullptr);

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.instance() == 0);
  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(nextInstanceAction->isEnabled());
  QVERIFY(lastInstanceAction->isEnabled());

  nextInstanceAction->trigger();
  QVERIFY(w.instance() == 1);
  QVERIFY(firstInstanceAction->isEnabled());
  QVERIFY(previousInstanceAction->isEnabled());
  QVERIFY(nextInstanceAction->isEnabled());
  QVERIFY(lastInstanceAction->isEnabled());

  lastInstanceAction->trigger();
  QVERIFY(w.instance() == 4);
  QVERIFY(firstInstanceAction->isEnabled());
  QVERIFY(previousInstanceAction->isEnabled());
  QVERIFY(!nextInstanceAction->isEnabled());
  QVERIFY(!lastInstanceAction->isEnabled());

  previousInstanceAction->trigger();
  QVERIFY(w.instance() == 3);
  QVERIFY(firstInstanceAction->isEnabled());
  QVERIFY(previousInstanceAction->isEnabled());
  QVERIFY(nextInstanceAction->isEnabled());
  QVERIFY(lastInstanceAction->isEnabled());

  firstInstanceAction->trigger();
  QVERIFY(w.instance() == 0);
  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(nextInstanceAction->isEnabled());
  QVERIFY(lastInstanceAction->isEnabled());
}

void TestNavigationMenu::stateAfterAlbumClosing()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* firstInstanceAction = w.findChild<QAction*>("actionFirstInstance");
  QVERIFY(firstInstanceAction != nullptr);
  QAction* previousInstanceAction = w.findChild<QAction*>("actionPreviousInstance");
  QVERIFY(previousInstanceAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);
  QAction* lastInstanceAction = w.findChild<QAction*>("actionLastInstance");
  QVERIFY(lastInstanceAction != nullptr);

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* closeAction = w.findChild<QAction*>("actionCloseAlbum");
  QVERIFY(closeAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  closeAction->trigger();

  QVERIFY(!firstInstanceAction->isEnabled());
  QVERIFY(!previousInstanceAction->isEnabled());
  QVERIFY(!nextInstanceAction->isEnabled());
  QVERIFY(!lastInstanceAction->isEnabled());
}
