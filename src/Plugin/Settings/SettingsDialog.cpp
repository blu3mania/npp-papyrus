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

#include "..\Common\NotepadPlusPlus.hpp"
#include "..\Common\Resources.hpp"
#include "..\Common\StringUtil.hpp"

#include "..\..\external\gsl\include\gsl\util"
#include "..\..\external\npp\PluginInterface.h"
#include "..\..\external\XMessageBox\XMessageBox.h"

#include <commctrl.h>

namespace papyrus {

  // Internal static variables
  namespace {
    dropdown_options_t tabNames {
      L"Lexer",
      L"Keyword Matcher",
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
  }

  void SettingsDialog::doDialog(callback_t callback) {
    settingsUpdatedFunc = callback;
    MultiTabbedDialog::doDialog();
  }

  // Protected methods
  //

  void SettingsDialog::initControls() {
    MultiTabbedDialog::initControls();
    addTab(std::to_underlying(Tab::Lexer), IDC_SETTINGS_TAB_LEXER, tabNames[std::to_underlying(Tab::Lexer)]);
    addTab(std::to_underlying(Tab::KeywordMatcher), IDC_SETTINGS_TAB_KEYWORD_MATCHER, tabNames[std::to_underlying(Tab::KeywordMatcher)]);
    addTab(std::to_underlying(Tab::ErrorAnnotator), IDC_SETTINGS_TAB_ERROR_ANNOTATOR, tabNames[std::to_underlying(Tab::ErrorAnnotator)]);
    addTab(std::to_underlying(Tab::Compiler), IDC_SETTINGS_TAB_COMPILER, tabNames[std::to_underlying(Tab::Compiler)]);

    for (int i = std::to_underlying(Game::Auto) + 1; i < static_cast<int>(game::games.size()); ++i) {
      auto game = static_cast<Game>(i);
      if (settings.compilerSettings.gameSettings(game).enabled) {
        addGameTab(game);
      }
    }

    showTab(std::to_underlying(Tab::Lexer));
  }

  void SettingsDialog::onTabDialogCreated(tab_id_t tab) {
    switch (tab) {
      case std::to_underlying(Tab::Lexer):
        enableGroup(Group::ClassLink, settings.lexerSettings.enableClassLink);
        setChecked(tab, IDC_SETTINGS_LEXER_FOLD_MIDDLE, settings.lexerSettings.enableFoldMiddle);
        foldMiddleTooltip = createToolTip(tab, IDC_SETTINGS_LEXER_FOLD_MIDDLE, IDS_SETTINGS_LEXER_FOLD_MIDDLE_TOOLTIP);

        setChecked(tab, IDC_SETTINGS_LEXER_CLASS_NAME_CACHING, settings.lexerSettings.enableClassNameCache);
        classNameCachingTooltip = createToolTip(tab, IDC_SETTINGS_LEXER_CLASS_NAME_CACHING, IDS_SETTINGS_LEXER_CLASS_NAME_CACHING_TOOLTIP);

        setChecked(tab, IDC_SETTINGS_LEXER_CLASS_LINK, settings.lexerSettings.enableClassLink);
        classLinkTooltip = createToolTip(tab, IDC_SETTINGS_LEXER_CLASS_LINK, IDS_SETTINGS_LEXER_CLASS_LINK_TOOLTIP);
        setChecked(tab, IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, settings.lexerSettings.classLinkUnderline);
        initColorPicker(tab, classLinkFgColorPicker, IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL);
        classLinkFgColorPicker.setColour(settings.lexerSettings.classLinkForegroundColor);
        initColorPicker(tab, classLinkBgColorPicker, IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL);
        classLinkBgColorPicker.setColour(settings.lexerSettings.classLinkBackgroundColor);
        setChecked(tab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_SHIFT, (settings.lexerSettings.classLinkClickModifier & SCMOD_SHIFT) != 0);
        setChecked(tab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_CTRL, (settings.lexerSettings.classLinkClickModifier & SCMOD_CTRL) != 0);
        setChecked(tab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_ALT, (settings.lexerSettings.classLinkClickModifier & SCMOD_ALT) != 0);

        stylerConfigLink.init(getHinst(), getHSelf());
        stylerConfigLink.create(getControl(tab, IDC_SETTINGS_LEXER_STYLER_CONFIG_LINK), IDC_SETTINGS_LEXER_STYLER_CONFIG_LINK);
        break;

      case std::to_underlying(Tab::KeywordMatcher):
        enableGroup(Group::Matcher, settings.keywordMatcherSettings.enableKeywordMatching);
        setChecked(tab, IDC_SETTINGS_MATCHER, settings.keywordMatcherSettings.enableKeywordMatching);
        matcherTooltip = createToolTip(tab, IDC_SETTINGS_MATCHER, IDS_SETTINGS_MATCHER_TOOLTIP);

        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_FUNCTION);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_STATE, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_STATE);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_EVENT, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_EVENT);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_PROPERTY);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_GROUP, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_GROUP);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_STRUCT, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_STRUCT);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_IF, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_IF);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_ELSE, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_ELSE);
        setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_WHILE, settings.keywordMatcherSettings.enabledKeywords & KEYWORD_WHILE);

        matcherIndicatorIdTooltip = createToolTip(tab, IDC_SETTINGS_MATCHER_INDICATOR_ID_LABEL, IDS_SETTINGS_MATCHER_INDICATOR_ID_TOOLTIP);
        setText(tab, IDC_SETTINGS_MATCHER_INDICATOR_ID, std::to_wstring(settings.keywordMatcherSettings.indicatorID));

        initDropdownList(tab, IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN, indicatorStyles, settings.keywordMatcherSettings.matchedIndicatorStyle);
        initColorPicker(tab, matchedIndicatorFgColorPicker, IDC_SETTINGS_MATCHER_MATCHED_FGCOLOR_LABEL);
        matchedIndicatorFgColorPicker.setColour(settings.keywordMatcherSettings.matchedIndicatorForegroundColor);

        initDropdownList(tab, IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN, indicatorStyles, settings.keywordMatcherSettings.unmatchedIndicatorStyle);
        initColorPicker(tab, unmatchedIndicatorFgColorPicker, IDC_SETTINGS_MATCHER_UNMATCHED_FGCOLOR_LABEL);
        unmatchedIndicatorFgColorPicker.setColour(settings.keywordMatcherSettings.unmatchedIndicatorForegroundColor);
        break;

      case std::to_underlying(Tab::ErrorAnnotator):
        enableGroup(Group::Annotation, settings.errorAnnotatorSettings.enableAnnotation);
        setChecked(tab, IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION, settings.errorAnnotatorSettings.enableAnnotation);
        annotationTooltip = createToolTip(tab, IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION, IDS_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION_TOOLTIP);
        initColorPicker(tab, annotationFgColorPicker, IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL);
        annotationFgColorPicker.setColour(settings.errorAnnotatorSettings.annotationForegroundColor);
        initColorPicker(tab, annotationBgColorPicker, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL);
        annotationBgColorPicker.setColour(settings.errorAnnotatorSettings.annotationBackgroundColor);
        setChecked(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC, settings.errorAnnotatorSettings.isAnnotationItalic);
        setChecked(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD, settings.errorAnnotatorSettings.isAnnotationBold);

        enableGroup(Group::Indication, settings.errorAnnotatorSettings.enableIndication);
        setChecked(tab, IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION, settings.errorAnnotatorSettings.enableIndication);
        indicationTooltip = createToolTip(tab, IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION, IDS_SETTINGS_ANNOTATOR_ENABLE_INDICATION_TOOLTIP);
        errorIndicatorIdTooltip = createToolTip(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL, IDS_SETTINGS_ANNOTATOR_INDICATOR_ID_TOOLTIP);
        setText(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID, std::to_wstring(settings.errorAnnotatorSettings.indicatorID));
        initDropdownList(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, indicatorStyles, settings.errorAnnotatorSettings.indicatorStyle);
        initColorPicker(tab, errorIndicatorFgColorPicker, IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL);
        errorIndicatorFgColorPicker.setColour(settings.errorAnnotatorSettings.indicatorForegroundColor);
        break;

      case std::to_underlying(Tab::Compiler):
        enableGroup(Group::GameSkyrim, settings.compilerSettings.skyrim.enabled);
        enableGroup(Group::GameSSE, settings.compilerSettings.sse.enabled);
        enableGroup(Group::GameFO4, settings.compilerSettings.fo4.enabled);

        setChecked(tab, IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE, settings.compilerSettings.allowUnmanagedSource);
        setChecked(tab, IDC_SETTINGS_COMPILER_RADIO_AUTO + std::to_underlying(settings.compilerSettings.gameMode), true);
        setText(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, settings.compilerSettings.autoModeOutputDirectory);
        updateAutoModeDefaultGame();
        if (settings.compilerSettings.autoModeDefaultGame != Game::Auto) {
          // Set current user prefs in auto mode default game dropdown list
          setDropdownSelectedText(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, game::gameNames[std::to_underlying(settings.compilerSettings.autoModeDefaultGame)].second);
        }

        autoModeTooltip = createToolTip(tab, IDC_SETTINGS_COMPILER_RADIO_AUTO, IDS_SETTINGS_COMPILER_RADIO_AUTO_TOOLTIP);
        break;

      default:
        if (tab > std::to_underlying(Tab::GameBase)) {
          // Update displayed values when showing, and save current values when hiding
          auto game = getGame(tab);
          const auto& gameSettings = settings.compilerSettings.gameSettings(game);

          setText(tab, IDC_SETTINGS_TAB_GAME_INSTALL_PATH, gameSettings.installPath);
          setText(tab, IDC_SETTINGS_TAB_GAME_COMPILER_PATH, gameSettings.compilerPath);
          setText(tab, IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY, gameSettings.outputDirectory);
          setText(tab, IDC_SETTINGS_TAB_GAME_FLAG_FILE, gameSettings.flagFile);
          setChecked(tab, IDC_SETTINGS_TAB_GAME_ANONYMIZE, gameSettings.anonynmizeFlag);
          setChecked(tab, IDC_SETTINGS_TAB_GAME_OPTIMIZE, gameSettings.optimizeFlag);
          if (game == Game::Fallout4) {
            setChecked(tab, IDC_SETTINGS_TAB_GAME_RELEASE, gameSettings.releaseFlag);
            setChecked(tab, IDC_SETTINGS_TAB_GAME_FINAL, gameSettings.finalFlag);
          } else {
            hideControl(tab, IDC_SETTINGS_TAB_GAME_RELEASE);
            hideControl(tab, IDC_SETTINGS_TAB_GAME_FINAL);
          }

          // Import directories is a semi-colon delimited multi-line text. Make a copy of it
          std::wstring importDirectories = gameSettings.importDirectories;
          size_t index = 0;
          while ((index = importDirectories.find(L";", index)) != std::wstring::npos) {
            importDirectories.replace(index, 1, L"\r\n");
          }
          setText(tab, IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES, importDirectories);
        }
        break;
    }
  }

  INT_PTR SettingsDialog::handleTabCommandMessage(tab_id_t tab, WPARAM wParam, LPARAM lParam) {
    if (wParam == IDC_SETTINGS_LEXER_STYLER_CONFIG_LINK) {
      // Special link for Notepad++'s Style Configurator dialog. Send a message to activate that menu
      ::SendMessage(getHParent(), NPPM_MENUCOMMAND, 0, IDM_LANGSTYLE_CONFIG_DLG);
    } else if (HIWORD(wParam) == BN_CLICKED) {
      // Only controls that would trigger live update are checked here
      switch (LOWORD(wParam)) {
        case IDC_SETTINGS_LEXER_FOLD_MIDDLE: {
          settings.lexerSettings.enableFoldMiddle = getChecked(tab, IDC_SETTINGS_LEXER_FOLD_MIDDLE);
          return FALSE;
        }

        case IDC_SETTINGS_LEXER_CLASS_LINK: {
          settings.lexerSettings.enableClassLink = getChecked(tab, IDC_SETTINGS_LEXER_CLASS_LINK);
          enableGroup(Group::ClassLink, settings.lexerSettings.enableClassLink);
          return FALSE;
        }

        case IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE: {
          settings.lexerSettings.classLinkUnderline = getChecked(tab, IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE);
          return FALSE;
        }

        case IDC_SETTINGS_MATCHER: {
          settings.keywordMatcherSettings.enableKeywordMatching = getChecked(tab, IDC_SETTINGS_MATCHER);
          enableGroup(Group::Matcher, settings.keywordMatcherSettings.enableKeywordMatching);
          return FALSE;
        }

        case IDC_SETTINGS_MATCHER_KEYWORD_IF: {
          // Enable/disable Else/ElseIf support based on If/EndIf support
          bool allowIf = getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_IF);
          setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_ELSE, allowIf);
          if (!allowIf) {
            setChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_ELSE, false);
          }

          // Let it fall through to the next block
        }
        [[fallthrough]];

        case IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION:
        case IDC_SETTINGS_MATCHER_KEYWORD_STATE:
        case IDC_SETTINGS_MATCHER_KEYWORD_EVENT:
        case IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY:
        case IDC_SETTINGS_MATCHER_KEYWORD_GROUP:
        case IDC_SETTINGS_MATCHER_KEYWORD_STRUCT:
        case IDC_SETTINGS_MATCHER_KEYWORD_ELSE:
        case IDC_SETTINGS_MATCHER_KEYWORD_WHILE: {
          updateEnabledKeywords();
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION: {
          settings.errorAnnotatorSettings.enableAnnotation = getChecked(tab, IDC_SETTINGS_ANNOTATOR_ENABLE_ANNOTATION);
          enableGroup(Group::Annotation, settings.errorAnnotatorSettings.enableAnnotation);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION: {
          settings.errorAnnotatorSettings.enableIndication = getChecked(tab, IDC_SETTINGS_ANNOTATOR_ENABLE_INDICATION);
          enableGroup(Group::Indication, settings.errorAnnotatorSettings.enableIndication);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC: {
          settings.errorAnnotatorSettings.isAnnotationItalic = getChecked(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC);
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD: {
          settings.errorAnnotatorSettings.isAnnotationBold = getChecked(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD);
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
          int selectedIndex = getDropdownSelectedIndex(tab, IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN);
          if (selectedIndex != CB_ERR) {
            settings.keywordMatcherSettings.matchedIndicatorStyle = selectedIndex;
          }
          return FALSE;
        }

        case IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN: {
          int selectedIndex = getDropdownSelectedIndex(tab, IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN);
          if (selectedIndex != CB_ERR) {
            settings.keywordMatcherSettings.unmatchedIndicatorStyle = selectedIndex;
          }
          return FALSE;
        }

        case IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN: {
          int selectedIndex = getDropdownSelectedIndex(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN);
          if (selectedIndex != CB_ERR) {
            settings.errorAnnotatorSettings.indicatorStyle = selectedIndex;
          }
          return FALSE;
        }
      }
    }

    return FALSE;
  }

  INT_PTR SettingsDialog::handleCloseMessage(WPARAM wParam, LPARAM lParam) {
    if (saveSettings()) {
      hide();
    }

    return MultiTabbedDialog::handleCloseMessage(wParam, lParam);
  }

  // Private methods
  //

  void SettingsDialog::enableGroup(Group group, bool enabled) const {
    switch (group) {
      case Group::ClassLink: {
        constexpr tab_id_t tab = std::to_underlying(Tab::Lexer);
        setControlEnabled(tab, IDC_SETTINGS_LEXER_CLASS_LINK_UNDERLINE, enabled);
        setControlEnabled(tab, IDC_SETTINGS_LEXER_CLASS_LINK_FGCOLOR_LABEL, enabled);
        ::EnableWindow(classLinkFgColorPicker.getHSelf(), enabled);
        setControlEnabled(tab, IDC_SETTINGS_LEXER_CLASS_LINK_BGCOLOR_LABEL, enabled);
        ::EnableWindow(classLinkBgColorPicker.getHSelf(), enabled);
        setControlEnabled(tab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_SHIFT, enabled);
        setControlEnabled(tab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_CTRL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_ALT, enabled);
        break;
      }

      case Group::Matcher: {
        constexpr tab_id_t tab = std::to_underlying(Tab::KeywordMatcher);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORDS_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_STATE, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_EVENT, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_GROUP, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_STRUCT, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_IF, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_ELSE, enabled && getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_IF));
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_KEYWORD_WHILE, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_INDICATOR_ID_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_INDICATOR_ID, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_MATCHED_STYLE_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_MATCHED_STYLE_DROPDOWN, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_MATCHED_FGCOLOR_LABEL, enabled);
        ::EnableWindow(matchedIndicatorFgColorPicker.getHSelf(), enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_UNMATCHED_STYLE_DROPDOWN, enabled);
        setControlEnabled(tab, IDC_SETTINGS_MATCHER_UNMATCHED_FGCOLOR_LABEL, enabled);
        ::EnableWindow(unmatchedIndicatorFgColorPicker.getHSelf(), enabled);
        break;
      }

      case Group::Annotation: {
        constexpr tab_id_t tab = std::to_underlying(Tab::ErrorAnnotator);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_FGCOLOR_LABEL, enabled);
        ::EnableWindow(annotationFgColorPicker.getHSelf(), enabled);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BGCOLOR_LABEL, enabled);
        ::EnableWindow(annotationBgColorPicker.getHSelf(), enabled);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_ITALIC, enabled);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_ANNOTATION_BOLD, enabled);
        break;
      }

      case Group::Indication: {
        constexpr tab_id_t tab = std::to_underlying(Tab::ErrorAnnotator);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID, enabled);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_STYLE_DROPDOWN, enabled);
        setControlEnabled(tab, IDC_SETTINGS_ANNOTATOR_INDICATOR_FGCOLOR_LABEL, enabled);
        ::EnableWindow(errorIndicatorFgColorPicker.getHSelf(), enabled);
        break;
      }

      case Group::GameAuto: {
        constexpr tab_id_t tab = std::to_underlying(Tab::Compiler);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_RADIO_AUTO, enabled);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, enabled);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT_LABEL, enabled);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT, enabled);
        break;
      }

      case Group::GameSkyrim: {
        constexpr tab_id_t tab = std::to_underlying(Tab::Compiler);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_RADIO_SKYRIM, enabled);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_SKYRIM_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SKYRIM_TOGGLE, enabled);
        break;
      }

      case Group::GameSSE: {
        constexpr tab_id_t tab = std::to_underlying(Tab::Compiler);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_RADIO_SSE, enabled);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_SSE_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_SSE_TOGGLE, enabled);
        break;
      }

      case Group::GameFO4: {
        constexpr tab_id_t tab = std::to_underlying(Tab::Compiler);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_RADIO_FO4, enabled);
        setControlEnabled(tab, IDC_SETTINGS_COMPILER_FO4_CONFIGURE, enabled);
        updateGameEnableButtonText(IDC_SETTINGS_COMPILER_FO4_TOGGLE, enabled);
        break;
      }

      default: {
        break;
      }
    }
  }

  void SettingsDialog::updateEnabledKeywords() const {
    constexpr tab_id_t tab = std::to_underlying(Tab::KeywordMatcher);
    settings.keywordMatcherSettings.enabledKeywords =
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_FUNCTION) ? KEYWORD_FUNCTION : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_STATE) ? KEYWORD_STATE : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_EVENT) ? KEYWORD_EVENT : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_PROPERTY) ? KEYWORD_PROPERTY : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_GROUP) ? KEYWORD_GROUP : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_STRUCT) ? KEYWORD_STRUCT : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_IF) ? KEYWORD_IF : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_ELSE) ? KEYWORD_ELSE : KEYWORD_NONE) |
      (getChecked(tab, IDC_SETTINGS_MATCHER_KEYWORD_WHILE) ? KEYWORD_WHILE : KEYWORD_NONE);
  }

  Game SettingsDialog::getGame(tab_id_t tab) const {
    if (tab > std::to_underlying(Tab::GameBase)) {
      return static_cast<Game>(std::to_underlying(Game::Auto) + (tab - std::to_underlying(Tab::GameBase)));
    }
    return Game::Auto;
  }

  tab_id_t SettingsDialog::getGameTab(Game game) const {
    if (!settings.compilerSettings.gameSettings(game).enabled) {
      return -1;
    }

    return std::to_underlying(Tab::GameBase) + (std::to_underlying(game) - std::to_underlying(Game::Auto));
  }

  void SettingsDialog::addGameTab(Game game) {
    TCITEM item {
      .mask = TCIF_TEXT,
      .pszText = const_cast<LPWSTR>(game::gameNames[std::to_underlying(game)].second.c_str()),
      .cchTextMax = static_cast<int>(_tcslen(item.pszText))
    };

    tab_id_t referenceTab = std::to_underlying(Tab::Compiler);
    for (int i = std::to_underlying(Game::Auto) + 1; i < std::to_underlying(game); ++i) {
      Game refGame = static_cast<Game>(i);
      if (settings.compilerSettings.gameSettings(refGame).enabled) {
        referenceTab = getGameTab(refGame);
      }
    }
    addTabAfter(getGameTab(game), IDC_SETTINGS_TAB_GAME, game::gameNames[std::to_underlying(game)].second, referenceTab);
  }

  void SettingsDialog::removeGameTab(Game game) {
    removeTab(getGameTab(game));
  }

  void SettingsDialog::toggleGame(Game game, int controlID, Group group) {
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
  }

  void SettingsDialog::configureGame(Game game) {
    int gameTab = getGameTab(game);
    if (gameTab != -1) {
      showTab(gameTab);
    }
  }

  void SettingsDialog::updateAutoModeDefaultGame() const {
    constexpr tab_id_t tab = std::to_underlying(Tab::Compiler);
    bool enabled = settings.compilerSettings.skyrim.enabled || settings.compilerSettings.sse.enabled || settings.compilerSettings.fo4.enabled;
    enableGroup(Group::GameAuto, enabled);

    // Keep current selection
    auto dropdown = getControl(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN);
    int length = ::GetWindowTextLength(dropdown);
    wchar_t* currentSelection = new wchar_t[static_cast<size_t>(length) + 1];
    auto autoCleanup = gsl::finally([&] { delete[] currentSelection; });
    ::GetWindowText(dropdown, currentSelection, length + 1);

    // Clear and re-populate default game dropdown list
    clearDropdownList(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN);
    dropdown_options_t gameOptions;
    if (enabled) {
      for (int i = static_cast<int>(Game::Auto) + 1; i < static_cast<int>(game::games.size()); ++i) {
        if (settings.compilerSettings.gameSettings(static_cast<Game>(i)).enabled) {
          // Use game's display name
          gameOptions.push_back(const_cast<LPWSTR>(game::gameNames[i].second.c_str()));
        }
      }
    } else {
      // No game is enabled
      gameOptions.push_back(const_cast<LPWSTR>(game::gameNames[std::to_underlying(Game::Auto)].second.c_str()));
    }
    initDropdownList(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, gameOptions);

    // Try to find the previous selection in the new list
    bool selectionSet = false;
    if (length > 0) {
      if (setDropdownSelectedText(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, currentSelection)) {
        selectionSet = true;
      }
    }
    if (selectionSet) {
      setDropdownSelectedIndex(tab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN, 0);
    }
  }

  void SettingsDialog::updateGameEnableButtonText(int controlID, bool enabled) const {
    constexpr tab_id_t tab = std::to_underlying(Tab::Compiler);
    setText(tab, controlID, enabled ? L"Disable" : L"Enable");
  }

  void SettingsDialog::saveGameSettings(tab_id_t tab, CompilerSettings::GameSettings& gameSettings) const {
    gameSettings.installPath = getText(tab, IDC_SETTINGS_TAB_GAME_INSTALL_PATH);
    gameSettings.compilerPath = getText(tab, IDC_SETTINGS_TAB_GAME_COMPILER_PATH);
    gameSettings.outputDirectory = getText(tab, IDC_SETTINGS_TAB_GAME_OUTPUT_DIRECTORY);
    gameSettings.flagFile = getText(tab, IDC_SETTINGS_TAB_GAME_FLAG_FILE);
    gameSettings.anonynmizeFlag = getChecked(tab, IDC_SETTINGS_TAB_GAME_ANONYMIZE);
    gameSettings.optimizeFlag = getChecked(tab, IDC_SETTINGS_TAB_GAME_OPTIMIZE);
    if (getGame(tab) == Game::Fallout4) {
      gameSettings.releaseFlag = getChecked(tab, IDC_SETTINGS_TAB_GAME_RELEASE);
      gameSettings.finalFlag = getChecked(tab, IDC_SETTINGS_TAB_GAME_FINAL);
    }

    // Import directories is a semi-colon delimited multi-line text
    gameSettings.importDirectories = getText(tab, IDC_SETTINGS_TAB_GAME_IMPORT_DIRECTORIES);
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
    constexpr tab_id_t errorAnnotatorTab = std::to_underlying(Tab::ErrorAnnotator);
    if (isTabDialogCreated(errorAnnotatorTab)) {
      std::wstring errorIndicatorIDStr = getText(errorAnnotatorTab, IDC_SETTINGS_ANNOTATOR_INDICATOR_ID);
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

      settings.errorAnnotatorSettings.indicatorID = errorIndicatorID;
    }

    constexpr tab_id_t keywordMatcherTab = std::to_underlying(Tab::KeywordMatcher);
    if (isTabDialogCreated(keywordMatcherTab)) {
      std::wstring matcherIndicatorIDStr = getText(keywordMatcherTab, IDC_SETTINGS_MATCHER_INDICATOR_ID);
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

      settings.keywordMatcherSettings.indicatorID = matcherIndicatorID;
    }

    constexpr tab_id_t lexerTab = std::to_underlying(Tab::Lexer);
    if (isTabDialogCreated(lexerTab)) {
      settings.lexerSettings.enableClassNameCache = getChecked(lexerTab, IDC_SETTINGS_LEXER_CLASS_NAME_CACHING);
      settings.lexerSettings.classLinkClickModifier = SCMOD_NORM |
        (getChecked(lexerTab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_SHIFT) ? SCMOD_SHIFT : SCMOD_NORM) |
        (getChecked(lexerTab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_CTRL) ? SCMOD_CTRL : SCMOD_NORM) |
        (getChecked(lexerTab, IDC_SETTINGS_LEXER_CLASS_LINK_MODIFIER_ALT) ? SCMOD_ALT : SCMOD_NORM);
    }

    constexpr tab_id_t compilerTab = std::to_underlying(Tab::Compiler);
    if (isTabDialogCreated(compilerTab)) {
      settings.compilerSettings.allowUnmanagedSource = getChecked(compilerTab, IDC_SETTINGS_COMPILER_ALLOW_UNMANAGED_SOURCE);
      settings.compilerSettings.autoModeOutputDirectory = getText(compilerTab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_OUTPUT);
      settings.compilerSettings.autoModeDefaultGame = game::games[getText(compilerTab, IDC_SETTINGS_COMPILER_AUTO_DEFAULT_GAME_DROPDOWN)];
    }

    for (int i = std::to_underlying(Game::Auto) + 1; i < static_cast<int>(game::games.size()); ++i) {
      Game game = static_cast<Game>(i);
      tab_id_t gameTab = getGameTab(game);
      if (gameTab > std::to_underlying(Tab::GameBase) && isTabDialogCreated(gameTab)) {
        saveGameSettings(gameTab, settings.compilerSettings.gameSettings(game));
      }
    }

    if (settingsUpdatedFunc) {
      settingsUpdatedFunc();
    }
    return true;
  }

} // namespace
