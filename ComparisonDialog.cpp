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

ComparisonDialog::ComparisonDialog(QWidget* parent, const QString& recentValuesSettingsKey)
  : QDialog(parent), recentValuesSettingsKey_(recentValuesSettingsKey)
{
  ui_.setupUi(this);

  ui_.fileDialogButtonA->adjustSize();
  ui_.swapABButton->adjustSize();
  const int maxToolButtonWidth =
    std::max(ui_.fileDialogButtonA->width(), ui_.swapABButton->width());
  for (QToolButton* button : fileDialogButtons())
    button->setMinimumWidth(maxToolButtonWidth);
  for (QToolButton* button : swapValuesButtons())
    button->setMinimumWidth(maxToolButtonWidth);

  connectSignals();
  loadRecentValues();
}

std::vector<QString> ComparisonDialog::values() const
{
  std::vector<QString> result;
  for (QComboBox* comboBox : valueComboBoxes())
  {
    if (QString value = comboBox->currentText(); !value.isEmpty())
    {
      value.replace('/', QDir::separator());
      value.replace('\\', QDir::separator());
      result.push_back(value);
    }
  }
  return result;
}

void ComparisonDialog::setValues(const std::vector<QString>& values)
{
  const std::vector<QComboBox*> comboBoxes = valueComboBoxes();

  for (size_t i = 0; i < comboBoxes.size(); ++i)
  {
    if (i < values.size())
    {
      if (int index = comboBoxes[i]->findText(values[i]); index >= 0)
        comboBoxes[i]->removeItem(index);
      comboBoxes[i]->insertItem(0, values[i]);
      comboBoxes[i]->setCurrentIndex(0);
    }
    else
    {
      comboBoxes[i]->setCurrentText(QString());
    }
  }
}

void ComparisonDialog::loadRecentValues()
{
  QSettings settings;
  const std::vector<QComboBox*> comboBoxes = valueComboBoxes();

  for (size_t i = 0; i < MAX_NUM_PATTERNS; ++i)
  {
    QComboBox* comboBox = comboBoxes[i];
    const QString key = QString("%1/%2").arg(recentValuesSettingsKey_).arg(i);
    QStringList values = settings.value(key, QStringList()).toStringList();
    values.resize(std::min<qsizetype>(values.size(), MAX_NUM_RECENT_VALUES));
    for (QString value : values)
      comboBox->addItem(value);
  }
}

void ComparisonDialog::saveRecentValues()
{
  QSettings settings;
  const std::vector<QComboBox*> comboBoxes = valueComboBoxes();

  for (size_t i = 0; i < comboBoxes.size(); ++i)
  {
    QComboBox* comboBox = comboBoxes[i];
    if (QString currentValue = comboBox->currentText(); !currentValue.isEmpty())
    {
      const QString key = QString("%1/%2").arg(recentValuesSettingsKey_).arg(i);
      QStringList values = settings.value(key, QStringList()).toStringList();
      values.resize(std::min<qsizetype>(values.size(), MAX_NUM_RECENT_VALUES));
      values.removeAll(currentValue);
      values.prepend(currentValue);
      settings.setValue(key, values);
    }
  }
}

void ComparisonDialog::done(int r)
{
  if (r == QDialog::Accepted)
    saveRecentValues();

  QDialog::done(r);
}

std::vector<QComboBox*> ComparisonDialog::valueComboBoxes() const
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

std::vector<QToolButton*> ComparisonDialog::swapValuesButtons() const
{
  return {ui_.swapABButton, ui_.swapBCButton, ui_.swapCDButton, ui_.swapDEButton,
          ui_.swapEFButton, ui_.swapFGButton, ui_.swapGHButton};
}

void ComparisonDialog::connectSignals()
{
  for (QToolButton* button : fileDialogButtons())
    connect(button, &QToolButton::clicked, this, &ComparisonDialog::onFileDialogButtonClicked);
  for (QToolButton* button : swapValuesButtons())
    connect(button, &QToolButton::clicked, this, &ComparisonDialog::onSwapValuesButtonClicked);
}

void ComparisonDialog::onFileDialogButtonClicked()
{
  const std::vector<QToolButton*> buttons = fileDialogButtons();
  const int index = std::find(buttons.begin(), buttons.end(), sender()) - buttons.begin();
  QComboBox* comboBox = valueComboBoxes()[index];

  QString value = comboBox->currentText();
  if (value.isEmpty() && comboBox->count() > 0)
    value = comboBox->itemText(0);
  QString dir = getDirPrefix(value);
  QString file = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, "caption", dir));
  if (!file.isEmpty())
    comboBox->setCurrentText(file);
}

void ComparisonDialog::onSwapValuesButtonClicked()
{
  const std::vector<QToolButton*> buttons = swapValuesButtons();
  const int index = std::find(buttons.begin(), buttons.end(), sender()) - buttons.begin();

  QComboBox* firstComboBox = valueComboBoxes()[index];
  QComboBox* secondComboBox = valueComboBoxes()[index + 1];
  const QString firstValue = firstComboBox->currentText();
  const QString secondValue = secondComboBox->currentText();

  firstComboBox->setCurrentText(secondValue);
  secondComboBox->setCurrentText(firstValue);
}