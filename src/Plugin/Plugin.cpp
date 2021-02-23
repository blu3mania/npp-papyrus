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
*/

#include "Plugin.hpp"

#include "Common\FinalAction.hpp"
#include "Common\Utility.hpp"
#include "Common\Version.hpp"
#include "Compiler\CompilationRequest.hpp"
#include "Lexer\Lexer.hpp"
#include "Lexer\LexerData.hpp"

#include "..\external\tinyxml2\tinyxml2.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

papyrus::Plugin papyrusPlugin;

namespace papyrus {

  // Static variables shared with Lexer
  std::unique_ptr<LexerData> lexerData;

  // Internal static variables
  namespace {
    std::vector<LPCWSTR> advancedMenuItems {
      L"Show langID",
      L"Add auto completion support",
      L"Add function list support"
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
    } {
  }

  void Plugin::onInit(HINSTANCE instance) {
    this->instance = instance;

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

  void Plugin::setNppData(NppData nppData) {
    this->nppData = nppData;
    initializeComponents();
  }

  void Plugin::onNotification(SCNotification* notification) {
    if ((notification->nmhdr.hwndFrom == nppData._scintillaMainHandle) || (notification->nmhdr.hwndFrom == nppData._scintillaSecondHandle)) {
      switch (notification->nmhdr.code) {
        case SCN_MODIFIED: {
          if (notification->modificationType & SC_MOD_INSERTTEXT || notification->modificationType & SC_MOD_DELETETEXT) {
            //::MessageBox(nppData._nppHandle, L"SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT", L"Papyrus", MB_OK);
          }
        }
        break;

        default: {
          break;
        }
      }
    } else if (notification->nmhdr.hwndFrom == nppData._nppHandle) {
      switch (notification->nmhdr.code) {
        case NPPN_BUFFERACTIVATED: {
          handleBufferActivation(notification->nmhdr.idFrom);
          break;
        }

        case NPPN_READY: {
          setupAdvancedMenu();
          break;
        }

        default: {
          break;
        }
      }
    }
  }

  LRESULT Plugin::handleNppMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
      case WM_COMMAND:
        // Menu command relayed by NPP
        UINT cmdId = static_cast<UINT>(wParam);
        if (cmdId >= advancedMenuBaseCmdID) {
          switch (cmdId - advancedMenuBaseCmdID) {
            case ShowLangID:
              showLangID();
              break;

            case AddAutoCompletion:
              addAutoCompletion();
              break;

            case AddFunctionList:
              addFunctionList();
              break;
          }
        }
        break;
    }
    return TRUE;
  }

  // Private methods
  //

  void Plugin::initializeComponents() {
    lexerData = std::make_unique<LexerData>(nppData, settings.lexerSettings);
    errorsWindow = std::make_unique<ErrorsWindow>(instance, nppData._nppHandle, messageWindow);
    errorAnnotator = std::make_unique<ErrorAnnotator>(nppData, settings.errorAnnotatorSettings);
    settingsDialog.init(instance, nppData._nppHandle);

    // Get Notepad++'s plugins config folder
    npp_size_t configPathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, 0, 0));
    if (configPathLength > 0) {
      wchar_t* configPathCharArray = new wchar_t[configPathLength + 1];
      auto autoCleanupConfigPath = utility::finally([&] { delete[] configPathCharArray; });
      ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, configPathLength + 1, reinterpret_cast<LPARAM>(configPathCharArray));
      std::wstring configPath(configPathCharArray);

      checkLexerConfigFile(configPath);

      // Load settings
      settingsStorage.init(std::filesystem::path(configPath) / PLUGIN_NAME L".ini");
      if (!settings.loadSettings(settingsStorage, utility::Version(PLUGIN_VERSION))) {
        // Settings didn't exist. Default settings initialized
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
  }

  void Plugin::checkLexerConfigFile(const std::wstring& configPath) {
    // Check if lexer configuration file is in config folder
    std::wstring lexerConfigFile = std::filesystem::path(configPath) / PLUGIN_NAME L".xml";
    if (!utility::fileExists(lexerConfigFile)) {
      // Lexer configuration file doesn't exist, try to generate one from the copy extracted from package
      npp_size_t homePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, 0, 0));
      if (homePathLength > 0) {
        wchar_t* homePathCharArray = new wchar_t[homePathLength + 1];
        auto autoCleanupHomePath = utility::finally([&] { delete[] homePathCharArray; });
        ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, homePathLength + 1, reinterpret_cast<LPARAM>(homePathCharArray));
        std::wstring pluginHomePath(homePathCharArray);

        if (!copyFile(std::filesystem::path(pluginHomePath) / PLUGIN_NAME / PLUGIN_NAME L".xml", lexerConfigFile)) {
          // Can't generate lexer configuration file. Mark lexer as unusable
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

  void Plugin::handleBufferActivation(npp_buffer_t bufferID) {
    // Make sure Papyrus script langID is detected
    if (scriptLangID == 0) {
      detectLangID();
    }

    npp_view_t currentView = static_cast<npp_view_t>(::SendMessage(nppData._nppHandle, NPPM_GETCURRENTVIEW, 0, 0));
    npp_size_t filePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, static_cast<WPARAM>(bufferID), 0));
    if (filePathLength > 0) {
      wchar_t* filePathCharArray = new wchar_t[filePathLength + 1];
      auto autoCleanup = utility::finally([&] { delete[] filePathCharArray; });
      if (::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, static_cast<WPARAM>(bufferID), reinterpret_cast<LPARAM>(filePathCharArray)) != -1) {
        std::wstring filePath(filePathCharArray);

        // Set detected game for lexer
        auto [detectedGame, useAutoModeOutputDirectory] = detectGameType(filePath, settings.compilerSettings);
        lexerData->currentGame = detectedGame;

        // Check if active compilation file is still the current one on either view
        if (activeCompilationRequest.bufferID != 0) {
          isComplingCurrentFile = utility::compare(activeCompilationRequest.filePath, filePath);
          if (isComplingCurrentFile) {
            ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Compiling..."));
          }
        }

        // Check if we are waiting for a file to open as a result of user selecting an error from list
        if (!activatedErrorsTrackingList.empty()) {
          // Check if activated file is in the tracking list
          auto iter = std::find_if(activatedErrorsTrackingList.begin(), activatedErrorsTrackingList.end(),
            [&](Error& error) {
              return utility::compare(error.file, filePath);
            }
          );
          if (iter != activatedErrorsTrackingList.end()) {
            // Scintilla's line numberis zero-based
            int line = iter->line - 1;
            HWND handle = (currentView == MAIN_VIEW) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

            // When the buffer is big, asking Scintilla to scroll immediately doesn't always work, so use a short timer
            jumpToErrorLineTimer = utility::startTimer(100, [=] { ::SendMessage(handle, SCI_GOTOLINE, line, 0); });

            // Get rid of other errors in the list for the same file
            while (iter != activatedErrorsTrackingList.end()) {
              activatedErrorsTrackingList.erase(iter++);
              iter = std::find_if(iter, activatedErrorsTrackingList.end(),
                [&](Error& error) {
                  return utility::compare(error.file, filePath);
                }
              );
            }
          }
        }

        npp_lang_type_t currentFileLangID;
        ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&currentFileLangID));
        if (utility::endsWith(filePath, L".psc")) {
          bool updateAnnotation = false;
          if (currentFileLangID == scriptLangID) {
            // Papyrus script file lexed by this plugin's lexer, need to check/update annotation.
            updateAnnotation = true;

            // If not compiling current file, check its game type (if applicable)
            if (!isComplingCurrentFile && detectedGame != game::Auto) {
              std::wstring gameSpecificStatus(L"[" + game::gameNames[detectedGame].second + L"] " + Lexer::statusText());
              ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(gameSpecificStatus.c_str()));
            }
          }

          // Even if the file is not lexed by this plugin's lexer, it might be compiled when compiling unmanaged files are allowed
          if (errorAnnotator && (updateAnnotation || settings.compilerSettings.allowUnmanagedSource)) {
            errorAnnotator->annotate(currentView, filePath);
          }
        }
      }
    }
 }

  void Plugin::onSettingsUpdated() {
    if (lexerData) {
      updateLexerDataGameSettings(game::Skyrim, settings.compilerSettings.skyrim);
      updateLexerDataGameSettings(game::SkyrimSE, settings.compilerSettings.sse);
      updateLexerDataGameSettings(game::Fallout4, settings.compilerSettings.fo4);
    }
  }

  void Plugin::updateLexerDataGameSettings(game::Game game, const CompilerSettings::GameSettings& gameSettings) {
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
          auto autoCleanup = utility::finally([&] { delete[] langNameCharArray; });
          ::SendMessage(nppData._nppHandle, NPPM_GETLANGUAGENAME, i, reinterpret_cast<LPARAM>(langNameCharArray));
          std::wstring langName(langNameCharArray);

          if (langName == lexerName) {
            scriptLangID = i;

            // Update lexer's data as it needs to know if current file is lexed by it
            if (lexerData) {
              lexerData->scriptLangID = scriptLangID;
            }
            break;
          }
        }
      }
    }
  }

  std::pair<game::Game, bool> Plugin::detectGameType(const std::wstring& filePath, const CompilerSettings& compilerSettings) {
    game::Game detectedGameType = compilerSettings.gameMode;
    bool useAutoModeOutputDirectory = false;
    if (detectedGameType == game::Auto) {
      for (int i = static_cast<int>(game::Auto) + 1; i < static_cast<int>(game::games.size()); i++) {
        auto game = static_cast<game::Game>(i);
        auto gameSettings = compilerSettings.gameSettings(game);
        if (gameSettings.enabled && !gameSettings.installPath.empty() && utility::startsWith(filePath, gameSettings.installPath)) {
          detectedGameType = game;
          break;
        }
      }

      if (detectedGameType == game::Auto) {
        // Can't detect, use auto mode default game instead
        detectedGameType = compilerSettings.autoModeDefaultGame;
        useAutoModeOutputDirectory = true;
      }
    }

    return std::pair<game::Game, bool>(detectedGameType, useAutoModeOutputDirectory);
  }

  void Plugin::clearActiveCompilation() {
    activeCompilationRequest = {
      .game = game::Auto,
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
        msg += L"successful";
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

        std::wstring msg(L"Compilation successful but anonymization failed: ");
        msg += reinterpret_cast<const wchar_t*>(wParam);
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
        Error* error = reinterpret_cast<Error*>(lParam);
        auto iter = std::find_if(activatedErrorsTrackingList.begin(), activatedErrorsTrackingList.end(),
          [&](Error& comparisionError) {
            return comparisionError.file == error->file && comparisionError.line == error->line;
          }
        );
        if (iter == activatedErrorsTrackingList.end()) {
          // The most recent error selection always takes priority so push it to the front of the queue
          activatedErrorsTrackingList.push_front(*error);
          ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(&error->file[0]));
        }
        return 0;
      }

      default: {
        return DefWindowProc(window, message, wParam, lParam);
      }
    }
  }

  bool Plugin::copyFile(std::wstring sourceFile, std::wstring destinationFile, int waitFor) {
    if (utility::fileExists(sourceFile)) {
      std::ofstream dest(destinationFile, std::ios::binary);
      auto autoCleanupDestStream = utility::finally([&] {
        dest.close();
      });

      if (dest.fail()) {
        // Likely a UAC issue since by default Notepad++ is installed under %PROGRAMFILES%. Try to execute copy command with administrator privilege.
        std::wstring msg(L"Cannot write to " + destinationFile + L". Will run COPY command with elevated privilege. Please accept UAC prompt if any.");
        ::MessageBox(nppData._nppHandle, msg.c_str(), PLUGIN_NAME L" Plugin", MB_ICONINFORMATION | MB_OK);
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
          std::wstring msg(L"Fail to copy to " + destinationFile + L". Please manually copy " + sourceFile + L" to it.");
          ::MessageBox(nppData._nppHandle, msg.c_str(), PLUGIN_NAME L" Plugin", MB_ICONINFORMATION | MB_OK);
          return false;
        }
      } else {
        std::ifstream source(sourceFile, std::ios::binary);
        auto autoCleanupSourceStream = utility::finally([&] {
          source.close();
        });

        if (!source.fail()) {
          dest << source.rdbuf();
        } else {
          std::wstring msg(L"Cannot read " + sourceFile + L". Please check permission.");
          ::MessageBox(nppData._nppHandle, msg.c_str(), PLUGIN_NAME L" Plugin", MB_ICONEXCLAMATION | MB_OK);
          return false;
        }
      }
    } else {
      std::wstring msg(L"Cannot find " + sourceFile + L". Please make sure the full package is extracted in plugin folder.");
      ::MessageBox(nppData._nppHandle, msg.c_str(), PLUGIN_NAME L" Plugin", MB_ICONEXCLAMATION | MB_OK);
      return false;
    }

    return true;
  }

  void Plugin::setupAdvancedMenu() {
    if (::SendMessage(nppData._nppHandle, NPPM_ALLOCATECMDID, advancedMenuItems.size(), reinterpret_cast<LPARAM>(&advancedMenuBaseCmdID)) != 0) {
      HMENU menu = reinterpret_cast<HMENU>(::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, 0, 0));
      HMENU advancedMenu = ::CreatePopupMenu();
      if (::ModifyMenu(menu, funcs[Advanced]._cmdID, MF_BYCOMMAND | MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(advancedMenu), funcs[Advanced]._itemName)) {
        for (UINT i = 0; i < advancedMenuItems.size(); i++) {
          ::InsertMenu(advancedMenu, i, MF_BYPOSITION, advancedMenuBaseCmdID + i, advancedMenuItems[i]);
        }
      }
    }
  }

  void Plugin::advancedMenuFunc() {
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

  void Plugin::addAutoCompletion() {
    // Get Notepad++'s plugin home path
    npp_size_t homePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, 0, 0));
    if (homePathLength > 0) {
      wchar_t* homePathCharArray = new wchar_t[homePathLength + 1];
      auto autoCleanupHomePath = utility::finally([&] { delete[] homePathCharArray; });
      ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, homePathLength + 1, reinterpret_cast<LPARAM>(homePathCharArray));
      std::wstring pluginHomePath(homePathCharArray);

      // Get Notepad++'s install path
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

  void Plugin::addFunctionList() {
    // Get Notepad++'s plugin home path
    npp_size_t homePathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, 0, 0));
    if (homePathLength > 0) {
      wchar_t* homePathCharArray = new wchar_t[homePathLength + 1];
      auto autoCleanupHomePath = utility::finally([&] { delete[] homePathCharArray; });
      ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, homePathLength + 1, reinterpret_cast<LPARAM>(homePathCharArray));
      std::wstring pluginHomePath(homePathCharArray);

      // Get Notepad++'s plugin config folder
      npp_size_t configPathLength = static_cast<npp_size_t>(::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, 0, 0));
      if (configPathLength > 0) {
        wchar_t* configPathCharArray = new wchar_t[configPathLength + 1];
        auto autoCleanupConfigPath = utility::finally([&] { delete[] configPathCharArray; });
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
              // Create a new association element for Papyrus script language
              associationElement = xmlDoc.NewElement("association");
              associationElement->SetAttribute("id", papyrusScriptAssociationId.c_str());

              tinyxml2::XMLElement* associationMapElement = associationMapHandle.ToElement();
              tinyxml2::XMLElement* prevFirstAssocaition = associationMapHandle.FirstChildElement("association").ToElement();
              if (prevFirstAssocaition) {
                // Insert as first child. Since tinyxml doesn't support "insert before", have to insert after first child then swap
                associationMapElement->InsertAfterChild(prevFirstAssocaition, associationElement);
                associationMapElement->InsertAfterChild(associationElement, prevFirstAssocaition);
              } else {
                // There isn't any existing association element. Just insert a new one
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
          // Check if current file is handled by Papyrus Script lexer
          if (scriptLangID == 0) {
            detectLangID();
          }
          npp_lang_type_t currentFileLangID;
          ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&currentFileLangID));

          // Check file extension to make sure it is ".psc", and is lexed by this plugin's lexer or compiling unmanaged files is allowed
          std::wstring currentFile(filePath);
          if (utility::endsWith(currentFile, L".psc") && (currentFileLangID == scriptLangID || settings.compilerSettings.allowUnmanagedSource)) {
            auto [detectedGame, useAutoModeOutputDirectory] = detectGameType(filePath, settings.compilerSettings);
            if (detectedGame != game::Auto) {
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
    settingsDialog.doDialog();
    settings.saveSettings(settingsStorage);
    onSettingsUpdated();
  }

  void Plugin::aboutMenuFunc() {
    papyrusPlugin.showAbout();
  }

  void Plugin::showAbout() {
    ::MessageBox(nppData._nppHandle,
      PLUGIN_NAME " " PLUGIN_VERSION " for Notepad++\r\n"
      "Copyright 2016 - 2021\r\n"
      "All Rights Reserved\r\n"
      "\r\n"
      "Contributors:\r\n"
      "Tschilkroete (original author)\r\n"
      "blu3mania\r\n"
      "\r\n"
      "This plugin is licensed under the GNU General Public Licence 3 https://www.gnu.org/licenses/gpl-3.0.txt\r\n"
      "Get the source code: https://github.com/blu3mania/npp-papyrus \r\n"
      "\r\n"
      "This plugin includes source code (original/modified) from the following libraries:\r\n"
      "Notepad++: see ? -> About Notepad++ for more information.\r\n"
      "Scintilla: https://www.scintilla.org/\r\n"
      "TinyXML2: http://www.grinninglizard.com/tinyxml2/\r\n"
      "GSL: https://github.com/microsoft/GSL\r\n",
      L"About " PLUGIN_NAME " Plugin", MB_OK);
  }

} // namespace
