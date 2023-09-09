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
#include "Layout.h"

Layout defaultLayout(size_t numImages)
{
  const size_t numRows = std::max<size_t>(0, static_cast<size_t>(std::ceil(numImages / 3.0)));
  const size_t numColumns =
    std::max<size_t>(0, static_cast<size_t>(std::ceil(numImages / double(numRows))));
  return Layout{numRows, numColumns};
}