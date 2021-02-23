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

#include "Version.hpp"

#include <sstream>
#include <string>

namespace utility {

  Version::Version(std::wstring versionStr) noexcept
    : Version(0, 0, 0) {
    std::wstring segment;
    std::wistringstream versionStream(versionStr);
    if (std::getline(versionStream, segment, L'.')) {
      major = std::stoi(segment);

      if (std::getline(versionStream, segment, L'.')) {
        minor = std::stoi(segment);

        if (std::getline(versionStream, segment, L'.')) {
          patch = std::stoi(segment);
        }
      }
    }
  }

  std::string Version::toString(bool full) const noexcept {
    std::string str = std::to_string(major) + '.' + std::to_string(minor);
    if (full || patch != 0) {
      str += '.';
      str += std::to_string(patch);
    }
    return str;
  }

  std::wstring Version::toWString(bool full) const noexcept {
    std::wstring str = std::to_wstring(major) + L'.' +std::to_wstring(minor);
    if (full || patch != 0) {
      str += L'.';
      str += std::to_wstring(patch);
    }
    return str;
  }

} // namespace
