/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2016 - 2017 Tschilkroete <tschilkroete@gmail.com> (original author)
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

// For the time being, as there is no good alternative way in C++17 to get stream working than using codecvt
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "SettingsStorage.hpp"

#include "..\Common\StringUtil.hpp"

#include "..\..\external\gsl\include\gsl\util"

#include <codecvt>
#include <fstream>

#include <windows.h>

#define VERSION_KEY L"version"

namespace papyrus {

  bool SettingsStorage::load() {
    if (!settingsPath.empty()) {
      std::wifstream settingsFile(settingsPath);
      settingsFile.imbue(std::locale(settingsFile.getloc(), new std::codecvt_utf8<wchar_t>())); // Use UTF-8 encoding
      std::wstring line;
      while (std::getline(settingsFile, line)) {
        size_t equalsIndex = line.find_first_of(L'=');
        std::wstring key = line.substr(0, equalsIndex);
        std::wstring value;
        if (equalsIndex != line.length() - 1) {
          value = line.substr(equalsIndex + 1);
        }
        if (utility::compare(key, VERSION_KEY)) {
          version = utility::Version(value);
        } else {
          data.push_back(key_value_t(key, value));
        }
      }
      settingsFile.close();
      return (data.size() > 0);
    }

    return false;
  }

  void SettingsStorage::save() const {
    if (!settingsPath.empty()) {
      std::wofstream settingsFile(settingsPath, std::wofstream::trunc);
      auto autoCleanup = gsl::finally([&] { settingsFile.close(); });
      settingsFile.imbue(std::locale(settingsFile.getloc(), new std::codecvt_utf8<wchar_t>())); // Use UTF-8 encoding
      settingsFile << VERSION_KEY << L'=' << version << std::endl;
      for (const auto& p : data) {
        settingsFile << p.first << L'=' << p.second << std::endl;
      }
    }
  }

  bool SettingsStorage::getString(std::wstring key, std::wstring& value) const {
    for (const auto& p : data) {
      if (p.first == key) {
        value = p.second;
        return true;
      }
    }
    return false;
  }

  void SettingsStorage::putString(std::wstring key, std::wstring value) {
    for (auto& p : data) {
      if (p.first == key) {
        p.second = value;
        return;
      }
    }
    data.push_back(key_value_t(key, value));
  }

} // namespace
