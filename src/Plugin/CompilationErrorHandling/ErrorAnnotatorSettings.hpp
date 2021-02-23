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

  struct ErrorAnnotatorSettings {
    utility::PrimitiveTypeValueMonitor<bool>     enableAnnotation;
    utility::PrimitiveTypeValueMonitor<COLORREF> annotationForegroundColor;
    utility::PrimitiveTypeValueMonitor<COLORREF> annotationBackgroundColor;
    utility::PrimitiveTypeValueMonitor<bool>     isAnnotationItalic;
    utility::PrimitiveTypeValueMonitor<bool>     isAnnotationBold;
    utility::PrimitiveTypeValueMonitor<bool>     enableIndication;
    utility::PrimitiveTypeValueMonitor<int>      indicatorID;
    utility::PrimitiveTypeValueMonitor<int>      indicatorStyle;
    utility::PrimitiveTypeValueMonitor<COLORREF> indicatorForegroundColor;
  };

} // namespace
