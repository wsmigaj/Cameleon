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

ComparisonDialog::ComparisonDialog(QWidget* parent) : QDialog(parent)
{
  ui_.setupUi(this);
  loadRecentPatterns();
}

std::vector<QString> ComparisonDialog::patterns() const
{
  std::vector<QString> result;
  for (QComboBox* comboBox : patternComboBoxes())
  {
    if (QString pattern = comboBox->currentText(); !pattern.isEmpty())
    {
      result.push_back(pattern);
    }
  }
  return result;
}

void ComparisonDialog::setPatterns(const std::vector<QString>& patterns)
{
  const std::vector<QComboBox*> comboBoxes = patternComboBoxes();

  for (size_t i = 0; i < comboBoxes.size(); ++i)
  {
    if (i < patterns.size())
    {
      if (int index = comboBoxes[i]->findText(patterns[i]); index >= 0)
        comboBoxes[i]->removeItem(index);
      comboBoxes[i]->insertItem(0, patterns[i]);
      comboBoxes[i]->setCurrentIndex(0);
    }
    else
    {
      comboBoxes[i]->setCurrentText(QString());
    }
  }
}

void ComparisonDialog::loadRecentPatterns()
{
  QSettings settings;
  const std::vector<QComboBox*> comboBoxes = patternComboBoxes();

  for (size_t i = 0; i < MAX_NUM_PATTERNS; ++i)
  {
    QComboBox* comboBox = comboBoxes[i];
    const QString key = QString("recentPatterns/%1").arg(i);
    QStringList patterns = settings.value(key, QStringList()).toStringList();
    patterns.resize(std::min<qsizetype>(patterns.size(), MAX_NUM_RECENT_PATTERNS));
    for (QString pattern : patterns)
      comboBox->addItem(pattern);
  }
}

void ComparisonDialog::saveRecentPatterns()
{
  QSettings settings;
  const std::vector<QComboBox*> comboBoxes = patternComboBoxes();

  for (size_t i = 0; i < comboBoxes.size(); ++i)
  {
    QComboBox* comboBox = comboBoxes[i];
    if (QString currentPattern = comboBox->currentText(); !currentPattern.isEmpty())
    {
      const QString key = QString("recentPatterns/%1").arg(i);
      QStringList patterns = settings.value(key, QStringList()).toStringList();
      patterns.resize(std::min<qsizetype>(patterns.size(), MAX_NUM_RECENT_PATTERNS));
      patterns.removeAll(currentPattern);
      patterns.prepend(currentPattern);
      settings.setValue(key, patterns);
    }
  }
}

void ComparisonDialog::done(int r)
{
  if (r == QDialog::Accepted)
    saveRecentPatterns();

  QDialog::done(r);
}

std::vector<QComboBox*> ComparisonDialog::patternComboBoxes() const
{
  return {ui_.patternAComboBox, ui_.patternBComboBox, ui_.patternCComboBox, ui_.patternDComboBox};
}