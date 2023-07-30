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

Document::Document(const QString& path) : path_(path)
{
}

void Document::setPatterns(std::vector<QString> patterns)
{
  patterns_ = std::move(patterns);
  regenerateInstances();
}

void Document::regenerateInstances()
{
  instances_ = {patterns_};
}

void Document::setLayout(const Layout& layout)
{
  layout_ = layout;
}

void Document::save(const QString& path)
{
  // TODO
  path_ = path;
}

//int Document::numCases() const
//{
//  // Temporary
//  return 1;
//}
//
//int Document::currentCaseIndex() const
//{
//  return currentCaseIndex_;
//}
//
//void Document::setCurrentCaseIndex(int index)
//{
//  // TODO: Validation
//  currentCaseIndex_ = index;
//}
//
//std::vector<QString> Document::currentCase() const
//{
//  // Temporary
//  return patterns_;
//}
