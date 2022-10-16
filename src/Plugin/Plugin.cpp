/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2016 - 2017 Tschilkroete <tschilkroete@gmail.com> (original author)
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

/*
Useful links:
https://www.scintilla.org/ScintillaDoc.html
https://npp-user-manual.org/docs/plugin-communication/
https://community.notepad-plus-plus.org/category/5/plugin-development

https://www.creationkit.com/index.php?title=Category:Papyrus
https://www.creationkit.com/fallout4/index.php?title=Category:Papyrus
*/

#include "Plugin.hpp"

#include "Common\FileSystemUtil.hpp"
#include "Common\Logger.hpp"
#include "Common\StringUtil.hpp"
#include "Common\Version.hpp"
#include "Compiler\CompilationRequest.hpp"
#include "Lexer\Lexer.hpp"
#include "Lexer\LexerData.hpp"

#include "..\external\gsl\include\gsl\util"
#include "..\external\npp\NppDarkMode.h"
#include "..\external\tinyxml2\tinyxml2.h"
#include "..\external\XMessageBox\XMessageBox.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

papyrus::Plugin papyrusPlugin;

namespace papyrus {

  // Static variables shared with Lexer
  std::unique_ptr<LexerData> lexerData;

  // Internal static variables
  namespace {
    std::vector<LPCWSTR> advancedMenuItems {
      L"Show langID...",
      L"Install auto completion support...",
      L"Install function list support..."
    };
  }

  Plugin::Plugin()
    : funcs{
      FuncItem{ L"Compile", compileMenuFunc, 0, false, new ShortcutKey{true, false, true, 0x43} },
      FuncItem{ L"Settings...", settingsMenuFunc, 0, false, nullptr },
      FuncItem{}, // Separator1
      FuncItem{ L"Advanced", advancedMenuFunc, 0, false, nullptr },
      FuncItem{}, // Separator2
      FuncItem{ L"About...", aboutMenuFunc, 0, false, nullptr }
    }, keywordMatcher(settings.keywordMatcherSettings) {
  }

  void Plugin::onInit(HINSTANCE instance) {
    myInstance = instance;

    WNDCLASS messageHandleClass {
      .lpfnWndProc = messageHandleProc,
      .hInstance = instance,
      .lpszClassName = L"MESSAGE_WINDOW"
    };
    ::RegisterClass(&messageHandleClass);
    messageWindow = ::CreateWindow(L"MESSAGE_WINDOW", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, nullptr);
  }

  void Plugin::cleanUp() {
  }

  void Plugin::setNppData(NppData data) {
    nppData = data;
    initializeComponents();
  }

  void Plugin::onNotification(SCNotification* notification) {
    if ((notification->nmhdr.hwndFrom == nppData._scintillaMainHandle) || (notification->nmhdr.hwndFrom == nppData._scintillaSecondHandle)) {
      switch (notification->nmhdr.code) {
        case SCN_HOTSPOTCLICK:
        case SCN_HOTSPOTDOUBLECLICK: {
          handleHotspotClick(notification);
          break;
        }

        case SCN_UPDATEUI: {
          if (notification->updated & SC_UPDATE_SELECTION) {
            handleSelectionChange(notification);
          }
          break;
        }

        case SCN_MODIFIED: {
          if (notification->modificationType & SC_MOD_INSERTTEXT || notification->modificationType & SC_MOD_DELETETEXT) {
            handleContentUpdate(notification);
          }
          break;
        }
      }
    } else if (notification->nmhdr.hwndFrom == nppData._nppHandle) {
      switch (notification->nmhdr.code) {
        case NPPN_READY: {
          setupAdvancedMenu();
          break;
        }

        case NPPN_BEFORESHUTDOWN: {
          isShuttingDown = true;
          break;
        }

        case NPPN_CANCELSHUTDOWN: {
          isShuttingDown = false;
          break;
        }

        case NPPN_BUFFERACTIVATED: {
          if (!isShuttingDown) {
            handleBufferActivation(notification->nmhdr.idFrom, false);
          }
          break;
        }

        case NPPN_LANGCHANGED: {
          handleBufferActivation(notification->nmhdr.idFrom, true);
          break;
        }

        case NPPN_DARKMODECHANGED: {
          updateNppUIParameters();
          break;
        }
      }
    }
  }

  LRESULT Plugin::handleNppMessage(UINT message, WPARAM wParam, LPARAM) {
    switch (message) {
      case WM_COMMAND: {
        // Menu command relayed by NPP
        UINT cmdId = static_cast<UINT>(wParam);
        if (cmdId >= advancedMenuBaseCmdID) {
          switch (static_cast<AdvancedMenu>(cmdId - advancedMenuBaseCmdID)) {
            case AdvancedMenu::ShowLangID:
              showLangID();
              break;

            case AdvancedMenu::InstallAutoCompletion:
              installAutoCompletion();
              break;

            case AdvancedMenu::InstallFunctionList:
              installFunctionList();
              break;
          }
        }
        break;
      }
    }
    return TRUE;
  }

  // Private methods
  //

  void Plugin::initializeComponents() {
    lexerData = std::make_unique<LexerData>(nppData, settings.lexerSettings);
    errorsWindow = std::make_unique<ErrorsWindow>(myInstance, nppData._nppHandle, messageWindow);
    errorAnnotator = std::make_unique<ErrorAnnotator>(nppData, settings.errorAnnotatorSettings);
    settingsDialog.init(myInstance, nppData._nppHandle);
    aboutDialog.init(myInstance, nppData._nppHandle);

    // Get Notepad++'s plugins config folder.
    npp_size_t configPathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, 0, 0));
    if (configPathLength > 0) {
      wchar_t* configPathCharArray = new wchar_t[configPathLength + 1];
      auto autoCleanupConfigPath = gsl::finally([&] { delete[] configPathCharArray; });
      ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, configPathLength + 1, reinterpret_cast<LPARAM>(configPathCharArray));
      std::wstring configPath(configPathCharArray);

      utility::logger.init(std::filesystem::path(configPath) / PLUGIN_NAME L".log");
      checkLexerConfigFile(configPath);

      // Load settings
      settingsStorage.init(std::filesystem::path(configPath) / PLUGIN_NAME L".ini");
      if (!settings.loadSettings(settingsStorage, utility::Version(PLUGIN_VERSION))) {
        // Settings didn't exist. Default settings initialized.
        settings.saveSettings(settingsStorage);
        onSettingsUpdated();
      } else {
        onSettingsUpdated();
      }

      // Only initialize compiler when settings are ready.
      CompilerMessages compilerMessages {
        .compilationDoneMessage = PPM_COMPILATION_DONE,
        .compilationFailureMessage = PPM_COMPILATION_FAILED,
        .anonymizationFailureMessage = PPM_ANONYMIZATION_FAILED,
        .compilerNotFoundMessage = PPM_COMPILER_NOT_FOUND,
        .otherErrordMessage = PPM_OTHER_ERROR,
        .withAnonymization = PARAM_COMPILATION_WITH_ANONYMIZATION,
        .compilationOnly = PARAM_COMPILATION_ONLY
      };
      compiler = std::make_unique<Compiler>(messageWindow, compilerMessages, settings.compilerSettings);
    }

    NppDarkMode::initDarkMode();
    updateNppUIParameters();
  }

  void Plugin::checkLexerConfigFile(const std::wstring& configPath) {
    // Check if lexer configuration file is in config folder.
    std::wstring lexerConfigFile = std::filesystem::path(configPath) / PLUGIN_NAME L".xml";
    if (!utility::fileExists(lexerConfigFile)) {
      // Lexer configuration file doesn't exist, try to generate one from the copy extracted from package.
      npp_size_t homePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, 0, 0));
      if (homePathLength > 0) {
        wchar_t* homePathCharArray = new wchar_t[homePathLength + 1];
        auto autoCleanupHomePath = gsl::finally([&] { delete[] homePathCharArray; });
        ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, homePathLength + 1, reinterpret_cast<LPARAM>(homePathCharArray));
        std::wstring pluginHomePath(homePathCharArray);

        if (!copyFile(std::filesystem::path(pluginHomePath) / PLUGIN_NAME / PLUGIN_NAME L".xml", lexerConfigFile)) {
          // Can't generate lexer configuration file. Mark lexer as unusable.
          lexerData->usable = false;
          std::wstring msg(PLUGIN_NAME L".xml is missing and cannot be autmatically generated.\r\n");
          msg += L"Please manually copy it to " + configPath + L".\r\n";
          msg += L"Close and restart Notepad++ afterwards.\r\n";
          msg += L"If Notepad++ asks if you want to remove " PLUGIN_NAME L".dll, please answer No.";
          ::MessageBox(nppData._nppHandle, msg.c_str(), PLUGIN_NAME L" plugin", MB_ICONEXCLAMATION | MB_OK);
        }
      }
    }
  }

  void Plugin::updateNppUIParameters() {
    bool darkModeEnabled = (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0) == TRUE);
    NppDarkMode::setDarkModeEnabled(darkModeEnabled);

    NppDarkMode::Colors nppDarkModeColors {};
    bool darkModeColorRetrieved = static_cast<bool>(::SendMessage(nppData._nppHandle, NPPM_GETDARKMODECOLORS, sizeof(NppDarkMode::Colors), reinterpret_cast<LPARAM>(&nppDarkModeColors)));
    //utility::logger.log(L"Dark mode colors retrieved? " + utility::boolToStr(darkModeColorRetrieved));

    if (darkModeColorRetrieved) {
      COLORREF nppDefaultFgColor = static_cast<COLORREF>(::SendMessage(nppData._nppHandle, NPPM_GETEDITORDEFAULTFOREGROUNDCOLOR, 0, 0));
      COLORREF nppDefaultBgColor = static_cast<COLORREF>(::SendMessage(nppData._nppHandle, NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR, 0, 0));
      NppDarkMode::setNppUIColors(nppDarkModeColors, nppDefaultFgColor, nppDefaultBgColor);
    }

    uiParameters.darkModeEnabled = darkModeEnabled;
  }

  void Plugin::handleBufferActivation(npp_buffer_t bufferID, bool fromLangChange) {
    // Make sure Papyrus script langID is detected.
    detectLangID();

    npp_view_t currentView = static_cast<npp_view_t>(::SendMessage(nppData._nppHandle, NPPM_GETCURRENTVIEW, 0, 0));
    npp_size_t filePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, static_cast<WPARAM>(bufferID), 0));
    if (filePathLength > 0) {
      wchar_t* filePathCharArray = new wchar_t[filePathLength + 1];
      auto autoCleanup = gsl::finally([&] { delete[] filePathCharArray; });
      if (::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, static_cast<WPARAM>(bufferID), reinterpret_cast<LPARAM>(filePathCharArray)) != -1) {
        std::wstring filePath(filePathCharArray);

        // Set detected game for lexer.
        auto [detectedGame, useAutoModeOutputDirectory] = detectGameType(filePath, settings.compilerSettings);
        if (lexerData && !fromLangChange) {
          lexerData->currentGame = detectedGame;
        }

        // Check if active compilation file is still the current one on either view.
        if (activeCompilationRequest.bufferID != 0) {
          isComplingCurrentFile = utility::compare(activeCompilationRequest.filePath, filePath);
          if (isComplingCurrentFile && !fromLangChange) {
            ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Compiling..."));
          }
        }

        // Check if we are waiting for a file to open as a result of user selecting an error from list.
        HWND scintillaHandle = (currentView == MAIN_VIEW) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
        if (!activatedErrorsTrackingList.empty() && !fromLangChange) {
          // Check if activated file is in the tracking list.
          auto iter = std::find_if(activatedErrorsTrackingList.begin(), activatedErrorsTrackingList.end(),
            [&](const auto& error) {
              return utility::compare(error.file, filePath);
            }
          );
          if (iter != activatedErrorsTrackingList.end()) {
            // Scintilla's line number is zero-based.
            int line = iter->line - 1;

            // When the buffer is big, asking Scintilla to scroll immediately doesn't always work, so use a short timer.
            jumpToErrorLineTimer = utility::startTimer(100, [=] {
              // Make sure the active document is still the one we are tracking before scrolling.
              if (bufferID == ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0)) {
                ::SendMessage(scintillaHandle, SCI_GOTOLINE, line, 0);
              }
            });

            // Get rid of other errors in the list for the same file.
            while (iter != activatedErrorsTrackingList.end()) {
              activatedErrorsTrackingList.erase(iter++);
              iter = std::find_if(iter, activatedErrorsTrackingList.end(),
                [&](const auto& error) {
                  return utility::compare(error.file, filePath);
                }
              );
            }
          }
        }

        bool isManagedBuffer = false;
        bool isPapyrusScriptFile = utility::endsWith(filePath, L".psc");
        npp_lang_type_t currentFileLangID;
        ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&currentFileLangID));
        if (currentFileLangID == scriptLangID) {
          // Papyrus script file lexed by this plugin's lexer, need to check/update annotation.
          isManagedBuffer = true;

          // If not compiling current file, check its game type and update status message (if applicable).
          if (!isComplingCurrentFile && detectedGame != Game::Auto) {
            std::wstring gameSpecificStatus(L"[" + game::gameNames[std::to_underlying(detectedGame)].second + L"] " + Lexer::statusText());
            ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(gameSpecificStatus.c_str()));
          }

          if (fromLangChange) {
            keywordMatcher.match(scintillaHandle);
          }
        } else if (isPapyrusScriptFile && fromLangChange) {
          // Papyrus script file changed to other language, clear keyword matching.
          keywordMatcher.clear();
        }

        // Only Papyrus script and assembly files can be annotated.
        if (errorAnnotator && (isPapyrusScriptFile || utility::endsWith(filePath, L".pas")) && !fromLangChange) {
          errorAnnotator->annotate(currentView, filePath);
        }

        if (lexerData) {
          lexerData->bufferActivated = std::make_pair(currentView, isManagedBuffer);
        }
      }
    }
  }

  void Plugin::handleHotspotClick(SCNotification* notification) {
    // Only handle hotspot click if it's from a document buffer shown on current view and is managed by this plugin's lexer, and key modifier/mouse click match configuration.
    if (lexerData
      && (notification->nmhdr.code == SCN_HOTSPOTDOUBLECLICK) == lexerData->settings.classLinkRequiresDoubleClick
      && notification->modifiers == lexerData->settings.classLinkClickModifier
      && isCurrentBufferManaged(static_cast<HWND>(notification->nmhdr.hwndFrom))) {
      lexerData->clickEventData = std::make_pair(static_cast<HWND>(notification->nmhdr.hwndFrom), notification->position);
    }
  }

  void Plugin::handleSelectionChange(SCNotification* notification) {
    // Only handle selection change if it's from a document buffer shown on current view and is managed by this plugin's lexer.
    if (isCurrentBufferManaged(static_cast<HWND>(notification->nmhdr.hwndFrom))) {
      keywordMatcher.match(static_cast<HWND>(notification->nmhdr.hwndFrom));
    }
  }

  void Plugin::handleContentUpdate(SCNotification* notification) {
    // Only handle content change if it's from a document buffer shown on current view and is managed by this plugin's lexer.
    if (lexerData && isCurrentBufferManaged(static_cast<HWND>(notification->nmhdr.hwndFrom))) {
      lexerData->changeEventData = std::make_tuple(static_cast<HWND>(notification->nmhdr.hwndFrom), notification->position, notification->linesAdded);
    }
  }

  void Plugin::onSettingsUpdated() {
    if (lexerData) {
      updateLexerDataGameSettings(Game::Skyrim, settings.compilerSettings.skyrim);
      updateLexerDataGameSettings(Game::SkyrimSE, settings.compilerSettings.sse);
      updateLexerDataGameSettings(Game::Fallout4, settings.compilerSettings.fo4);
    }
  }

  void Plugin::updateLexerDataGameSettings(Game game, const CompilerSettings::GameSettings& gameSettings) {
    if (lexerData) {
      lexerData->importDirectories[game].clear();
      std::wstringstream stream(gameSettings.importDirectories);
      std::wstring path;
      while (std::getline(stream, path, L';')) {
        lexerData->importDirectories[game].push_back(path);
      }
    }
  }

  void Plugin::detectLangID() {
    if (scriptLangID == 0) {
      std::wstring lexerName = string2wstring(LEXER_NAME, SC_CP_UTF8);
      for (npp_lang_type_t i = L_EXTERNAL; i < L_EXTERNAL + NB_MAX_EXTERNAL_LANG; ++i) {
        // Get language name of the given langID.
        npp_size_t langNameLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETLANGUAGENAME, i, 0));
        if (langNameLength > 0) {
          wchar_t* langNameCharArray = new wchar_t[langNameLength + 1];
          auto autoCleanup = gsl::finally([&] { delete[] langNameCharArray; });
          ::SendMessage(nppData._nppHandle, NPPM_GETLANGUAGENAME, i, reinterpret_cast<LPARAM>(langNameCharArray));
          std::wstring langName(langNameCharArray);

          if (langName == lexerName) {
            scriptLangID = i;

            // Update lexer's data as it needs to know if current file is lexed by it.
            if (lexerData) {
              lexerData->scriptLangID = scriptLangID;
            }
            break;
          }
        }
      }
    }
  }

  bool Plugin::isCurrentBufferManaged(HWND scintillaHandle) {
    // Check if current view matches Scintilla handle.
    npp_view_t currentView = static_cast<npp_view_t>(::SendMessage(nppData._nppHandle, NPPM_GETCURRENTVIEW, 0, 0));
    if ((currentView == MAIN_VIEW && scintillaHandle != nppData._scintillaMainHandle) || (currentView == SUB_VIEW && scintillaHandle != nppData._scintillaSecondHandle)) {
      return false;
    }

    // Make sure Papyrus script langID is detected.
    detectLangID();

    npp_lang_type_t currentFileLangID;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&currentFileLangID));
    return (currentFileLangID == scriptLangID);
  }

  std::pair<Game, bool> Plugin::detectGameType(const std::wstring& filePath, const CompilerSettings& compilerSettings) {
    Game detectedGameType = compilerSettings.gameMode;
    bool useAutoModeOutputDirectory = false;
    if (detectedGameType == Game::Auto) {
      for (int i = std::to_underlying(Game::Auto) + 1; i < static_cast<int>(game::games.size()); ++i) {
        auto game = static_cast<Game>(i);
        const CompilerSettings::GameSettings& gameSettings = compilerSettings.gameSettings(game);
        if (gameSettings.enabled && !gameSettings.installPath.empty() && utility::startsWith(filePath, gameSettings.installPath)) {
          detectedGameType = game;
          break;
        }
      }

      if (detectedGameType == Game::Auto) {
        // Can't detect, use auto mode default game instead.
        detectedGameType = compilerSettings.autoModeDefaultGame;
        useAutoModeOutputDirectory = true;
      }
    }

    return std::make_pair(detectedGameType, useAutoModeOutputDirectory);
  }

  void Plugin::clearActiveCompilation() {
    activeCompilationRequest = {
      .game = Game::Auto,
      .bufferID = 0
    };
    isComplingCurrentFile = false;
  }

  LRESULT CALLBACK Plugin::messageHandleProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    return papyrusPlugin.handleOwnMessage(window, message, wParam, lParam);
  }

  LRESULT Plugin::handleOwnMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
      case PPM_COMPILATION_DONE: {
        if (errorsWindow) {
          errorsWindow->clear();
          errorsWindow->hide();
        }

        std::wstring msg(L"Compilation ");
        if (wParam == PARAM_COMPILATION_WITH_ANONYMIZATION) {
          msg += L"and anonymization ";
        }
        msg += L"succeeded";
        if (!isComplingCurrentFile) {
          msg += L": " + activeCompilationRequest.filePath;
        }
        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(msg.c_str()));
        clearActiveCompilation();
        return 0;
      }

      case PPM_COMPILATION_FAILED: {
        if (errorsWindow) {
          errorsWindow->clear();
          if (wParam) {
            std::vector<Error>* errors = reinterpret_cast<std::vector<Error>*>(wParam);

            errorsWindow->show(*errors);

            if (errorAnnotator) {
              errorAnnotator->annotate(*errors);
            }
          }
        }

        std::wstring msg(L"Compilation failed");
        if (!isComplingCurrentFile) {
          msg += L": " + activeCompilationRequest.filePath;
        }
        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(msg.c_str()));
        clearActiveCompilation();

        if (lParam) {
          ::MessageBox(nppData._nppHandle, L"There are unparsable compilation errors.", PLUGIN_NAME L" error", MB_ICONEXCLAMATION | MB_OK);
        }
        return 0;
      }

      case PPM_COMPILER_NOT_FOUND: {
        clearActiveCompilation();
        ::MessageBox(nppData._nppHandle, L"Can't find the compiler executable", PLUGIN_NAME L" error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
      }

      case PPM_ANONYMIZATION_FAILED: {
        if (errorsWindow) {
          errorsWindow->clear();
        }

        std::wstring msg(L"Compilation succeeded but anonymization failed: ");
        msg += *reinterpret_cast<std::wstring*>(wParam);
        if (!isComplingCurrentFile) {
          msg += L" File: " + activeCompilationRequest.filePath;
        }
        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(msg.c_str()));
        clearActiveCompilation();
        return 0;
      }

      case PPM_OTHER_ERROR: {
        clearActiveCompilation();
        ::MessageBox(nppData._nppHandle, reinterpret_cast<wchar_t*>(wParam), reinterpret_cast<wchar_t*>(lParam), MB_ICONEXCLAMATION | MB_OK);
        return 0;
      }

      case PPM_JUMP_TO_ERROR: {
        Error* error = reinterpret_cast<Error*>(wParam);
        if (!error->file.empty()) {
          // Error message with file name. Check if the file is already being actively tracked.
          auto iter = std::find_if(activatedErrorsTrackingList.begin(), activatedErrorsTrackingList.end(),
            [&](const auto& comparisionError) {
              return comparisionError.file == error->file && comparisionError.line == error->line;
            }
          );
          if (iter == activatedErrorsTrackingList.end()) {
            // The most recent error selection always takes priority so push it to the front of the queue.
            activatedErrorsTrackingList.push_front(*error);
            ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(error->file.c_str()));
          }
        } else {
          // Generic error message that is not file specific. Show it in a message box.
          ::MessageBox(nppData._nppHandle, error->message.c_str(), PLUGIN_NAME L" Compilation Error", MB_OK);
        }
        return 0;
      }

      default: {
        return DefWindowProc(window, message, wParam, lParam);
      }
    }
  }

  bool Plugin::copyFile(const std::wstring& sourceFile, const std::wstring& destinationFile, int waitFor) {
    if (utility::fileExists(sourceFile)) {
      std::ofstream dest(destinationFile, std::ios::binary);
      auto autoCleanupDestStream = gsl::finally([&] { dest.close(); });

      if (dest.fail()) {
        // Likely a UAC issue since by default Notepad++ is installed under %PROGRAMFILES%. Try to execute copy command with administrator privilege.
        std::wstring infoMessage(L"Cannot write to " + destinationFile + L". Will run COPY command with elevated privilege. Please accept UAC prompt if any.");
        ::MessageBox(nppData._nppHandle, infoMessage.c_str(), PLUGIN_NAME L" Plugin", MB_ICONINFORMATION | MB_OK);
        std::wstring parameter(L"/c copy /y \"" + sourceFile + L"\" \"" + destinationFile + L"\"");
        SHELLEXECUTEINFO shellExecutionInfo {
          .cbSize = sizeof(SHELLEXECUTEINFO),
          .fMask = SEE_MASK_NOCLOSEPROCESS,
          .lpVerb = L"runas",
          .lpFile = L"cmd.exe",
          .lpParameters = parameter.c_str(),
          .nShow = SW_HIDE
        };
        if (::ShellExecuteEx(&shellExecutionInfo)) {
          ::WaitForSingleObject(shellExecutionInfo.hProcess, waitFor);
          ::CloseHandle(shellExecutionInfo.hProcess);
        }

        if (!utility::fileExists(destinationFile)) {
          std::wstring errorMessage(L"Fail to copy to " + destinationFile + L". Please manually copy " + sourceFile + L" to it.");
          ::MessageBox(nppData._nppHandle, errorMessage.c_str(), PLUGIN_NAME L" Plugin", MB_ICONEXCLAMATION | MB_OK);
          return false;
        }
      } else {
        std::ifstream source(sourceFile, std::ios::binary);
        auto autoCleanupSourceStream = gsl::finally([&] { source.close(); });

        if (!source.fail()) {
          dest << source.rdbuf();
        } else {
          std::wstring errorMessage(L"Cannot read " + sourceFile + L". Please check permission.");
          ::MessageBox(nppData._nppHandle, errorMessage.c_str(), PLUGIN_NAME L" Plugin", MB_ICONEXCLAMATION | MB_OK);
          return false;
        }
      }
    } else {
      std::wstring errorMessage(L"Cannot find " + sourceFile + L". Please make sure the full package is extracted in plugin folder.");
      ::MessageBox(nppData._nppHandle, errorMessage.c_str(), PLUGIN_NAME L" Plugin", MB_ICONEXCLAMATION | MB_OK);
      return false;
    }

    return true;
  }

  void Plugin::setupAdvancedMenu() {
    if (::SendMessage(nppData._nppHandle, NPPM_ALLOCATECMDID, advancedMenuItems.size(), reinterpret_cast<LPARAM>(&advancedMenuBaseCmdID)) != 0) {
      HMENU menu = reinterpret_cast<HMENU>(::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, 0, 0));
      HMENU advancedMenu = ::CreatePopupMenu();
      if (::ModifyMenu(menu, funcs[std::to_underlying(Menu::Advanced)]._cmdID, MF_BYCOMMAND | MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(advancedMenu), funcs[std::to_underlying(Menu::Advanced)]._itemName)) {
        for (UINT i = 0; i < advancedMenuItems.size(); ++i) {
          ::InsertMenu(advancedMenu, i, MF_BYPOSITION | MF_STRING, static_cast<UINT_PTR>(advancedMenuBaseCmdID) + i, advancedMenuItems[i]);
        }
      }
    }
  }

  void Plugin::showLangID() {
    detectLangID();
    if (scriptLangID > 0) {
      std::wstring msg(L"Assigned lexer langIDs are listed below\r\n\r\nPapyrus Script: "+ std::to_wstring(scriptLangID));
      ::MessageBox(nppData._nppHandle, msg.c_str(), PLUGIN_NAME L" Plugin", MB_ICONINFORMATION | MB_OK);
    } else {
      ::MessageBox(nppData._nppHandle, L"Cannot determine assigned langID!", PLUGIN_NAME L" Plugin", MB_ICONEXCLAMATION | MB_OK);
    }
  }

  void Plugin::installAutoCompletion() {
    // Get Notepad++'s plugin home path.
    npp_size_t homePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, 0, 0));
    if (homePathLength > 0) {
      wchar_t* homePathCharArray = new wchar_t[homePathLength + 1];
      auto autoCleanupHomePath = gsl::finally([&] { delete[] homePathCharArray; });
      ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, homePathLength + 1, reinterpret_cast<LPARAM>(homePathCharArray));
      std::wstring pluginHomePath(homePathCharArray);

      // Get Notepad++'s install path.
      wchar_t nppPath[MAX_PATH];
      if (::SendMessage(nppData._nppHandle, NPPM_GETNPPDIRECTORY, MAX_PATH, reinterpret_cast<LPARAM>(nppPath))) {
        std::wstring nppHomePath(nppPath);
        std::string autoCompletionConfigFileName = std::string(Lexer::name()) + ".xml";
        if (copyFile(std::filesystem::path(pluginHomePath) / + PLUGIN_NAME / "extras" / "autoCompletion" / autoCompletionConfigFileName, std::filesystem::path(nppHomePath) / "autoCompletion" / autoCompletionConfigFileName)) {
          ::MessageBox(nppData._nppHandle, L"Successfully copied auto completion config file. Please relaunch Notepad++ for it to take effect.", PLUGIN_NAME L" Plugin", MB_ICONINFORMATION | MB_OK);
        }
      }
    }
  }

  void Plugin::installFunctionList() {
    // Get Notepad++'s plugin home path.
    npp_size_t homePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, 0, 0));
    if (homePathLength > 0) {
      wchar_t* homePathCharArray = new wchar_t[homePathLength + 1];
      auto autoCleanupHomePath = gsl::finally([&] { delete[] homePathCharArray; });
      ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, homePathLength + 1, reinterpret_cast<LPARAM>(homePathCharArray));
      std::wstring pluginHomePath(homePathCharArray);

      // Get Notepad++'s plugin config folder.
      npp_size_t configPathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, 0, 0));
      if (configPathLength > 0) {
        wchar_t* configPathCharArray = new wchar_t[configPathLength + 1];
        auto autoCleanupConfigPath = gsl::finally([&] { delete[] configPathCharArray; });
        ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, configPathLength + 1, reinterpret_cast<LPARAM>(configPathCharArray));
        std::wstring configPath(configPathCharArray);
        std::string functionListConfigFileName = std::string(Lexer::name()) + ".xml";
        auto destinationDirectory = std::filesystem::path(configPath).parent_path().parent_path() / "functionList";
        if (copyFile(std::filesystem::path(pluginHomePath) / + PLUGIN_NAME / "extras" / "functionList" / functionListConfigFileName, destinationDirectory / functionListConfigFileName)) {
          std::string overrideMapFileName = (destinationDirectory / "overrideMap.xml").string();
          tinyxml2::XMLDocument xmlDoc;
          if (xmlDoc.LoadFile(overrideMapFileName.c_str()) == tinyxml2::XML_SUCCESS) {
            std::string papyrusScriptAssociationId(Lexer::name());
            papyrusScriptAssociationId += ".xml";

            bool found = false;
            tinyxml2::XMLHandle docHandle(&xmlDoc);
            tinyxml2::XMLHandle associationMapHandle = docHandle.FirstChildElement("NotepadPlus").FirstChildElement("functionList").FirstChildElement("associationMap");
            tinyxml2::XMLElement* associationElement = associationMapHandle.FirstChildElement("association").ToElement();
            while (associationElement) {
              auto id = associationElement->Attribute("id");
              if (id && utility::compare(papyrusScriptAssociationId, id)) {
                found = true;
                break;
              }
              associationElement = associationElement->NextSiblingElement();
            }

            bool needUpdate = false;
            if (found) {
              auto langID = associationElement->Attribute("langID");
              needUpdate = (!langID || std::stoi(langID) != scriptLangID);
            } else {
              // Create a new association element for Papyrus script language.
              associationElement = xmlDoc.NewElement("association");
              associationElement->SetAttribute("id", papyrusScriptAssociationId.c_str());

              tinyxml2::XMLElement* associationMapElement = associationMapHandle.ToElement();
              tinyxml2::XMLElement* prevFirstAssocaition = associationMapHandle.FirstChildElement("association").ToElement();
              if (prevFirstAssocaition) {
                // Insert as first child. Since tinyxml doesn't support "insert before", have to insert after first child then swap.
                associationMapElement->InsertAfterChild(prevFirstAssocaition, associationElement);
                associationMapElement->InsertAfterChild(associationElement, prevFirstAssocaition);
              } else {
                // There isn't any existing association element. Just insert a new one.
                associationMapElement->InsertEndChild(associationElement);
              }
              needUpdate = true;
            }

            if (needUpdate) {
              associationElement->SetAttribute("langID", scriptLangID);
              xmlDoc.SaveFile(overrideMapFileName.c_str());
            }
          }
          ::MessageBox(nppData._nppHandle, L"Successfully copied function list config file. Please relaunch Notepad++ for it to take effect.", PLUGIN_NAME L" Plugin", MB_ICONINFORMATION | MB_OK);
        }
      }
    }
  }

  void Plugin::compileMenuFunc() {
    papyrusPlugin.compile();
  }

  void Plugin::compile() {
    if (compiler) {
      if (activeCompilationRequest.bufferID == 0) {
        // Get current file path.
        wchar_t filePath[MAX_PATH];
        if (::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, reinterpret_cast<LPARAM>(filePath))) {
          // Check if current file is handled by Papyrus Script lexer.
          detectLangID();
          npp_lang_type_t currentFileLangID;
          ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&currentFileLangID));

          // Check file extension to make sure it is ".psc", and is lexed by this plugin's lexer or compiling unmanaged files is allowed.
          std::wstring currentFile(filePath);
          if (utility::endsWith(currentFile, L".psc") && (currentFileLangID == scriptLangID || settings.compilerSettings.allowUnmanagedSource)) {
            auto [detectedGame, useAutoModeOutputDirectory] = detectGameType(filePath, settings.compilerSettings);
            if (detectedGame != Game::Auto) {
              if (errorsWindow) {
                errorsWindow->clear();
                errorsWindow->hide();
              }

              if (errorAnnotator) {
                errorAnnotator->clear();
              }

              activeCompilationRequest = {
                .game = detectedGame,
                .bufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0),
                .filePath { currentFile },
                .useAutoModeOutputDirectory = useAutoModeOutputDirectory
              };
              isComplingCurrentFile = true;
              ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Compiling..."));
              ::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILE, 0, 0);

              compiler->start(activeCompilationRequest);
            } else {
              ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Cannot start compilation because no game is configured. Please at least enable one game in Settings dialog!"));
            }
          } else {
            ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"File is not a Papyrus script processed by this lexer!"));
          }
        } else {
          std::wstring errorMsg(L"Can't start compilation due to file path exceeding ");
          errorMsg += MAX_PATH + L" chars";
          ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(errorMsg.c_str()));
        }
      } else {
        if (activeCompilationRequest.bufferID == ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0)) {
          ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Already compiling!"));
        } else {
          std::wstring errorMsg(L"Can't start compilation due to active compilation of " + activeCompilationRequest.filePath);
          ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(errorMsg.c_str()));
        }
      }
    } else {
      ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Waiting for completing Papyrus settings..."));
    }
  }

  void Plugin::settingsMenuFunc() {
    papyrusPlugin.showSettings();
  }

  void Plugin::showSettings() {
    settingsDialog.doDialog([&]() {
      settings.saveSettings(settingsStorage);
      onSettingsUpdated();
    });
  }

  void Plugin::aboutMenuFunc() {
    papyrusPlugin.showAbout();
  }

  void Plugin::showAbout() {
    aboutDialog.doDialog();
  }

} // namespace
