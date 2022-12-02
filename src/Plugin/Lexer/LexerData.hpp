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

#include "LexerSettings.hpp"
#include "..\Common\Game.hpp"
#include "..\Common\NotepadPlusPlus.hpp"
#include "..\Common\Topic.hpp"

#include "..\..\external\npp\PluginInterface.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace papyrus {

  using Game = game::Game;
  using game_import_dirs_t = std::map<Game, std::vector<std::wstring>>;

  struct BufferActivationEventData {
    npp_view_t view;
    npp_buffer_t bufferID;
    bool isManagedBuffer;
  };
  using buffer_activated_topic_t = utility::Topic<BufferActivationEventData>;

  struct ClickEventData {
    HWND scintillaHandle;
    npp_buffer_t bufferID;
    Sci_Position position;
  };
  using click_event_topic_t = utility::Topic<ClickEventData>;

  struct ChangeEventData {
    HWND scintillaHandle;
    npp_buffer_t bufferID;
    Sci_Position position;
    Sci_Position linesAdded;
  };
  using change_event_topic_t = utility::Topic<ChangeEventData>;

  // Pass data from plugin to lexer, e.g. settings, and event data received from NPP or Scintilla
  struct LexerData {
    LexerData(const NppData& nppData, const LexerSettings& settings, Game currentGame = Game::Auto, game_import_dirs_t importDirectories = game_import_dirs_t(), bool usable = true)
      : nppData(nppData), settings(settings), currentGame(currentGame), importDirectories(importDirectories), scriptLangID(0), usable(usable) {
    }

    const NppData& nppData;
    const LexerSettings& settings;
    Game currentGame;
    game_import_dirs_t importDirectories;
    npp_lang_type_t scriptLangID;
    buffer_activated_topic_t bufferActivated;
    click_event_topic_t clickEventData;
    change_event_topic_t changeEventData;
    utility::PrimitiveTypeValueMonitor<bool> nppReady {false};
    bool usable;
  };

  extern std::unique_ptr<LexerData> lexerData;

} // namespace