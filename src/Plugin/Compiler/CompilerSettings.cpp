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

#include "CompilerSettings.hpp"

namespace papyrus {

  const CompilerSettings::GameSettings& CompilerSettings::gameSettings(game::Game game) const {
    switch (game) {
      case game::Skyrim: {
        return skyrim;
      }

      case game::SkyrimSE: {
        return sse;
      }

      case game::Fallout4: {
        return fo4;
      }

      default: {
        throw std::runtime_error("Should not get here");
      }
    }
  }

  CompilerSettings::GameSettings& CompilerSettings::gameSettings(game::Game game) {
    return const_cast<GameSettings&>(const_cast<const CompilerSettings*>(this)->gameSettings(game));
  }

} // namespace
