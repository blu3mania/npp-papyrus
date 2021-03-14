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

#include <string>

#include "windows.h"

namespace utility {

  // Conversion between string and other types
  //
  bool strToBool(const std::wstring& str) noexcept;
  std::wstring boolToStr(bool boolValue) noexcept;
  int hexStrToInt(const std::wstring& hexStr) noexcept;
  std::wstring intToHexStr(int intValue) noexcept;
  COLORREF hexStrToColor(const std::wstring& hexStr) noexcept;
  std::wstring colorToHexStr(COLORREF color) noexcept;

  // String utilites
  //
  inline bool isNumber(const std::wstring& str) noexcept { return !str.empty() && std::find_if(str.begin(), str.end(), [](wchar_t ch) { return !iswdigit(ch); }) == str.end(); }
  inline bool isHexNumber(const std::wstring& str) noexcept { return !str.empty() && std::find_if(str.begin(), str.end(), [](wchar_t ch) { return !iswxdigit(ch); }) == str.end(); }
  bool compare(const std::string& str1, const std::string& str2, bool ignoreCase = true) noexcept;
  bool compare(const std::wstring& str1, const std::wstring& str2, bool ignoreCase = true) noexcept;
  bool startsWith(const std::wstring& str1, const std::wstring& str2, bool ignoreCase = true) noexcept;
  bool endsWith(const std::wstring& str1, const std::wstring& str2, bool ignoreCase = true) noexcept;
  size_t findIndex(const std::wstring& str1, const std::wstring& str2, bool ignoreCase = true) noexcept;
  std::wstring toUpper(const std::wstring& str) noexcept;
  std::wstring toLower(const std::wstring& str) noexcept;

  // Date/Time utilities
  int currentYear() noexcept;

  // File utilities
  //
  bool fileExists(const std::wstring& filePath);

} // namespace
