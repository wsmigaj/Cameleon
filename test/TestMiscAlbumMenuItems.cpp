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

#include "TestMiscAlbumMenuItems.h"
#include "AlbumEditorDialog.h"
#include "Document.h"
#include "Instance.h"
#include "MainWindow.h"
#include "TestDataDir.h"
#include "TestUtils.h"

#include <QAction>
#include <QAbstractButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTest>
#include <QTimer>

QTEST_MAIN(TestMiscAlbumMenuItems)

void TestMiscAlbumMenuItems::stateBeforeAlbumOpening()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);
  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* saveAction = w.findChild<QAction*>("actionSaveAlbum");
  QVERIFY(saveAction != nullptr);
  QAction* saveAsAction = w.findChild<QAction*>("actionSaveAlbumAs");
  QVERIFY(saveAsAction != nullptr);
  QAction* closeAction = w.findChild<QAction*>("actionCloseAlbum");
  QVERIFY(closeAction != nullptr);
  QAction* editAction = w.findChild<QAction*>("actionEditAlbum");
  QVERIFY(editAction != nullptr);
  QAction* quitAction = w.findChild<QAction*>("actionQuit");
  QVERIFY(quitAction != nullptr);

  QVERIFY(newAction->isEnabled());
  QVERIFY(openAction->isEnabled());
  QVERIFY(!saveAction->isEnabled());
  QVERIFY(!saveAsAction->isEnabled());
  QVERIFY(!closeAction->isEnabled());
  QVERIFY(!editAction->isEnabled());
  QVERIFY(quitAction->isEnabled());
}

void TestMiscAlbumMenuItems::stateAfterAlbumClosing()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);
  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* saveAction = w.findChild<QAction*>("actionSaveAlbum");
  QVERIFY(saveAction != nullptr);
  QAction* saveAsAction = w.findChild<QAction*>("actionSaveAlbumAs");
  QVERIFY(saveAsAction != nullptr);
  QAction* closeAction = w.findChild<QAction*>("actionCloseAlbum");
  QVERIFY(closeAction != nullptr);
  QAction* editAction = w.findChild<QAction*>("actionEditAlbum");
  QVERIFY(editAction != nullptr);
  QAction* quitAction = w.findChild<QAction*>("actionQuit");
  QVERIFY(quitAction != nullptr);

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

  QVERIFY(newAction->isEnabled());
  QVERIFY(openAction->isEnabled());
  QVERIFY(!saveAction->isEnabled());
  QVERIFY(!saveAsAction->isEnabled());
  QVERIFY(!closeAction->isEnabled());
  QVERIFY(!editAction->isEnabled());
  QVERIFY(quitAction->isEnabled());
}

void TestMiscAlbumMenuItems::save()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath));
  const QByteArray originalDocContents = readFile(docPath);

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* saveAction = w.findChild<QAction*>("actionSaveAlbum");
  QVERIFY(saveAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QVERIFY(!saveAction->isEnabled());
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
  QVERIFY(!saveAction->isEnabled());

  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());
  QVERIFY(saveAction->isEnabled());

  saveAction->trigger();
  QVERIFY(!saveAction->isEnabled());

  QVERIFY(!w.document()->modified());
  const QByteArray currentDocContents = readFile(docPath);
  QVERIFY(currentDocContents != originalDocContents);
}

void TestMiscAlbumMenuItems::saveAs_cancelSave()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath1 = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath1));

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* saveAsAction = w.findChild<QAction*>("actionSaveAlbumAs");
  QVERIFY(saveAsAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QVERIFY(!saveAsAction->isEnabled());
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
  QVERIFY(saveAsAction->isEnabled());

  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());
  QVERIFY(saveAsAction->isEnabled());

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  saveAsAction->trigger();
  QVERIFY(saveAsAction->isEnabled());

  QVERIFY(w.document()->modified());
}

void TestMiscAlbumMenuItems::saveAs_okSave()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath1 = tempDir->filePath("colours.cml");
  QVERIFY(QFile::copy(TEST_DATA_DIR "/colours.cml", docPath1));
  const QByteArray originalDocContents = readFile(docPath1);

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* saveAsAction = w.findChild<QAction*>("actionSaveAlbumAs");
  QVERIFY(saveAsAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QVERIFY(!saveAsAction->isEnabled());
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
  QVERIFY(saveAsAction->isEnabled());

  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());
  QVERIFY(saveAsAction->isEnabled());

  const QString docPath2 = tempDir->filePath("colours_v2.cml");
  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->path(), "colours_v2.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  saveAsAction->trigger();
  QVERIFY(saveAsAction->isEnabled());

  QVERIFY(!w.document()->modified());
  const QByteArray currentDocContents = readFile(docPath2);
  QVERIFY(currentDocContents != originalDocContents);
}

void TestMiscAlbumMenuItems::useRelativePaths()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  QDir tempQDir(tempDir->path());

  QVERIFY(tempQDir.mkpath("1/blue"));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/checkerboard.png",
                      tempDir->filePath("1/blue/checkerboard.png")));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/inverted_checkerboard.png",
                      tempDir->filePath("1/blue/inverted_checkerboard.png")));

  QVERIFY(tempQDir.mkpath("2/blue"));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/checkerboard.png",
                      tempDir->filePath("2/blue/checkerboard.png")));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/inverted_checkerboard.png",
                      tempDir->filePath("2/blue/inverted_checkerboard.png")));

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);
  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* closeAction = w.findChild<QAction*>("actionCloseAlbum");
  QVERIFY(closeAction != nullptr);
  QAction* saveAction = w.findChild<QAction*>("actionSaveAlbum");
  QVERIFY(saveAction != nullptr);
  QAction* useRelativePathsAction = w.findChild<QAction*>("actionUseRelativePathsInSavedAlbum");
  QVERIFY(useRelativePathsAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({tempDir->filePath("1/blue/*.png")});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  newAction->trigger();
  QVERIFY(*asyncSuccess);

  useRelativePathsAction->trigger();

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->filePath("1"), "checkerboards.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  saveAction->trigger();

  closeAction->trigger();
  QVERIFY(w.document() == nullptr);

  QVERIFY(QFile::rename(tempDir->filePath("1/checkerboards.cml"),
                        tempDir->filePath("2/checkerboards.cml")));

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       selectFile(dlg, tempDir->filePath("2"), "checkerboards.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->instances().size() == 2);
  QVERIFY(w.document()->instances()[0].paths.size() == 1);
  QVERIFY(w.document()->instances()[0].paths[0] ==
          QDir::toNativeSeparators(tempDir->filePath("2/blue/checkerboard.png")));
  QVERIFY(w.document()->instances()[1].paths.size() == 1);
  QVERIFY(w.document()->instances()[1].paths[0] ==
          QDir::toNativeSeparators(tempDir->filePath("2/blue/inverted_checkerboard.png")));
}

void TestMiscAlbumMenuItems::edit_cancelEdit()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* editAction = w.findChild<QAction*>("actionEditAlbum");
  QVERIFY(editAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QVERIFY(!editAction->isEnabled());
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
  QVERIFY(!w.document()->modified());
  QVERIFY(editAction->isEnabled());

  const std::vector<QString> patterns = w.document()->patterns();
  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  editAction->trigger();
  QVERIFY(editAction->isEnabled());
  QVERIFY(!w.document()->modified());
  QVERIFY(w.document()->patterns() == patterns);
}

void TestMiscAlbumMenuItems::edit_okEdit()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* editAction = w.findChild<QAction*>("actionEditAlbum");
  QVERIFY(editAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QVERIFY(!editAction->isEnabled());
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
  QVERIFY(!w.document()->modified());
  QVERIFY(editAction->isEnabled());

  const std::vector<QString> origPatterns = w.document()->patterns();
  const size_t origNumInstances = w.document()->instances().size();
  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({TEST_DATA_DIR "/black/*", TEST_DATA_DIR "/blue/*"});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  editAction->trigger();
  QVERIFY(editAction->isEnabled());
  QVERIFY(w.document()->modified());
  QVERIFY(w.document()->patterns() != origPatterns);
  QVERIFY(w.document()->instances().size() != origNumInstances);
}

void TestMiscAlbumMenuItems::editWithInconsistentNumberOfWildcardsPerPattern()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* editAction = w.findChild<QAction*>("actionEditAlbum");
  QVERIFY(editAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QVERIFY(!editAction->isEnabled());
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
  QVERIFY(!w.document()->modified());
  QVERIFY(editAction->isEnabled());

  const std::vector<QString> origPatterns = w.document()->patterns();
  const size_t origNumInstances = w.document()->instances().size();
  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
      AlbumEditorDialog* dlg = dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
      dlg->setValues({TEST_DATA_DIR "/black/*", TEST_DATA_DIR "/*/*"});

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
  editAction->trigger();
  QVERIFY(editAction->isEnabled());
  QVERIFY(!w.document()->modified());
  QVERIFY(w.document()->patterns() == origPatterns);
}

void TestMiscAlbumMenuItems::refresh()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QDir tempQDir(tempDir->path());

  QVERIFY(tempQDir.mkdir("blue"));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/checkerboard.png",
                      tempDir->filePath("blue/checkerboard.png")));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/inverted_checkerboard.png",
                      tempDir->filePath("blue/inverted_checkerboard.png")));

  QVERIFY(tempQDir.mkdir("green"));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/green/checkerboard.png",
                      tempDir->filePath("green/checkerboard.png")));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/green/inverted_checkerboard.png",
                      tempDir->filePath("green/inverted_checkerboard.png")));

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);
  QAction* refreshAction = w.findChild<QAction*>("actionRefreshAlbum");
  QVERIFY(refreshAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({tempDir->filePath("*/checkerboard.png"),
                                       tempDir->filePath("*/inverted_checkerboard.png")});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  newAction->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->instances().size() == 2);

  QVERIFY(w.document()->instanceKey(w.instance()) == "blue");

  QVERIFY(tempQDir.mkdir("black"));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/black/checkerboard.png",
                      tempDir->filePath("black/checkerboard.png")));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/black/inverted_checkerboard.png",
                      tempDir->filePath("black/inverted_checkerboard.png")));

  refreshAction->trigger();
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->instances().size() == 3);
  // The active instance should be preserved.
  QVERIFY(w.document()->instanceKey(w.instance()) == "blue");

  tempQDir.remove("blue/checkerboard.png");
  tempQDir.remove("blue/inverted_checkerboard.png");
  QVERIFY(tempQDir.rmdir("blue"));

  refreshAction->trigger();
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->instances().size() == 2);
  // The previously active instance has been removed, so the 0th instance should be activated instead.
  QVERIFY(w.instance() == 0);
}

void TestMiscAlbumMenuItems::refreshAfterDeletingAllInstances()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());
  const QString docPath = tempDir->filePath("colours.cml");
  QDir tempQDir(tempDir->path());

  QVERIFY(tempQDir.mkdir("blue"));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/checkerboard.png",
                      tempDir->filePath("blue/checkerboard.png")));
  QVERIFY(QFile::copy(TEST_DATA_DIR "/blue/inverted_checkerboard.png",
                      tempDir->filePath("blue/inverted_checkerboard.png")));

  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* newAction = w.findChild<QAction*>("actionNewAlbum");
  QVERIFY(newAction != nullptr);
  QAction* refreshAction = w.findChild<QAction*>("actionRefreshAlbum");
  QVERIFY(refreshAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);
  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({tempDir->filePath("*/checkerboard.png"),
                                       tempDir->filePath("*/inverted_checkerboard.png")});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  newAction->trigger();

  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->instances().size() == 1);

  tempQDir.remove("blue/checkerboard.png");
  tempQDir.remove("blue/inverted_checkerboard.png");
  QVERIFY(tempQDir.rmdir("blue"));

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QMessageBox>());
                       QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
                       QVERIFY(box->text() == "No pattern matches found.");
                       QTest::keyClick(box, Qt::Key_Enter);
                     });
  refreshAction->trigger();
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->instances().size() == 0);
  QVERIFY(w.instance() == 0);
}

void TestMiscAlbumMenuItems::open_close()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* closeAction = w.findChild<QAction*>("actionCloseAlbum");
  QVERIFY(closeAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QVERIFY(!closeAction->isEnabled());

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
  QVERIFY(w.document() != nullptr);
  QVERIFY(closeAction->isEnabled());

  closeAction->trigger();

  QVERIFY(w.document() == nullptr);
  QVERIFY(!closeAction->isEnabled());
}

void TestMiscAlbumMenuItems::open_modify_close_cancelSave()
{
  const QString docPath = TEST_DATA_DIR "/colours.cml";
  const QByteArray originalDocContents = readFile(docPath);

  MainWindow w = createMainWindowForTest();
  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* closeAction = w.findChild<QAction*>("actionCloseAlbum");
  QVERIFY(openAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = QTest::qWaitFor(
                [] { return dynamic_cast<QFileDialog*>(qApp->activeModalWidget()) != nullptr; }));
      QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
      selectFile(dlg, TEST_DATA_DIR, "colours.cml");
      QTest::keyClick(dlg, Qt::Key_Enter);
    });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(closeAction->isEnabled());
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = QTest::qWaitFor(
                [] { return dynamic_cast<QMessageBox*>(qApp->activeModalWidget()) != nullptr; }));
      QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
      QAbstractButton* btn = box->button(QMessageBox::Cancel);
      btn->click();
    });
  closeAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(closeAction->isEnabled());

  const QByteArray currentDocContents = readFile(docPath);
  QVERIFY(currentDocContents == originalDocContents);
}

void TestMiscAlbumMenuItems::open_modify_close_doNotSave()
{
  const QString docPath = TEST_DATA_DIR "/colours.cml";
  const QByteArray originalDocContents = readFile(docPath);

  MainWindow w = createMainWindowForTest();
  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* closeAction = w.findChild<QAction*>("actionCloseAlbum");
  QVERIFY(openAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = QTest::qWaitFor(
                [] { return dynamic_cast<QFileDialog*>(qApp->activeModalWidget()) != nullptr; }));
      QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
      selectFile(dlg, TEST_DATA_DIR, "colours.cml");
      QTest::keyClick(dlg, Qt::Key_Enter);
    });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(closeAction->isEnabled());
  bookmarkAction->trigger();
  QVERIFY(w.document()->modified());

  QTimer::singleShot(
    0,
    [asyncSuccess]
    {
      QVERIFY(*asyncSuccess = QTest::qWaitFor(
                [] { return dynamic_cast<QMessageBox*>(qApp->activeModalWidget()) != nullptr; }));
      QMessageBox* box = dynamic_cast<QMessageBox*>(qApp->activeModalWidget());
      QAbstractButton* btn = box->button(QMessageBox::No);
      btn->click();
    });
  closeAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() == nullptr);
  QVERIFY(!closeAction->isEnabled());

  const QByteArray currentDocContents = readFile(docPath);
  QVERIFY(currentDocContents == originalDocContents);
}

void TestMiscAlbumMenuItems::quit()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* quitAction = w.findChild<QAction*>("actionQuit");
  QVERIFY(quitAction != nullptr);

  quitAction->trigger();
  QVERIFY(QTest::qWaitFor([&w] { return !w.isVisible(); }));
}

void TestMiscAlbumMenuItems::open_quit()
{
  MainWindow w = createMainWindowForTest();

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* quitAction = w.findChild<QAction*>("actionQuit");
  QVERIFY(quitAction != nullptr);

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
  QVERIFY(w.document() != nullptr);

  quitAction->trigger();
  QVERIFY(QTest::qWaitFor([&w] { return !w.isVisible(); }));
}

void TestMiscAlbumMenuItems::open_modify_quit_cancelSave()
{
  const QString docPath = TEST_DATA_DIR "/colours.cml";
  const QByteArray originalDocContents = readFile(docPath);

  MainWindow w = createMainWindowForTest();
  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* quitAction = w.findChild<QAction*>("actionQuit");
  QVERIFY(quitAction != nullptr);

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
  quitAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(w.document() != nullptr);
  QVERIFY(w.isVisible());

  const QByteArray currentDocContents = readFile(docPath);
  QVERIFY(currentDocContents == originalDocContents);
}

void TestMiscAlbumMenuItems::open_modify_quit_doNotSave()
{
  const QString docPath = TEST_DATA_DIR "/colours.cml";
  const QByteArray originalDocContents = readFile(docPath);

  MainWindow w = createMainWindowForTest();
  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* bookmarkAction = w.findChild<QAction*>("actionBookmarkPage");
  QVERIFY(bookmarkAction != nullptr);
  QAction* quitAction = w.findChild<QAction*>("actionQuit");
  QVERIFY(quitAction != nullptr);

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
                       QAbstractButton* btn = box->button(QMessageBox::No);
                       btn->click();
                     });
  quitAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(QTest::qWaitFor([&w] { return !w.isVisible(); }));
  QVERIFY(w.document() == nullptr);

  const QByteArray currentDocContents = readFile(docPath);
  QVERIFY(currentDocContents == originalDocContents);
}
