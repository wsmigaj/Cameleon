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

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class QString;

struct PatternMatch
{
  std::filesystem::path path;
  std::vector<std::wstring> magicExpressionMatches;
};

struct PatternMatchingResult
{
  size_t numMagicExpressions = 0;
  std::vector<PatternMatch> patternMatches;
};

void checkAllPatternsContainSameNumberOfMagicExpressionsOrNone(
  const std::vector<QString>& patterns);

std::vector<std::shared_ptr<PatternMatchingResult>> matchPatterns(
  const std::vector<QString>& patterns,
  const std::function<void()>& onFilesystemTraversalProgress = []() {});

std::vector<std::shared_ptr<PatternMatchingResult>> matchPatternsReusingPreviousResults(
  const std::vector<QString>& patterns, const std::vector<QString>& previousPatterns,
  const std::vector<std::shared_ptr<PatternMatchingResult>>& previousResults,
  const std::function<void()>& onFilesystemTraversalProgress = []() {});
