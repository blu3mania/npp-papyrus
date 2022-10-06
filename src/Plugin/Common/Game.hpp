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

#include <map>
#include <string>

namespace papyrus {

  namespace game {

    enum class Game {
      Auto,
      Skyrim,
      SkyrimSE,
      Fallout4
    };

    // Game names, in a pair of {alias, displayName}
    static std::pair<std::wstring, std::wstring> gameNames[] {
      {L"", L""},
      {L"skyrim", L"Skyrim"},
      {L"sse", L"Skyrim SE/AE"},
      {L"fo4", L"Fallout 4"}
    };

    static std::map<std::wstring, Game> gameAliases {
      {gameNames[std::to_underlying(Game::Auto)].first, Game::Auto},
      {gameNames[std::to_underlying(Game::Skyrim)].first, Game::Skyrim},
      {gameNames[std::to_underlying(Game::SkyrimSE)].first, Game::SkyrimSE},
      {gameNames[std::to_underlying(Game::Fallout4)].first, Game::Fallout4}
    };

    static std::map<std::wstring, Game> games {
      {gameNames[std::to_underlying(Game::Auto)].second, Game::Auto},
      {gameNames[std::to_underlying(Game::Skyrim)].second, Game::Skyrim},
      {gameNames[std::to_underlying(Game::SkyrimSE)].second, Game::SkyrimSE},
      {gameNames[std::to_underlying(Game::Fallout4)].second, Game::Fallout4}
    };

    std::wstring installationPath(Game game);

  } // namespace game

} // namespace papyrus
