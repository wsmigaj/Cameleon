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

#include "stdafx.h"
#include "ComparisonDialog.h"

#include <array>

ComparisonDialog::ComparisonDialog(QWidget* parent) : QDialog(parent)
{
  ui.setupUi(this);
}

std::vector<QString> ComparisonDialog::patterns() const
{
  std::vector<QString> result;
  for (QLineEdit* edit : {ui.patternAEdit, ui.patternBEdit, ui.patternCEdit, ui.patternDEdit})
  {
    if (QString pattern = edit->text(); !pattern.isEmpty())
    {
      result.push_back(pattern);
    }
  }
  return result;
}

void ComparisonDialog::setPatterns(const std::vector<QString>& patterns)
{
  std::array<QLineEdit*, 4> edits = {ui.patternAEdit, ui.patternBEdit, ui.patternCEdit,
                                     ui.patternDEdit};

  for (size_t i = 0; i < edits.size(); ++i)
  {
    if (i < patterns.size())
      edits[i]->setText(patterns[i]);
    else
      edits[i]->setText(QString());
  }
}
