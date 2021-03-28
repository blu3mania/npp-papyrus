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

#include "Settings.hpp"

#include "..\Common\EnumUtil.hpp"
#include "..\Common\Utility.hpp"
#include "..\CompilationErrorHandling\ErrorAnnotator.hpp"

#include <fstream>

namespace papyrus {

  bool Settings::loadSettings(SettingsStorage& storage, utility::Version currentVersion) {
    if (!storage.load()) {
      // This would initialize settings to current version's default
      readSettings(storage);
      storage.setVersion(currentVersion);
      return false;
    } else {
      // If settings are changed upon loading, or the stored settings are from an older version (in this case they should have been migrated by readSettings()), save the updated settings.
      if (readSettings(storage) || storage.getVersion() < currentVersion) {
        storage.setVersion(currentVersion);
        saveSettings(storage);
      }
      return true;
    }
  }

  void Settings::saveSettings(SettingsStorage& storage) {
    storage.putString(L"lexer.enableFoldMiddle", utility::boolToStr(lexerSettings.enableFoldMiddle));
    storage.putString(L"lexer.enableClassNameCache", utility::boolToStr(lexerSettings.enableClassNameCache));
    storage.putString(L"lexer.enableClassLink", utility::boolToStr(lexerSettings.enableClassLink));
    storage.putString(L"lexer.classLinkUnderline", utility::boolToStr(lexerSettings.classLinkUnderline));
    storage.putString(L"lexer.classLinkForegroundColor", utility::colorToHexStr(lexerSettings.classLinkForegroundColor));
    storage.putString(L"lexer.classLinkBackgroundColor", utility::colorToHexStr(lexerSettings.classLinkBackgroundColor));
    storage.putString(L"lexer.classLinkRequiresDoubleClick", utility::boolToStr(lexerSettings.classLinkRequiresDoubleClick));
    storage.putString(L"lexer.classLinkClickModifier", std::to_wstring(lexerSettings.classLinkClickModifier));

    storage.putString(L"keywordMatcher.enableKeywordMatching", utility::boolToStr(keywordMatcherSettings.enableKeywordMatching));
    storage.putString(L"keywordMatcher.enabledKeywords", std::to_wstring(keywordMatcherSettings.enabledKeywords));
    storage.putString(L"keywordMatcher.indicatorID", std::to_wstring(keywordMatcherSettings.indicatorID));
    storage.putString(L"keywordMatcher.matchedIndicatorStyle", std::to_wstring(keywordMatcherSettings.matchedIndicatorStyle));
    storage.putString(L"keywordMatcher.matchedIndicatorForegroundColor", utility::colorToHexStr(keywordMatcherSettings.matchedIndicatorForegroundColor));
    storage.putString(L"keywordMatcher.unmatchedIndicatorStyle", std::to_wstring(keywordMatcherSettings.unmatchedIndicatorStyle));
    storage.putString(L"keywordMatcher.unmatchedIndicatorForegroundColor", utility::colorToHexStr(keywordMatcherSettings.unmatchedIndicatorForegroundColor));

    storage.putString(L"errorAnnotator.enableAnnotation", utility::boolToStr(errorAnnotatorSettings.enableAnnotation));
    storage.putString(L"errorAnnotator.annotationForegroundColor", utility::colorToHexStr(errorAnnotatorSettings.annotationForegroundColor));
    storage.putString(L"errorAnnotator.annotationBackgroundColor", utility::colorToHexStr(errorAnnotatorSettings.annotationBackgroundColor));
    storage.putString(L"errorAnnotator.isAnnotationItalic", utility::boolToStr(errorAnnotatorSettings.isAnnotationItalic));
    storage.putString(L"errorAnnotator.isAnnotationBold", utility::boolToStr(errorAnnotatorSettings.isAnnotationBold));
    storage.putString(L"errorAnnotator.enableIndication", utility::boolToStr(errorAnnotatorSettings.enableIndication));
    storage.putString(L"errorAnnotator.indicatorID", std::to_wstring(errorAnnotatorSettings.indicatorID));
    storage.putString(L"errorAnnotator.indicatorStyle", std::to_wstring(errorAnnotatorSettings.indicatorStyle));
    storage.putString(L"errorAnnotator.indicatorForegroundColor", utility::colorToHexStr(errorAnnotatorSettings.indicatorForegroundColor));

    storage.putString(L"compiler.common.allowUnmanagedSource", utility::boolToStr(compilerSettings.allowUnmanagedSource));
    storage.putString(L"compiler.common.gameMode", game::gameNames[utility::underlying(compilerSettings.gameMode)].first);
    storage.putString(L"compiler.auto.defaultGame", game::gameNames[utility::underlying(compilerSettings.autoModeDefaultGame)].first);
    storage.putString(L"compiler.auto.outputDirectory", compilerSettings.autoModeOutputDirectory);

    saveGameSettings(storage, Game::Skyrim, compilerSettings.skyrim);
    saveGameSettings(storage, Game::SkyrimSE, compilerSettings.sse);
    saveGameSettings(storage, Game::Fallout4, compilerSettings.fo4);
    storage.save();
  }

  // Private methods
  //

  bool Settings::readSettings(const SettingsStorage& storage) {
    bool updated = false;
    std::wstring value;

    // Lexer settings
    //
    if (storage.getString(L"lexer.enableFoldMiddle", value)) {
      lexerSettings.enableFoldMiddle = utility::strToBool(value);
    } else {
      lexerSettings.enableFoldMiddle = true;
      updated = true;
    }

    if (storage.getString(L"lexer.enableClassNameCache", value)) {
      lexerSettings.enableClassNameCache = utility::strToBool(value);
    } else {
      lexerSettings.enableClassNameCache = false;
      updated = true;
    }

    if (storage.getString(L"lexer.enableClassLink", value)) {
      lexerSettings.enableClassLink = utility::strToBool(value);
    } else {
      lexerSettings.enableClassLink = true;
      updated = true;
    }

    if (storage.getString(L"lexer.classLinkUnderline", value)) {
      lexerSettings.classLinkUnderline = utility::strToBool(value);
    } else {
      lexerSettings.classLinkUnderline = true;
      updated = true;
    }

    if (storage.getString(L"lexer.classLinkForegroundColor", value)) {
      lexerSettings.classLinkForegroundColor = utility::hexStrToColor(value);
    } else {
      lexerSettings.classLinkForegroundColor = 0xFF0000; // BGR
      updated = true;
    }

    if (storage.getString(L"lexer.classLinkBackgroundColor", value)) {
      lexerSettings.classLinkBackgroundColor = utility::hexStrToColor(value);
    } else {
      lexerSettings.classLinkBackgroundColor = 0xFFFFFF; // BGR
      updated = true;
    }

    if (storage.getString(L"lexer.classLinkRequiresDoubleClick", value)) {
      lexerSettings.classLinkRequiresDoubleClick = utility::strToBool(value);
    } else {
      lexerSettings.classLinkRequiresDoubleClick = true;
      updated = true;
    }

    if (storage.getString(L"lexer.classLinkClickModifier", value)) {
      lexerSettings.classLinkClickModifier = std::stoi(value);
    } else {
      lexerSettings.classLinkClickModifier = SCMOD_CTRL;
      updated = true;
    }

    // Keyword matcher settings
    //
    if (storage.getString(L"keywordMatcher.enableKeywordMatching", value)) {
      keywordMatcherSettings.enableKeywordMatching = utility::strToBool(value);
    } else {
      keywordMatcherSettings.enableKeywordMatching = true;
      updated = true;
    }

    if (storage.getString(L"keywordMatcher.enabledKeywords", value)) {
      keywordMatcherSettings.enabledKeywords = std::stoi(value);
    } else {
      keywordMatcherSettings.enabledKeywords = KEYWORD_ALL;
      updated = true;
    }

    if (storage.getString(L"keywordMatcher.indicatorID", value)) {
      keywordMatcherSettings.indicatorID = std::stoi(value);
      if (keywordMatcherSettings.indicatorID < 9 || keywordMatcherSettings.indicatorID > 20) {
        keywordMatcherSettings.indicatorID = DEFAULT_MATCHER_INDICATOR;
        updated = true;
      }
    } else {
      keywordMatcherSettings.indicatorID = DEFAULT_MATCHER_INDICATOR;
      updated = true;
    }

    if (storage.getString(L"keywordMatcher.matchedIndicatorStyle", value)) {
      keywordMatcherSettings.matchedIndicatorStyle = std::stoi(value);
      if (keywordMatcherSettings.matchedIndicatorStyle > INDIC_GRADIENTCENTRE) {
        keywordMatcherSettings.matchedIndicatorStyle = INDIC_ROUNDBOX;
        updated = true;
      }
    } else {
      keywordMatcherSettings.matchedIndicatorStyle = INDIC_ROUNDBOX;
      updated = true;
    }

    if (storage.getString(L"keywordMatcher.matchedIndicatorForegroundColor", value)) {
      keywordMatcherSettings.matchedIndicatorForegroundColor = utility::hexStrToColor(value);
    } else {
      keywordMatcherSettings.matchedIndicatorForegroundColor = 0xFF0080; // BGR
      updated = true;
    }

    if (storage.getString(L"keywordMatcher.unmatchedIndicatorStyle", value)) {
      keywordMatcherSettings.unmatchedIndicatorStyle = std::stoi(value);
      if (keywordMatcherSettings.unmatchedIndicatorStyle > INDIC_GRADIENTCENTRE) {
        keywordMatcherSettings.unmatchedIndicatorStyle = INDIC_BOX;
        updated = true;
      }
    } else {
      keywordMatcherSettings.unmatchedIndicatorStyle = INDIC_BOX;
      updated = true;
    }

    if (storage.getString(L"keywordMatcher.unmatchedIndicatorForegroundColor", value)) {
      keywordMatcherSettings.unmatchedIndicatorForegroundColor = utility::hexStrToColor(value);
    } else {
      keywordMatcherSettings.unmatchedIndicatorForegroundColor = 0x0000FF; // BGR
      updated = true;
    }

    // Error annotator settings
    //
    if (storage.getString(L"errorAnnotator.enableAnnotation", value)) {
      errorAnnotatorSettings.enableAnnotation = utility::strToBool(value);
    } else {
      errorAnnotatorSettings.enableAnnotation = true;
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.annotationForegroundColor", value)) {
      errorAnnotatorSettings.annotationForegroundColor = utility::hexStrToColor(value);
    } else {
      errorAnnotatorSettings.annotationForegroundColor = 0x0000C0; // BGR
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.annotationBackgroundColor", value)) {
      errorAnnotatorSettings.annotationBackgroundColor = utility::hexStrToColor(value);
    } else {
      errorAnnotatorSettings.annotationBackgroundColor = 0xF0F0F0; // BGR
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.isAnnotationItalic", value)) {
      errorAnnotatorSettings.isAnnotationItalic = utility::strToBool(value);
    } else {
      errorAnnotatorSettings.isAnnotationItalic = true;
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.isAnnotationBold", value)) {
      errorAnnotatorSettings.isAnnotationBold = utility::strToBool(value);
    } else {
      errorAnnotatorSettings.isAnnotationBold = false;
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.enableIndication", value)) {
      errorAnnotatorSettings.enableIndication = utility::strToBool(value);
    } else {
      errorAnnotatorSettings.enableIndication = true;
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.indicatorID", value)) {
      errorAnnotatorSettings.indicatorID = std::stoi(value);
      if (errorAnnotatorSettings.indicatorID < 9 || errorAnnotatorSettings.indicatorID > 20) {
        errorAnnotatorSettings.indicatorID = DEFAULT_ERROR_INDICATOR;
        updated = true;
      }
    } else {
      errorAnnotatorSettings.indicatorID = DEFAULT_ERROR_INDICATOR;
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.indicatorStyle", value)) {
      errorAnnotatorSettings.indicatorStyle = std::stoi(value);
      if (errorAnnotatorSettings.indicatorStyle > INDIC_GRADIENTCENTRE) {
        errorAnnotatorSettings.indicatorStyle = INDIC_SQUIGGLEPIXMAP;
        updated = true;
      }
    } else {
      errorAnnotatorSettings.indicatorStyle = INDIC_SQUIGGLEPIXMAP;
      updated = true;
    }

    if (storage.getString(L"errorAnnotator.indicatorForegroundColor", value)) {
      errorAnnotatorSettings.indicatorForegroundColor = utility::hexStrToColor(value);
    } else {
      errorAnnotatorSettings.indicatorForegroundColor = 0x0000FF; // BGR
      updated = true;
    }

    // Game specific compiler settings
    //
    const std::vector<const wchar_t*> defaultSkyrimImportDirectories{ L"Data\\Scripts\\Source" };
    auto [skyrimConfigured, skyrimSettingsUpdated] = readGameSettings(storage, Game::Skyrim, compilerSettings.skyrim, defaultSkyrimImportDirectories, L"TESV_Papyrus_Flags.flg");
    updated = updated || skyrimSettingsUpdated;

    const std::vector<const wchar_t*>  defaultSseImportDirectories { L"Data\\Scripts\\Source", L"Data\\Source\\Scripts" };
    auto [sseConfigured, sseSettingsUpdated] = readGameSettings(storage, Game::SkyrimSE, compilerSettings.sse, defaultSseImportDirectories, L"TESV_Papyrus_Flags.flg");
    updated = updated || sseSettingsUpdated;

    const std::vector<const wchar_t*>  defaultFo4ImportDirectories { L"Data\\Scripts\\Source\\User", L"Data\\Scripts\\Source\\Base", L"Data\\Scripts\\Source" };
    auto [fo4Configured, fo4SettingsUpdated] = readGameSettings(storage, Game::Fallout4, compilerSettings.fo4, defaultFo4ImportDirectories, L"Institute_Papyrus_Flags.flg");
    updated = updated || fo4SettingsUpdated;

    // General compiler settings
    //
    if (storage.getString(L"compiler.common.allowUnmanagedSource", value)) {
      compilerSettings.allowUnmanagedSource = utility::strToBool(value);
    } else {
      compilerSettings.allowUnmanagedSource = false;
      updated = true;
    }

    if (storage.getString(L"compiler.common.gameMode", value)) {
      auto iter = game::gameAliases.find(value);
      if (iter != game::gameAliases.end()) {
        compilerSettings.gameMode = iter->second;

        // If a game was selected but is now disabled, change to auto mode
        if (compilerSettings.gameMode != Game::Auto && !compilerSettings.gameSettings(compilerSettings.gameMode).enabled) {
          compilerSettings.gameMode = Game::Auto;
          updated = true;
        }
      } else {
        compilerSettings.gameMode = Game::Auto;
        updated = true;
      }
    } else {
      compilerSettings.gameMode = Game::Auto;
      updated = true;
    }

    if (storage.getString(L"compiler.auto.defaultGame", value)) {
      auto iter = game::gameAliases.find(value);
      if (iter != game::gameAliases.end()) {
        compilerSettings.autoModeDefaultGame = iter->second;

        // If a game was selected but is now disabled, change to none so we can choose a suitable game
        if (compilerSettings.autoModeDefaultGame != Game::Auto && !compilerSettings.gameSettings(compilerSettings.autoModeDefaultGame).enabled) {
          compilerSettings.autoModeDefaultGame = Game::Auto;
          updated = true;
        }
      } else {
        compilerSettings.autoModeDefaultGame = Game::Auto;
        updated = true;
      }
    } else {
      compilerSettings.autoModeDefaultGame = Game::Auto;
      updated = true;
    }

    // If auto mode default game is not selected yet, check configured games and enabled games to make a best guess
    if (compilerSettings.autoModeDefaultGame == Game::Auto) {
      if (skyrimConfigured && compilerSettings.skyrim.enabled) {
        compilerSettings.autoModeDefaultGame = Game::Skyrim;
        updated = true;
      } else if (sseConfigured && compilerSettings.sse.enabled) {
        compilerSettings.autoModeDefaultGame = Game::SkyrimSE;
        updated = true;
      } else if (fo4Configured && compilerSettings.fo4.enabled) {
        compilerSettings.autoModeDefaultGame = Game::Fallout4;
        updated = true;
      }
    }

    if (storage.getString(L"compiler.auto.outputDirectory", value)) {
      compilerSettings.autoModeOutputDirectory = value;
    } else {
      compilerSettings.autoModeOutputDirectory = L"Scripts";
      updated = true;
    }

    return updated;
  }

  std::pair<bool, bool> Settings::readGameSettings(const SettingsStorage& storage, Game game, CompilerSettings::GameSettings& gameSettings, const std::vector<const wchar_t*>& defaultImportDirs, const wchar_t* defaultFlagFile) {
    bool gameConfigured = true;
    bool updated = false;
    std::wstring value;
    std::wstring gameSettingsPrefix(L"compiler." + game::gameNames[utility::underlying(game)].first + L'.');
    std::wstring gamePath = game::installationPath(game);

    // Enabled flag
    //
    if (storage.getString(gameSettingsPrefix + L"enabled", value)) {
      gameSettings.enabled = utility::strToBool(value);
    } else if (!gamePath.empty()) {
      gameSettings.enabled = true;
      updated = true;
    } else {
      gameConfigured = false;
    }

    // Install path
    //
    if (storage.getString(gameSettingsPrefix + L"installPath", value)) {
      gameSettings.installPath = value;
      if (gameSettings.installPath.empty() && !gamePath.empty()) {
        // Game is newly installed and not recognized previously. Enable it now
        gameSettings.enabled = true;
        updated = true;
      }
    } else if (gamePath.empty()) {
      gameConfigured = false;
    }
    
    if (gameSettings.installPath.empty() && !gamePath.empty()) {
      gameSettings.installPath = gamePath;
      updated = true;
    }

    if (gameSettings.installPath.empty() && gameSettings.enabled) {
      // Cannot find install path (possibly game was uninstalled). Disable the game
      gameSettings.enabled = false;
      updated = true;
    }

    // Compiler path
    //
    if (storage.getString(gameSettingsPrefix + L"compilerPath", value)) {
      gameSettings.compilerPath = value;
    } else if (gameSettings.installPath.empty()) {
      gameConfigured = false;
    }
    
    if (gameSettings.compilerPath.empty() && !gameSettings.installPath.empty()) {
      gameSettings.compilerPath = gameSettings.installPath + L"Papyrus Compiler\\PapyrusCompiler.exe";
      updated = true;
    }

    // Import directories
    //
    if (storage.getString(gameSettingsPrefix + L"importDirectories", value)) {
      gameSettings.importDirectories = value;
    } else if (gameSettings.installPath.empty()) {
      gameConfigured = false;
    }

    if (gameSettings.importDirectories.empty() && !gameSettings.installPath.empty()) {
      for (const wchar_t* importDirectory : defaultImportDirs) {
        gameSettings.importDirectories += gameSettings.installPath + importDirectory + L';';
      }
      if (!gameSettings.importDirectories.empty()) {
        gameSettings.importDirectories.pop_back(); // Remove the trailing ';'
      }
      updated = true;
    }

    // Output directory
    //
    if (storage.getString(gameSettingsPrefix + L"outputDirectory", value)) {
      gameSettings.outputDirectory = value;
    } else if (gameSettings.installPath.empty()) {
      gameConfigured =  false;
    }

    if (gameSettings.outputDirectory.empty() && !gameSettings.installPath.empty()) {
      gameSettings.outputDirectory = gameSettings.installPath + L"Data\\Scripts";
      updated = true;
    }

    // Flag file
    //
    if (storage.getString(gameSettingsPrefix + L"flagFile", value)) {
      gameSettings.flagFile = value;
    }

    if (gameSettings.flagFile.empty()) {
      gameSettings.flagFile = defaultFlagFile;
      updated = true;
    }

    // Additional arguments
    //
    if (storage.getString(gameSettingsPrefix + L"additionalArguments", value)) {
      gameSettings.additionalArguments = value;
    } else {
      gameSettings.additionalArguments.clear();
      updated = true;
    }

    // Anonynmize flag
    //
    if (storage.getString(gameSettingsPrefix + L"anonynmize", value)) {
      gameSettings.anonynmizeFlag = utility::strToBool(value);
    } else {
      gameSettings.anonynmizeFlag = true;
      updated = true;
    }

    // Optimize flag
    //
    if (storage.getString(gameSettingsPrefix + L"optimize", value)) {
      gameSettings.optimizeFlag = utility::strToBool(value);
    } else {
      gameSettings.optimizeFlag = true;
      updated = true;
    }

    // Release flag. Only applicable to Fallout 4
    //
    if (storage.getString(gameSettingsPrefix + L"release", value)) {
      gameSettings.releaseFlag = utility::strToBool(value);
    } else {
      gameSettings.releaseFlag = (game == Game::Fallout4);
      updated = true;
    }

    // Final flag. Only applicable to Fallout 4
    //
    if (storage.getString(gameSettingsPrefix + L"final", value)) {
      gameSettings.finalFlag = utility::strToBool(value);
    } else {
      gameSettings.finalFlag = (game == Game::Fallout4);
      updated = true;
    }

    return std::make_pair(gameConfigured, updated);
  }

  void Settings::saveGameSettings(SettingsStorage& storage, Game game, const CompilerSettings::GameSettings& gameSettings) {
    std::wstring gameSettingsPrefix(L"compiler." + game::gameNames[utility::underlying(game)].first + L'.');
    storage.putString(gameSettingsPrefix + L"enabled", utility::boolToStr(gameSettings.enabled));
    storage.putString(gameSettingsPrefix + L"installPath", gameSettings.installPath);
    storage.putString(gameSettingsPrefix + L"compilerPath", gameSettings.compilerPath);
    storage.putString(gameSettingsPrefix + L"importDirectories", gameSettings.importDirectories);
    storage.putString(gameSettingsPrefix + L"outputDirectory", gameSettings.outputDirectory);
    storage.putString(gameSettingsPrefix + L"flagFile", gameSettings.flagFile);
    storage.putString(gameSettingsPrefix + L"additionalArguments", gameSettings.additionalArguments);
    storage.putString(gameSettingsPrefix + L"anonynmize", utility::boolToStr(gameSettings.anonynmizeFlag));
    storage.putString(gameSettingsPrefix + L"optimize", utility::boolToStr(gameSettings.optimizeFlag));
    storage.putString(gameSettingsPrefix + L"release", utility::boolToStr(gameSettings.releaseFlag));
    storage.putString(gameSettingsPrefix + L"final", utility::boolToStr(gameSettings.finalFlag));
  }

} // namespace
