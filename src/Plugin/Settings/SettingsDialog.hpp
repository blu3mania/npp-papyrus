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

#include "..\Common\Resources.hpp"
#include "..\UI\MultiTabbedDialog.hpp"
#include "..\UI\UIParameters.hpp"

#include "..\..\external\npp\URLCtrl.h"

namespace papyrus {

  using Game = game::Game;

  class SettingsDialog : public MultiTabbedDialog {

    public:
      using callback_t = std::function<void()>;

      SettingsDialog(Settings& settings, const UIParameters& uiParameters);
      ~SettingsDialog();

      void doDialog(callback_t callback);

    protected:
      void initControls() override;
      INT_PTR handleCloseMessage(WPARAM wParam, LPARAM lParam) override;
      INT_PTR handleTabCommandMessage(tab_id_t tab, WPARAM wParam, LPARAM lParam) override;

      void onTabDialogCreated(tab_id_t tab) override;

    private:
      enum class Tab {
        Lexer,
        KeywordMatcher,
        ErrorAnnotator,
        Compiler,
        GameBase
      };

      enum class Group {
        ClassLink,
        Hover,
        Matcher,
        Annotation,
        Indication,
        GameAuto,
        GameSkyrim,
        GameSSE,
        GameFO4
      };

      void enableGroup(Group group, bool enabled) const;

      void updateEnabledKeywords() const;

      Game getGame(tab_id_t tab) const;
      tab_id_t getGameTab(Game game) const;
      void addGameTab(Game game);
      void removeGameTab(Game game);
      void toggleGame(Game game, int controlID, Group group);
      void configureGame(Game game);

      void updateAutoModeDefaultGame() const;
      void updateGameEnableButtonText(int controlID, bool enabled) const;

      void saveGameSettings(tab_id_t tab, CompilerSettings::GameSettings& gameSettings) const;
      bool saveSettings();

      // Private members
      //
      Settings& settings;
      callback_t settingsUpdatedFunc {};

      URLCtrl stylerConfigLink;

      ColourPicker classLinkFgColorPicker;
      ColourPicker classLinkBgColorPicker;
      ColourPicker matchedIndicatorFgColorPicker;
      ColourPicker unmatchedIndicatorFgColorPicker;
      ColourPicker annotationFgColorPicker;
      ColourPicker annotationBgColorPicker;
      ColourPicker errorIndicatorFgColorPicker;
  };

} // namespace
