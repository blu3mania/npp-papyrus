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

#define KEYWORD_NONE        0
#define KEYWORD_FUNCTION    1
#define KEYWORD_STATE       2
#define KEYWORD_EVENT       4
#define KEYWORD_PROPERTY    8
#define KEYWORD_GROUP       16
#define KEYWORD_STRUCT      32
#define KEYWORD_IF          64
#define KEYWORD_ELSE        128
#define KEYWORD_WHILE       256
#define KEYWORD_ALL         KEYWORD_FUNCTION | KEYWORD_STATE | KEYWORD_EVENT | KEYWORD_PROPERTY | KEYWORD_GROUP | KEYWORD_STRUCT | KEYWORD_IF | KEYWORD_ELSE | KEYWORD_WHILE

#define DEFAULT_MATCHER_INDICATOR 17

namespace papyrus {

  struct KeywordMatcherSettings {
    utility::PrimitiveTypeValueMonitor<bool>     enableKeywordMatching;
    utility::PrimitiveTypeValueMonitor<int>      enabledKeywords;
    utility::PrimitiveTypeValueMonitor<int>      indicatorID;
    utility::PrimitiveTypeValueMonitor<int>      matchedIndicatorStyle;
    utility::PrimitiveTypeValueMonitor<COLORREF> matchedIndicatorForegroundColor;
    utility::PrimitiveTypeValueMonitor<int>      unmatchedIndicatorStyle;
    utility::PrimitiveTypeValueMonitor<COLORREF> unmatchedIndicatorForegroundColor;
  };

} // namespace
