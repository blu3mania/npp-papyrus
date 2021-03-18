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

namespace papyrus {

  struct LexerSettings {
    utility::PrimitiveTypeValueMonitor<bool>     enableFoldMiddle;
    utility::PrimitiveTypeValueMonitor<bool>     enableClassNameCache;
    utility::PrimitiveTypeValueMonitor<bool>     enableClassLink;
    utility::PrimitiveTypeValueMonitor<bool>     classLinkUnderline;
    utility::PrimitiveTypeValueMonitor<COLORREF> classLinkForegroundColor;
    utility::PrimitiveTypeValueMonitor<COLORREF> classLinkBackgroundColor;
    utility::PrimitiveTypeValueMonitor<bool>     classLinkRequiresDoubleClick;
    utility::PrimitiveTypeValueMonitor<int>      classLinkClickModifier;
  };

} // namespace
