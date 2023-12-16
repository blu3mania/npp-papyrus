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

#include "Lexer.hpp"

#include "LexerIDs.hpp"
#include "..\Common\FileSystemUtil.hpp"
#include "..\Common\Logger.hpp"
#include "..\Common\StringUtil.hpp"

#include "..\..\external\gsl\include\gsl\util"
#include "..\..\external\lexilla\LexerModule.h"
#include "..\..\external\npp\Common.h"
#include "..\..\external\scintilla\Scintilla.h"

#include <filesystem>
#include <map>
#include <memory>

namespace papyrus {

  using Helper = Lexer::Helper;
  using Lock = std::lock_guard<std::mutex>;

  // Static shared helper and other lexer data
  namespace {
    std::unique_ptr<Helper> helper;
    std::mutex lexerListMutex;
    std::vector<Lexer*> lexerList;
    std::mutex scriptNameMapMutex;
    std::map<npp_buffer_t, std::string> scriptNameMap;
  }

  Lexer::Lexer()
    : SimpleLexerBase(LEXER_NAME, SCLEX_PAPYRUS_SCRIPT),
      instreWordLists{&wordListOperators, &wordListFlowControl},
      typeWordLists{&wordListTypes, &wordListKeywords, &wordListKeywords2, &wordListFoldOpen, &wordListFoldMiddle, &wordListFoldClose} {
    // Setup settings change listeners.
    if (isUsable() && !helper) {
      helper = std::make_unique<Helper>();
    }

    hoverEventSubscription = lexerData->hoverEventData.subscribe([&](auto eventData) {
      if (isUsable()) {
        detectBufferId();
        if (bufferID == eventData.bufferID) {
          // Mouse hovering over a word in current file.
          handleMouseHover(eventData.scintillaHandle, eventData.hovering, eventData.position);
        }
      }
    });

    changeEventSubscription = lexerData->changeEventData.subscribe([&](auto eventData) {
      if (isUsable()) {
        detectBufferId();
        if (bufferID == eventData.bufferID) {
          // Change happened on current file.
          handleContentChange(eventData.scintillaHandle, eventData.position, eventData.linesAdded);
        }
      }
    });

    // Add this instance to lexer list
    Lock lock(lexerListMutex);
    lexerList.push_back(this);
  }

  Lexer::~Lexer() {
    hoverEventSubscription->unsubscribe();
    changeEventSubscription->unsubscribe();

    // Remove this instance from lexer list
    Lock lock(lexerListMutex);
    auto iter = std::find(lexerList.begin(), lexerList.end(), this);
    if (iter != lexerList.end()) {
      lexerList.erase(iter);
    }
  }

  void Lexer::assignBufferID(npp_buffer_t bufferID) {
    Lock lock(lexerListMutex);
    if (!lexerList.empty()) {
      // Since NPPN_EXTERNALLEXERBUFFER always happens right after a lexer is instantiated, we only need to check the last instance in lexer list.
      // Though, since this message can be received when another plugin's lexer is instantiated, we should prevent overwriting an already assigned buffer ID.
      Lexer* pLexer = lexerList.back();
      if (pLexer->bufferID == 0) {
        pLexer->bufferID = bufferID;
      }
    }
  }

  std::string Lexer::getScriptName(npp_buffer_t bufferID) {
    Lock lock(scriptNameMapMutex);
    utility::logger.log(L"[Retrieve] Buffer ID: " +  std::to_wstring(bufferID));
    if (scriptNameMap.contains(bufferID)) {
      utility::logger.log(L"[Retrieve] Script name: " + string2wstring(scriptNameMap[bufferID], SC_CP_UTF8));
      return scriptNameMap[bufferID];
    } else {
      return std::string();
    }
  }

  void SCI_METHOD Lexer::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int, IDocument* pAccess) {
    if (isUsable()) {
      detectBufferId();

      Accessor accessor(pAccess, nullptr);
      StyleContext styleContext(startPos, lengthDoc, accessor.StyleAt(startPos - 1), accessor);

      // This state is saved in the line feed character. It can be used to initialize the state of the next line.
      State messageStateLast = static_cast<State>(accessor.StyleAt(startPos - 1));
      for (auto line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc - 1); ++line) {
        auto tokens = tokenize(accessor, line);
        State messageState = messageStateLast;

        // Styling
        for (auto iterTokens = tokens.begin(); iterTokens != tokens.end(); ++iterTokens) {
          const auto& tokenString = iterTokens->content;

          if (messageState == State::CommentDoc) {
            colorToken(styleContext, *iterTokens, State::CommentDoc);
            if (tokenString == "}") {
              messageState = State::Default;
            }
          } else if (messageState == State::CommentMultiLine) {
            colorToken(styleContext, *iterTokens, State::CommentMultiLine);
              // A multi-line comment ends with "/;" and there can't be spaces in between.
            if (tokenString == ";" && iterTokens != tokens.begin() && std::prev(iterTokens)->content == "/" && iterTokens->startPos == std::prev(iterTokens)->startPos + 1) {
              messageState = State::Default;
            }
          } else if (messageState == State::Comment) {
            colorToken(styleContext, *iterTokens, State::Comment);
          } else if (messageState == State::String) {
            colorToken(styleContext, *iterTokens, State::String);
            if (tokenString == "\"") {
              // This may be an escape for double quote. Check previous tokens.
              int numBackslash = 0;
              auto iterCheck = iterTokens;
              while (iterCheck != tokens.begin()) {
                if ((--iterCheck)->content == "\\") {
                  numBackslash++;
                } else {
                  break;
                }
              }
              if (numBackslash % 2 == 0) {
                messageState = State::Default;
              }
            }
          } else {
            // Determine the type of the token and color it.
            if (tokenString == "{") {
              colorToken(styleContext, *iterTokens, messageState = State::CommentDoc);
            } else if (tokenString == ";") {
              // A multi-line comment starts with ";/" and there can't be spaces in between.
              if (std::next(iterTokens) != tokens.end() && std::next(iterTokens)->content == "/" && iterTokens->startPos == std::next(iterTokens)->startPos - 1) {
                colorToken(styleContext, *iterTokens, messageState = State::CommentMultiLine);
              } else {
                colorToken(styleContext, *iterTokens, messageState = State::Comment);
              }
            } else if (tokenString == "\"") {
              colorToken(styleContext, *iterTokens, messageState = State::String);
            } else if (iterTokens->tokenType == TokenType::Numeric) {
              colorToken(styleContext, *iterTokens, State::Number);
            } else if (iterTokens->tokenType == TokenType::Identifier) {
              if (!wordListFlowControl.InList(tokenString.c_str()) && isalnum(tokenString.back()) && std::next(iterTokens) != tokens.end() && std::next(iterTokens)->content == "(") {
                // If next token is ( and current token is an identifier but not if/elseif/while, it is a function name.
                colorToken(styleContext, *iterTokens, State::Function);
              } else if (wordListTypes.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, State::Type);
              } else if (wordListFlowControl.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, State::FlowControl);
              } else if (wordListKeywords.InList(tokenString.c_str())) {
                // Check if a new property needs to be added, and update existing property list
                if (tokenString == "scriptname" && std::next(iterTokens) != tokens.end()) {
                  const auto& fullScriptName = std::next(iterTokens)->content;
                  auto detectedScriptName = utility::split(fullScriptName, ":").back();
                  if (!utility::compare(scriptName, detectedScriptName)) {
                    scriptName = detectedScriptName;
                    detectBufferId();

                    // Add full script name to map
                    Lock lock(scriptNameMapMutex);
                    scriptNameMap[bufferID] = fullScriptName;
                  }
                } else if (tokenString == "property" && std::next(iterTokens) != tokens.end() && std::next(iterTokens)->content != ";") {
                  std::string propertyName = std::next(iterTokens)->content;
                  auto iter = std::find_if(propertyLines.begin(), propertyLines.end(),
                    [&](const auto& property) {
                      return property.name == propertyName;
                    }
                  );
                  if (iter != propertyLines.end()) {
                    // See if there are properties marked as need to re-check due to line addition.
                    if (iter->line < line) {
                      iter = std::find_if(++iter, propertyLines.end(),
                        [&](const auto& property) {
                         return property.name == propertyName && iter->line > line; // Due to line addition
                        }
                      );
                    }
                    if (iter != propertyLines.end() && iter->needRecheck) {
                      iter->line = line;
                      iter->needRecheck = false;
                    }
                  } else {
                    Property property {
                      .name = propertyName,
                      .line = line
                    };
                    propertyLines.push_back(property);
                    propertyNames.insert(propertyName);
                  }
                }

                colorToken(styleContext, *iterTokens, State::Keyword);
              } else if (wordListKeywords2.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, State::Keyword2);
              } else if (wordListOperators.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, State::Operator);
              } else {
                bool found = (propertyNames.find(tokenString) != propertyNames.end());
                if (found) {
                  colorToken(styleContext, *iterTokens, State::Property);
                } else {
                  if (lexerData->currentGame != game::Game::Auto) {
                    if (lexerData->settings.enableClassNameCache) {
                      auto& currentGameClassNames = helper->getClassNamesForGame(lexerData->currentGame);
                      if (isNameInCache(tokenString, currentGameClassNames.first, currentGameClassNames.second)) {
                        colorToken(styleContext, *iterTokens, State::Class);
                        found = true;
                      } else {
                        auto& currentGameNonClassNames = helper->getNonClassNamesForGame(lexerData->currentGame);
                        if (!isNameInCache(tokenString, currentGameNonClassNames.first, currentGameNonClassNames.second)) {
                          if (!getClassFilePath(bufferID, tokenString).empty()) {
                            colorToken(styleContext, *iterTokens, State::Class);
                            addNameToCache(tokenString, currentGameClassNames.first, currentGameClassNames.second);
                            found = true;
                          }

                          if (!found) {
                            addNameToCache(tokenString, currentGameNonClassNames.first, currentGameNonClassNames.second);
                          }
                        }
                      }
                    } else if (!getClassFilePath(bufferID, tokenString).empty()) {
                        colorToken(styleContext, *iterTokens, State::Class);
                        found = true;
                    }
                  }

                  if (!found) {
                    colorToken(styleContext, *iterTokens, State::Default);
                  }
                }
              }
            } else if (iterTokens->tokenType == TokenType::Special) {
              if (wordListOperators.InList(iterTokens->content.c_str())) {
                colorToken(styleContext, *iterTokens, State::Operator);
              } else {
                colorToken(styleContext, *iterTokens, State::Default);
              }
            }
          }
        }
        if (messageState == State::Comment || messageState == State::String) {
          messageState = State::Default;
        }
        if (styleContext.ch == '\r') {
          styleContext.Forward();
        }
        if (styleContext.ch == '\n') {
          styleContext.SetState(std::to_underlying(messageState));
          styleContext.Forward();
        }
        messageStateLast = messageState;
      }
      styleContext.Complete();
    }
  }

  void SCI_METHOD Lexer::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int, IDocument* pAccess) {
    if (isUsable()) {
      Accessor accessor(pAccess, nullptr);

      int levelPrev = accessor.LevelAt(accessor.GetLine(startPos)) & SC_FOLDLEVELNUMBERMASK;
      // Lines
      for (auto line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc); ++line) {
        int numFoldOpen = 0;
        int numFoldClose = 0;
        bool hasFoldMiddle = false;
        // Chars
        auto tokens = tokenize(accessor, line);
        for (const Token& token : tokens) {
          if (!isComment(accessor.StyleAt(token.startPos)) && accessor.StyleAt(token.startPos) != std::to_underlying(State::String)) {
            if (wordListFoldOpen.InList(token.content.c_str())) {
              numFoldOpen++;
            } else if (wordListFoldClose.InList(token.content.c_str())) {
              numFoldClose++;
            } else if (lexerData->settings.enableFoldMiddle && wordListFoldMiddle.InList(token.content.c_str())) {
              hasFoldMiddle = true;
            }
          }
        }

        // Skip the lines that have matching start and end keywords.
        int level = levelPrev;
        int levelDelta = numFoldOpen - numFoldClose;
        if (levelDelta > 0) {
          level |= SC_FOLDLEVELHEADERFLAG;
        }
        if (hasFoldMiddle && numFoldOpen == 0 && numFoldClose == 0) {
          level--;
          level |= SC_FOLDLEVELHEADERFLAG;
        }
        accessor.SetLevel(line, level);
        levelPrev += levelDelta;
      }
    }
  }

  // Protected methods
  //

  bool Lexer::isUsable() const {
    return helper->isUsable();
  }

  // Private methods
  //

  std::vector<Lexer::Token> Lexer::tokenize(Accessor& accessor, Sci_Position line) const {
    std::vector<Token> tokens;
    TokenType previousTokenType = TokenType::Special;
    auto index = accessor.LineStart(line);
    auto indexNext = index;
    int ch = getNextChar(accessor, index, indexNext);
    while (index < accessor.LineEnd(line)) {
      if (ch == '\r' || ch == '\n') {
        break;
      }

      bool processed = false;
      if (ch <= 255) {
        if (std::isblank(ch)) {
          ch = getNextChar(accessor, index, indexNext);
          processed = true;
        } else if (std::isalpha(ch) || ch == '_') {
          Token token {
            .tokenType = TokenType::Identifier,
            .startPos = index
          };
          while (ch <= 255 && (std::isalnum(ch) || ch == '_' || ch == ':')) {
            token.content.push_back(std::tolower(ch)); // Papyrus script is case insensitive
            ch = getNextChar(accessor, index, indexNext);
          }
          tokens.push_back(token);
          previousTokenType = token.tokenType;
          processed = true;
        } else if (std::isdigit(ch) || (ch == '-' && previousTokenType == TokenType::Special)) { // For a minus sign to be treated as leading minus sign rather than minus operator, previous token cannot be an identifier or a number
          Token token {
            .tokenType = TokenType::Numeric,
            .startPos = index
          };
          bool hasDigit = false;
          while (ch <= 255
            && (std::isdigit(ch)
              || (ch == '-' && index == token.startPos) // leading minus sign
              || (ch == '.' && hasDigit) // decimal point after at least a digit
              || ((ch == 'x' || ch == 'X') && index == token.startPos + 1 && token.content.front() == '0') // 0x
              || (std::isxdigit(ch) && token.content.size() > 1 && std::tolower(token.content.at(1)) == 'x'))) { // hex value after 0x
            token.content.push_back(std::tolower(ch));
            if (!hasDigit && std::isdigit(ch)) {
              hasDigit = true;
            }
            ch = getNextChar(accessor, index, indexNext);
          }

          // In the case when the token is a single '-', it's not numeric.
          if (token.content.front() == '-' && token.content.size() == 1) {
            token.tokenType = TokenType::Special;
          }
          tokens.push_back(token);
          previousTokenType = token.tokenType;
          processed = true;
        }
      }

      if (!processed) {
        Token token {
          .tokenType = TokenType::Special,
          .startPos = index
        };
        token.content.push_back(ch);
        tokens.push_back(token);
        previousTokenType = token.tokenType;
        ch = getNextChar(accessor, index, indexNext);
      }
    }
    return tokens;
  }

  void Lexer::colorToken(StyleContext & styleContext, Token token, State state) const {
    if (styleContext.currentPos < (Sci_PositionU)token.startPos) {
      // White spaces
      styleContext.SetState(std::to_underlying(State::Default));
      styleContext.Forward(token.startPos - styleContext.currentPos);
    }

    styleContext.SetState(std::to_underlying(state));
    styleContext.Forward(token.content.length());
  }

  int Lexer::getNextChar(Accessor& accessor, Sci_Position& index, Sci_Position& indexNext) const {
    index = indexNext;
    if (accessor.Encoding() != EncodingType::eightBit) {
      Sci_Position length {};
      int ch = accessor.MultiByteAccess()->GetCharacterAndWidth(index, &length);
      indexNext = index + length;
      return ch;
    } else {
      indexNext = index + 1;
      return accessor.SafeGetCharAt(index);
    }
  }

  bool Lexer::isNameInCache(const std::string& name, const std::set<std::string>& namesCache, std::mutex& mutex) const {
    Lock lock(mutex);
    return namesCache.find(name) != namesCache.end();
  }

  void Lexer::addNameToCache(const std::string& name, std::set<std::string>& namesCache, std::mutex& mutex) {
    Lock lock(mutex);
    namesCache.insert(name);
  }

  void Lexer::handleMouseHover(HWND handle, bool hovering, Sci_Position position) const {
    if (isUsable() && lexerData->settings.enableHover) {
      // Cancel any displayed call tips
      ::SendMessage(handle, SCI_CALLTIPCANCEL, 0, 0);

      if (hovering) {
        Sci_Position start = ::SendMessage(handle, SCI_WORDSTARTPOSITION, position, true);
        Sci_Position end = ::SendMessage(handle, SCI_WORDENDPOSITION, position, true);

        if (end > start) {
          char* callTips = nullptr;
          auto autoCleanupCallTips = gsl::finally([&] { delete[] callTips; });

          int style = static_cast<int>(::SendMessage(handle, SCI_GETSTYLEAT, start, 0));
          switch (style) {
            case std::to_underlying(State::Property): {
              if (lexerData->settings.enabledHoverCategories & HOVER_CATEGORY_PROPERTY) {
                char* propertyName = new char[end - start + 1];
                auto autoCleanupPropertyName = gsl::finally([&] { delete[] propertyName; });

                Sci_TextRange propertyNameTextRange {
                  .chrg = {
                    .cpMin = static_cast<Sci_PositionCR>(start),
                    .cpMax = static_cast<Sci_PositionCR>(end)
                  },
                  .lpstrText = propertyName
                };
                ::SendMessage(handle, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&propertyNameTextRange));

                auto iter = std::find_if(propertyLines.begin(), propertyLines.end(),
                  [&](const auto& property) {
                    return property.name == utility::toLower(propertyName);
                  }
                );
                if (iter != propertyLines.end()) {
                  Sci_Position propertyDefinitionStart = ::SendMessage(handle, SCI_POSITIONFROMLINE, iter->line, 0);
                  Sci_Position propertyDefinitionEnd = ::SendMessage(handle, SCI_GETLINEENDPOSITION, iter->line, 0);
                  callTips = new char[propertyDefinitionEnd - propertyDefinitionStart + 1];

                  Sci_TextRange propertyDefinitionTextRange {
                    .chrg = {
                      .cpMin = static_cast<Sci_PositionCR>(propertyDefinitionStart),
                      .cpMax = static_cast<Sci_PositionCR>(propertyDefinitionEnd)
                    },
                    .lpstrText = callTips
                  };
                  ::SendMessage(handle, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&propertyDefinitionTextRange));
                }
              }
              break;
            }
          }

          if (callTips != nullptr) {
            ::SendMessage(handle, SCI_CALLTIPSETPOSITION, true, 0);
            ::SendMessage(handle, SCI_CALLTIPSHOW, start, reinterpret_cast<LPARAM>(callTips));
          }
        }
      }
    }
  }

  void Lexer::handleContentChange(HWND handle, Sci_Position position, Sci_Position linesAdded) {
    Sci_Position line = static_cast<Sci_Position>(::SendMessage(handle, SCI_LINEFROMPOSITION, position, 0));

    // Update property list
    for (auto iter = propertyLines.begin(); iter != propertyLines.end();) {
      if (iter->line >= line) {
        // If property is within the # of lines deleted, or is on the line when changes were made, delete the property.
        // Note, deleting the property on the line being edited won't be an issue because Lex will be called later.
        if ((linesAdded < 0 && iter->line <= line - linesAdded) || (linesAdded == 0 && iter->line == line)) {
          // Delete this property
          propertyNames.erase(iter->name);
          iter = propertyLines.erase(iter);
          continue;
        }

        if (iter->line == line) {
          // Since it's not clear if the addition of lines happened before property definition or after, the property defined
          // on the exact line where changes happened need to be re-checked in Lex.
          iter->needRecheck = true;
        }

        // Update line #
        iter->line += linesAdded;
      }
      ++iter;
    }
  }

  // For Notepad++ 8.4.9 or older releases, before NPPN_EXTERNALLEXERBUFFER message was introduced
  void Lexer::detectBufferId() {
    // Can only detect buffer ID if script name is known
    if (bufferID == 0 && !scriptName.empty()) {
      // Check if the file name of the active document on current view matches detected script name
      npp_view_t currentView = static_cast<npp_view_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETCURRENTVIEW, 0, 0));
      npp_buffer_t candidateBufferID = utility::getActiveBufferIdOnView(lexerData->nppData._nppHandle, currentView);
      std::filesystem::path filePath = utility::getFilePathFromBuffer(lexerData->nppData._nppHandle, candidateBufferID);
      if (utility::compare(scriptName + ".psc", filePath.filename().string())) {
        bufferID = candidateBufferID;
      } else {
        // Does not match. Check the other view
        candidateBufferID = utility::getActiveBufferIdOnView(lexerData->nppData._nppHandle, currentView == MAIN_VIEW ? SUB_VIEW : MAIN_VIEW);
        filePath = utility::getFilePathFromBuffer(lexerData->nppData._nppHandle, candidateBufferID);
        if (utility::compare(scriptName + ".psc", filePath.filename().string())) {
          bufferID = candidateBufferID;
        }
      }
    }
  }

  std::wstring Lexer::getClassFilePath(npp_buffer_t bufferID, std::string className) {
    // Find relative path from search directory. Support FO4's namespace.
    std::filesystem::path relativePath;
    auto pathComponents = utility::split(className, ":");
    for (const auto& pathComponent : pathComponents) {
      relativePath /= pathComponent;
    }
    relativePath.replace_extension(".psc");

    // PapyrusCompiler searches in current directory before searching in import directories.
    auto currentBufferFilePath = utility::getFilePathFromBuffer(lexerData->nppData._nppHandle, bufferID);
    if (!currentBufferFilePath.empty()) {
      std::wstring filePath = std::filesystem::path(currentBufferFilePath).parent_path() / relativePath;
      if (utility::fileExists(filePath)) {
        return filePath;
      }
    }

    // Find the relative path in configured import directories.
    for (const auto& path : lexerData->importDirectories[lexerData->currentGame]) {
      std::wstring filePath = std::filesystem::path(path) / relativePath;
      if (utility::fileExists(filePath)) {
        return filePath;
      }
    }

    // Not found
    return std::wstring();
  }

  // Helper class methods
  //

  Helper::Helper() {
    lexerData->bufferActivated.subscribe([&](auto eventData) {
      if (isUsable()) {
        SavedScintillaSettings& savedScintillaSettings = (eventData.view == MAIN_VIEW) ? savedMainViewScintillaSettings : savedSecondViewScintillaSettings;
        HWND handle = (eventData.view == MAIN_VIEW) ? lexerData->nppData._scintillaMainHandle : lexerData->nppData._scintillaSecondHandle;
        if (eventData.isManagedBuffer) {
          // Save current Scintilla settings as we are about to change them
          if (!savedScintillaSettings.saved) {
            savedScintillaSettings.hotspotActiveForegroundColor = ::SendMessage(handle, SCI_GETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, 0);
            savedScintillaSettings.hotspotActiveBackgroundColor = ::SendMessage(handle, SCI_GETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE_BACK, 0);
            savedScintillaSettings.hotspotActiveUnderline = ::SendMessage(handle, SCI_GETHOTSPOTACTIVEUNDERLINE, 0, 0);
            savedScintillaSettings.mouseDwellTime = ::SendMessage(handle, SCI_GETMOUSEDWELLTIME, 0, 0);
            savedScintillaSettings.saved = true;
          }

          if (lexerData->settings.enableClassLink) {
            ::SendMessage(handle, SCI_STYLESETHOTSPOT, std::to_underlying(State::Class), true);
            ::SendMessage(handle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, lexerData->settings.classLinkForegroundColor | 0xFF000000); // Element color is ABGR
            ::SendMessage(handle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE_BACK, lexerData->settings.classLinkBackgroundColor);
            ::SendMessage(handle, SCI_SETHOTSPOTACTIVEUNDERLINE, lexerData->settings.classLinkUnderline, 0);
          }

          if (lexerData->settings.enableHover) {
            ::SendMessage(handle, SCI_SETMOUSEDWELLTIME, lexerData->settings.hoverDelay, 0);
          } else {
            ::SendMessage(handle, SCI_SETMOUSEDWELLTIME, SC_TIME_FOREVER, 0);
          }
        } else {
          // Re-apply saved Scintilla settings as current buffer is not managed by this lexer
          if (savedScintillaSettings.saved) {
            ::SendMessage(handle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, savedScintillaSettings.hotspotActiveForegroundColor);
            ::SendMessage(handle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE_BACK, savedScintillaSettings.hotspotActiveBackgroundColor);
            ::SendMessage(handle, SCI_SETHOTSPOTACTIVEUNDERLINE, savedScintillaSettings.hotspotActiveUnderline, 0);
            ::SendMessage(handle, SCI_SETMOUSEDWELLTIME, savedScintillaSettings.mouseDwellTime, 0);

            // Other plugins may change these settings so we better reset the cached flag to make sure we don't use stale saved settings
            savedScintillaSettings.saved = false;
          }
        }
      }
    });

    LexerSettings& lexerSettings = const_cast<LexerSettings&>(lexerData->settings);
    lexerSettings.enableClassLink.subscribe([&](auto eventData) {
      if (isUsable()) {
        if (getApplicableBufferIdOnView(MAIN_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaMainHandle, SCI_STYLESETHOTSPOT, std::to_underlying(State::Class), eventData.newValue);
        }
        if (getApplicableBufferIdOnView(SUB_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_STYLESETHOTSPOT, std::to_underlying(State::Class), eventData.newValue);
        }
      }
    });

    lexerSettings.classLinkForegroundColor.subscribe([&](auto eventData) {
      if (isUsable()) {
        if (getApplicableBufferIdOnView(MAIN_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaMainHandle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, eventData.newValue | 0xFF000000); // Element color is ABGR
        }
        if (getApplicableBufferIdOnView(SUB_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, eventData.newValue | 0xFF000000); // Element color is ABGR
        }
      }
    });

    lexerSettings.classLinkBackgroundColor.subscribe([&](auto eventData) {
      if (isUsable()) {
        if (getApplicableBufferIdOnView(MAIN_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaMainHandle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE_BACK, eventData.newValue);
        }
        if (getApplicableBufferIdOnView(SUB_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE_BACK, eventData.newValue);
        }
      }
    });

    lexerSettings.classLinkUnderline.subscribe([&](auto eventData) {
      if (isUsable()) {
        if (getApplicableBufferIdOnView(MAIN_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaMainHandle, SCI_SETHOTSPOTACTIVEUNDERLINE, eventData.newValue, 0);
        }
        if (getApplicableBufferIdOnView(SUB_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETHOTSPOTACTIVEUNDERLINE, eventData.newValue, 0);
        }
      }
    });

    lexerSettings.enableHover.subscribe([&](auto eventData) {
      if (isUsable()) {
        if (getApplicableBufferIdOnView(MAIN_VIEW) != 0) {
          if (eventData.newValue) {
            ::SendMessage(lexerData->nppData._scintillaMainHandle, SCI_SETMOUSEDWELLTIME, lexerData->settings.hoverDelay, 0);
          } else {
            ::SendMessage(lexerData->nppData._scintillaMainHandle, SCI_SETMOUSEDWELLTIME, SC_TIME_FOREVER, 0);
          }
        }
        if (getApplicableBufferIdOnView(SUB_VIEW) != 0) {
          if (eventData.newValue) {
            ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETMOUSEDWELLTIME, lexerData->settings.hoverDelay, 0);
          } else {
            ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETMOUSEDWELLTIME, SC_TIME_FOREVER, 0);
          }
        }
      }
    });

    lexerSettings.hoverDelay.subscribe([&](auto eventData) {
      if (isUsable()) {
        if (getApplicableBufferIdOnView(MAIN_VIEW) != 0) {
          if (lexerData->settings.enableHover) {
            ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETMOUSEDWELLTIME, eventData.newValue, 0);
          }
        }
        if (getApplicableBufferIdOnView(SUB_VIEW) != 0) {
          if (lexerData->settings.enableHover) {
            ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETMOUSEDWELLTIME, eventData.newValue, 0);
          }
        }
      }
    });

    lexerData->clickEventData.subscribe([&](auto eventData) {
      handleHotspotClick(eventData.scintillaHandle, eventData.bufferID, eventData.position);
    });

    lexerSettings.enableFoldMiddle.subscribe([&](auto) { restyleDocument(); });

    lexerSettings.enableClassNameCache.subscribe([&](auto eventData) {
      if (!eventData.newValue) {
        clearClassNames();
        clearNonClassNames();
      }
      restyleDocument();
    });
  }

  npp_buffer_t Helper::getApplicableBufferIdOnView(npp_view_t view) const {
    npp_buffer_t viewBufferID = utility::getActiveBufferIdOnView(lexerData->nppData._nppHandle, view);
    return (viewBufferID != 0 && lexerData->scriptLangID == static_cast<npp_lang_type_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETBUFFERLANGTYPE, static_cast<WPARAM>(viewBufferID), 0)) ? viewBufferID : 0);
  }

  void Helper::restyleDocument() const {
    if (isUsable()) {
      restyleDocument(MAIN_VIEW);
      restyleDocument(SUB_VIEW);
    }
  }

  void Helper::restyleDocument(npp_view_t view) const {
    // Ask Scintilla to restyle current document on the given view, but only when it is using this lexer.
    if (getApplicableBufferIdOnView(view) != 0) {
      HWND handle = (view == MAIN_VIEW ? lexerData->nppData._scintillaMainHandle : lexerData->nppData._scintillaSecondHandle);
      ::SendMessage(handle, SCI_COLOURISE, 0, -1);
    }
  }

  names_cache_t& Helper::getClassNamesForGame(Game game) {
    Lock lock(classNamesMutex);
    return classNames[game];
  }

  names_cache_t& Helper::getNonClassNamesForGame(Game game)  {
    Lock lock(nonClassNamesMutex);
    return nonClassNames[game];
  }

  void Helper::clearClassNames() {
    Lock lock(classNamesMutex);
    classNames.clear();
  }

  void Helper::clearNonClassNames() {
    Lock lock(nonClassNamesMutex);
    nonClassNames.clear();
  }

  void Helper::handleHotspotClick(HWND handle, npp_buffer_t bufferID, Sci_Position position) const {
    if (isUsable() && lexerData->settings.enableClassLink && lexerData->currentGame != game::Game::Auto) {
      // Change Scintilla word chars to include ':' to support FO4's namespaces.
      size_t length = ::SendMessage(handle, SCI_GETWORDCHARS, 0, 0);
      char* wordChars = new char[length + 2]; // To add ':' and also null terminator
      auto autoCleanupWordChars = gsl::finally([&] { delete[] wordChars; });
      ::SendMessage(handle, SCI_GETWORDCHARS, 0, reinterpret_cast<LPARAM>(wordChars + 1));

      wordChars[0] = ':';
      wordChars[length + 1] = '\0';
      ::SendMessage(handle, SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>(wordChars));

      Sci_Position start = ::SendMessage(handle, SCI_WORDSTARTPOSITION, position, true);
      Sci_Position end = ::SendMessage(handle, SCI_WORDENDPOSITION, position, true);

      // Restore previous word chars setting after search.
      ::SendMessage(handle, SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>(wordChars + 1));

      if (end > start) {
        char* className = new char[end - start + 1];
        auto autoCleanupClassName = gsl::finally([&] { delete[] className; });

        Sci_TextRange textRange {
          .chrg = {
            .cpMin = static_cast<Sci_PositionCR>(start),
            .cpMax = static_cast<Sci_PositionCR>(end)
          },
          .lpstrText = className
        };
        ::SendMessage(handle, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&textRange));

        std::wstring filePath = getClassFilePath(bufferID, className);
        if (!filePath.empty()) {
          ::SendMessage(lexerData->nppData._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(filePath.c_str()));
        }
      }
    }
  }

} // namespace
