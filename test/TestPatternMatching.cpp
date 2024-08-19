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

#include "TestPatternMatching.h"
#include "PatternMatching.h"

#include <QString>
#include <fstream>
#include <vector>

QTEST_MAIN(TestPatternMatching)

void TestPatternMatching::emptyPattern()
{
  QString pattern = "";
  std::vector<fs::path> objects{{"ab/cde.png", "cde.png", "ab/xyz.jpg"}};
  PatternMatchingResult expectedResult{0, {}};
  runTest(pattern, objects, expectedResult);
}

void TestPatternMatching::noWildcards()
{
  QString pattern = "ab/cde.png";
  std::vector<fs::path> objects{{"ab/cde.png", "cde.png", "ab/xyz.jpg"}};
  PatternMatchingResult expectedResult{0, {{"ab/cde.png", {}}}};
  runTest(pattern, objects, expectedResult);
}

void TestPatternMatching::twoAsterisks()
{
  QString pattern = "a*b/foo/*.png";
  std::vector<fs::path> objects{{"a1b/foo/c1.png", "a1b/foo/c1.jpg", "a1b/foo/c2.png",
                                 "a1b/foo/c3.png/", // directory -- shouldn't be matched
                                 "ab/foo/d1.png", "ab/foo/d1.jpg", "ab/foo/d2.png",
                                 "a2b/bar/e1.png", "a3b"}};
  PatternMatchingResult expectedResult{2,
                                       {{"a1b/foo/c1.png", {L"1", L"c1"}},
                                        {"a1b/foo/c2.png", {L"1", L"c2"}},
                                        {"ab/foo/d1.png", {L"", L"d1"}},
                                        {"ab/foo/d2.png", {L"", L"d2"}}}};
  runTest(pattern, objects, expectedResult);
}

void TestPatternMatching::threeQuestionMarks()
{
  QString pattern = "a??b/foo/?.png";
  std::vector<fs::path> objects{{
    "a1b/foo/c.png", "a23b/foo/d.jpg", "a23b/foo/ef.png", "a23b/foo/g.png", "a23b/foo/.png",
    "a23b/foo/g.jpg/" // directory -- shouldn't be matched
  }};
  PatternMatchingResult expectedResult{3, {{"a23b/foo/g.png", {L"2", L"3", L"g"}}}};
  runTest(pattern, objects, expectedResult);
}

void TestPatternMatching::noMatches()
{
  QString pattern = "a*b/*.png";
  std::vector<fs::path> objects{{"a1b/a.jpg"}};
  PatternMatchingResult expectedResult{2, {}};
  runTest(pattern, objects, expectedResult);
}

void TestPatternMatching::runTest(QString pattern, const std::vector<fs::path>& objects,
                                  PatternMatchingResult expectedResult)
{
  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());
  const fs::path tempDirPath = tempDir.path().toStdWString();

  pattern = tempDir.path() + "/" + pattern;

  for (fs::path path : objects)
  {
    path = tempDirPath / path;
    if (path.has_filename())
    {
      fs::create_directories(path.parent_path());
      std::ofstream fileStream(path);
    }
    else
    {
      fs::create_directories(path);
    }
  }

  PatternMatchingResult result = matchPattern(pattern);
  // Matches are returned in an unspecified order; sort them before comparing.
  std::sort(result.patternMatches.begin(), result.patternMatches.end(),
            [](const PatternMatch& a, const PatternMatch& b) { return a.path < b.path; });
  std::sort(expectedResult.patternMatches.begin(), expectedResult.patternMatches.end(),
            [](const PatternMatch& a, const PatternMatch& b) { return a.path < b.path; });

  {
    const fs::path tempDirPath = tempDir.path().toStdWString();
    for (PatternMatch& match : expectedResult.patternMatches)
    {
      match.path = tempDirPath / match.path;
    }
  }

  QCOMPARE(result, expectedResult);
}
