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

#include "Settings.hpp"

#include "..\..\external\npp\ColourPicker.h"
#include "..\..\external\npp\StaticDialog.h"

#include <string>
#include <vector>

#include <windows.h>

namespace papyrus {

  using dropdown_options_t = std::vector<LPCWSTR>;

  class SettingsDialog : public StaticDialog {
    public:
      inline SettingsDialog(Settings& settings) : StaticDialog(), settings(settings) {}
      ~SettingsDialog();

      INT_PTR doDialog();

    protected:
	    INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

    private:
      enum {
        TAB_LEXER,
        TAB_ERROR_ANNOTATOR,
        TAB_COMPILER,
        TAB_GAME_BASE
      };

      enum {
        GROUP_ANNOTATION,
        GROUP_INDICATION,
        GROUP_GAME_AUTO,
        GROUP_GAME_SKYRIM,
        GROUP_GAME_SSE,
        GROUP_GAME_FO4
      };

      HWND getControl(int controlID) const;
      void initControls();
      void initDropdownList(int controlID, const dropdown_options_t& options) const;
      void initColorPicker(ColourPicker& colorPicker, int labelControlID) const;
      HWND createToolTip(int controlID, std::wstring toolTip, int delayTime = 15) const;

      void switchTab(int newTab);
      void showTab(int tab, bool show, bool intializing = false) const;

      void enableGroup(int group, bool enabled) const;

      void setChecked(int controlID, bool checked) const;
      bool getChecked(int controlID) const;

      void setText(int controlID, std::wstring text) const;
      std::wstring getText(int controlID) const;

      game::Game getGame(int tab) const;
      int getGameTab(game::Game game) const;
      void addGameTab(game::Game game) const;
      void removeGameTab(game::Game game) const;
      void toggleGame(game::Game game, int controlID, int group) const;
      void configureGame(game::Game game);

      void updateAutoModeDefaultGame() const;
      void updateGameEnableButtonText(int controlID, bool enabled) const;

      void loadGameSettings(const CompilerSettings::GameSettings& gameSettings, bool isFallout4) const;
      void saveGameSettings(CompilerSettings::GameSettings& gameSettings, bool isFallout4) const;
      bool saveSettings();

      // Private members
      //
      Settings& settings;
      int currentTab { TAB_LEXER };

      HWND indicatorIdTooltip {};
      HWND autoModeTooltip {};

      ColourPicker annotationFgColorPicker;
      ColourPicker annotationBgColorPicker;
      ColourPicker indicatorFgColorPicker;
  };

} // namespace
