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
#include "..\Common\NotepadPlusPlus.hpp"

#include <string>

namespace papyrus {

  using Game = game::Game;

  struct CompilationRequest {
    Game game = Game::Auto;
    npp_buffer_t bufferID = 0;
    std::wstring filePath;
    bool useAutoModeOutputDirectory = false;
  };

} // namespace
