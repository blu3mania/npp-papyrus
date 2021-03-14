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

#include "SettingsStorage.hpp"

#include "..\Common\Version.hpp"
#include "..\CompilationErrorHandling\ErrorAnnotatorSettings.hpp"
#include "..\Compiler\CompilerSettings.hpp"
#include "..\Lexer\LexerSettings.hpp"

namespace papyrus {

  using Game = game::Game;

  struct Settings {
    CompilerSettings        compilerSettings;
    ErrorAnnotatorSettings  errorAnnotatorSettings;
    LexerSettings           lexerSettings;

    bool loadSettings(SettingsStorage& storage, utility::Version currentVersion);
    void saveSettings(SettingsStorage& storage);

    private:
      // Read settings from storage. Returns true if some settings are updated (due to missing or invalid value, etc.)
      bool readSettings(const SettingsStorage& storage);

      // Read game settings from storage. Returns a pair of booleans. The first indicates whether the game is configured;
      // The 2nd indicates if some settings are updated (due to missing or invalid value, etc.)
      std::pair<bool, bool> readGameSettings(const SettingsStorage& storage, Game game, CompilerSettings::GameSettings& gameSettings, const std::vector<const wchar_t*>& defaultImportDirs, const wchar_t* defaultFlagFile);

      void saveGameSettings(SettingsStorage& storage, Game game, const CompilerSettings::GameSettings& gameSettings);
  };

} // namespace
