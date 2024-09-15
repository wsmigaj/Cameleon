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

#pragma once

#include "Constants.h"

#include <QDialog>
#include <functional>
#include <memory>
#include <vector>

class QComboBox;
class QLabel;
class QToolButton;

namespace Ui
{
class AlbumEditorDialogClass;
}

class AlbumEditorDialog : public QDialog
{
  Q_OBJECT

public:
  AlbumEditorDialog(QWidget* parent, const QString& recentValuesSettingsKey);
  ~AlbumEditorDialog();

  std::vector<QString> values() const;
  void setValues(const std::vector<QString>& patterns);

  void loadRecentValues();
  void saveRecentValues();

  void setPrompt(const QString& prompt);
  void setNumberOfRows(size_t n);
  void setComboBoxPrompts(const std::vector<QString>& prompts);
  void setFileDialogButtonsVisibility(bool visible);
  void setInfoLabelsVisibility(bool visible);
  void setInfoLabels(const std::vector<QString>& labels);
  void setSwapValuesButtonsVisibility(bool visible);
  void setValidator(std::function<bool(AlbumEditorDialog&)> validator);
  void normalisePathSeparators(bool normalise);

  static bool defaultValidator(AlbumEditorDialog&);

public slots:
  void done(int r) override;
  void onFileDialogButtonClicked();
  void onSwapValuesButtonClicked();

private slots:
  void onOk();
  void onPromptLabelLinkHovered(const QString& link);

private:
  std::vector<QLabel*> rowLabels() const;
  std::vector<QComboBox*> valueComboBoxes() const;
  std::vector<QToolButton*> fileDialogButtons() const;
  std::vector<QLabel*> infoLabels() const;
  std::vector<QToolButton*> swapValuesButtons() const;

  void connectSignals();

  void updateRowLabelsVisibility();
  void updateComboBoxesVisibility();
  void updateFileDialogButtonsVisibility();
  void updateInfoLabelsVisibility();
  void updateSwapValuesButtonsVisibility();

private:
  static const size_t MAX_NUM_RECENT_VALUES = 20;

private:
  std::unique_ptr<Ui::AlbumEditorDialogClass> ui_;
  QString recentValuesSettingsKey_;
  size_t numRows_ = MAX_NUM_PATTERNS;
  bool fileDialogButtonsVisible_ = true;
  bool infoLabelsVisible_ = false;
  bool swapValuesButtonsVisible_ = true;
  bool normalisePathSeparators_ = false;
  std::function<bool(AlbumEditorDialog&)> validator_ = defaultValidator;
};

void setComboBoxPromptsToPatternExamples(AlbumEditorDialog& dialog);
