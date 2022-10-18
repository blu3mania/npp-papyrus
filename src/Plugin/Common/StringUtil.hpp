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

#include <cwctype>
#include <format>
#include <sstream>
#include <string>
#include <vector>

#include "windows.h"

namespace utility {

  // Conversion between string and other types
  //
  inline bool strToBool(const std::wstring& str) noexcept {
    bool boolValue = false;
    std::wistringstream(str) >> std::boolalpha >> boolValue;
    return boolValue;
  }
  inline std::wstring boolToStr(bool boolValue) noexcept { return std::format(L"{}", boolValue); }

  inline int hexStrToInt(const std::wstring& hexStr) noexcept {
    int intValue {};
    std::wstringstream strStream;
    strStream << std::hex << hexStr;
    strStream >> intValue;
    return intValue;
  }
  inline std::wstring intToHexStr(int intValue) noexcept { return std::format(L"{:X}", intValue); }

  inline COLORREF hexStrToColor(const std::wstring& hexStr) noexcept {
    COLORREF color {};
    std::wstringstream strStream;
    strStream << std::hex << hexStr;
    strStream >> color;

    // COLORREF is BGR
    return ((color >> 16) & 0xFF) | (color & 0xFF00) | ((color & 0xFF) << 16);
  }
  inline std::wstring colorToHexStr(COLORREF color) noexcept { return std::format(L"{:06X}", (((color >> 16) & 0xFF) | (color & 0xFF00) | ((color & 0xFF) << 16))); } // COLORREF is BGR

  // String utilities
  //
  inline bool isNumber(const std::wstring& str) noexcept { return !str.empty() && std::find_if(str.begin(), str.end(), [](wchar_t ch) { return !iswdigit(ch); }) == str.end(); }
  inline bool isHexNumber(const std::wstring& str) noexcept { return !str.empty() && std::find_if(str.begin(), str.end(), [](wchar_t ch) { return !iswxdigit(ch); }) == str.end(); }

  bool compare(const std::string& str1, const std::string& str2, bool ignoreCase = true) noexcept;
  bool compare(const std::wstring& str1, const std::wstring& str2, bool ignoreCase = true) noexcept;

  inline bool startsWith(const std::string& str1, const std::string& str2, bool ignoreCase = true) noexcept {
    if (str1.length() < str2.length()) {
      return false;
    }

    return compare(str1.substr(0, str2.length()), str2, ignoreCase);
  }
  inline bool startsWith(const std::wstring& str1, const std::wstring& str2, bool ignoreCase = true) noexcept {
    if (str1.length() < str2.length()) {
      return false;
    }

    return compare(str1.substr(0, str2.length()), str2, ignoreCase);
  }

  inline bool endsWith(const std::string& str1, const std::string& str2, bool ignoreCase = true) noexcept {
    if (str1.length() < str2.length()) {
      return false;
    }

    return compare(str1.substr(str1.length() - str2.length(), std::string::npos), str2, ignoreCase);
  }
  inline bool endsWith(const std::wstring& str1, const std::wstring& str2, bool ignoreCase = true) noexcept {
    if (str1.length() < str2.length()) {
      return false;
    }

    return compare(str1.substr(str1.length() - str2.length(), std::string::npos), str2, ignoreCase);
  }

  size_t indexOf(const std::string& str1, const std::string& str2, size_t startIndex = 0, bool ignoreCase = true) noexcept;
  size_t indexOf(const std::wstring& str1, const std::wstring& str2, size_t startIndex = 0, bool ignoreCase = true) noexcept;

  std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool ignoreCase = true) noexcept;
  std::vector<std::wstring> split(const std::wstring& str, const std::wstring& delimiter, bool ignoreCase = true) noexcept;

  inline std::wstring toUpper(const std::wstring& str) noexcept {
    std::wstring upper;
    std::transform(str.begin(), str.end(), std::back_inserter(upper), std::towupper);
    return upper;
  }
  inline std::string toUpper(const std::string& str) noexcept {
    std::string upper;
    std::transform(str.begin(), str.end(), std::back_inserter(upper), [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
    return upper;
  }

  inline std::wstring toLower(const std::wstring& str) noexcept {
    std::wstring lower;
    std::transform(str.begin(), str.end(), std::back_inserter(lower), std::towlower);
    return lower;
  }
  inline std::string toLower(const std::string& str) noexcept {
    std::string lower;
    std::transform(str.begin(), str.end(), std::back_inserter(lower), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
    return lower;
  }

} // namespace
