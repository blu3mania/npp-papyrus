/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2016 Tschilkroete <tschilkroete@gmail.com> (original author)
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

#include "..\Common\Version.hpp"

#include <string>
#include <vector>

namespace papyrus {

  using key_value_t = std::pair<std::wstring, std::wstring>;

  class SettingsStorage {
    public:
      inline void init(const std::wstring& path) { settingsPath = path; }

      bool load();
      void save() const;

      bool getString(std::wstring key, std::wstring& value) const;
      void putString(std::wstring key, std::wstring value);

      // Setting file is marked with a version, which can be compared with current plugin version
      inline utility::Version getVersion() const { return version; }
      inline void setVersion(utility::Version newVersion) { version = newVersion; }

    private:
      std::wstring settingsPath;
      std::vector<key_value_t> data;
      utility::Version version;
  };

} // namespace
