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

#include "TestOpenAlbum.h"
#include "Document.h"
#include "MainWindow.h"
#include "TestDataDir.h"
#include "TestUtils.h"

#include <QAbstractButton>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTest>
#include <QTimer>

QTEST_MAIN(TestOpenAlbum)

void TestOpenAlbum::open_cancelOpen()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(action != nullptr);

  // All variables passed to timer callback functions (executed asynchronously) are wrapped with
  // shared pointers to ensure they remain alive until these callback functions finish execution.
  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  action->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() == nullptr);
}

void TestOpenAlbum::open_okOpen()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  action->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == TEST_DATA_DIR "/colours.cml");
  QVERIFY(!w.document()->modified());
}

void TestOpenAlbum::open_open_cancelOpen()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  action->trigger();
  QVERIFY(*asyncSuccess);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  action->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == TEST_DATA_DIR "/colours.cml");
  QVERIFY(!w.document()->modified());
}

void TestOpenAlbum::open_open_okOpen()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  action->trigger();
  QVERIFY(*asyncSuccess);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, TEST_DATA_DIR, "green-checkerboards.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  action->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == TEST_DATA_DIR "/green-checkerboards.cml");
  QVERIFY(!w.document()->modified());
}

void TestOpenAlbum::open_modify_open_cancelSave()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);

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
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QAbstractButton* btn = box->button(QMessageBox::Cancel);
                       btn->click();
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == TEST_DATA_DIR "/colours.cml");
  QVERIFY(w.document()->modified());
}

void TestOpenAlbum::open_modify_open_okSave_cancelOpen()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath));
  const QByteArray docOriginalContents = readFile(docPath);

  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QAbstractButton* btn = box->button(QMessageBox::Yes);
                       btn->click();

                       QTimer::singleShot(
                         0,
                         [asyncSuccess, tempDir]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                           QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                           QTest::keyClick(dlg, Qt::Key_Escape);
                         });
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == docPath);
  QVERIFY(!w.document()->modified());

  const QByteArray docCurrentContents = readFile(docPath);
  QVERIFY(docCurrentContents != docOriginalContents);
}

void TestOpenAlbum::open_modify_open_okSave_errorOnSave()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath));
  QFile::setPermissions(docPath, QFileDevice::ReadOwner);

  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QAbstractButton* btn = box->button(QMessageBox::Yes);
                       btn->click();

                       QTimer::singleShot(
                         0,
                         [asyncSuccess, tempDir]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                           QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                           QVERIFY(*asyncSuccess = box->text().contains("Could not open file"));
                           QTest::keyClick(box, Qt::Key_Enter);
                         });
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == docPath);
  QVERIFY(w.document()->modified());
}

void TestOpenAlbum::open_modify_open_okSave_okOpen()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString firstDocPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", firstDocPath));
  const QByteArray firstDocOriginalContents = readFile(firstDocPath);

  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QAbstractButton* btn = box->button(QMessageBox::Yes);
                       btn->click();

                       QTimer::singleShot(
                         0,
                         [asyncSuccess, tempDir]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                           QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                           selectFile(dlg, TEST_DATA_DIR, "green-checkerboards.cml");
                           QTest::keyClick(dlg, Qt::Key_Enter);
                         });
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == TEST_DATA_DIR "/green-checkerboards.cml");
  QVERIFY(!w.document()->modified());

  const QByteArray firstDocCurrentContents = readFile(firstDocPath);
  QVERIFY(firstDocCurrentContents != firstDocOriginalContents);
}

void TestOpenAlbum::open_modify_open_doNotSave_cancelOpen()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath));

  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QAbstractButton* btn = box->button(QMessageBox::No);
                       btn->click();

                       QTimer::singleShot(
                         0,
                         [asyncSuccess, tempDir]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                           QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                           QTest::keyClick(dlg, Qt::Key_Escape);
                         });
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == docPath);
  QVERIFY(w.document()->modified());
}

void TestOpenAlbum::open_modify_open_doNotSave_okOpen()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString firstDocPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", firstDocPath));
  const QByteArray firstDocOriginalContents = readFile(firstDocPath);

  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QAbstractButton* btn = box->button(QMessageBox::No);
                       btn->click();

                       QTimer::singleShot(
                         0,
                         [asyncSuccess, tempDir]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                           QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                           selectFile(dlg, TEST_DATA_DIR, "green-checkerboards.cml");
                           QTest::keyClick(dlg, Qt::Key_Enter);
                         });
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == TEST_DATA_DIR "/green-checkerboards.cml");
  QVERIFY(!w.document()->modified());

  const QByteArray firstDocCurrentContents = readFile(firstDocPath);
  QVERIFY(firstDocCurrentContents == firstDocOriginalContents);
}