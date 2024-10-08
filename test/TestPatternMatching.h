// This file is part of Caméléon.
//
// Copyright (C) 2024 Wojciech Śmigaj
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

#include "ghc/fs_std_fwd.hpp"

#include <QTest>

struct PatternMatchingResult;

class TestPatternMatching : public QObject
{
  Q_OBJECT
private slots:
  void emptyPattern();
  void noWildcards();
  void twoAsterisks();
  void threeQuestionMarks();
  void noMatches();

private:
  void runTest(QString pattern, const std::vector<fs::path>& objects,
               PatternMatchingResult expectedResult);
};
