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

#include "TestViewMenu.h"
#include "AlbumEditorDialog.h"
#include "Document.h"
#include "HeaderBar.h"
#include "ImageView.h"
#include "ImageWidget.h"
#include "MainWindow.h"
#include "MainView.h"
#include "TestDataDir.h"
#include "TestUtils.h"

#include <QAction>
#include <QAbstractButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTest>
#include <QTimer>

QTEST_MAIN(TestViewMenu)

namespace
{
std::vector<QRectF> panelImageRects(const MainWindow& w)
{
  std::vector<QRectF> rects;
  for (const ImageView* imageView : w.mainView()->imageViews())
    rects.push_back(imageView->imageWidget()->imageRect());
  return rects;
}
std::vector<QTransform> panelTransforms(const MainWindow& w)
{
  std::vector<QTransform> transforms;
  for (const ImageView* imageView : w.mainView()->imageViews())
    transforms.push_back(imageView->imageWidget()->transform());
  return transforms;
}
} // namespace

void TestViewMenu::stateBeforeAlbumOpening()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* zoomInAction = w.findChild<QAction*>("actionZoomIn");
  QVERIFY(zoomInAction != nullptr);
  QAction* zoomOutAction = w.findChild<QAction*>("actionZoomOut");
  QVERIFY(zoomOutAction != nullptr);
  QAction* zoom1to1Action = w.findChild<QAction*>("actionZoom1to1");
  QVERIFY(zoom1to1Action != nullptr);
  QAction* editCaptionsAction = w.findChild<QAction*>("actionEditCaptions");
  QVERIFY(editCaptionsAction != nullptr);
  QAction* saveScreenshotAction = w.findChild<QAction*>("actionSaveScreenshot");
  QVERIFY(saveScreenshotAction != nullptr);

  QVERIFY(!zoomInAction->isEnabled());
  QVERIFY(!zoomOutAction->isEnabled());
  QVERIFY(!zoom1to1Action->isEnabled());
  QVERIFY(!editCaptionsAction->isEnabled());
  QVERIFY(!saveScreenshotAction->isEnabled());
}

void TestViewMenu::stateAfterAlbumClosing()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* zoomInAction = w.findChild<QAction*>("actionZoomIn");
  QVERIFY(zoomInAction != nullptr);
  QAction* zoomOutAction = w.findChild<QAction*>("actionZoomOut");
  QVERIFY(zoomOutAction != nullptr);
  QAction* zoom1to1Action = w.findChild<QAction*>("actionZoom1to1");
  QVERIFY(zoom1to1Action != nullptr);
  QAction* editCaptionsAction = w.findChild<QAction*>("actionEditCaptions");
  QVERIFY(editCaptionsAction != nullptr);
  QAction* saveScreenshotAction = w.findChild<QAction*>("actionSaveScreenshot");
  QVERIFY(saveScreenshotAction != nullptr);

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
                       dlg->selectFile(TEST_DATA_DIR "/colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  closeAction->trigger();

  QVERIFY(!zoomInAction->isEnabled());
  QVERIFY(!zoomOutAction->isEnabled());
  QVERIFY(!zoom1to1Action->isEnabled());
  QVERIFY(!editCaptionsAction->isEnabled());
  QVERIFY(!saveScreenshotAction->isEnabled());
}

void TestViewMenu::zoom()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);

  QAction* zoomInAction = w.findChild<QAction*>("actionZoomIn");
  QVERIFY(zoomInAction != nullptr);
  QAction* zoomOutAction = w.findChild<QAction*>("actionZoomOut");
  QVERIFY(zoomOutAction != nullptr);
  QAction* zoom1to1Action = w.findChild<QAction*>("actionZoom1to1");
  QVERIFY(zoom1to1Action != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       dlg->selectFile(TEST_DATA_DIR "/colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(zoomInAction->isEnabled());
  QVERIFY(zoomOutAction->isEnabled());
  QVERIFY(zoom1to1Action->isEnabled());

  const std::vector<QTransform> initialTransforms = panelTransforms(w);

  zoomInAction->trigger();
  QVERIFY(QTest::qWaitFor(
    [&w]
    {
      const std::vector<QTransform> transforms = panelTransforms(w);
      return (transforms[0].m11() > 1.1 && transforms[0].m22() > 1.1 && transforms[1].m11() > 1.1 &&
              transforms[1].m22() > 1.1);
    }));
  zoomOutAction->trigger();
  QVERIFY(QTest::qWaitFor(
    [&w, &initialTransforms]
    {
      const std::vector<QTransform> transforms = panelTransforms(w);
      return qFuzzyCompare(transforms[0], initialTransforms[0]) &&
             qFuzzyCompare(transforms[1], initialTransforms[1]);
    }));

  zoomInAction->trigger();
  QVERIFY(QTest::qWaitFor(
    [&w]
    {
      const std::vector<QTransform> transforms = panelTransforms(w);
      return (transforms[0].m11() > 1.1 && transforms[0].m22() > 1.1 && transforms[1].m11() > 1.1 &&
              transforms[1].m22() > 1.1);
    }));

  zoom1to1Action->trigger();
  QVERIFY(QTest::qWaitFor(
    [&w, &initialTransforms]
    {
      const std::vector<QTransform> transforms = panelTransforms(w);
      return transforms[0] == initialTransforms[0] && transforms[1] == initialTransforms[1];
    }));
}

void TestViewMenu::editCaptions_cancelEdit()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);

  QAction* editCaptionsAction = w.findChild<QAction*>("actionEditCaptions");
  QVERIFY(editCaptionsAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       dlg->selectFile(TEST_DATA_DIR "/colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(editCaptionsAction->isEnabled());

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({"Checkerboard", "%p"});
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  editCaptionsAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->captionTemplates() == std::vector<QString>(2, "%p"));
  QVERIFY(w.mainView()->imageViews().size() == 2);
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() ==
          w.document()->instances()[0].paths[0]);
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() ==
          w.document()->instances()[0].paths[1]);

  nextInstanceAction->trigger();
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() ==
          w.document()->instances()[1].paths[0]);
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() ==
          w.document()->instances()[1].paths[1]);
}

void TestViewMenu::editCaptions_okEdit()
{
  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);
  QAction* nextInstanceAction = w.findChild<QAction*>("actionNextInstance");
  QVERIFY(nextInstanceAction != nullptr);

  QAction* editCaptionsAction = w.findChild<QAction*>("actionEditCaptions");
  QVERIFY(editCaptionsAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       dlg->selectFile(TEST_DATA_DIR "/colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document() != nullptr);
  QVERIFY(w.document()->captionTemplates() == std::vector<QString>(2, "%p"));
  QVERIFY(w.mainView()->imageViews().size() == 2);
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() ==
          w.document()->instances()[0].paths[0]);
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() ==
          w.document()->instances()[0].paths[1]);

  nextInstanceAction->trigger();
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() ==
          w.document()->instances()[1].paths[0]);
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() ==
          w.document()->instances()[1].paths[1]);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({"Checkerboard", "%p"});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  editCaptionsAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document()->captionTemplates() == std::vector<QString>({"Checkerboard", "%p"}));
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() == "Checkerboard");
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() ==
          w.document()->instances()[1].paths[1]);

  nextInstanceAction->trigger();
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() == "Checkerboard");
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() ==
          w.document()->instances()[2].paths[1]);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<AlbumEditorDialog>());
                       AlbumEditorDialog* dlg =
                         dynamic_cast<AlbumEditorDialog*>(qApp->activeModalWidget());
                       dlg->setValues({"%p", "Inverted checkerboard"});
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  editCaptionsAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(w.document()->captionTemplates() ==
          std::vector<QString>({"%p", "Inverted checkerboard"}));
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() ==
          w.document()->instances()[2].paths[0]);
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() == "Inverted checkerboard");

  nextInstanceAction->trigger();
  QVERIFY(w.mainView()->imageViews()[0]->headerBar()->caption() ==
          w.document()->instances()[3].paths[0]);
  QVERIFY(w.mainView()->imageViews()[1]->headerBar()->caption() == "Inverted checkerboard");
}

void TestViewMenu::saveScreenshot_cancelSave()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());

  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);

  QAction* saveScreenshotAction = w.findChild<QAction*>("actionSaveScreenshot");
  QVERIFY(saveScreenshotAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       dlg->selectFile(TEST_DATA_DIR "/colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);
  QVERIFY(saveScreenshotAction->isEnabled());

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       dlg->selectFile(tempDir->filePath("screenshot.png"));
                       QTest::keyClick(dlg, Qt::Key_Escape);
                     });
  saveScreenshotAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(!QFileInfo::exists(tempDir->filePath("screenshot.png")));
}

void TestViewMenu::saveScreenshot_okSave()
{
  auto tempDir = std::make_shared<QTemporaryDir>();
  QVERIFY(tempDir->isValid());

  MainWindow w(nullptr /*parent*/, true /*dontUseNativeDialogs*/);

  w.show();
  QVERIFY(QTest::qWaitForWindowActive(&w));

  QAction* openAction = w.findChild<QAction*>("actionOpenAlbum");
  QVERIFY(openAction != nullptr);

  QAction* saveScreenshotAction = w.findChild<QAction*>("actionSaveScreenshot");
  QVERIFY(saveScreenshotAction != nullptr);

  std::shared_ptr<bool> asyncSuccess = std::make_shared<bool>(false);

  QTimer::singleShot(0,
                     [asyncSuccess]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       dlg->selectFile(TEST_DATA_DIR "/colours.cml");
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  openAction->trigger();
  QVERIFY(*asyncSuccess);

  QTimer::singleShot(0,
                     [asyncSuccess, tempDir]
                     {
                       QVERIFY(*asyncSuccess = waitForActiveModalWidgetOfType<QFileDialog>());
                       QFileDialog* dlg = dynamic_cast<QFileDialog*>(qApp->activeModalWidget());
                       dlg->selectFile(tempDir->filePath("screenshot.png"));
                       QTest::keyClick(dlg, Qt::Key_Enter);
                     });
  saveScreenshotAction->trigger();
  QVERIFY(*asyncSuccess);

  QVERIFY(QFileInfo::exists(tempDir->filePath("screenshot.png")));
}
