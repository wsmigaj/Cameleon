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

#include "TestFindInstances.h"
#include "Instance.h"
#include "PatternMatching.h"
#include "RuntimeError.h"

#include <QString>
#include <QTest>
#include <fstream>
#include <vector>

QTEST_MAIN(TestFindInstances)

// TODO: Move to a standalone file
void createFilesystemObjects(const std::filesystem::path& topPath,
                             const std::vector<std::filesystem::path>& relativePaths)
{
  for (const std::filesystem::path& relativePath : relativePaths)
  {
    std::filesystem::path path = topPath / relativePath;
    if (path.has_filename())
    {
      std::filesystem::create_directories(path.parent_path());
      std::ofstream fileStream(path);
    }
    else
    {
      std::filesystem::create_directories(path);
    }
  }
}

void TestFindInstances::noPatterns()
{
  std::vector<QString> patterns{};
  std::vector<std::filesystem::path> objects{};
  std::vector<Instance> expectedInstances{};
  runTest(patterns, objects, expectedInstances);
}

void TestFindInstances::onePatternWithTwoWildcards()
{
  std::vector<QString> patterns{{"p1/ab*/foo/*.png"}};
  std::vector<std::filesystem::path> objects{{
    "p1/ab1/foo/c1.png", "p1/ab1/foo/c2.png",
    "p1/ab1/foo/c3.jpg", // no match
    "p1/ab23/foo/c1.png", "p1/ab23/foo/c2.png",
    "p1/ab23/bar/c1.png", // no match
  }};
  std::vector<Instance> expectedInstances{
    {{"p1/ab1/foo/c1.png"}, {"1", "c1"}},
    {{"p1/ab1/foo/c2.png"}, {"1", "c2"}},
    {{"p1/ab23/foo/c1.png"}, {"23", "c1"}},
    {{"p1/ab23/foo/c2.png"}, {"23", "c2"}},
  };
  for (Instance& instance : expectedInstances)
    for (QString& path : instance.paths)
      path = QDir::toNativeSeparators(path);
  runTest(patterns, objects, expectedInstances);
}

void TestFindInstances::twoPatternsEachWithTwoWildcards()
{
  std::vector<QString> patterns{{"p1/ab*/foo/*.png", "p2/*ab/foo/*.png"}};
  std::vector<std::filesystem::path> objects{{
    "p1/ab1/foo/c1.png",
    "p1/ab1/foo/c2.png",
    "p1/ab1/foo/c3.jpg", // no match
    "p1/ab23/foo/c1.png",
    "p1/ab23/foo/c2.png",
    "p1/ab23/bar/c1.png", // no match
    "p2/1ab/foo/c1.png",
    "p2/1ab/foo/c2.png",
    "p2/1ab/foo/c3.png",
    "p2/456ab/foo/c1.png",
  }};
  std::vector<Instance> expectedInstances{
    {{"p1/ab1/foo/c1.png", "p2/1ab/foo/c1.png"}, {"1", "c1"}},
    {{"p1/ab1/foo/c2.png", "p2/1ab/foo/c2.png"}, {"1", "c2"}},
    {{"", "p2/1ab/foo/c3.png"}, {"1", "c3"}},
    {{"p1/ab23/foo/c1.png", ""}, {"23", "c1"}},
    {{"p1/ab23/foo/c2.png", ""}, {"23", "c2"}},
    {{"", "p2/456ab/foo/c1.png"}, {"456", "c1"}},
  };
  for (Instance& instance : expectedInstances)
    for (QString& path : instance.paths)
      path = QDir::toNativeSeparators(path);
  runTest(patterns, objects, expectedInstances);
}

void TestFindInstances::twoPatternsOneWithZeroWildcardsAnotherWithTwo()
{
  std::vector<QString> patterns{{"p1/ab*/foo/*.png", "p2/456ab/foo/c1.png"}};
  std::vector<std::filesystem::path> objects{{
    "p1/ab1/foo/c1.png",
    "p1/ab1/foo/c2.png",
    "p1/ab1/foo/c3.jpg", // no match
    "p1/ab23/foo/c1.png",
    "p1/ab23/foo/c2.png",
    "p1/ab23/bar/c1.png", // no match
    "p2/1ab/foo/c1.png",
    "p2/1ab/foo/c2.png",
    "p2/1ab/foo/c3.png",
    "p2/456ab/foo/c1.png",
  }};
  std::vector<Instance> expectedInstances{
    {{"p1/ab1/foo/c1.png", "p2/456ab/foo/c1.png"}, {"1", "c1"}},
    {{"p1/ab1/foo/c2.png", "p2/456ab/foo/c1.png"}, {"1", "c2"}},
    {{"p1/ab23/foo/c1.png", "p2/456ab/foo/c1.png"}, {"23", "c1"}},
    {{"p1/ab23/foo/c2.png", "p2/456ab/foo/c1.png"}, {"23", "c2"}},
  };
  for (Instance& instance : expectedInstances)
    for (QString& path : instance.paths)
      path = QDir::toNativeSeparators(path);
  runTest(patterns, objects, expectedInstances);
}

void TestFindInstances::twoPatternsOneWithOneWildcardAnotherWithTwo()
{
  std::vector<QString> patterns{{"p1/ab*/foo/*.png", "p2/*ab/foo/c1.png"}};
  std::vector<std::filesystem::path> objects{};
  runTest(
    patterns, objects,
    std::nullopt // expect an exception to be thrown due to the mismatch in the number of wildcards
  );
}

void TestFindInstances::threePatternsEachWithTwoWildcards()
{
  std::vector<QString> patterns{{"p1/ab*/foo/*.png", "p2/*ab/foo/*.png", "p3/a*b/foo/*.png"}};
  std::vector<std::filesystem::path> objects{
    {"p1/ab1/foo/c1.png", "p1/ab1/foo/c2.png",
     "p1/ab1/foo/c3.jpg", // no match
     "p1/ab23/foo/c1.png", "p1/ab23/foo/c2.png",
     "p1/ab23/bar/c1.png", // no match
     "p2/1ab/foo/c1.png", "p2/1ab/foo/c2.png", "p2/1ab/foo/c3.png", "p2/456ab/foo/c1.png",
     "p3/a1b/foo/c1.png", "p3/a23b/foo/c2.png", "p3/a456b/foo/c1.png"}};
  std::vector<Instance> expectedInstances{
    {{"p1/ab1/foo/c1.png", "p2/1ab/foo/c1.png", "p3/a1b/foo/c1.png"}, {"1", "c1"}},
    {{"p1/ab1/foo/c2.png", "p2/1ab/foo/c2.png", ""}, {"1", "c2"}},
    {{"", "p2/1ab/foo/c3.png", ""}, {"1", "c3"}},
    {{"p1/ab23/foo/c1.png", "", ""}, {"23", "c1"}},
    {{"p1/ab23/foo/c2.png", "", "p3/a23b/foo/c2.png"}, {"23", "c2"}},
    {{"", "p2/456ab/foo/c1.png", "p3/a456b/foo/c1.png"}, {"456", "c1"}},
  };
  for (Instance& instance : expectedInstances)
    for (QString& path : instance.paths)
      path = QDir::toNativeSeparators(path);
  runTest(patterns, objects, expectedInstances);
}

void TestFindInstances::runTest(std::vector<QString> patterns,
                                const std::vector<std::filesystem::path>& objects,
                                std::optional<std::vector<Instance>> expectedInstances)
{
  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());
  const std::filesystem::path tempDirPath = tempDir.path().toStdWString();

  for (QString& pattern : patterns)
    pattern = tempDir.path() + "/" + pattern;

  createFilesystemObjects(tempDirPath, objects);
  std::vector<std::shared_ptr<PatternMatchingResult>> patternMatchingResults =
    matchPatterns(patterns);

  if (expectedInstances)
  {
    std::vector<Instance> instances = ::findInstances(patternMatchingResults);
    ::sortInstances(*expectedInstances);

    {
      for (Instance& instance : *expectedInstances)
      {
        for (QString& path : instance.paths)
          if (!path.isEmpty())
            path = QDir::toNativeSeparators(tempDir.path() + "/") + path;
      }
    }

    QCOMPARE(instances, *expectedInstances);
  }
  else
  {
    QVERIFY_EXCEPTION_THROWN(::findInstances(patternMatchingResults), RuntimeError);
  }
}
