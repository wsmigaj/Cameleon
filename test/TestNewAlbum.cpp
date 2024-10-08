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

#include "TestNewAlbum.h"
#include "AlbumEditorDialog.h"
#include "Document.h"
#include "MainWindow.h"
#include "TestDataDir.h"
#include "TestUtils.h"

#include <QAbstractButton>
#include <QAction>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QTest>
#include <QTimer>

QTEST_MAIN(TestNewAlbum)

void TestNewAlbum::new_cancelNew()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(action != nullptr);

  // All variables passed to timer callback functions (executed asynchronously) are wrapped with
  // shared pointers to ensure they remain alive until these callback functions finish execution.
  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  action->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() == nullptr);
}

void TestNewAlbum::new_okNew()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({TEST_DATA_DIR "/red/*", TEST_DATA_DIR "/blue/*"});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  action->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path().isEmpty());
  QVERIFY(!w.document()->instances().empty());
  QVERIFY(w.document()->modified());
}

void TestNewAlbum::newAlbumWithoutPatterns()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
      AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
      QTest::keyClick(dlg, Qt::Key_Enter);

      QTimer::singleShot(0,
                         [asyncSuccess]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                           QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                           QVERIFY(*asyncSuccess = (box->text() == "No pattern matches found."));
                           QTest::keyClick(box, Qt::Key_Enter);
                         });
    });
  action->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path().isEmpty());
  QVERIFY(w.document()->instances().empty());
  QVERIFY(!w.document()->modified());
}

void TestNewAlbum::newAlbumWithoutMatches()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
      AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
      dlg->setValues({TEST_DATA_DIR "/purple/*"});
      QTest::keyClick(dlg, Qt::Key_Enter);

      QTimer::singleShot(0,
                         [asyncSuccess]
                         {
                           QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                           QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                           QVERIFY(*asyncSuccess = (box->text() == "No pattern matches found."));
                           QTest::keyClick(box, Qt::Key_Enter);
                         });
    });
  action->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path().isEmpty());
  QVERIFY(w.document()->instances().empty());
  QVERIFY(w.document()->modified());
}

void TestNewAlbum::newAlbumWithInconsistentNumberOfWildcardsPerPattern()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* action = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
      AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
      dlg->setValues({TEST_DATA_DIR "/purple/*", TEST_DATA_DIR "/*/*"});

      QTimer::singleShot(
        0,
        [asyncSuccess]
        {
          QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
          QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
          QVERIFY(*asyncSuccess =
                    (box->text() == "The number of wildcards must be the same in all paths "
                                    "containing any wildcards."));
          QTest::keyClick(box, Qt::Key_Enter);

          QTimer::singleShot(
            0,
            [asyncSuccess]
            {
              QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
              AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
              QTest::keyClick(dlg, Qt::Key_Escape);
            });
        });
      QTest::keyClick(dlg, Qt::Key_Enter);
    });
  action->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() == nullptr);
}

void TestNewAlbum::open_new_cancelNew()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == QDir::toNativeSeparators(TEST_DATA_DIR "/colours.cml"));
  QVERIFY(!w.document()->modified());
}

void TestNewAlbum::open_new_okNew()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({TEST_DATA_DIR "/red/*", TEST_DATA_DIR "/blue/*"});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path().isEmpty());
  QVERIFY(w.document()->modified());
}

void TestNewAlbum::open_modify_new_cancelSave()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == QDir::toNativeSeparators(TEST_DATA_DIR "/colours.cml"));
  QVERIFY(w.document()->modified());
}

void TestNewAlbum::open_modify_new_okSave_cancelNew()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath));
  const QByteArray docOriginalContents = readFile(docPath);

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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

  QTimer::singleShot(
    0,
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
          QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
          AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
          QTest::keyClick(dlg, Qt::Key_Escape);
        });
    });
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == QDir::toNativeSeparators(docPath));
  QVERIFY(!w.document()->modified());

  const QByteArray docCurrentContents = readFile(docPath);
  QVERIFY(docCurrentContents != docOriginalContents);
}

void TestNewAlbum::open_modify_new_okSave_errorOnSave()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath));
  QFile::setPermissions(docPath, QFileDevice::ReadOwner);

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == QDir::toNativeSeparators(docPath));
  QVERIFY(w.document()->modified());
}

void TestNewAlbum::open_modify_new_okSave_okNew()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString firstDocPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", firstDocPath));
  const QByteArray firstDocOriginalContents = readFile(firstDocPath);

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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

  QTimer::singleShot(
    0,
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
          QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
          AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
          dlg->setValues({TEST_DATA_DIR "/red/*", TEST_DATA_DIR "/blue/*"});
          QTest::keyClick(dlg, Qt::Key_Enter);
        });
    });
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path().isEmpty());
  QVERIFY(w.document()->modified());

  const QByteArray firstDocCurrentContents = readFile(firstDocPath);
  QVERIFY(firstDocCurrentContents != firstDocOriginalContents);
}

void TestNewAlbum::open_modify_new_doNotSave_cancelNew()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath));

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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

  QTimer::singleShot(
    0,
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
          QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
          AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
          QTest::keyClick(dlg, Qt::Key_Escape);
        });
    });
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path() == QDir::toNativeSeparators(docPath));
  QVERIFY(w.document()->modified());
}

void TestNewAlbum::open_modify_new_doNotSave_okNew()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString firstDocPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", firstDocPath));
  const QByteArray firstDocOriginalContents = readFile(firstDocPath);

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);

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

  QTimer::singleShot(
    0,
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
          QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
          AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
          dlg->setValues({TEST_DATA_DIR "/red/*", TEST_DATA_DIR "/blue/*"});
          QTest::keyClick(dlg, Qt::Key_Enter);
        });
    });
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->path().isEmpty());
  QVERIFY(w.document()->modified());

  const QByteArray firstDocCurrentContents = readFile(firstDocPath);
  QVERIFY(firstDocCurrentContents == firstDocOriginalContents);
}