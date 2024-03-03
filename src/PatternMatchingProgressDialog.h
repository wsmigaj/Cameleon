// This file is part of Caméléon.
//
// Copyright (C) 2023 Wojciech Śmigaj
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

#include <QProgressDialog>

class PatternMatchingProgressDialog : public QProgressDialog
{
public:
  PatternMatchingProgressDialog(QWidget* parent = nullptr);

  QSize sizeHint() const override;

  /// Increment numVisitedFiles_, process events and throw an exception
  /// if cancellation has been requested.
  void incrementProgressAndCheckForCancellation();

private slots:
  void onTimeout();

private:
  int numVisitedFiles_ = 0;
  QString labelTextTemplate_;
};
