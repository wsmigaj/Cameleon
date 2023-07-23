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

#include <QString>

#include <vector>

class Document
{
public:
  Document();

  void load(const QString& path);
  void save(const QString& path);

  const std::vector<QString>& patterns() const;
  void setPatterns(std::vector<QString> patterns, int numRows, int numColumns);

  int numRows() const;
  int numColumns() const;
  void setLayout(int numRows, int numColumns);

  int numCases() const;
  int currentCaseIndex() const;
  void setCurrentCaseIndex(int index);
  std::vector<QString> currentCase() const;

private:
  std::vector<QString> patterns_;
  int numRows_ = 1;
  int numColumns_ = 1;
  int currentCaseIndex_ = -1;
};
