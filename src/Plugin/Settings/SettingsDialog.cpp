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
#include "..\Common\NotepadPlusPlus.hpp"
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
      L"Keyword Matcher",
      L"Error Annotator",
      L"Compiler",
      L"Skyrim",
      L"Skyrim SE/AE",
      L"Fallout 4"
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

    struct TabItem {
        HWND tabDlgHwnd = 0;
        bool isVisible = true;
        int game = NULL;
    };

    struct TabData {
        std::vector<TabItem> tabItems;
        TabItem activeTabItem;
        HWND tabControlId;
    } tabData;

    std::vector<int> tabDialogIds{
        IDD_SETTINGS_LEXER_TAB_DLG,
        IDD_SETTINGS_TAB_KEYWORD_MATCHER_DLG,
        IDD_SETTINGS_TAB_ERROR_ANNOTATOR_DLG,
        IDD_SETTINGS_TAB_COMPILER_DLG
        //IDD_SETTINGS_TAB_GAME_DLG
    };
  }

  SettingsDialog::~SettingsDialog() {
    classLinkFgColorPicker.destroy();
    classLinkBgColorPicker.destroy();
    matchedIndicatorFgColorPicker.destroy();
    unmatchedIndicatorFgColorPicker.destroy();
    annotationFgColorPicker.destroy();
    annotationBgColorPicker.destroy();
    errorIndicatorFgColorPicker.destroy();

    stylerConfigLink.destroy();

    if (foldMiddleTooltip) {
      ::DestroyWindow(foldMiddleTooltip);
      foldMiddleTooltip = nullptr;
    }
    if (classNameCachingTooltip) {
      ::DestroyWindow(classNameCachingTooltip);
      classNameCachingTooltip = nullptr;
    }
    if (classLinkTooltip) {
      ::DestroyWindow(classLinkTooltip);
      classLinkTooltip = nullptr;
    }
    if (matcherTooltip) {
      ::DestroyWindow(matcherTooltip);
      matcherTooltip = nullptr;
    }
    if (matcherIndicatorIdTooltip) {
      ::DestroyWindow(matcherIndicatorIdTooltip);
      matcherIndicatorIdTooltip = nullptr;
    }
    if (annotationTooltip) {
      ::DestroyWindow(annotationTooltip);
      annotationTooltip = nullptr;
    }
    if (indicationTooltip) {
      ::DestroyWindow(indicationTooltip);
      indicationTooltip = nullptr;
    }
    if (errorIndicatorIdTooltip) {
      ::DestroyWindow(errorIndicatorIdTooltip);
      errorIndicatorIdTooltip = nullptr;
    }
    if (autoModeTooltip) {
      ::DestroyWindow(autoModeTooltip);
      autoModeTooltip = nullptr;
    }
    for (int i = 0; i < tabData.tabItems.size(); i++) {
        ::DestroyWindow(tabData.tabItems[i].tabDlgHwnd);
    }
  }

  void SettingsDialog::doDialog(callback_t callback) {
    settingsUpdatedFunc = callback;
    DialogBase::doDialog();
  }

  // Protected methods
  //
  void SettingsDialog::initLexerControls() {
      setChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_FOLD_MIDDLE, settings.lexerSettings.enableFoldMiddle);
      foldMiddleTooltip = createToolTip(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_FOLD_MIDDLE, IDS_SETTINGS_LEXER_FOLD_MIDDLE_TOOLTIP);

      setChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_NAME_CACHING, settings.lexerSettings.enableClassNameCache);
      classNameCachingTooltip = createToolTip(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_NAME_CACHING, IDS_SETTINGS_LEXER_CLASS_NAME_CACHING_TOOLTIP);

      setChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK, settings.lexerSettings.enableClassLink);
      classLinkTooltip = createToolTip(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK, IDS_SETTINGS_LEXER_CLASS_LINK_TOOLTIP);
      setChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, settings.lexerSettings.classLinkUnderline);

      initColorPicker(classLinkFgColorPicker, tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL);
      classLinkFgColorPicker.setColour(settings.lexerSettings.classLinkForegroundColor);
      initColorPicker(classLinkBgColorPicker, tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL);
      classLinkBgColorPicker.setColour(settings.lexerSettings.classLinkBackgroundColor);

      setChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_SHIFT, (settings.lexerSettings.classLinkClickModifier & SCMOD_SHIFT) != 0);
      setChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_CTRL, (settings.lexerSettings.classLinkClickModifier & SCMOD_CTRL) != 0);
      setChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_ALT, (settings.lexerSettings.classLinkClickModifier & SCMOD_ALT) != 0);

      stylerConfigLink.init(getHinst(), tabData.tabItems[0].tabDlgHwnd);
      stylerConfigLink.create(GetDlgItem(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_STYLER_CONFIG_LINK), IDC_SETTINGS_LEXER_STYLER_CONFIG_LINK);

  }

  void SettingsDialog::initMatcherControls() {
     setChecked(tabData.tabItems[1].tabDlgHwnd,IDC_SETTINGS_MATCHER, settings.keywordMatcherSettings.enableKeywordMatching);
     matcherTooltip = createToolTip(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER, IDS_SETTINGS_MATCHER_TOOLTIP);

     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_FUNCTION);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_STATE, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_STATE);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_EVENT, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_EVENT);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_PROPERTY);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_GROUP, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_GROUP);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_STRUCT, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_STRUCT);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_IF, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_IF);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_ELSE, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_ELSE);
     setChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_WHILE, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_WHILE);

     matcherIndicatorIdTooltip = createToolTip(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_INDICATOR_ID_LABEL, IDS_SETTINGS_MATCHER_INDICATOR_ID_TOOLTIP);
     setText(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_INDICATOR_ID, std::to_wstring(settings.keywordMatcherSettings.indicatorID));

     initDropdownList(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN, indicatorStyles, settings.keywordMatcherSettings.matchedIndicatorStyle);
     initColorPicker(matchedIndicatorFgColorPicker, tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_MATCHED_FGCOLOR_LABEL);
     matchedIndicatorFgColorPicker.setColour(settings.keywordMatcherSettings.matchedIndicatorForegroundColor);

     initDropdownList(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN, indicatorStyles, settings.keywordMatcherSettings.unmatchedIndicatorStyle);
     initColorPicker(unmatchedIndicatorFgColorPicker, tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_UNMATCHED_FGCOLOR_LABEL);
     unmatchedIndicatorFgColorPicker.setColour(settings.keywordMatcherSettings.unmatchedIndicatorForegroundColor);
  }

  void SettingsDialog::initAnnotatorControls() {
      setChecked(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION, settings.errorAnnotatorSettings.enableAnnotation);
      annotationTooltip = createToolTip(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION, IDS_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION_TOOLTIP);
      initColorPicker(annotationFgColorPicker, tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL);
      annotationFgColorPicker.setColour(settings.errorAnnotatorSettings.annotationForegroundColor);
      initColorPicker(annotationBgColorPicker, tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL);
      annotationBgColorPicker.setColour(settings.errorAnnotatorSettings.annotationBackgroundColor);
      setChecked(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC, settings.errorAnnotatorSettings.isAnnotationItalic);
      setChecked(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD, settings.errorAnnotatorSettings.isAnnotationBold);

      setChecked(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION, settings.errorAnnotatorSettings.enableIndication);
      indicationTooltip = createToolTip(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION, IDS_SETTINGS_ANNOTATOR_ENABLE_INDICATION_TOOLTIP);
      errorIndicatorIdTooltip = createToolTip(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL, IDS_SETTINGS_ANNOTATOR_INDICATOR_ID_TOOLTIP);
      setText(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID, std::to_wstring(settings.errorAnnotatorSettings.indicatorID));
      initDropdownList(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, indicatorStyles, settings.errorAnnotatorSettings.indicatorStyle);
      initColorPicker(errorIndicatorFgColorPicker, tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL);
      errorIndicatorFgColorPicker.setColour(settings.errorAnnotatorSettings.indicatorForegroundColor);
  }

  void SettingsDialog::initCompilerControls() {
      setChecked(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE, settings.compilerSettings.allowUnmanagedSource);
      setChecked(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_RADIO_AUTO + utility::underlying(settings.compilerSettings.gameMode), true);
      setText(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, settings.compilerSettings.autoModeOutputDirectory);
      updateAutoModeDefaultGame();
      if (settings.compilerSettings.autoModeDefaultGame != Game::Auto) {
        // Try to find and set current user prefs in auto mode default game dropdown list
        auto gameName = game::gameNames[utility::underlying(settings.compilerSettings.autoModeDefaultGame)].second;
        auto index = ::SendDlgItemMessage(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_FINDSTRINGEXACT, 0, reinterpret_cast<WPARAM>(gameName.c_str()));
        if (index != CB_ERR) {
          ::SendDlgItemMessage(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_SETCURSEL, index, 0);
        }
      }

      autoModeTooltip = createToolTip(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_RADIO_AUTO, IDS_SETTINGS_COMPILER_RADIO_AUTO_TOOLTIP);
  }

  void SettingsDialog::initControls() {
      
      for (int i = 0; i < tabDialogIds.size(); i++)
      {
          TabItem _tabItem;

          _tabItem.tabDlgHwnd = CreateDialogParam(getHinst(), MAKEINTRESOURCE(tabDialogIds[i]), _hSelf, tabDlgProc, reinterpret_cast<LPARAM>(this));
          if (!_tabItem.tabDlgHwnd)
          {
              generic_string errMsg = TEXT("CreateDialogParam() return NULL.\rGetLastError(): ");
              errMsg += GetLastErrorAsString();
              ::MessageBox(NULL, errMsg.c_str(), TEXT("Create tab dialog failed"), MB_OK);
              return;
          }
          tabData.tabItems.push_back(_tabItem);
      }
      for (int i = utility::underlying(Game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) 
      {
          TabItem _tabItem;
          _tabItem.game = i;

          auto game = static_cast<Game>(i);
          const CompilerSettings::GameSettings& gameSettings = settings.compilerSettings.gameSettings(game);
          if (!gameSettings.enabled)
              _tabItem.isVisible = false;

          _tabItem.tabDlgHwnd = CreateDialogParam(getHinst(), MAKEINTRESOURCE(IDD_SETTINGS_TAB_GAME_DLG), _hSelf, tabDlgProc, reinterpret_cast<LPARAM>(this));
          if (!_tabItem.tabDlgHwnd)
          {
              generic_string errMsg = TEXT("CreateDialogParam() return NULL.\rGetLastError(): ");
              errMsg += GetLastErrorAsString();
              ::MessageBox(NULL, errMsg.c_str(), TEXT("Create game tab dialog failed"), MB_OK);
              return;
          }
          tabData.tabItems.push_back(_tabItem);
      }

      tabData.tabControlId = GetDlgItem(_hSelf, IDC_SETTINGS_TABS);
      // Set tab texts
      if (tabData.tabControlId) {
          TCITEM item{
            .mask = TCIF_TEXT
          };
          for (int i = 0; i < tabData.tabItems.size(); i++) {
              if (tabData.tabItems[i].isVisible) {
                  item.pszText = const_cast<LPWSTR>(tabNames[i]);
                  item.cchTextMax = static_cast<int>(_tcslen(tabNames[i]));
                  ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_INSERTITEM, i, reinterpret_cast<LPARAM>(&item));
              }
          }
          

          RECT rc;//find tab control's rectangle
          GetWindowRect(tabData.tabControlId, &rc);
          POINT offset = { 0 };
          ScreenToClient(_hSelf, &offset);
          OffsetRect(&rc, offset.x, offset.y); //convert to client coordinates
          rc.top += 25;

          for (int i = 0; i < tabData.tabItems.size(); i++) {
              SetWindowPos(tabData.tabItems[i].tabDlgHwnd, 0, (rc.left + 1), rc.top, rc.right - rc.left - 2, rc.bottom - rc.top - 2, SWP_HIDEWINDOW);
          }
          initLexerControls();
          initMatcherControls();
          initAnnotatorControls();
          initCompilerControls();

          onSelChange();
      }
  }

  void SettingsDialog::onSelChange()
  {
      int sel = TabCtrl_GetCurSel(tabData.tabControlId);
      int count = 0;
      for (int i = 0; i < tabData.tabItems.size(); i++)
      {
          if (tabData.tabItems[i].isVisible) {
              if (sel == i - count) {
                  ShowWindow(tabData.tabItems[i].tabDlgHwnd, SW_SHOW);
                  tabData.activeTabItem = tabData.tabItems[i];
              }
              else
                  ShowWindow(tabData.tabItems[i].tabDlgHwnd, SW_HIDE);
          }
          else count++;
      }
      switch (sel)
      {
      case utility::underlying(Tab::Lexer): {
          enableGroup(Group::ClassLink, settings.lexerSettings.enableClassLink);
          break;
      }
      case utility::underlying(Tab::KeywordMatcher): {
          enableGroup(Group::Matcher, settings.keywordMatcherSettings.enableKeywordMatching);
          break;
      }
      case utility::underlying(Tab::ErrorAnnotator): {
          enableGroup(Group::Annotation, settings.errorAnnotatorSettings.enableAnnotation);
          enableGroup(Group::Indication, settings.errorAnnotatorSettings.enableIndication);
      }
      case utility::underlying(Tab::Compiler): {
          enableGroup(Group::GameSkyrim, settings.compilerSettings.skyrim.enabled);
          enableGroup(Group::GameSSE, settings.compilerSettings.sse.enabled);
          enableGroup(Group::GameFO4, settings.compilerSettings.fo4.enabled);
      }
      default:
          break;
      }
      
  }
   /* for (int i = utility::underlying(Game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
      auto game = static_cast<Game>(i);
      const CompilerSettings::GameSettings& gameSettings = settings.compilerSettings.gameSettings(game);
      if (gameSettings.enabled) {
        addGameTab(game);
      }
    }

    if (currentTab != Tab::Lexer) {
      ::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_SETCURSEL, utility::underlying(currentTab), 0);
    }

    if (currentTab >= Tab::GameBase) {
      showTab(currentTab, true, true);
    } else {
      showTab(Tab::GameBase, false, true);
    }*/
  
  INT_PTR CALLBACK SettingsDialog::tabDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
      switch (msg) {
        case WM_INITDIALOG:
        {
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
           return TRUE;
        }
        default:
        {
            SettingsDialog* pSettingsDlg = reinterpret_cast<SettingsDialog*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!pSettingsDlg)
                return FALSE;
            return pSettingsDlg->run_dlgProc(msg, wp, lp);
        }
    }
  }

  INT_PTR SettingsDialog::handleCommandMessage(WPARAM wParam, LPARAM lParam) {
    if (wParam == IDC_SETTINGS_LEXER_STYLER_CONFIG_LINK) {
      // Special link for Notepad++'s Style Configurator dialog. Send a message to activate that menu
      ::SendMessage(getHParent(), NPPM_MENUCOMMAND, 0, IDM_LANGSTYLE_CONFIG_DLG);
    } else if (HIWORD(wParam) == BN_CLICKED) {
      // Only controls that would trigger live update are checked here
      switch (LOWORD(wParam)) {
        case IDC_SETTINGS_LEXER_FOLD_MIDDLE: {
          settings.lexerSettings.enableFoldMiddle = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_LEXER_FOLD_MIDDLE);
          return FALSE;
        }

        case IDC_SETTINGS_LEXER_CLASS_LINK: {
          settings.lexerSettings.enableClassLink = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_LEXER_CLASS_LINK);
          enableGroup(Group::ClassLink, settings.lexerSettings.enableClassLink);
          return FALSE;
        }

        case IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE: {
          settings.lexerSettings.classLinkUnderline = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE);
          return FALSE;
        }

        case IDC_SETTINGS_MATCHER: {
          settings.keywordMatcherSettings.enableKeywordMatching = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_MATCHER);
          enableGroup(Group::Matcher, settings.keywordMatcherSettings.enableKeywordMatching);
          return FALSE;
        }

        case IDC_SETTINGS_MATCHER_KEYWORD_IF: {
          // Enable/disable Else/ElseIf support based on If/EndIf support
          bool allowIf = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_MATCHER_KEYWORD_IF);
          setControlEnabled(::GetParent((HWND)lParam), IDC_SETTINGS_MATCHER_KEYWORD_ELSE, allowIf);
          if (!allowIf) {
            setChecked(::GetParent((HWND)lParam), IDC_SETTINGS_MATCHER_KEYWORD_ELSE, false);
          }

          // Let it fall through to the next block
        }

        case IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION:
        case IDC_SETTINGS_MATCHER_KEYWORD_STATE:
        case IDC_SETTINGS_MATCHER_KEYWORD_EVENT:
        case IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY:
        case IDC_SETTINGS_MATCHER_KEYWORD_GROUP:
        case IDC_SETTINGS_MATCHER_KEYWORD_STRUCT:
        case IDC_SETTINGS_MATCHER_KEYWORD_ELSE:
        case IDC_SETTINGS_MATCHER_KEYWORD_WHILE: {
          updateEnabledKeywords(::GetParent((HWND)lParam));
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION: {
          settings.errorAnnotatorSettings.enableAnnotation = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION);
          enableGroup(Group::Annotation, settings.errorAnnotatorSettings.enableAnnotation);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION: {
          settings.errorAnnotatorSettings.enableIndication = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION);
          enableGroup(Group::Indication, settings.errorAnnotatorSettings.enableIndication);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC: {
          settings.errorAnnotatorSettings.isAnnotationItalic = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD: {
          settings.errorAnnotatorSettings.isAnnotationBold = getChecked(::GetParent((HWND)lParam), IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD);
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
          } else if (window == matchedIndicatorFgColorPicker.getHSelf()) {
            settings.keywordMatcherSettings.matchedIndicatorForegroundColor = matchedIndicatorFgColorPicker.getColour();
          } else if (window == unmatchedIndicatorFgColorPicker.getHSelf()) {
            settings.keywordMatcherSettings.unmatchedIndicatorForegroundColor = unmatchedIndicatorFgColorPicker.getColour();
          } else if (window == annotationFgColorPicker.getHSelf()) {
            settings.errorAnnotatorSettings.annotationForegroundColor = annotationFgColorPicker.getColour();
          } else if (window == annotationBgColorPicker.getHSelf()) {
            settings.errorAnnotatorSettings.annotationBackgroundColor = annotationBgColorPicker.getColour();
          } else if (window == errorIndicatorFgColorPicker.getHSelf()) {
            settings.errorAnnotatorSettings.indicatorForegroundColor = errorIndicatorFgColorPicker.getColour();
          }
          return FALSE;
        }
      }
    } else if (HIWORD(wParam) == CBN_SELCHANGE) {
      switch (LOWORD(wParam)) {
        case IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN: {
          int selectedIndex = getDropdownSelectedIndex(::GetParent((HWND)lParam), IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN);
          if (selectedIndex != CB_ERR) {
            settings.keywordMatcherSettings.matchedIndicatorStyle = selectedIndex;
          }
          return FALSE;
        }

        case IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN: {
          int selectedIndex = getDropdownSelectedIndex(::GetParent((HWND)lParam), IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN);
          if (selectedIndex != CB_ERR) {
            settings.keywordMatcherSettings.unmatchedIndicatorStyle = selectedIndex;
          }
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN: {
          int selectedIndex = getDropdownSelectedIndex(::GetParent((HWND)lParam), IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN);
          if (selectedIndex != CB_ERR) {
            settings.errorAnnotatorSettings.indicatorStyle = selectedIndex;
          }
          return FALSE;
        }

        default: {
          break;
        }
      }
    }

    return FALSE;
  }

  INT_PTR SettingsDialog::handleNotifyMessage(WPARAM wParam, LPARAM lParam) {
    /*auto nmhdr = *(reinterpret_cast<LPNMHDR>(lParam));
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

    return FALSE;*/
    switch (((LPNMHDR)lParam)->code) {
    case TCN_SELCHANGE:
        onSelChange();
        break;
    }

    return FALSE;
  }

  INT_PTR SettingsDialog::handleCloseMessage(WPARAM wParam, LPARAM lParam) {
    if (saveSettings()) {
      hide();
    }

    return FALSE;
  }

  // Private methods
  //

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
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL, show);
        classLinkFgColorPicker.display(show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL, show);
        classLinkBgColorPicker.display(show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_LABEL, show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_SHIFT, show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_CTRL, show);
        setControlVisibility(IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_ALT, show);
        setControlVisibility(IDC_SETTINGS_LEXER_STYLER_CONFIG_TEXT1, show);
        setControlVisibility(IDC_SETTINGS_LEXER_STYLER_CONFIG_TEXT2, show);
        setControlVisibility(IDC_SETTINGS_LEXER_STYLER_CONFIG_LINK, show);
        break;
      }

      case Tab::KeywordMatcher: {
        if (show) {
          enableGroup(Group::Matcher, settings.keywordMatcherSettings.enableKeywordMatching);
        }
        setControlVisibility(IDC_SETTINGS_MATCHER, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORDS_LABEL, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_STATE, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_EVENT, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_GROUP, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_STRUCT, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_IF, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_ELSE, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_KEYWORD_WHILE, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_INDICATOR_ID_LABEL, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_INDICATOR_ID, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_MATCHED_STYLE_LABEL, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_MATCHED_FGCOLOR_LABEL, show);
        matchedIndicatorFgColorPicker.display(show);
        setControlVisibility(IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_LABEL, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN, show);
        setControlVisibility(IDC_SETTINGS_MATCHER_UNMATCHED_FGCOLOR_LABEL, show);
        unmatchedIndicatorFgColorPicker.display(show);
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
        errorIndicatorFgColorPicker.display(show);
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
            show ? loadGameSettings(settings.compilerSettings.gameSettings(game), isFallout4) : saveGameSettings(settings.compilerSettings.gameSettings(game), isFallout4);
          }

          setControlVisibility(IDC_SETTINGS_TAB_GAME_INSTALL_PATH_LABEL, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_INSTALL_PATH, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_COMPILER_PATH_LABEL, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_COMPILER_PATH, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES_LABEL1, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES_LABEL2, show);
          setControlVisibility(IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES_LABEL3, show);
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
        setControlEnabled(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, enabled);
        setControlEnabled(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL, enabled);
        ::EnableWindow(classLinkFgColorPicker.getHSelf(), enabled);
        setControlEnabled(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL, enabled);
        ::EnableWindow(classLinkBgColorPicker.getHSelf(), enabled);
        setControlEnabled(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_LABEL, enabled);
        setControlEnabled(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_SHIFT, enabled);
        setControlEnabled(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_CTRL, enabled);
        setControlEnabled(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_ALT, enabled);
        break;
      }

      case Group::Matcher: {
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORDS_LABEL, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_STATE, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_EVENT, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_GROUP, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_STRUCT, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_IF, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_ELSE, enabled && getChecked(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_IF));
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_KEYWORD_WHILE, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_INDICATOR_ID_LABEL, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_INDICATOR_ID, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_MATCHED_STYLE_LABEL, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_MATCHED_FGCOLOR_LABEL, enabled);
        ::EnableWindow(matchedIndicatorFgColorPicker.getHSelf(), enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_LABEL, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN, enabled);
        setControlEnabled(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_UNMATCHED_FGCOLOR_LABEL, enabled);
        ::EnableWindow(unmatchedIndicatorFgColorPicker.getHSelf(), enabled);
        break;
      }

      case Group::Annotation: {
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL, enabled);
        ::EnableWindow(annotationFgColorPicker.getHSelf(), enabled);
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL, enabled);
        ::EnableWindow(annotationBgColorPicker.getHSelf(), enabled);
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC, enabled);
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD, enabled);
        break;
      }

      case Group::Indication: {
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL, enabled);
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID, enabled);
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_LABEL, enabled);
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, enabled);
        setControlEnabled(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL, enabled);
        ::EnableWindow(errorIndicatorFgColorPicker.getHSelf(), enabled);
        break;
      }

      case Group::GameAuto: {
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_RADIO_AUTO, enabled);
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_LABEL, enabled);
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, enabled);
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT_LABEL, enabled);
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, enabled);
        break;
      }

      case Group::GameSkyrim: {
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_RADIO_SKYRIM, enabled);
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE, enabled);
        break;
      }

      case Group::GameSSE: {
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_RADIO_SSE, enabled);
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_SSE_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SSE_TOGGLE, enabled);
        break;
      }

      case Group::GameFO4: {
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_RADIO_FO4, enabled);
        setControlEnabled(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_FO4_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_FO4_TOGGLE, enabled);
        break;
      }

      default: {
        break;
      }
    }
  }

  void SettingsDialog::updateEnabledKeywords(HWND hwnd) const {
    settings.keywordMatcherSettings.enabledKeywords =
      (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION) ? KEYWORD_FUNCTION : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_STATE) ? KEYWORD_STATE : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_EVENT) ? KEYWORD_EVENT : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY) ? KEYWORD_PROPERTY : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_GROUP) ? KEYWORD_GROUP : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_STRUCT) ? KEYWORD_STRUCT : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_IF) ? KEYWORD_IF : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_ELSE) ? KEYWORD_ELSE : KEYWORD_NONE)
      | (getChecked(hwnd, IDC_SETTINGS_MATCHER_KEYWORD_WHILE) ? KEYWORD_WHILE : KEYWORD_NONE);
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
    auto dropdown = getControl(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN);
    int length = ::GetWindowTextLength(dropdown);
    wchar_t* currentSelection = new wchar_t[static_cast<size_t>(length) + 1];
    auto autoCleanup = gsl::finally([&] { delete[] currentSelection; });
    ::GetWindowText(dropdown, currentSelection, length + 1);

    // Clear and re-populate default game dropdown list
    ::SendDlgItemMessage(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_RESETCONTENT, 0, 0);
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
    initDropdownList(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, gameOptions);

    // Try to find the previous selection in the new list
    int newSelection = 0;
    if (length > 0) {
      auto index = ::SendDlgItemMessage(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_FINDSTRINGEXACT, 0, reinterpret_cast<LPARAM>(currentSelection));
      if (index != CB_ERR) {
        newSelection = static_cast<int>(index);
      }
    }
    ::SendDlgItemMessage(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, CB_SETCURSEL, newSelection, 0);
  }

  void SettingsDialog::updateGameEnableButtonText(int controlID, bool enabled) const {
    setText(tabData.tabItems[3].tabDlgHwnd, controlID, enabled ? L"Disable" : L"Enable");
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
    std::wstring errorIndicatorIDStr = getText(tabData.tabItems[2].tabDlgHwnd, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID);
    if (!utility::isNumber(errorIndicatorIDStr)) {
      ::MessageBox(getHSelf(), L"Indicator ID needs to be a number between 9 and 20", L"Invalid setting", MB_ICONEXCLAMATION | MB_OK);
      return false;
    }
    int errorIndicatorID {};
    std::wistringstream(errorIndicatorIDStr) >> errorIndicatorID;
    if (errorIndicatorID < 9 || errorIndicatorID > 20) {
      ::MessageBox(getHSelf(), L"Indicator ID needs to be a number between 9 and 20", L"Invalid setting", MB_ICONEXCLAMATION | MB_OK);
      return false;
    }

    std::wstring matcherIndicatorIDStr = getText(tabData.tabItems[1].tabDlgHwnd, IDC_SETTINGS_MATCHER_INDICATOR_ID);
    if (!utility::isNumber(matcherIndicatorIDStr)) {
      ::MessageBox(getHSelf(), L"Indicator ID needs to be a number between 9 and 20", L"Invalid setting", MB_ICONEXCLAMATION | MB_OK);
      return false;
    }
    int matcherIndicatorID {};
    std::wistringstream(matcherIndicatorIDStr) >> matcherIndicatorID;
    if (matcherIndicatorID < 9 || matcherIndicatorID > 20) {
      ::MessageBox(getHSelf(), L"Indicator ID needs to be a number between 9 and 20", L"Invalid setting", MB_ICONEXCLAMATION | MB_OK);
      return false;
    }

    settings.errorAnnotatorSettings.indicatorID = errorIndicatorID;
    settings.keywordMatcherSettings.indicatorID = matcherIndicatorID;

    settings.lexerSettings.enableClassNameCache = getChecked(tabData.tabItems[0].tabDlgHwnd,IDC_SETTINGS_LEXER_CLASS_NAME_CACHING);
    settings.lexerSettings.classLinkClickModifier = SCMOD_NORM
      | (getChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_SHIFT) ? SCMOD_SHIFT : SCMOD_NORM)
      | (getChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_CTRL) ? SCMOD_CTRL : SCMOD_NORM)
      | (getChecked(tabData.tabItems[0].tabDlgHwnd, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_ALT) ? SCMOD_ALT : SCMOD_NORM);

    settings.compilerSettings.allowUnmanagedSource = getChecked(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE);
    settings.compilerSettings.autoModeOutputDirectory = getText(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT);
    settings.compilerSettings.autoModeDefaultGame = game::games[getText(tabData.tabItems[3].tabDlgHwnd, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN)];

    Tab tab = static_cast<Tab>(::SendDlgItemMessage(getHSelf(), IDC_SETTINGS_TABS, TCM_GETCURSEL, 0, 0));
    if (tab >= Tab::GameBase) {
      auto game = getGame(tab);
      saveGameSettings(settings.compilerSettings.gameSettings(game), game == Game::Fallout4);
    }

    if (settingsUpdatedFunc) {
      settingsUpdatedFunc();
    }
    return true;
  }

} // namespace
