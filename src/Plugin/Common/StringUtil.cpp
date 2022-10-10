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

#include "StringUtil.hpp"

#include <algorithm>

namespace utility {

  // String utilities
  //
  bool compare(const std::string& str1, const std::string& str2, bool ignoreCase) noexcept {
    if (str1.length() != str2.length()) {
      return false;
    }

    if (!ignoreCase) {
      return str1.compare(str2) == 0;
    }

    return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(),
      [](const char ch1, const char ch2) {
        return toupper(ch1) == toupper(ch2);
      }
    );
  }

  bool compare(const std::wstring& str1, const std::wstring& str2, bool ignoreCase) noexcept {
    if (str1.length() != str2.length()) {
      return false;
    }

    if (!ignoreCase) {
      return str1.compare(str2) == 0;
    }

    return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(),
      [](const wchar_t ch1, const wchar_t ch2) {
        return towupper(ch1) == towupper(ch2);
      }
    );
  }

  size_t indexOf(const std::string& str1, const std::string& str2, size_t startIndex, bool ignoreCase) noexcept {
    if (startIndex >= str1.size()) {
      return std::string::npos;
    }

    if (!ignoreCase) {
      return str1.find(str2, startIndex);
    }

    auto start = str1.begin();
    std::advance(start, startIndex);
    auto iter = std::search(start, str1.end(), str2.begin(), str2.end(),
      [](const char ch1, const char ch2) {
        return toupper(ch1) == toupper(ch2);
      }
    );

    if (iter != str1.end()) {
      return iter - str1.begin();
    } else {
      return std::string::npos;
    }
  }

  size_t indexOf(const std::wstring& str1, const std::wstring& str2, size_t startIndex, bool ignoreCase) noexcept {
    if (startIndex >= str1.size()) {
      return std::string::npos;
    }

    if (!ignoreCase) {
      return str1.find(str2, startIndex);
    }

    auto start = str1.begin();
    std::advance(start, startIndex);
    auto iter = std::search(start, str1.end(), str2.begin(), str2.end(),
      [](const wchar_t ch1, const wchar_t ch2) {
        return towupper(ch1) == towupper(ch2);
      }
    );

    if (iter != str1.end()) {
      return iter - str1.begin();
    } else {
      return std::string::npos;
    }
  }

  std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool ignoreCase) noexcept {
    std::vector<std::string> result;
    size_t prevPos = 0;
    size_t pos = 0;
    while ((pos = indexOf(str, delimiter, prevPos, ignoreCase)) != std::string::npos) {
      result.push_back(str.substr(prevPos, pos));
      pos += delimiter.size();
      prevPos = pos;
    }
    result.push_back(str.substr(prevPos, pos));

    return result;
  }

  std::vector<std::wstring> split(const std::wstring& str, const std::wstring& delimiter, bool ignoreCase) noexcept {
    std::vector<std::wstring> result;
    size_t prevPos = 0;
    size_t pos = 0;
    while ((pos = indexOf(str, delimiter, prevPos, ignoreCase)) != std::string::npos) {
      result.push_back(str.substr(prevPos, pos));
      pos += delimiter.size();
      prevPos = pos;
    }
    result.push_back(str.substr(prevPos, pos));

    return result;
  }

} // namespace
