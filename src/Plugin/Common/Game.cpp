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

#include "Game.hpp"

#include "FinalAction.hpp"

#include <string>
#include <vector>

#include <windows.h>

#ifdef _WIN64
#define REGKEY_SOFTWARE(path) L"SOFTWARE\\WOW6432NODE\\" path
#else
#define REGKEY_SOFTWARE(path) L"SOFTWARE\\" path
#endif

namespace papyrus {

  namespace game {

    std::wstring installationPath(Game game) {
      const wchar_t* regKey = nullptr;
      switch (game) {
        case Game::Skyrim: {
          regKey = REGKEY_SOFTWARE(L"Bethesda Softworks\\Skyrim");
          break;
        }

        case Game::SkyrimSE: {
          regKey = REGKEY_SOFTWARE(L"Bethesda Softworks\\Skyrim Special Edition");
          break;
        }

        case Game::Fallout4: {
          regKey = REGKEY_SOFTWARE(L"Bethesda Softworks\\Fallout4");
          break;
        }
      }

      std::wstring path;
      if (regKey != nullptr) {
        // Check if the game's registry key exists
        HKEY gameRegKey;
        if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, regKey, 0, KEY_READ, &gameRegKey) == ERROR_SUCCESS) {
          auto autoCleanup = utility::finally([&] { ::RegCloseKey(gameRegKey); });

          // Get game's installation path
          DWORD size;
          ::RegQueryValueEx(gameRegKey, L"Installed Path", 0, nullptr, nullptr, &size);
          if (size > 0) {
            std::vector<wchar_t> gamePath(size / sizeof(wchar_t));
            ::RegQueryValueEx(gameRegKey, L"Installed Path", 0, nullptr, (LPBYTE)&gamePath[0], &size);
            path = std::wstring(&gamePath[0]);
          }
        }
      }

      return path;
    }

  } // namespace game

} // namespace papyrus
