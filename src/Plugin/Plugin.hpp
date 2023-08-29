/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2016 Tschilkroete <tschilkroete@gmail.com> (original author)
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

#include "Common\Game.hpp"
#include "Common\NotepadPlusPlus.hpp"
#include "Common\Timer.hpp"
#include "CompilationErrorHandling\ErrorAnnotator.hpp"
#include "CompilationErrorHandling\ErrorsWindow.hpp"
#include "Compiler\Compiler.hpp"
#include "Compiler\CompilerSettings.hpp"
#include "KeywordMatcher\KeywordMatcher.hpp"
#include "Settings\Settings.hpp"
#include "Settings\SettingsDialog.hpp"
#include "UI\AboutDialog.hpp"
#include "UI\UIParameters.hpp"

#include "..\external\npp\PluginInterface.h"

#include <memory>

// Plugin constants
//
namespace papyrus {

  using Game = game::Game;

  class Plugin {
    public:
      Plugin();

      // DLL initialization/cleanup
      void onInit(HINSTANCE instance);
      void cleanUp();

      // Interface functions with Notepad++
      inline TCHAR* name() const { return const_cast<TCHAR*>(PLUGIN_NAME); }
      inline BOOL useUnicode() const { return USE_UNICODE; }
      inline int numFuncs() const { return std::to_underlying(Menu::COUNT); }
      inline  FuncItem* getFuncs() { return funcs; }
      void setNppData(NppData data);
      void onNotification(SCNotification* notification);
      LRESULT handleNppMessage(UINT message, WPARAM wParam, LPARAM lParam);

    private:
      enum class Menu {
        Compile,
        GoToMatch,
        Options,
        Seperator1,
        Advanced,
        Seperator2,
        About,
        COUNT
      };

      enum class AdvancedMenu {
        ResetLexerStyles,
        ShowLangID,
        InstallAutoCompletion,
        InstallFunctionList
      };

      void initializeComponents();

      // Check/copy lexer's config file
      void copyLexerConfigFile(bool isStartupCheck = false);

      // Update Notepad++ UI parameters, such as dark mode and default fore/background colors
      void updateNppUIParameters();

      // Notepad++ notification NPPN_BUFFERACTIVATED and NPPN_LANGCHANGED handler
      void handleBufferActivation(npp_buffer_t bufferID, bool fromLangChange);

      // Scintilla notification SCN_HOTSPOTCLICK handler
      void handleHotspotClick(SCNotification* notification);

      // Scintilla notification SCN_DWELLSTART/SCN_DWELLEND handler
      void handleMouseHover(SCNotification* notification, bool hovering);

      // Scintilla notification SCN_MODIFIED handler, when texts are added/deleted
      void handleContentChange(SCNotification* notification);

      // Scintilla notification SCN_UPDATEUI handler, when selection updated
      void handleSelectionChange(SCNotification* notification);

      // Handle setting changes
      void onSettingsUpdated();
      void updateLexerDataGameSettings(Game game, const CompilerSettings::GameSettings& gameSettings);

      // Find out langID assigned to Papyrus Script lexer
      void detectLangID();
      bool checkLangName(npp_lang_type_t langID, std::wstring lexerName);

      // Check if current buffer on given Scintilla view is the active buffer and is managed by this plugin's lexer
      bool isCurrentBufferManaged(HWND scintillaHandle);

      // Retrieve Notepad++ buffer ID on the view that matches passed in Scintilla handle
      npp_buffer_t getBufferFromScintillaHandle(HWND scintillaHandle) const;

      // Find out game type based on file path and settings
      std::pair<Game, bool> detectGameType(const std::wstring& filePath, const CompilerSettings& compilerSettings) const;

      // Clear cached active compilation request, so when buffer gets switched
      // in NPP it can be properly handled
      void clearActiveCompilation();

      // Plugin's own message handling
      static LRESULT CALLBACK messageHandleProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
      LRESULT handleOwnMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

      // Copy source file to destination (possibly overwrite). May invoke shell command to execute if privilege
      // elevation is needed. In that case, "waitFor" will be used to determien how long the process is going
      // to wait for the execution. By default it only waits for up to 3 seconds.
      bool copyFile(const std::wstring& sourceFile, const std::wstring& destinationFile, int waitFor = 3000);
      bool copyFile(const std::wstring& sourceFile, const std::wstring& destinationFile, HWND ownerWindow, int waitFor = 3000);

      // Menu functions
      //
      void setupAdvancedMenu();
      static void advancedMenuFunc() {}; // Still need an empty func so NPP won't render the menu item as a separator
      void resetLexerStyles();
      void showLangID();
      void installAutoCompletion();
      void installFunctionList();

      static void compileMenuFunc();
      void compile();
      static void goToMatchMenuFunc();
      void goToMatch();
      static void settingsMenuFunc();
      void showSettings();
      static void aboutMenuFunc();
      void showAbout();

      // Private members
      //
      FuncItem funcs[std::to_underlying(Menu::COUNT)];

      UINT advancedMenuBaseCmdID {};

      HWND messageWindow {};

      HINSTANCE myInstance {};
      NppData nppData;

      UIParameters uiParameters {};

      Settings settings;
      SettingsStorage settingsStorage;
      SettingsDialog settingsDialog {settings, uiParameters};

      std::unique_ptr<Compiler> compiler;
      CompilationRequest activeCompilationRequest;
      bool isCompilingCurrentFile {false};

      std::unique_ptr<ErrorsWindow> errorsWindow;
      std::unique_ptr<ErrorAnnotator> errorAnnotator;
      std::unique_ptr<KeywordMatcher> keywordMatcher;
      std::list<Error> activatedErrorsTrackingList;
      std::unique_ptr<utility::Timer> jumpToErrorLineTimer;

      npp_lang_type_t scriptLangID {0};

      AboutDialog aboutDialog;

      bool isShuttingDown {false};
  };

} // namespace

extern papyrus::Plugin papyrusPlugin;
