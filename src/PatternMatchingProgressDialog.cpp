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

#include "PatternMatchingProgressDialog.h"
#include "CancellationException.h"

PatternMatchingProgressDialog::PatternMatchingProgressDialog(QWidget* parent)
  : QProgressDialog(parent), labelTextTemplate_("Number of files visited so far: %1")
{
  setWindowTitle("Searching for matching files...");
  setWindowModality(Qt::WindowModal);
  setRange(0, 0);
  setLabelText(labelTextTemplate_.arg(0));

  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &PatternMatchingProgressDialog::onTimeout);
  timer->setInterval(1000);
  timer->start();
}

QSize PatternMatchingProgressDialog::sizeHint() const
{
  // Widen the dialog to accommodate the whole window title.
  QSize size = QProgressDialog::sizeHint();
  size.setWidth(3 * size.width() / 2);
  return size;
}

void PatternMatchingProgressDialog::incrementProgressAndCheckForCancellation()
{
  ++numVisitedFiles_;
  QCoreApplication::processEvents();
  if (wasCanceled())
    throw CancellationException();
}

void PatternMatchingProgressDialog::onTimeout()
{
  setLabelText(labelTextTemplate_.arg(numVisitedFiles_));
}