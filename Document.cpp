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
#include "Document.h"

Document::Document()
{
}

void Document::load(const QString& path)
{
}

void Document::save(const QString& path)
{
}

const std::vector<QString>& Document::patterns() const
{
  return patterns_;
}

void Document::setPatterns(std::vector<QString> patterns, int numRows, int numColumns)
{
  if (numRows * numColumns < patterns.size())
  {
    throw std::invalid_argument("Layout cannot accomodate all images");
  }
  patterns_ = std::move(patterns);
  numRows_ = numRows;
  numColumns_ = numColumns;
}

int Document::numRows() const
{
  return numRows_;
}

int Document::numColumns() const
{
  return numColumns_;
}

void Document::setLayout(int numRows, int numColumns)
{
  if (numRows * numColumns < patterns_.size())
  {
    throw std::invalid_argument("Layout cannot accomodate all images");
  }
  numRows_ = numRows;
  numColumns_ = numColumns;
}

int Document::numCases() const
{
  // Temporary
  return 1;
}

int Document::currentCaseIndex() const
{
  return currentCaseIndex_;
}

void Document::setCurrentCaseIndex(int index)
{
  // TODO: Validation
  currentCaseIndex_ = index;
}

std::vector<QString> Document::currentCase() const
{
  // Temporary
  return patterns_;
}
