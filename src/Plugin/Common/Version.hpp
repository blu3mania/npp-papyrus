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

#include <iostream>
#include <string>

namespace utility {

  struct Version {
    inline Version(int major = 0, int minor = 0, int patch = 0) noexcept : major(major), minor(minor), patch(patch) {};
    Version(std::wstring versionStr) noexcept;

    // Use C++20's default three-way comparison to generate all relational operators
    auto operator<=>(const Version& other) const noexcept = default;

    std::string toString(bool full = true) const noexcept;
    std::wstring toWString(bool full = true) const noexcept;

    inline operator std::string() const noexcept { return toString(); }
    inline operator std::wstring() const noexcept { return toWString(); }

    friend std::ostream& operator<<(std::ostream& output, const Version& version) noexcept { output << version.toString(); return output; }
    friend std::wostream& operator<<(std::wostream& output, const Version& version) noexcept { output << version.toWString(); return output; }

    // Private members
    //
    int major;
    int minor;
    int patch;
  };

} // namespace
