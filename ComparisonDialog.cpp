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
#include "Constants.h"

namespace
{
QString getDirPrefix(const QString& pattern)
{
  QFileInfo fileInfo(pattern);
  while (!fileInfo.filePath().isEmpty() && !fileInfo.isDir())
  {
    fileInfo.setFile(fileInfo.dir().path());
  }
  return fileInfo.filePath();
}
} // namespace

ComparisonDialog::ComparisonDialog(QWidget* parent) : QDialog(parent)
{
  ui_.setupUi(this);

  ui_.fileDialogButtonA->adjustSize();
  ui_.swapABButton->adjustSize();
  const int maxToolButtonWidth =
    std::max(ui_.fileDialogButtonA->width(), ui_.swapABButton->width());
  for (QToolButton* button : fileDialogButtons())
    button->setMinimumWidth(maxToolButtonWidth);
  for (QToolButton* button : swapPatternsButtons())
    button->setMinimumWidth(maxToolButtonWidth);

  connectSignals();
  loadRecentPatterns();
}

std::vector<QString> ComparisonDialog::patterns() const
{
  std::vector<QString> result;
  for (QComboBox* comboBox : patternComboBoxes())
  {
    if (QString pattern = comboBox->currentText(); !pattern.isEmpty())
    {
      pattern.replace('/', QDir::separator());
      pattern.replace('\\', QDir::separator());
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
  return {ui_.patternAComboBox, ui_.patternBComboBox, ui_.patternCComboBox, ui_.patternDComboBox,
          ui_.patternEComboBox, ui_.patternFComboBox, ui_.patternGComboBox, ui_.patternHComboBox};
}

std::vector<QToolButton*> ComparisonDialog::fileDialogButtons() const
{
  return {ui_.fileDialogButtonA, ui_.fileDialogButtonB, ui_.fileDialogButtonC,
          ui_.fileDialogButtonD, ui_.fileDialogButtonE, ui_.fileDialogButtonF,
          ui_.fileDialogButtonG, ui_.fileDialogButtonH};
}

std::vector<QToolButton*> ComparisonDialog::swapPatternsButtons() const
{
  return {ui_.swapABButton, ui_.swapBCButton, ui_.swapCDButton, ui_.swapDEButton,
          ui_.swapEFButton, ui_.swapFGButton, ui_.swapGHButton};
}

void ComparisonDialog::connectSignals()
{
  for (QToolButton* button : fileDialogButtons())
    connect(button, &QToolButton::clicked, this, &ComparisonDialog::onFileDialogButtonClicked);
  for (QToolButton* button : swapPatternsButtons())
    connect(button, &QToolButton::clicked, this, &ComparisonDialog::onSwapPatternsButtonClicked);
}

void ComparisonDialog::onFileDialogButtonClicked()
{
  const std::vector<QToolButton*> buttons = fileDialogButtons();
  const int index = std::find(buttons.begin(), buttons.end(), sender()) - buttons.begin();
  QComboBox* comboBox = patternComboBoxes()[index];

  QString pattern = comboBox->currentText();
  if (pattern.isEmpty() && comboBox->count() > 0)
    pattern = comboBox->itemText(0);
  QString dir = getDirPrefix(pattern);
  QString file = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, "caption", dir));
  if (!file.isEmpty())
    comboBox->setCurrentText(file);
}

void ComparisonDialog::onSwapPatternsButtonClicked()
{
  const std::vector<QToolButton*> buttons = swapPatternsButtons();
  const int index = std::find(buttons.begin(), buttons.end(), sender()) - buttons.begin();

  QComboBox* firstComboBox = patternComboBoxes()[index];
  QComboBox* secondComboBox = patternComboBoxes()[index + 1];
  const QString firstPattern = firstComboBox->currentText();
  const QString secondPattern = secondComboBox->currentText();

  firstComboBox->setCurrentText(secondPattern);
  secondComboBox->setCurrentText(firstPattern);
}