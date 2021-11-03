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

#include "..\Common\Game.hpp"
#include "..\Common\PrimitiveTypeValueMonitor.hpp"

#include <map>
#include <stdexcept>
#include <string>

#include <windows.h>

namespace papyrus {

  using Game = game::Game;

  struct CompilerSettings {

    struct GameSettings {
      utility::PrimitiveTypeValueMonitor<bool> enabled;
      std::wstring installPath;
      std::wstring compilerPath;
      std::wstring importDirectories;
      std::wstring outputDirectory;
      std::wstring flagFile;
      std::wstring additionalArguments;
      utility::PrimitiveTypeValueMonitor<bool> anonynmizeFlag;
      utility::PrimitiveTypeValueMonitor<bool> optimizeFlag;
      utility::PrimitiveTypeValueMonitor<bool> releaseFlag;
      utility::PrimitiveTypeValueMonitor<bool> finalFlag;
    };

    GameSettings skyrim;
    GameSettings sse;
    GameSettings fo4;
    Game gameMode {Game::Auto};
    Game autoModeDefaultGame {Game::Auto};
    std::wstring autoModeOutputDirectory;
    utility::PrimitiveTypeValueMonitor<bool> allowUnmanagedSource;

    const GameSettings& gameSettings(Game game) const;
    GameSettings& gameSettings(Game game);
  };

} // namespace
