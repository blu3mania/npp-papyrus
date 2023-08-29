/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2021 blu3mania <blu3mania@hotmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "..\Common\PrimitiveTypeValueMonitor.hpp"

#include <string>

#include <windows.h>

namespace papyrus {

  constexpr int KEYWORD_NONE     = 0;
  constexpr int KEYWORD_FUNCTION = 0b1;
  constexpr int KEYWORD_STATE    = 0b10;
  constexpr int KEYWORD_EVENT    = 0b100;
  constexpr int KEYWORD_PROPERTY = 0b1000;
  constexpr int KEYWORD_GROUP    = 0b10000;
  constexpr int KEYWORD_STRUCT   = 0b100000;
  constexpr int KEYWORD_IF       = 0b1000000;
  constexpr int KEYWORD_ELSE     = 0b10000000;
  constexpr int KEYWORD_WHILE    = 0b100000000;
  constexpr int KEYWORD_ALL      = KEYWORD_FUNCTION | KEYWORD_STATE | KEYWORD_EVENT | KEYWORD_PROPERTY | KEYWORD_GROUP | KEYWORD_STRUCT | KEYWORD_IF | KEYWORD_ELSE | KEYWORD_WHILE;

  constexpr int DEFAULT_MATCHER_INDICATOR = 17;

  struct KeywordMatcherSettings {
    utility::PrimitiveTypeValueMonitor<bool>     enableKeywordMatching;
    utility::PrimitiveTypeValueMonitor<int>      enabledKeywords;
    utility::PrimitiveTypeValueMonitor<bool>     autoAllocateIndicatorID;
    utility::PrimitiveTypeValueMonitor<int>      defaultIndicatorID;
    utility::PrimitiveTypeValueMonitor<int>      matchedIndicatorStyle;
    utility::PrimitiveTypeValueMonitor<COLORREF> matchedIndicatorForegroundColor;
    utility::PrimitiveTypeValueMonitor<int>      unmatchedIndicatorStyle;
    utility::PrimitiveTypeValueMonitor<COLORREF> unmatchedIndicatorForegroundColor;
  };

} // namespace
