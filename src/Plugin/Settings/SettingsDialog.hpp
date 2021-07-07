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
#include "..\UI\DialogBase.hpp"

#include "..\..\external\npp\URLCtrl.h"

namespace papyrus {

  using Game = game::Game;

  class SettingsDialog : public DialogBase {
    public:
      inline SettingsDialog(Settings& settings, utility::PrimitiveTypeValueMonitor<bool>& settingsUpdated) : DialogBase(IDD_SETTINGS_DIALOG), settings(settings), settingsUpdated(settingsUpdated) {}
      ~SettingsDialog();

      void doDialog() override;

    protected:
      void initControls() override;
      INT_PTR handleCommandMessage(WPARAM wParam, LPARAM lParam) override;
      INT_PTR handleNotifyMessage(WPARAM wParam, LPARAM lParam) override;
      INT_PTR handleCloseMessage(WPARAM wParam, LPARAM lParam) override;

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
        Matcher,
        Annotation,
        Indication,
        GameAuto,
        GameSkyrim,
        GameSSE,
        GameFO4
      };

      void switchTab(Tab newTab);
      void showTab(Tab tab, bool show, bool intializing = false) const;

      void enableGroup(Group group, bool enabled) const;

      void updateEnabledKeywords() const;

      Game getGame(Tab tab) const;
      int getGameTab(Game game) const;
      void addGameTab(Game game) const;
      void removeGameTab(Game game) const;
      void toggleGame(Game game, int controlID, Group group) const;
      void configureGame(Game game);

      void updateAutoModeDefaultGame() const;
      void updateGameEnableButtonText(int controlID, bool enabled) const;

      void loadGameSettings(const CompilerSettings::GameSettings& gameSettings, bool isFallout4) const;
      void saveGameSettings(CompilerSettings::GameSettings& gameSettings, bool isFallout4) const;
      bool saveSettings();

      // Private members
      //
      Settings& settings;
      utility::PrimitiveTypeValueMonitor<bool>& settingsUpdated;
      Tab currentTab { Tab::Lexer };

      HWND matcherIndicatorIdTooltip {};
      HWND errorIndicatorIdTooltip {};
      HWND autoModeTooltip {};

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
