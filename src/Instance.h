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

#include <memory>
#include <vector>

class QString;

struct PatternMatchingResult;

struct Instance
{
  std::vector<QString> paths;
  std::vector<QString> magicExpressionMatches;
};

bool operator==(const Instance& a, const Instance& b);
bool operator!=(const Instance& a, const Instance& b);

std::vector<Instance>
findInstances(const std::vector<std::shared_ptr<PatternMatchingResult>>& patternMatchingResults);

void sortInstances(std::vector<Instance>& instances);
