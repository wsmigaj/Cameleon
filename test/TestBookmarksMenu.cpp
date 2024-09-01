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

#include "TestBookmarksMenu.h"
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

QTEST_MAIN(TestBookmarksMenu)

void TestBookmarksMenu::stateBeforeAlbumOpening()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* bookmarkPageAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkPageAction != nullptr);
  QAction* removeAllBookmarksPageAction = w.findChild<QAction*>("actionRemoveAllBookmarks");
  QVERIFY(removeAllBookmarksPageAction != nullptr);
  QAction* firstBookmarkAction = w.findChild<QAction*>("actionFirstBookmark");
  QVERIFY(firstBookmarkAction != nullptr);
  QAction* previousBookmarkAction = w.findChild<QAction*>("actionPreviousBookmark");
  QVERIFY(previousBookmarkAction != nullptr);
  QAction* nextBookmarkAction = w.findChild<QAction*>("actionNextBookmark");
  QVERIFY(nextBookmarkAction != nullptr);
  QAction* lastBookmarkAction = w.findChild<QAction*>("actionLastBookmark");
  QVERIFY(lastBookmarkAction != nullptr);
  QAction* exportBookmarksAction = w.findChild<QAction*>("actionExportBookmarks");
  QVERIFY(exportBookmarksAction != nullptr);
  QAction* importBookmarksAction = w.findChild<QAction*>("actionImportBookmarks");
  QVERIFY(importBookmarksAction != nullptr);

  QVERIFY(!bookmarkPageAction->isEnabled());
  QVERIFY(!removeAllBookmarksPageAction->isEnabled());
  QVERIFY(!firstBookmarkAction->isEnabled());
  QVERIFY(!previousBookmarkAction->isEnabled());
  QVERIFY(!nextBookmarkAction->isEnabled());
  QVERIFY(!lastBookmarkAction->isEnabled());
  QVERIFY(!exportBookmarksAction->isEnabled());
  QVERIFY(!importBookmarksAction->isEnabled());
}

void TestBookmarksMenu::navigation()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkPageAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkPageAction != nullptr);

  QAction* firstInstanceAction = w.findChild<QAction*>("actionFirstInstance");
  QVERIFY(firstInstanceAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);

  QAction* firstBookmarkAction = w.findChild<QAction*>("actionFirstBookmark");
  QVERIFY(firstBookmarkAction != nullptr);
  QAction* previousBookmarkAction = w.findChild<QAction*>("actionPreviousBookmark");
  QVERIFY(previousBookmarkAction != nullptr);
  QAction* nextBookmarkAction = w.findChild<QAction*>("actionNextBookmark");
  QVERIFY(nextBookmarkAction != nullptr);
  QAction* lastBookmarkAction = w.findChild<QAction*>("actionLastBookmark");
  QVERIFY(lastBookmarkAction != nullptr);

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

  QVERIFY(!firstBookmarkAction->isEnabled());
  QVERIFY(!previousBookmarkAction->isEnabled());
  QVERIFY(!nextBookmarkAction->isEnabled());
  QVERIFY(!lastBookmarkAction->isEnabled());

  // Bookmark pages 1 and 3.
  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  nextInstanceAction->trigger();
  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  firstInstanceAction->trigger();

  QVERIFY(w.instance() == 0);
  QVERIFY(firstBookmarkAction->isEnabled());
  QVERIFY(!previousBookmarkAction->isEnabled());
  QVERIFY(nextBookmarkAction->isEnabled());
  QVERIFY(lastBookmarkAction->isEnabled());

  nextBookmarkAction->trigger();
  QVERIFY(w.instance() == 1);
  QVERIFY(!firstBookmarkAction->isEnabled());
  QVERIFY(!previousBookmarkAction->isEnabled());
  QVERIFY(nextBookmarkAction->isEnabled());
  QVERIFY(lastBookmarkAction->isEnabled());

  nextInstanceAction->trigger();
  QVERIFY(w.instance() == 2);
  QVERIFY(firstBookmarkAction->isEnabled());
  QVERIFY(previousBookmarkAction->isEnabled());
  QVERIFY(nextBookmarkAction->isEnabled());
  QVERIFY(lastBookmarkAction->isEnabled());

  lastBookmarkAction->trigger();
  QVERIFY(w.instance() == 3);
  QVERIFY(firstBookmarkAction->isEnabled());
  QVERIFY(previousBookmarkAction->isEnabled());
  QVERIFY(!nextBookmarkAction->isEnabled());
  QVERIFY(!lastBookmarkAction->isEnabled());

  nextInstanceAction->trigger();
  QVERIFY(w.instance() == 4);
  QVERIFY(firstBookmarkAction->isEnabled());
  QVERIFY(previousBookmarkAction->isEnabled());
  QVERIFY(!nextBookmarkAction->isEnabled());
  QVERIFY(lastBookmarkAction->isEnabled());

  previousBookmarkAction->trigger();
  QVERIFY(w.instance() == 3);
  QVERIFY(firstBookmarkAction->isEnabled());
  QVERIFY(previousBookmarkAction->isEnabled());
  QVERIFY(!nextBookmarkAction->isEnabled());
  QVERIFY(!lastBookmarkAction->isEnabled());

  firstBookmarkAction->trigger();
  QVERIFY(w.instance() == 1);
  QVERIFY(!firstBookmarkAction->isEnabled());
  QVERIFY(!previousBookmarkAction->isEnabled());
  QVERIFY(nextBookmarkAction->isEnabled());
  QVERIFY(lastBookmarkAction->isEnabled());
}

void TestBookmarksMenu::stateAfterAlbumClosing()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* bookmarkPageAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkPageAction != nullptr);
  QAction* removeAllBookmarksPageAction = w.findChild<QAction*>("actionRemoveAllBookmarks");
  QVERIFY(removeAllBookmarksPageAction != nullptr);
  QAction* firstBookmarkAction = w.findChild<QAction*>("actionFirstBookmark");
  QVERIFY(firstBookmarkAction != nullptr);
  QAction* previousBookmarkAction = w.findChild<QAction*>("actionPreviousBookmark");
  QVERIFY(previousBookmarkAction != nullptr);
  QAction* nextBookmarkAction = w.findChild<QAction*>("actionNextBookmark");
  QVERIFY(nextBookmarkAction != nullptr);
  QAction* lastBookmarkAction = w.findChild<QAction*>("actionLastBookmark");
  QVERIFY(lastBookmarkAction != nullptr);
  QAction* exportBookmarksAction = w.findChild<QAction*>("actionExportBookmarks");
  QVERIFY(exportBookmarksAction != nullptr);
  QAction* importBookmarksAction = w.findChild<QAction*>("actionImportBookmarks");
  QVERIFY(importBookmarksAction != nullptr);

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

  QVERIFY(!bookmarkPageAction->isEnabled());
  QVERIFY(!removeAllBookmarksPageAction->isEnabled());
  QVERIFY(!firstBookmarkAction->isEnabled());
  QVERIFY(!previousBookmarkAction->isEnabled());
  QVERIFY(!nextBookmarkAction->isEnabled());
  QVERIFY(!lastBookmarkAction->isEnabled());
  QVERIFY(!exportBookmarksAction->isEnabled());
  QVERIFY(!importBookmarksAction->isEnabled());
}

void TestBookmarksMenu::bookmarkPage()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkPageAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkPageAction != nullptr);

  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);

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

  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  QVERIFY(bookmarkPageAction->isChecked());
  QVERIFY(w.document()->bookmarks() == std::set<size_t>{1});

  bookmarkPageAction->trigger();
  QVERIFY(!bookmarkPageAction->isChecked());
  QVERIFY(w.document()->bookmarks().empty());
}

void TestBookmarksMenu::removeAllBookmarks_cancelRemoval()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkPageAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkPageAction != nullptr);
  QAction* removeAllBookmarksPageAction = w.findChild<QAction*>("actionRemoveAllBookmarks");
  QVERIFY(removeAllBookmarksPageAction != nullptr);

  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);

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

  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  QVERIFY(w.document()->bookmarks() == std::set<size_t>({1, 2}));

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QVERIFY(box->text().contains("Do you wish to remove all bookmarks?"));
                       QAbstractButton* btn = box->button(QMessageBox::No);
                       btn->click();
                     });
  removeAllBookmarksPageAction->trigger();
  QVERIFY(w.document()->bookmarks() == std::set<size_t>({1, 2}));
}

void TestBookmarksMenu::removeAllBookmarks_okRemoval()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkPageAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkPageAction != nullptr);
  QAction* removeAllBookmarksPageAction = w.findChild<QAction*>("actionRemoveAllBookmarks");
  QVERIFY(removeAllBookmarksPageAction != nullptr);

  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);

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

  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  QVERIFY(w.document()->bookmarks() == std::set<size_t>({1, 2}));

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QVERIFY(box->text().contains("Do you wish to remove all bookmarks?"));
                       QAbstractButton* btn = box->button(QMessageBox::Yes);
                       btn->click();
                     });
  removeAllBookmarksPageAction->trigger();
  QVERIFY(w.document()->bookmarks().empty());
  QVERIFY(!bookmarkPageAction->isChecked());
}

void TestBookmarksMenu::exportAndImport()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkPageAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkPageAction != nullptr);
  QAction* exportBookmarksAction = w.findChild<QAction*>("actionExportBookmarks");
  QVERIFY(exportBookmarksAction != nullptr);
  QAction* importBookmarksAction = w.findChild<QAction*>("actionImportBookmarks");
  QVERIFY(importBookmarksAction != nullptr);

  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);
  QAction* previousInstanceAction = w.findChild<QAction*>("actionPreviousInstance");
  QVERIFY(previousInstanceAction != nullptr);

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

  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  QVERIFY(w.document()->bookmarks() == std::set<size_t>({1, 2, 3}));

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "bookmarks.txt");
                       QTest::keyClick(dlg, Qt::Key_Enter);

                       QTimer::singleShot(
                         0,
                         [asyncSuccess]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                           QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                           QVERIFY(box->text().contains("3 bookmarks have been exported."));
                           QAbstractButton* btn = box->button(QMessageBox::Ok);
                           btn->click();
                         });
                     });
  exportBookmarksAction->trigger();

  nextInstanceAction->trigger();
  bookmarkPageAction->trigger();
  previousInstanceAction->trigger();
  bookmarkPageAction->trigger();
  previousInstanceAction->trigger();
  bookmarkPageAction->trigger();
  QVERIFY(w.document()->bookmarks() == std::set<size_t>({1, 4}));

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "bookmarks.txt");
                       QTest::keyClick(dlg, Qt::Key_Enter);

                       QTimer::singleShot(
                         0,
                         [asyncSuccess]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                           QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                           QVERIFY(box->text().contains("2 bookmarks have been imported."));
                           QAbstractButton* btn = box->button(QMessageBox::Ok);
                           btn->click();
                         });
                     });
  importBookmarksAction->trigger();
  QVERIFY(w.document()->bookmarks() == std::set<size_t>({1, 2, 3, 4}));
}