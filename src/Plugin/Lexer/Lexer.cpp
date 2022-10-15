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
#include "..\Common\StringUtil.hpp"

#include "..\..\external\gsl\include\gsl\util"
#include "..\..\external\lexilla\LexerModule.h"
#include "..\..\external\npp\Common.h"
#include "..\..\external\scintilla\Scintilla.h"

#include <filesystem>
#include <map>
#include <memory>

namespace papyrus {

  using SubscriptionHelper = Lexer::SubscriptionHelper;

  // Static shared subscription helper
  namespace {
    std::unique_ptr<SubscriptionHelper> subscriptionHelper;

    // Cache names that are classes (i.e. files in import directories) used in current file, and names that aren't, for better performance.
    // Caveat: when a new class is saved to the import directory it won't be reflected, so current file needs to be reloaded. This can be
    // fixed by toggling off this option then toggling it back on in Settings dialog.
    std::map<Game, std::set<std::string>> classNames;
    std::map<Game, std::set<std::string>> nonClassNames;
  }

  Lexer::Lexer()
    : SimpleLexerBase(LEXER_NAME, SCLEX_PAPYRUS_SCRIPT),
      instreWordLists{&wordListOperators, &wordListFlowControl},
      typeWordLists{&wordListTypes, &wordListKeywords, &wordListKeywords2, &wordListFoldOpen, &wordListFoldMiddle, &wordListFoldClose} {
    // Setup settings change listeners.
    if (isUsable() && !subscriptionHelper) {
      subscriptionHelper = std::make_unique<SubscriptionHelper>();
    }

    changeEventSubscription = lexerData->changeEventData.subscribe([&](auto eventData) {
      if (isUsable()) {
        HWND handle = std::get<0>(eventData);
        if (docPointer == reinterpret_cast<npp_ptr_t>(::SendMessage(handle, SCI_GETDOCPOINTER, 0, 0))) {
          // Change happened on current file.
          handleContentChange(handle, std::get<1>(eventData), std::get<2>(eventData));
        }
      }
    });
  }

  Lexer::~Lexer() {
    changeEventSubscription->unsubscribe();
  }

  void SCI_METHOD Lexer::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int, IDocument* pAccess) {
    if (isUsable()) {
      if (docPointer == nullptr) {
        npp_view_t currentView = static_cast<npp_view_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETCURRENTVIEW, 0, 0));
        HWND handle = (currentView == MAIN_VIEW) ? lexerData->nppData._scintillaMainHandle : lexerData->nppData._scintillaSecondHandle;
        docPointer = reinterpret_cast<npp_ptr_t>(::SendMessage(handle, SCI_GETDOCPOINTER, 0, 0));
      }

      Accessor accessor(pAccess, nullptr);
      StyleContext styleContext(startPos, lengthDoc, accessor.StyleAt(startPos - 1), accessor);

      // This state is saved in the line feed character. It can be used to initialize the state of the next line.
      State messageStateLast = static_cast<State>(accessor.StyleAt(startPos - 1));
      for (auto line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc - 1); ++line) {
        auto tokens = tokenize(accessor, line);
        State messageState = messageStateLast;

        // Styling
        for (auto iterTokens = tokens.begin(); iterTokens != tokens.end(); ++iterTokens) {
          std::string tokenString = iterTokens->content;

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
              colorToken(styleContext, *iterTokens, State::CommentDoc);
              messageState = State::CommentDoc;
            } else if (tokenString == ";") {
              // A multi-line comment starts with ";/" and there can't be spaces in between.
              if (std::next(iterTokens) != tokens.end() && std::next(iterTokens)->content == "/" && iterTokens->startPos == std::next(iterTokens)->startPos - 1) {
                colorToken(styleContext, *iterTokens, State::CommentMultiLine);
                messageState = State::CommentMultiLine;
              } else {
                colorToken(styleContext, *iterTokens, State::Comment);
                messageState = State::Comment;
              }
            } else if (tokenString == "\"") {
              colorToken(styleContext, *iterTokens, State::String);
              messageState = State::String;
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
                if (tokenString == "property" && std::next(iterTokens) != tokens.end() && std::next(iterTokens)->content != ";") {
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
                    auto& currentGameClassNames = classNames[lexerData->currentGame];
                    if (currentGameClassNames.find(tokenString) != currentGameClassNames.end()) {
                      colorToken(styleContext, *iterTokens, State::Class);
                      found = true;
                    } else {
                      auto& currentGameNonClassNames = nonClassNames[lexerData->currentGame];
                      if (currentGameNonClassNames.find(tokenString) == currentGameNonClassNames.end()) {
                        if (!getClassFilePath(tokenString).empty()) {
                          colorToken(styleContext, *iterTokens, State::Class);
                          if (lexerData->settings.enableClassNameCache) {
                            currentGameClassNames.insert(tokenString);
                          }
                          found = true;
                        }
                      }

                      if (!found && lexerData->settings.enableClassNameCache) {
                        currentGameNonClassNames.insert(tokenString);
                      }
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

  bool Lexer::isKeyword(int style) {
    State styleState = static_cast<State>(style);
    return styleState == State::Keyword || styleState == State::Keyword2;
  }

  bool Lexer::isFlowControl(int style) {
    State styleState = static_cast<State>(style);
    return styleState == State::FlowControl;
  }

  bool Lexer::isComment(int style) {
    State styleState = static_cast<State>(style);
    return styleState == State::Comment || styleState == State::CommentMultiLine || styleState == State::CommentDoc;
  }

  std::wstring Lexer::getClassFilePath(std::string className) {
    // Find relative path from current directory. Support FO4's namespace.
    std::filesystem::path relativePath;
    auto pathComponents = utility::split(className, ":");
    for (const auto& pathComponent : pathComponents) {
      relativePath /= pathComponent;
    }
    relativePath.replace_extension(".psc");

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

  // Protected methods
  //

  bool Lexer::isUsable() const {
    return (lexerData != nullptr && lexerData->usable);
  }

  const std::vector<WordList*>& Lexer::getInstreWordLists() const {
    return instreWordLists;
  }

  const std::vector<WordList*>& Lexer::getTypeWordLists() const {
    return typeWordLists;
  }

  // Private methods
  //

  std::vector<Lexer::Token> Lexer::tokenize(Accessor& accessor, Sci_Position line) const {
    std::vector<Token> tokens;
    auto index = accessor.LineStart(line);
    auto indexNext = index;
    int ch = getNextChar(accessor, index, indexNext);
    while (index < accessor.LineEnd(line)) {
      if (ch == '\r' || ch == '\n') {
        break;
      }

      if (isblank(ch)) {
        ch = getNextChar(accessor, index, indexNext);
      } else if (isalpha(ch) || ch == '_') {
        Token token {
          .tokenType = TokenType::Identifier,
          .startPos = index
        };
        while (isalnum(ch) || ch == '_' || ch == ':') {
          token.content.push_back(tolower(ch));
          ch = getNextChar(accessor, index, indexNext);
        }
        tokens.push_back(token);
      } else if (isdigit(ch) || ch == '-') {
        Token token {
          .tokenType = TokenType::Numeric,
          .startPos = index
        };
        bool hasDigit = false;
        while (isdigit(ch)
          || (ch == '-' && index == token.startPos) // leading -
          || (ch == '.' && hasDigit) // decimal point after at least a digit
          || (tolower(ch) == 'x' && index == token.startPos + 1 && token.content.at(0) == '0') // 0x
          || (isxdigit(ch) && token.content.size() > 1 && tolower(token.content.at(1)) == 'x')) { // hex value after 0x
          token.content.push_back(tolower(ch));
          if (isdigit(ch)) {
            hasDigit = true;
          }
          ch = getNextChar(accessor, index, indexNext);
        }

        // In the case when the token is a single '-', it's not numeric.
        if (token.content.at(0) == '-' && token.content.size() == 1) {
          token.tokenType = TokenType::Special;
        }
        tokens.push_back(token);
      } else {
        Token token {
          .tokenType = TokenType::Special,
          .startPos = index
        };
        token.content.push_back(tolower(ch));
        tokens.push_back(token);
        ch = getNextChar(accessor, index, indexNext);
      }
    }
    return tokens;
  }

  void Lexer::colorToken(StyleContext & styleContext, Token token, State state) const {
    if (styleContext.currentPos < (Sci_PositionU)token.startPos) {
      styleContext.Forward(token.startPos - styleContext.currentPos);
    }

    styleContext.SetState(std::to_underlying(state));
    styleContext.Forward(token.content.size());
    styleContext.SetState(std::to_underlying(State::Default));
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

  SubscriptionHelper::SubscriptionHelper() {
    lexerData->bufferActivated.subscribe([&](auto eventData) {
      if (isUsable() && lexerData->settings.enableClassLink && eventData.second) { // isManagedBuffer
        HWND handle = (eventData.first == MAIN_VIEW) ? lexerData->nppData._scintillaMainHandle : lexerData->nppData._scintillaSecondHandle;
        ::SendMessage(handle, SCI_STYLESETHOTSPOT, std::to_underlying(State::Class), true);
        ::SendMessage(handle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, lexerData->settings.classLinkForegroundColor | 0xFF000000);
        ::SendMessage(handle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE_BACK, lexerData->settings.classLinkBackgroundColor);
        ::SendMessage(handle, SCI_SETHOTSPOTACTIVEUNDERLINE, lexerData->settings.classLinkUnderline, 0);
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
          ::SendMessage(lexerData->nppData._scintillaMainHandle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, eventData.newValue | 0xFF000000);
        }
        if (getApplicableBufferIdOnView(SUB_VIEW) != 0) {
          ::SendMessage(lexerData->nppData._scintillaSecondHandle, SCI_SETELEMENTCOLOUR, SC_ELEMENT_HOT_SPOT_ACTIVE, eventData.newValue | 0xFF000000);
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

    lexerData->clickEventData.subscribe([&](auto eventData) {
      handleHotspotClick(eventData.first, eventData.second);
    });

    lexerSettings.enableFoldMiddle.subscribe([&](auto) { restyleDocument(); });

    lexerSettings.enableClassNameCache.subscribe([&](auto eventData) {
      if (!eventData.newValue) {
        classNames.clear();
        nonClassNames.clear();
      }
      restyleDocument();
    });
  }

  bool SubscriptionHelper::isUsable() const {
    return (lexerData != nullptr && lexerData->usable);
  }

  npp_buffer_t SubscriptionHelper::getApplicableBufferIdOnView(npp_view_t view) const {
    npp_index_t viewDocIndex = static_cast<npp_index_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, static_cast<LPARAM>(view)));
    if (viewDocIndex != -1) {
      npp_buffer_t viewBufferID = static_cast<npp_buffer_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, static_cast<WPARAM>(viewDocIndex), static_cast<LPARAM>(view)));
      if (viewBufferID != 0 && lexerData->scriptLangID == static_cast<npp_lang_type_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETBUFFERLANGTYPE, static_cast<WPARAM>(viewBufferID), 0))) {
        return viewBufferID;
      }
    }

    return 0;
  }

  void SubscriptionHelper::restyleDocument() const {
    if (isUsable()) {
      restyleDocument(MAIN_VIEW);
      restyleDocument(SUB_VIEW);
    }
  }

  void SubscriptionHelper::restyleDocument(npp_view_t view) const {
    // Ask Scintilla to restyle urrent document on the given view, but only when it is using this lexer.
    if (getApplicableBufferIdOnView(view) != 0) {
      HWND handle = (view == MAIN_VIEW ? lexerData->nppData._scintillaMainHandle : lexerData->nppData._scintillaSecondHandle);
      ::SendMessage(handle, SCI_COLOURISE, 0, -1);
    }
  }

  void SubscriptionHelper::handleHotspotClick(HWND handle, Sci_Position position) const {
    if (isUsable() && lexerData->currentGame != game::Game::Auto) {
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

      std::wstring filePath = getClassFilePath(className);
      if (!filePath.empty()) {
        ::SendMessage(lexerData->nppData._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(filePath.c_str()));
      }
    }
  }

} // namespace
