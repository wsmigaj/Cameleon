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
  updateInfoLabelsVisibility();

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
      if (normalisePathSeparators_)
      {
        value.replace('/', QDir::separator());
        value.replace('\\', QDir::separator());
      }
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

void ComparisonDialog::setPrompt(const QString& prompt)
{
  ui_.promptLabel->setText(prompt);
}

void ComparisonDialog::setNumberOfRows(size_t n)
{
  if (n != numRows_)
  {
    numRows_ = n;
    updateRowLabelsVisibility();
    updateComboBoxesVisibility();
    updateFileDialogButtonsVisibility();
    updateSwapValuesButtonsVisibility();
    adjustSize();
  }
}

void ComparisonDialog::setComboBoxPrompts(const std::vector<QString>& prompts)
{
  const std::vector<QComboBox*> comboBoxes = valueComboBoxes();
  for (size_t i = 0; i < prompts.size() && i < numRows_; ++i)
    comboBoxes[i]->lineEdit()->setPlaceholderText(prompts[i]);
  for (size_t i = prompts.size(); i < numRows_; ++i)
    comboBoxes[i]->lineEdit()->setPlaceholderText(QString());
}

void ComparisonDialog::setInfoLabelsVisibility(bool visible)
{
  if (visible != infoLabelsVisible_)
  {
    infoLabelsVisible_ = visible;
    updateInfoLabelsVisibility();
    adjustSize();
  }
}

void ComparisonDialog::setInfoLabels(const std::vector<QString>& labels)
{
  const std::vector<QLabel*> widgets = infoLabels();

  for (size_t i = 0; i < widgets.size(); ++i)
  {
    if (i < labels.size())
    {
      widgets[i]->setToolTip(labels[i]);
    }
    else
    {
      widgets[i]->setToolTip(QString());
    }
  }
}

void ComparisonDialog::setFileDialogButtonsVisibility(bool visible)
{
  if (visible != fileDialogButtonsVisible_)
  {
    fileDialogButtonsVisible_ = visible;
    updateFileDialogButtonsVisibility();
    adjustSize();
  }
}

void ComparisonDialog::setSwapValuesButtonsVisibility(bool visible)
{
  if (visible != swapValuesButtonsVisible_)
  {
    swapValuesButtonsVisible_ = visible;
    updateSwapValuesButtonsVisibility();
    adjustSize();
  }
}

void ComparisonDialog::setValidator(std::function<bool(ComparisonDialog&)> validator)
{
  validator_ = std::move(validator);
}

void ComparisonDialog::normalisePathSeparators(bool normalise)
{
  normalisePathSeparators_ = normalise;
}

bool ComparisonDialog::defaultValidator(ComparisonDialog&)
{
  return true;
}

void ComparisonDialog::updateRowLabelsVisibility()
{
  const std::vector<QLabel*> v = rowLabels();
  for (size_t i = 0; i < numRows_; ++i)
    v[i]->setVisible(true);
  for (size_t i = numRows_; i < MAX_NUM_PATTERNS; ++i)
    v[i]->setVisible(false);
}

void ComparisonDialog::updateComboBoxesVisibility()
{
  const std::vector<QComboBox*> v = valueComboBoxes();
  for (size_t i = 0; i < numRows_; ++i)
    v[i]->setVisible(true);
  for (size_t i = numRows_; i < MAX_NUM_PATTERNS; ++i)
    v[i]->setVisible(false);
}

void ComparisonDialog::updateFileDialogButtonsVisibility()
{
  const std::vector<QToolButton*> v = fileDialogButtons();
  for (size_t i = 0; i < numRows_; ++i)
    v[i]->setVisible(fileDialogButtonsVisible_);
  for (size_t i = numRows_; i < MAX_NUM_PATTERNS; ++i)
    v[i]->setVisible(false);
}

void ComparisonDialog::updateInfoLabelsVisibility()
{
  const std::vector<QLabel*> v = infoLabels();
  for (size_t i = 0; i < numRows_; ++i)
    v[i]->setVisible(infoLabelsVisible_);
  for (size_t i = numRows_; i < MAX_NUM_PATTERNS; ++i)
    v[i]->setVisible(false);
}

void ComparisonDialog::updateSwapValuesButtonsVisibility()
{
  const std::vector<QToolButton*> v = swapValuesButtons();
  for (size_t i = 0; i + 1 < numRows_; ++i)
    v[i]->setVisible(swapValuesButtonsVisible_);
  for (size_t i = (numRows_ == 0 ? 0 : numRows_ - 1); i + 1 < MAX_NUM_PATTERNS; ++i)
    v[i]->setVisible(false);
}

void ComparisonDialog::done(int r)
{
  if (r == QDialog::Accepted)
    saveRecentValues();

  QDialog::done(r);
}

void ComparisonDialog::onOk()
{
  if (validator_(*this))
    accept();
}

std::vector<QLabel*> ComparisonDialog::rowLabels() const
{
  return {ui_.labelA, ui_.labelB, ui_.labelC, ui_.labelD,
          ui_.labelE, ui_.labelF, ui_.labelG, ui_.labelH};
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

std::vector<QLabel*> ComparisonDialog::infoLabels() const
{
  return {ui_.infoLabelA, ui_.infoLabelB, ui_.infoLabelC, ui_.infoLabelD,
          ui_.infoLabelE, ui_.infoLabelF, ui_.infoLabelG, ui_.infoLabelH};
}

std::vector<QToolButton*> ComparisonDialog::swapValuesButtons() const
{
  return {ui_.swapABButton, ui_.swapBCButton, ui_.swapCDButton, ui_.swapDEButton,
          ui_.swapEFButton, ui_.swapFGButton, ui_.swapGHButton};
}

void ComparisonDialog::connectSignals()
{
  connect(ui_.okButton, &QPushButton::clicked, this, &ComparisonDialog::onOk);
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

  {
    QComboBox* firstComboBox = valueComboBoxes()[index];
    QComboBox* secondComboBox = valueComboBoxes()[index + 1];
    const QString firstValue = firstComboBox->currentText();
    const QString secondValue = secondComboBox->currentText();

    firstComboBox->setCurrentText(secondValue);
    secondComboBox->setCurrentText(firstValue);
  }

  {
    QLabel* firstInfoLabel = infoLabels()[index];
    QLabel* secondInfoLabel = infoLabels()[index + 1];
    const QString firstToolTip = firstInfoLabel->toolTip();
    const QString secondToolTip = secondInfoLabel->toolTip();

    firstInfoLabel->setToolTip(secondToolTip);
    secondInfoLabel->setToolTip(firstToolTip);
  }
}

void setComboBoxPromptsToPatternExamples(ComparisonDialog& dialog)
{
  dialog.setComboBoxPrompts({"Example: C:\\TestSet\\input*.png",
                             "Example: C:\\TestSet\\ground-truth*.png",
                             "Example: C:\\TestSet\\prediction*.png"});
}
