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

#include "SettingsDialog.hpp"

#include "..\Common\EnumUtil.hpp"
#include "..\Common\Resources.hpp"
#include "..\Common\Utility.hpp"

#include "..\..\external\gsl\include\gsl\util"
#include "..\..\external\npp\PluginInterface.h"

#include <sstream>

#include <commctrl.h>

namespace papyrus {

  // Internal static variables
  namespace {
    dropdown_options_t tabNames {
      L"Lexer",
      L"Error Annotator",
      L"Compiler"
    };

    dropdown_options_t indicatorStyles {
      L"Plain underline",
      L"Squiggle underline",
      L"Line of T shapes",
      L"Diagonal hatching",
      L"Strike out",
      L"Hidden",
      L"Rectangle box",
      L"Filled round box",
      L"Filled straight box",
      L"Dashed underline",
      L"Dotted underline",
      L"Smaller squiggle underline",
      L"Dotted rectangle box",
      L"Alternative squiggle underline",
      L"Thick composition underline",
      L"Thin composition underline",
      L"Filled full box",
      L"Text fore color",
      L"Triangle",
      L"Alternative triangle",
      L"Gradient",
      L"Alternative gradient"
    };
  }

  SettingsDialog::~SettingsDialog() {
    cleanup();
  }

  // Protected methods
  //

  void SettingsDialog::initControls() {
    // Lexer settings
    //
    setChecked(IDC_SETTINGS_LEXER_FOLD_MIDDLE, settings.lexerSettings.enableFoldMiddle);
    setChecked(IDC_SETTINGS_LEXER_CLASS_NAME_CACHING, settings.lexerSettings.enableClassNameCache);

    setChecked(IDC_SETTINGS_LEXER_CLASS_LINK, settings.lexerSettings.enableClassLink);
    initColorPicker(classLinkFgColorPicker, IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL);
    classLinkFgColorPicker.setColour(settings.lexerSettings.classLinkForegroundColor);
    initColorPicker(classLinkBgColorPicker, IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL);
    classLinkBgColorPicker.setColour(settings.lexerSettings.classLinkBackgroundColor);
    setChecked(IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, settings.lexerSettings.classLinkUnderline);

    // Error annotator settings
    //
    setChecked(IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION, settings.errorAnnotatorSettings.enableAnnotation);
    initColorPicker(annotationFgColorPicker, IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL);
    annotationFgColorPicker.setColour(settings.errorAnnotatorSettings.annotationForegroundColor);
    initColorPicker(annotationBgColorPicker, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL);
    annotationBgColorPicker.setColour(settings.errorAnnotatorSettings.annotationBackgroundColor);
    setChecked(IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC, settings.errorAnnotatorSettings.isAnnotationItalic);
    setChecked(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD, settings.errorAnnotatorSettings.isAnnotationBold);
    setChecked(IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION, settings.errorAnnotatorSettings.enableIndication);

    indicatorIdTooltip = createToolTip(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL,
      L"Choose a number between 9 and 31. Keep in mind other plugins may use indiator IDs as well, "
      L"e.g. DSpellCheck uses 19, so if there are conflicts, just choose a different number.\r\n"
      L"Note, if changes have been made after indications were shown, changing ID again may cause "
      L"indications to be rendered incorrectly. Trigger recompilation to fix them if needed."
    );
    setText(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID, std::to_wstring(settings.errorAnnotatorSettings.indicatorID));

    initDropdownList(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, indicatorStyles);
    if (settings.errorAnnotatorSettings.indicatorStyle > INDIC_GRADIENTCENTRE) {
      settings.errorAnnotatorSettings.indicatorStyle = INDIC_SQUIGGLEPIXMAP;
    }
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, CB_SETCURSEL, settings.errorAnnotatorSettings.indicatorStyle, 0);

    initColorPicker(indicatorFgColorPicker, IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL);
    indicatorFgColorPicker.setColour(settings.errorAnnotatorSettings.indicatorForegroundColor);

    // Compiler settings
    //
    setChecked(IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE, settings.compilerSettings.allowUnmanagedSource);
    setChecked(IDC_SETTINGS_COMPILER_RADIO_AUTO + utility::underlying(settings.compilerSettings.gameMode), true);
    setText(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, settings.compilerSettings.autoModeOutputDirectory);
    updateAutoModeDefaultGame();
    if (settings.compilerSettings.autoModeDefaultGame != Game::Auto) {
      // Try to find and set current user prefs in auto mode default game dropdown list
      auto gameName = game::gameNames[utility::underlying(settings.compilerSettings.autoModeDefaultGame)].second;
      auto index = ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_FINDSTRINGEXACT, 0, reinterpret_cast<WPARAM>(gameName.c_str()));
      if (index != CB_ERR) {
        ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_SETCURSEL, index, 0);
      }
    }

    autoModeTooltip = createToolTip(IDC_SETTINGS_COMPILER_RADIO_AUTO,
      L"In this mode, Papyrus compiler to be used is determined by the path of source script file. "
      L"If it's under a detected game's directory, that game's settings will be used. If not, default "
      L"game's settings will be used, except that default output directory will be used instead of "
      L"default game's."
    );

    // Set tab texts
    TCITEM item {
      .mask = TCIF_TEXT
    };
    for (int i = 0; i < utility::underlying(Tab::GameBase); i++) {
      item.pszText = const_cast<LPWSTR>(tabNames[i]);
      item.cchTextMax = static_cast<int>(_tcslen(tabNames[i]));
      ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_INSERTITEM, i, reinterpret_cast<LPARAM>(&item));
      showTab(static_cast<Tab>(i), false, true);
    }

     // During initialization, hide game tab controls
    showTab(Tab::GameBase, false, true);

    for (int i = utility::underlying(Game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
      auto game = static_cast<Game>(i);
      const CompilerSettings::GameSettings& gameSettings = settings.compilerSettings.gameSettings(game);
      if (gameSettings.enabled) {
        addGameTab(game);
      }
    }

    currentTab = Tab::Lexer;
    showTab(currentTab, true);
  }

  INT_PTR SettingsDialog::handleCommandMessage(WPARAM wParam, LPARAM lParam) {
    if (HIWORD(wParam) == BN_CLICKED) {
      // Only controls that would trigger live update are checked here
      switch (LOWORD(wParam)) {
        case IDC_SETTINGS_LEXER_FOLD_MIDDLE: {
          settings.lexerSettings.enableFoldMiddle = getChecked(IDC_SETTINGS_LEXER_FOLD_MIDDLE);
          return FALSE;
        }

        case IDC_SETTINGS_LEXER_CLASS_LINK: {
          settings.lexerSettings.enableClassLink = getChecked(IDC_SETTINGS_LEXER_CLASS_LINK);
          enableGroup(Group::ClassLink, settings.lexerSettings.enableClassLink);
          return FALSE;
        }

        case IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE: {
          settings.lexerSettings.classLinkUnderline = getChecked(IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION: {
          settings.errorAnnotatorSettings.enableAnnotation = getChecked(IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION);
          enableGroup(Group::Annotation, settings.errorAnnotatorSettings.enableAnnotation);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION: {
          settings.errorAnnotatorSettings.enableIndication = getChecked(IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION);
          enableGroup(Group::Indication, settings.errorAnnotatorSettings.enableIndication);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC: {
          settings.errorAnnotatorSettings.isAnnotationItalic = getChecked(IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD: {
          settings.errorAnnotatorSettings.isAnnotationBold = getChecked(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD);
          return FALSE;
        }

        case IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE: {
          toggleGame(Game::Skyrim, IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE, Group::GameSkyrim);
          return FALSE;
        }

        case IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE: {
          configureGame(Game::Skyrim);
          return FALSE;
        }

        case IDC_SETTINGS_COMPILER_SSE_TOGGLE: {
          toggleGame(Game::SkyrimSE, IDC_SETTINGS_COMPILER_SSE_TOGGLE, Group::GameSSE);
          return FALSE;
        }

        case IDC_SETTINGS_COMPILER_SSE_CONFIGURE: {
          configureGame(Game::SkyrimSE);
          return FALSE;
        }

        case IDC_SETTINGS_COMPILER_FO4_TOGGLE: {
          toggleGame(Game::Fallout4, IDC_SETTINGS_COMPILER_FO4_TOGGLE, Group::GameFO4);
          return FALSE;
        }

        case IDC_SETTINGS_COMPILER_FO4_CONFIGURE: {
          configureGame(Game::Fallout4);
          return FALSE;
        }

        default: {
          HWND window = reinterpret_cast<HWND>(lParam);
          if (window == classLinkFgColorPicker.getHSelf()) {
            settings.lexerSettings.classLinkForegroundColor = classLinkFgColorPicker.getColour();
          } else if (window == classLinkBgColorPicker.getHSelf()) {
            settings.lexerSettings.classLinkBackgroundColor = classLinkBgColorPicker.getColour();
          } else if (window == annotationFgColorPicker.getHSelf()) {
            settings.errorAnnotatorSettings.annotationForegroundColor = annotationFgColorPicker.getColour();
          } else if (window == annotationBgColorPicker.getHSelf()) {
            settings.errorAnnotatorSettings.annotationBackgroundColor = annotationBgColorPicker.getColour();
          } else if (window == indicatorFgColorPicker.getHSelf()) {
            settings.errorAnnotatorSettings.indicatorForegroundColor = indicatorFgColorPicker.getColour();
          }
          return FALSE;
        }
      }
    } else if (HIWORD(wParam) == CBN_SELCHANGE) {
      switch (LOWORD(wParam)) {
        case IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN: {
          auto selectedIndex = ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, CB_GETCURSEL, 0, 0);
          if (selectedIndex != CB_ERR) {
            settings.errorAnnotatorSettings.indicatorStyle = static_cast<int>(selectedIndex);
          }
          return FALSE;
        }
      }
    }

    return FALSE;
  }

  INT_PTR SettingsDialog::handleNotifyMessage(WPARAM wParam, LPARAM lParam) {
    auto nmhdr = *(reinterpret_cast<LPNMHDR>(lParam));
    switch (nmhdr.idFrom) {
      case IDC_SETTINGS_TABS: {
        if (nmhdr.code == TCN_SELCHANGE) {
          switchTab(static_cast<Tab>(::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_GETCURSEL, 0, 0)));
        }
        return TRUE;
      }

      default: {
        break;
      }
    }

    return FALSE;
  }

  INT_PTR SettingsDialog::handleCloseMessage(WPARAM wParam, LPARAM lParam) {
    if (saveSettings()) {
      cleanup();
      ::EndDialog(getHSelf(), IDOK);
    }

    return FALSE;
  }

  // Private methods
  //

  void SettingsDialog::cleanup() {
    classLinkFgColorPicker.destroy();
    classLinkBgColorPicker.destroy();
    annotationFgColorPicker.destroy();
    annotationBgColorPicker.destroy();
    indicatorFgColorPicker.destroy();
    if (indicatorIdTooltip) {
      ::DestroyWindow(indicatorIdTooltip);
      indicatorIdTooltip = nullptr;
    }
    if (autoModeTooltip) {
      ::DestroyWindow(autoModeTooltip);
      autoModeTooltip = nullptr;
    }
  }

  void SettingsDialog::switchTab(Tab newTab) {
    if (newTab != currentTab) {
      showTab(currentTab, false);
      showTab(newTab, true);
      currentTab = newTab;
    }
  }

  void SettingsDialog::showTab(Tab tab, bool show, bool intializing) const {
    int showCommand = show ? SW_SHOW : SW_HIDE;
    switch (tab) {
      case Tab::Lexer: {
        if (show) {
          enableGroup(Group::ClassLink, settings.lexerSettings.enableClassLink);
        }
        setControlVisibility(IDC_SETTINGS_LEXER_SCRIPT_GROUP, show);
        setControlVisibility(IDC_SETTINGS_LEXER_FOLD_MIDDLE, show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_NAME_CACHING, show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK, show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL, show);
        classLinkFgColorPicker.display(show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL, show);
        classLinkBgColorPicker.display(show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, show);
        break;
      }

      case Tab::ErrorAnnotator: {
        if (show) {
          enableGroup(Group::Annotation, settings.errorAnnotatorSettings.enableAnnotation);
          enableGroup(Group::Indication, settings.errorAnnotatorSettings.enableIndication);
        }

        setControlVisibility(IDC_SETTINGS_ANNOTATOR_ANNOTATION_GROUP, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL, show);
        annotationFgColorPicker.display(show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL, show);
        annotationBgColorPicker.display(show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD, show);

        setControlVisibility(IDC_SETTINGS_ANNOTATOR_INDICATOR_GROUP, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_LABEL, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, show);
        setControlVisibility(IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL, show);
        indicatorFgColorPicker.display(show);
        break;
      }

      case Tab::Compiler: {
        if (show) {
          enableGroup(Group::GameSkyrim, settings.compilerSettings.skyrim.enabled);
          enableGroup(Group::GameSSE, settings.compilerSettings.sse.enabled);
          enableGroup(Group::GameFO4, settings.compilerSettings.fo4.enabled);
          updateAutoModeDefaultGame();
        }

        setControlVisibility(IDC_SETTINGS_COMPILER_GAMES_GROUP, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_RADIO_AUTO, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_RADIO_SKYRIM, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_RADIO_SSE, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_RADIO_FO4, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_LABEL, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT_LABEL, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_SSE_TOGGLE, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_SSE_CONFIGURE, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_FO4_TOGGLE, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_FO4_CONFIGURE, show);
        setControlVisibility(IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE, show);
        break;
      }

      default: {
        if (tab >= Tab::GameBase) {
          // Update displayed values when showing, and save current values when hiding
          auto game = getGame(tab);
          bool isFallout4 = (game == Game::Fallout4);

          // Only handle current game tab settings after UI initialization
          if (!intializing) {
            if (show) {
              loadGameSettings(settings.compilerSettings.gameSettings(game), isFallout4);
            } else {
              saveGameSettings(settings.compilerSettings.gameSettings(game), isFallout4);
            }
          }

          setControlVisibility(IDC_SETTINGS_TAB_GAME_INSTALL_PATH_LABEL, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_INSTALL_PATH, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_COMPILER_PATH_LABEL, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_COMPILER_PATH, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES_LABEL1, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES_LABEL2, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY_LABEL, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_FLAG_FILE_LABEL, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_FLAG_FILE, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_ANONYMIZE, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_OPTIMIZE, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_RELEASE, show && isFallout4);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_FINAL, show && isFallout4);
        }
        break;
      }
    }
  }

  void SettingsDialog::enableGroup(Group group, bool enabled) const {
    switch (group) {
      case Group::ClassLink: {
        setControlEnabled(IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL, enabled);
        ::EnableWindow(classLinkFgColorPicker.getHSelf(), enabled);
        setControlEnabled(IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL, enabled);
        ::EnableWindow(classLinkBgColorPicker.getHSelf(), enabled);
        setControlEnabled(IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, enabled);
        break;
      }

      case Group::Annotation: {
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL, enabled);
        ::EnableWindow(annotationFgColorPicker.getHSelf(), enabled);
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL, enabled);
        ::EnableWindow(annotationBgColorPicker.getHSelf(), enabled);
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC, enabled);
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD, enabled);
        break;
      }

      case Group::Indication: {
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL, enabled);
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID, enabled);
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_LABEL, enabled);
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, enabled);
        setControlEnabled(IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL, enabled);
        ::EnableWindow(indicatorFgColorPicker.getHSelf(), enabled);
        break;
      }

      case Group::GameAuto: {
        setControlEnabled(IDC_SETTINGS_COMPILER_RADIO_AUTO, enabled);
        setControlEnabled(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_LABEL, enabled);
        setControlEnabled(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, enabled);
        setControlEnabled(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT_LABEL, enabled);
        setControlEnabled(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, enabled);
        break;
      }

      case Group::GameSkyrim: {
        setControlEnabled(IDC_SETTINGS_COMPILER_RADIO_SKYRIM, enabled);
        setControlEnabled(IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE, enabled);
        break;
      }

      case Group::GameSSE: {
        setControlEnabled(IDC_SETTINGS_COMPILER_RADIO_SSE, enabled);
        setControlEnabled(IDC_SETTINGS_COMPILER_SSE_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SSE_TOGGLE, enabled);
        break;
      }

      case Group::GameFO4: {
        setControlEnabled(IDC_SETTINGS_COMPILER_RADIO_FO4, enabled);
        setControlEnabled(IDC_SETTINGS_COMPILER_FO4_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_FO4_TOGGLE, enabled);
        break;
      }
    }
  }

  Game SettingsDialog::getGame(Tab tab) const {
    if (tab >= Tab::GameBase) {
      int count = utility::underlying(tab) - utility::underlying(Tab::GameBase) + 1;
      for (int i = static_cast<int>(Game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
        auto game = static_cast<Game>(i);
        const CompilerSettings::GameSettings& gameSettings = settings.compilerSettings.gameSettings(game);
        if (gameSettings.enabled && --count == 0) {
          return game;
        }
      }
    }
    return Game::Auto;
  }

  int SettingsDialog::getGameTab(Game game) const {
    if (!settings.compilerSettings.gameSettings(game).enabled) {
      return -1;
    }

    int count = 0;
    for (int i = utility::underlying(Game::Auto) + 1; i <= utility::underlying(game); i++) {
      const CompilerSettings::GameSettings& gameSettings = settings.compilerSettings.gameSettings(static_cast<Game>(i));
      if (gameSettings.enabled) {
        count++;
      }
    }
    return utility::underlying(Tab::GameBase) + count - 1;
  }

  void SettingsDialog::addGameTab(Game game) const {
    TCITEM item {
      .mask = TCIF_TEXT,
      .pszText = const_cast<LPWSTR>(game::gameNames[utility::underlying(game)].second.c_str()),
      .cchTextMax = static_cast<int>(_tcslen(item.pszText))
    };
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_INSERTITEM, getGameTab(game), reinterpret_cast<LPARAM>(&item));
  }

  void SettingsDialog::removeGameTab(Game game) const {
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_DELETEITEM, getGameTab(game), 0);
  }

  void SettingsDialog::toggleGame(Game game, int controlID, Group group) const {
    auto& gameSettings = settings.compilerSettings.gameSettings(game);
    if (gameSettings.enabled) {
      removeGameTab(game);
      gameSettings.enabled = false;
    } else {
      gameSettings.enabled = true;
      addGameTab(game);
    }

    updateGameEnableButtonText(controlID, gameSettings.enabled);
    enableGroup(group, gameSettings.enabled);
    updateAutoModeDefaultGame();

    // Redraw the UI since adding new tab would affect the shared window area
    redraw();
  }

  void SettingsDialog::configureGame(Game game) {
    int gameTab = getGameTab(game);
    if (gameTab != -1) {
      ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_SETCURSEL, gameTab, 0);
      switchTab(static_cast<Tab>(gameTab));
    }
  }

  void SettingsDialog::updateAutoModeDefaultGame() const {
    bool enabled = settings.compilerSettings.skyrim.enabled || settings.compilerSettings.sse.enabled || settings.compilerSettings.fo4.enabled;
    enableGroup(Group::GameAuto, enabled);

    // Keep current selection
    auto dropdown = getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN);
    int length = ::GetWindowTextLength(dropdown);
    wchar_t* currentSelection = new wchar_t[length + 1];
    auto autoCleanup = gsl::finally([&] { delete[] currentSelection; });
    ::GetWindowText(dropdown, currentSelection, length + 1);

    // Clear and re-populate default game dropdown list
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_RESETCONTENT, 0, 0);
    dropdown_options_t gameOptions;
    if (enabled) {
      for (int i = static_cast<int>(Game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
        if (settings.compilerSettings.gameSettings(static_cast<Game>(i)).enabled) {
          // Use game's display name
          gameOptions.push_back(const_cast<LPWSTR>(game::gameNames[i].second.c_str()));
        }
      }
    } else {
      // No game is enabled
      gameOptions.push_back(const_cast<LPWSTR>(game::gameNames[utility::underlying(Game::Auto)].second.c_str()));
    }
    initDropdownList(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, gameOptions);

    // Try to find the previous selection in the new list
    int newSelection = 0;
    if (length > 0) {
      auto index = ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_FINDSTRINGEXACT, 0, reinterpret_cast<LPARAM>(currentSelection));
      if (index != CB_ERR) {
        newSelection = static_cast<int>(index);
      }
    }
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_SETCURSEL, newSelection, 0);
  }

  void SettingsDialog::updateGameEnableButtonText(int controlID, bool enabled) const {
    setText(controlID, enabled ? L"Disable" : L"Enable");
  }

  void SettingsDialog::loadGameSettings(const CompilerSettings::GameSettings& gameSettings, bool isFallout4) const {
    setText(IDC_SETTINGS_TAB_GAME_INSTALL_PATH, gameSettings.installPath);
    setText(IDC_SETTINGS_TAB_GAME_COMPILER_PATH, gameSettings.compilerPath);
    setText(IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY, gameSettings.outputDirectory);
    setText(IDC_SETTINGS_TAB_GAME_FLAG_FILE, gameSettings.flagFile);
    setChecked(IDC_SETTINGS_TAB_GAME_ANONYMIZE, gameSettings.anonynmizeFlag);
    setChecked(IDC_SETTINGS_TAB_GAME_OPTIMIZE, gameSettings.optimizeFlag);
    if (isFallout4) {
      setChecked(IDC_SETTINGS_TAB_GAME_RELEASE, gameSettings.releaseFlag);
      setChecked(IDC_SETTINGS_TAB_GAME_FINAL, gameSettings.finalFlag);
    }

    // Import directories is a semi-colon delimited multi-line text. Make a copy of it
    std::wstring importDirectories = gameSettings.importDirectories;
    size_t index = 0;
    while ((index = importDirectories.find(L";", index)) != std::wstring::npos) {
      importDirectories.replace(index, 1, L"\r\n");
    }
    setText(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES, importDirectories);
  }

  void SettingsDialog::saveGameSettings(CompilerSettings::GameSettings& gameSettings, bool isFallout4) const {
    gameSettings.installPath = getText(IDC_SETTINGS_TAB_GAME_INSTALL_PATH);
    gameSettings.compilerPath = getText(IDC_SETTINGS_TAB_GAME_COMPILER_PATH);
    gameSettings.outputDirectory = getText(IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY);
    gameSettings.flagFile = getText(IDC_SETTINGS_TAB_GAME_FLAG_FILE);
    gameSettings.anonynmizeFlag = getChecked(IDC_SETTINGS_TAB_GAME_ANONYMIZE);
    gameSettings.optimizeFlag = getChecked(IDC_SETTINGS_TAB_GAME_OPTIMIZE);
    if (isFallout4) {
      gameSettings.releaseFlag = getChecked(IDC_SETTINGS_TAB_GAME_RELEASE);
      gameSettings.finalFlag = getChecked(IDC_SETTINGS_TAB_GAME_FINAL);
    }

    // Import directories is a semi-colon delimited multi-line text
    gameSettings.importDirectories = getText(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES);
    size_t index {};
    while ((index = gameSettings.importDirectories .find(L"\r\n", index)) != std::wstring::npos) {
      gameSettings.importDirectories.replace(index, 2, L";");
    }

    // Trim trailing semicolons
    if (!gameSettings.importDirectories.empty() && gameSettings.importDirectories.back() == L';') {
      gameSettings.importDirectories = gameSettings.importDirectories.substr(0, gameSettings.importDirectories.find_last_not_of(L';') + 1);
    }
  }

  bool SettingsDialog::saveSettings() {
    std::wstring indicatorIDStr = getText(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID);
    if (!utility::isNumber(indicatorIDStr)) {
      ::MessageBox(getHSelf(), L"Indicator ID needs to be a number between 9 and 31", L"Invalid setting", MB_ICONEXCLAMATION | MB_OK);
      return false;
    }
    int indicatorID {};
    std::wistringstream(indicatorIDStr) >> indicatorID;
    if (indicatorID < 9 || indicatorID > 31) {
      ::MessageBox(getHSelf(), L"Indicator ID needs to be a number between 9 and 31", L"Invalid setting", MB_OK);
      return false;
    }

    settings.errorAnnotatorSettings.indicatorID = indicatorID;

    settings.lexerSettings.enableClassNameCache = getChecked(IDC_SETTINGS_LEXER_CLASS_NAME_CACHING);
    settings.compilerSettings.allowUnmanagedSource = getChecked(IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE);
    settings.compilerSettings.autoModeOutputDirectory = getText(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT);
    settings.compilerSettings.autoModeDefaultGame = game::games[getText(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN)];

    Tab tab = static_cast<Tab>(::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_GETCURSEL, 0, 0));
    if (tab >= Tab::GameBase) {
      auto game = getGame(tab);
      saveGameSettings(settings.compilerSettings.gameSettings(game), game == Game::Fallout4);
    }

    return true;
  }

} // namespace
