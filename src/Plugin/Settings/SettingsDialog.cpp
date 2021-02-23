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

#include "..\Common\FinalAction.hpp"
#include "..\Common\Resources.hpp"
#include "..\Common\Utility.hpp"

#include "..\..\external\npp\PluginInterface.h"

#include <sstream>

#include <commctrl.h>
#include <uxtheme.h>

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
    annotationFgColorPicker.destroy();
    annotationBgColorPicker.destroy();
    indicatorFgColorPicker.destroy();
  }

  INT_PTR SettingsDialog::doDialog() {
    return ::DialogBoxParam(getHinst(), MAKEINTRESOURCE(IDD_SETTINGS_DIALOG), getHParent(), dlgProc, reinterpret_cast<LPARAM>(this));
  }

  // Protected methods
  //

  INT_PTR CALLBACK SettingsDialog::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
      case WM_INITDIALOG: {
        goToCenter();

        auto EnableDlgTheme = reinterpret_cast<ETDTProc>(::SendMessage(getHParent(), NPPM_GETENABLETHEMETEXTUREFUNC, 0, 0));
        if (EnableDlgTheme != nullptr) {
          EnableDlgTheme(getHSelf(), ETDT_ENABLETAB);
        }

        initControls();
        break;
      }

      case WM_COMMAND: {
        if (HIWORD(wParam) == BN_CLICKED) {
          // Only controls that would trigger live update are checked here
          switch (LOWORD(wParam)) {
            case IDC_SETTINGS_LEXER_FOLDMIDDLE: {
              settings.lexerSettings.enableFoldMiddle = getChecked(IDC_SETTINGS_LEXER_FOLDMIDDLE);
              return FALSE;
            }

            case IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION: {
              settings.errorAnnotatorSettings.enableAnnotation = getChecked(IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION);
              enableGroup(GROUP_ANNOTATION, settings.errorAnnotatorSettings.enableAnnotation);
              return FALSE;
            }

            case IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION: {
              settings.errorAnnotatorSettings.enableIndication = getChecked(IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION);
              enableGroup(GROUP_INDICATION, settings.errorAnnotatorSettings.enableIndication);
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
              toggleGame(game::Skyrim, IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE, GROUP_GAME_SKYRIM);
              return FALSE;
            }

            case IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE: {
              configureGame(game::Skyrim);
              return FALSE;
            }

            case IDC_SETTINGS_COMPILER_SSE_TOGGLE: {
              toggleGame(game::SkyrimSE, IDC_SETTINGS_COMPILER_SSE_TOGGLE, GROUP_GAME_SSE);
              return FALSE;
            }

            case IDC_SETTINGS_COMPILER_SSE_CONFIGURE: {
              configureGame(game::SkyrimSE);
              return FALSE;
            }

            case IDC_SETTINGS_COMPILER_FO4_TOGGLE: {
              toggleGame(game::Fallout4, IDC_SETTINGS_COMPILER_FO4_TOGGLE, GROUP_GAME_FO4);
              return FALSE;
            }

            case IDC_SETTINGS_COMPILER_FO4_CONFIGURE: {
              configureGame(game::Fallout4);
              return FALSE;
            }

            default: {
              HWND window = reinterpret_cast<HWND>(lParam);
              if (window == annotationFgColorPicker.getHSelf()) {
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
        break;
      }

      case WM_NOTIFY: {
        auto nmhdr = *(reinterpret_cast<LPNMHDR>(lParam));
        switch (nmhdr.idFrom) {
          case IDC_SETTINGS_TABS: {
            if (nmhdr.code == TCN_SELCHANGE) {
              switchTab(static_cast<int>(::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_GETCURSEL, 0, 0)));
            }
            return TRUE;
          }
        }

        break;
      }

      case WM_CLOSE: {
        if (saveSettings()) {
          annotationFgColorPicker.destroy();
          annotationBgColorPicker.destroy();
          indicatorFgColorPicker.destroy();
          if (indicatorIdTooltip) {
            ::DestroyWindow(indicatorIdTooltip);
          }
          if (autoModeTooltip) {
            ::DestroyWindow(autoModeTooltip);
          }
          ::EndDialog(getHSelf(), IDOK);
        }
        break;
      }

      default: {
        break;
      }
    }

    return FALSE;
  }

  // Private methods
  //

  HWND SettingsDialog::getControl(int controlID) const {
    return ::GetDlgItem(getHSelf(), controlID);
  }

  void SettingsDialog::initControls() {
    // Lexer settings
    //
    setChecked(IDC_SETTINGS_LEXER_FOLDMIDDLE, settings.lexerSettings.enableFoldMiddle);
    setChecked(IDC_SETTINGS_LEXER_CLASSNAMECACHING, settings.lexerSettings.enableClassNameCache);

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
    setChecked(IDC_SETTINGS_COMPILER_RADIO_AUTO + settings.compilerSettings.gameMode, true);
    setText(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, settings.compilerSettings.autoModeOutputDirectory);
    updateAutoModeDefaultGame();
    if (settings.compilerSettings.autoModeDefaultGame != game::Auto) {
      // Try to find and set current user prefs in auto mode default game dropdown list
      auto gameName = game::gameNames[settings.compilerSettings.autoModeDefaultGame].second;
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
    for (int i = 0; i < TAB_GAME_BASE; i++) {
      item.pszText = const_cast<LPWSTR>(tabNames[i]);
      item.cchTextMax = static_cast<int>(_tcslen(tabNames[i]));
      ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_INSERTITEM, i, reinterpret_cast<LPARAM>(&item));
      showTab(i, false, true);
    }

     // During initialization, hide game tab controls
    showTab(TAB_GAME_BASE, false, true);

    for (int i = static_cast<int>(game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
      auto game = static_cast<game::Game>(i);
      auto gameSettings = settings.compilerSettings.gameSettings(game);
      if (gameSettings.enabled) {
        addGameTab(game);
      }
    }

    currentTab = TAB_LEXER;
    showTab(currentTab, true);
  }

  void SettingsDialog::initDropdownList(int controlID, const dropdown_options_t& options) const {
    for (const auto& option : options) {
      ::SendDlgItemMessage(getHSelf(), controlID, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(option));
    }
  }

  void SettingsDialog::initColorPicker(ColourPicker& colorPicker, int labelControlID) const {
    colorPicker.init(getHinst(), getHSelf());
    auto label = ::GetDlgItem(getHSelf(), labelControlID);
    RECT rc {};
    ::GetWindowRect(label, &rc);
    POINT p {
      .x = rc.right,
      .y = rc.top
    };
    ::ScreenToClient(getHSelf(), &p);
    ::MoveWindow(colorPicker.getHSelf(), p.x + 8, p.y - 8, 30, 30, TRUE);
  }

  HWND SettingsDialog::createToolTip(int controlID, std::wstring toolTip, int delayTime) const {
    auto control = getControl(controlID);
    if (!control) {
      return nullptr;
    }

    // Create the tooltip. g_hInst is the global instance handle
    HWND hwndToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, getHSelf(), nullptr, getHinst(), nullptr);
    if (!hwndToolTip) {
      return nullptr;
    }

    // Associate the tooltip with the tool
    TOOLINFO toolInfo {
      .cbSize = sizeof(toolInfo),
      .uFlags = TTF_IDISHWND | TTF_SUBCLASS,
      .hwnd = getHSelf(),
      .uId = reinterpret_cast<UINT_PTR>(control),
      .lpszText = const_cast<LPWSTR>(toolTip.c_str())
    };
    if (!::SendMessage(hwndToolTip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&toolInfo))) {
      DestroyWindow(hwndToolTip);
      return nullptr;
    }

    ::SendMessage(hwndToolTip, TTM_ACTIVATE, TRUE, 0);
    ::SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 200);
    
    // Convert delay time to milliseconds
    delayTime *= 1000;
    ::SendMessage(hwndToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM((delayTime), (0)));

    return hwndToolTip;
  }

  void SettingsDialog::switchTab(int newTab) {
    if (newTab != currentTab) {
      showTab(currentTab, false);
      showTab(newTab, true);
      currentTab = newTab;
    }
  }

  void SettingsDialog::showTab(int tab, bool show, bool intializing) const {
    int showCommand = show ? SW_SHOW : SW_HIDE;
    switch (tab) {
      case TAB_LEXER: {
        ::ShowWindow(getControl(IDC_SETTINGS_LEXER_SCRIPT_GROUP), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_LEXER_FOLDMIDDLE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_LEXER_CLASSNAMECACHING), showCommand);
        break;
      }

      case TAB_ERROR_ANNOTATOR: {
        if (show) {
          enableGroup(GROUP_ANNOTATION, settings.errorAnnotatorSettings.enableAnnotation);
          enableGroup(GROUP_INDICATION, settings.errorAnnotatorSettings.enableIndication);
        }

        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_GROUP), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL), showCommand);
        annotationFgColorPicker.display(show);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL), showCommand);
        annotationBgColorPicker.display(show);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD), showCommand);

        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_GROUP), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_LABEL), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL), showCommand);
        indicatorFgColorPicker.display(show);
        break;
      }

      case TAB_COMPILER: {
        if (show) {
          enableGroup(GROUP_GAME_SKYRIM, settings.compilerSettings.skyrim.enabled);
          enableGroup(GROUP_GAME_SSE, settings.compilerSettings.sse.enabled);
          enableGroup(GROUP_GAME_FO4, settings.compilerSettings.fo4.enabled);
          updateAutoModeDefaultGame();
        }

        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_GAMES_GROUP), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_AUTO), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_SKYRIM), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_SSE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_FO4), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_LABEL), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT_LABEL), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_SSE_TOGGLE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_SSE_CONFIGURE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_FO4_TOGGLE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_FO4_CONFIGURE), showCommand);
        ::ShowWindow(getControl(IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE), showCommand);
        break;
      }

      default: {
        if (tab >= TAB_GAME_BASE) {
          // Update displayed values when showing, and save current values when hiding
          auto game = getGame(tab);
          bool isFallout4 = (game == game::Fallout4);

          // Only handle current game tab settings after UI initialization
          if (!intializing) {
            if (show) {
              loadGameSettings(settings.compilerSettings.gameSettings(game), isFallout4);
            } else {
              saveGameSettings(settings.compilerSettings.gameSettings(game), isFallout4);
            }
          }

          int showExtraFlag = (show && isFallout4) ? SW_SHOW : SW_HIDE;
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_INSTALL_PATH_LABEL), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_INSTALL_PATH), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_COMPILER_PATH_LABEL), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_COMPILER_PATH), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES_LABEL1), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES_LABEL2), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY_LABEL), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_FLAG_FILE_LABEL), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_FLAG_FILE), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_ANONYMIZE), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_OPTIMIZE), showCommand);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_RELEASE), showExtraFlag);
          ::ShowWindow(getControl(IDC_SETTINGS_TAB_GAME_FINAL), showExtraFlag);
        }
        break;
      }
    }
  }

  void SettingsDialog::enableGroup(int group, bool enabled) const {
    switch (group) {
      case GROUP_ANNOTATION: {
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL), enabled);
        ::EnableWindow(annotationFgColorPicker.getHSelf(), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL), enabled);
        ::EnableWindow(annotationBgColorPicker.getHSelf(), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD), enabled);
        break;
      }

      case GROUP_INDICATION: {
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_ID), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_LABEL), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL), enabled);
        ::EnableWindow(indicatorFgColorPicker.getHSelf(), enabled);
        break;
      }

      case GROUP_GAME_AUTO: {
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_AUTO), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_LABEL), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT_LABEL), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT), enabled);
        break;
      }

      case GROUP_GAME_SKYRIM: {
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_SKYRIM), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE), enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE, enabled);
        break;
      }

      case GROUP_GAME_SSE: {
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_SSE), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_SSE_CONFIGURE), enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SSE_TOGGLE, enabled);
        break;
      }

      case GROUP_GAME_FO4: {
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_RADIO_FO4), enabled);
        ::EnableWindow(getControl(IDC_SETTINGS_COMPILER_FO4_CONFIGURE), enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_FO4_TOGGLE, enabled);
        break;
      }
    }
  }

  void SettingsDialog::setChecked(int controlID, bool checked) const {
    ::SendDlgItemMessage(getHSelf(), controlID, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
  }

  bool SettingsDialog::getChecked(int controlID) const {
    return (::SendDlgItemMessage(getHSelf(), controlID, BM_GETCHECK, 0, 0) == BST_CHECKED);
  }

  void SettingsDialog::setText(int controlID, std::wstring text) const {
    ::SetWindowText(getControl(controlID), text.c_str());
  }

  std::wstring SettingsDialog::getText(int controlID) const {
    auto control = getControl(controlID);
    std::wstring content(::GetWindowTextLength(control) + 1, L' ');
    ::GetWindowText(control, &content[0], static_cast<int>(content.size()));
    content.pop_back(); // Remove trailing NULL
    return content;
  }

  game::Game SettingsDialog::getGame(int tab) const {
    if (tab >= TAB_GAME_BASE) {
      int count = tab - TAB_GAME_BASE + 1;
      for (int i = static_cast<int>(game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
        auto game = static_cast<game::Game>(i);
        auto gameSettings = settings.compilerSettings.gameSettings(game);
        if (gameSettings.enabled && --count == 0) {
          return game;
        }
      }
    }
    return game::Auto;
  }

  int SettingsDialog::getGameTab(game::Game game) const {
    if (!settings.compilerSettings.gameSettings(game).enabled) {
      return -1;
    }

    int count = 0;
    for (int i = static_cast<int>(game::Auto) + 1; i <= game; i++) {
      auto gameSettings = settings.compilerSettings.gameSettings((game::Game)i);
      if (gameSettings.enabled) {
        count++;
      }
    }
    return TAB_GAME_BASE + count - 1;
  }

  void SettingsDialog::addGameTab(game::Game game) const {
    TCITEM item {
      .mask = TCIF_TEXT,
      .pszText = const_cast<LPWSTR>(game::gameNames[game].second.c_str()),
      .cchTextMax = static_cast<int>(_tcslen(item.pszText))
    };
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_INSERTITEM, getGameTab(game), reinterpret_cast<LPARAM>(&item));
  }

  void SettingsDialog::removeGameTab(game::Game game) const {
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_DELETEITEM, getGameTab(game), 0);
  }

  void SettingsDialog::toggleGame(game::Game game, int controlID, int group) const {
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

  void SettingsDialog::configureGame(game::Game game) {
    int gameTab = getGameTab(game);
    if (gameTab != -1) {
      ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_SETCURSEL, gameTab, 0);
      switchTab(gameTab);
    }
  }

  void SettingsDialog::updateAutoModeDefaultGame() const {
    bool enabled = settings.compilerSettings.skyrim.enabled || settings.compilerSettings.sse.enabled || settings.compilerSettings.fo4.enabled;
    enableGroup(GROUP_GAME_AUTO, enabled);

    // Keep current selection
    auto dropdown = getControl(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN);
    int length = ::GetWindowTextLength(dropdown);
    wchar_t* currentSelection = new wchar_t[length + 1];
    auto autoCleanup = utility::finally([&] { delete[] currentSelection; });
    ::GetWindowText(dropdown, currentSelection, length + 1);

    // Clear and re-populate default game dropdown list
    ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_RESETCONTENT, 0, 0);
    dropdown_options_t gameOptions;
    if (enabled) {
      for (int i = static_cast<int>(game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
        auto game = static_cast<game::Game>(i);
        if (settings.compilerSettings.gameSettings((game::Game)i).enabled) {
          // Use game's display name
          gameOptions.push_back(const_cast<LPWSTR>(game::gameNames[i].second.c_str()));
        }
      }
    } else {
      // No game is enabled
      gameOptions.push_back(const_cast<LPWSTR>(game::gameNames[game::Auto].second.c_str()));
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

    settings.lexerSettings.enableClassNameCache = getChecked(IDC_SETTINGS_LEXER_CLASSNAMECACHING);
    settings.compilerSettings.allowUnmanagedSource = getChecked(IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE);
    settings.compilerSettings.autoModeOutputDirectory = getText(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT);
    settings.compilerSettings.autoModeDefaultGame = game::games[getText(IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN)];

    int tab = static_cast<int>(::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_GETCURSEL, 0, 0));
    if (tab >= TAB_GAME_BASE) {
      auto game = getGame(tab);
      saveGameSettings(settings.compilerSettings.gameSettings(game), game == game::Fallout4);
    }

    return true;
  }

} // namespace
