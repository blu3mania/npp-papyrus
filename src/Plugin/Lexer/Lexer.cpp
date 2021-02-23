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

#include "LexerData.hpp"
#include "..\Common\Utility.hpp"

#include "..\..\external\npp\Common.h"
#include "..\..\external\scintilla\LexerModule.h"
#include "..\..\external\scintilla\Scintilla.h"

#include <filesystem>
#include <locale>
#include <string>
#include <vector>

namespace papyrus {

  Lexer::Lexer()
    : instreWordLists{&wordListOperators, &wordListFlowControl},
      typeWordLists{&wordListTypes, &wordListKeywords, &wordListKeywords2, &wordListFoldOpen, &wordListFoldMiddle, &wordListFoldClose} {
    // Setup settings change listeners
    if (isUsable()) {
      lexerData->settings.enableFoldMiddle.addWatcher([&](bool oldValue, bool newValue) { restyleDocument(); });
      lexerData->settings.enableClassNameCache.addWatcher([&](bool oldValue, bool newValue) { 
        if (!newValue) {
          classNames.clear();
          nonClassNames.clear();
        }
        restyleDocument();
      });
    }
  }

  void SCI_METHOD Lexer::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess) {
    if (isUsable()) {
      Accessor accessor(pAccess, nullptr);
      StyleContext styleContext(startPos, lengthDoc, accessor.StyleAt(startPos - 1), accessor);

      // Check if the properties still exist and update the content
      for (auto iterProperties = propertyLines.begin(); iterProperties != propertyLines.end();) {
        auto tokens = tokenize(accessor, (*iterProperties).line);
        bool found = false;
        for (auto iterToken = tokens.begin(); iterToken != tokens.end(); iterToken++) {
          if ((*iterToken).content == "property" && std::next(iterToken) != tokens.end() && !isComment(accessor.StyleAt((*iterToken).startPos)) && !isComment(accessor.StyleAt((*std::next(iterToken)).startPos))) {
            std::string currentName = (*std::next(iterToken)).content;
            if ((*iterProperties).name != currentName) {
              propertyNames.erase((*iterProperties).name);
              (*iterProperties).name = currentName;
              propertyNames.insert(currentName);
            }
            found = true;
            break;
          }
        }
        if (found) {
          iterProperties++;
        } else {
          iterProperties = propertyLines.erase(iterProperties);
        }
      }

      // This state is saved in the line feed character. It can be used to initialize the state of the next line
      State messageStateLast = static_cast<State>(accessor.StyleAt(startPos - 1));
      for (auto line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc - 1); line++) {
        auto tokens = tokenize(accessor, line);
        State messageState = messageStateLast;

        // Styling
        for (auto iterTokens = tokens.begin(); iterTokens != tokens.end(); iterTokens++) {
          std::string tokenString = (*iterTokens).content;
          
          // Check if a new property needs to be added
          if (tokenString == "property" && std::next(iterTokens) != tokens.end() && !isComment(accessor.StyleAt((*iterTokens).startPos)) && !isComment(accessor.StyleAt((*std::next(iterTokens)).startPos))) {
            bool found = false;
            for (auto iterProperties = propertyLines.begin(); iterProperties != propertyLines.end(); iterProperties++) {
              if ((*iterProperties).line == line) {
                found = true;
                break;
              }
            }

            if (!found) {
              Property property {
                .name = (*std::next(iterTokens)).content,
                .line = line
              };
              propertyLines.push_back(property);
              propertyNames.insert(property.name);

              // Always style "property" keyword as KEYWORD
              colorToken(styleContext, *iterTokens, KEYWORD);
              continue;
            }
          }

          if (messageState == COMMENTDOC) {
            colorToken(styleContext, *iterTokens, COMMENTDOC);
            if (tokenString == "}") {
              messageState = DEFAULT;
            }
          } else if(messageState == COMMENTMULTILINE) {
            colorToken(styleContext, *iterTokens, COMMENTMULTILINE);
            if (tokenString == ";" && iterTokens != tokens.begin() && (*std::prev(iterTokens)).content == "/") {
              messageState = DEFAULT;
            }
          } else if(messageState == COMMENT) {
            colorToken(styleContext, *iterTokens, COMMENT);
          } else if(messageState == STRING) {
            colorToken(styleContext, *iterTokens, STRING);
            if (tokenString == "\"") {
              // This may be an escape for double quote. Check previous tokens
              int numBackslash = 0;
              auto iterCheck = iterTokens;
              while (iterCheck != tokens.begin()) {
                if ((*(--iterCheck)).content == "\\") {
                  numBackslash++;
                } else {
                  break;
                }
              }
              if (numBackslash % 2 == 0) {
                messageState = DEFAULT;
              }
            }
          } else {
            // Determine the type of the token and color it
            if (tokenString == "{") {
              colorToken(styleContext, *iterTokens, COMMENTDOC);
              messageState = COMMENTDOC;
            } else if (tokenString == ";") {
              if (std::next(iterTokens) != tokens.end() && (*std::next(iterTokens)).content == "/") {
                colorToken(styleContext, *iterTokens, COMMENTMULTILINE);
                messageState = COMMENTMULTILINE;
              } else {
                colorToken(styleContext, *iterTokens, COMMENT);
                messageState = COMMENT;
              }
            } else if (tokenString == "\"") {
              colorToken(styleContext, *iterTokens, STRING);
              messageState = STRING;
            } else if ((*iterTokens).tokenType == NUMERIC) {
              colorToken(styleContext, *iterTokens, NUMBER);
            } else if ((*iterTokens).tokenType == IDENTIFIER) {
              if (!wordListFlowControl.InList(tokenString.c_str()) && isalnum(tokenString.back()) && std::next(iterTokens) != tokens.end() && (*std::next(iterTokens)).content == "(") {
                // If next token is ( and current token is an identifier but not if/elseif/while, it is a function name.
                colorToken(styleContext, *iterTokens, FUNCTION);
              } else if (wordListTypes.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, TYPE);
              } else if (wordListFlowControl.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, FLOWCONTROL);
              } else if (wordListKeywords.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, KEYWORD);
              } else if (wordListKeywords2.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, KEYWORD2);
              } else if (wordListOperators.InList(tokenString.c_str())) {
                colorToken(styleContext, *iterTokens, OPERATOR);
              } else {
                bool found = (propertyNames.find(tokenString) != propertyNames.end());
                if (found) {
                  colorToken(styleContext, *iterTokens, PROPERTY);
                } else {
                  if (classNames.find(tokenString) != classNames.end()) {
                    colorToken(styleContext, *iterTokens, CLASS);
                    found = true;
                  } else if (nonClassNames.find(tokenString) == nonClassNames.end()) {
                    // Check all import directories for a source file with given name
                    if (lexerData->currentGame != game::Auto) {
                      for (const auto& path : lexerData->importDirectories[lexerData->currentGame]) {
                        if (utility::fileExists(std::filesystem::path(path) / (tokenString + ".psc"))) {
                          colorToken(styleContext, *iterTokens, CLASS);
                          if (lexerData->settings.enableClassNameCache) {
                            classNames.insert(tokenString);
                          }
                          found = true;
                          break;
                        }
                      }
                    }

                    if (!found && lexerData->settings.enableClassNameCache) {
                      nonClassNames.insert(tokenString);
                    }
                  }

                  if (!found) {
                    colorToken(styleContext, *iterTokens, DEFAULT);
                  }
                }
              }
            } else if ((*iterTokens).tokenType == SPECIAL) {
              if (wordListOperators.InList((*iterTokens).content.c_str())) {
                colorToken(styleContext, *iterTokens, OPERATOR);
              } else {
                colorToken(styleContext, *iterTokens, DEFAULT);
              }
            }
          }
        }
        if (messageState == COMMENT || messageState == STRING) {
          messageState = DEFAULT;
        }
        if (styleContext.ch == '\r') {
          styleContext.Forward();
        }
        if (styleContext.ch == '\n') {
          styleContext.SetState(messageState);
          styleContext.Forward();
        }
        messageStateLast = messageState;
      }
      styleContext.Complete();
    }
  }

  void SCI_METHOD Lexer::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess) {
    if (isUsable()) {
      Accessor accessor(pAccess, nullptr);

      int levelPrev = accessor.LevelAt(accessor.GetLine(startPos)) & SC_FOLDLEVELNUMBERMASK;
      // Lines
      for (auto line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc); line++) {
        int numFoldOpen = 0;
        int numFoldClose = 0;
        bool hasFoldMiddle = false;
        // Chars
        auto tokens = tokenize(accessor, line);
        for (const Token& token : tokens) {
          if (!isComment(accessor.StyleAt(token.startPos)) && accessor.StyleAt(token.startPos) != STRING) {
            if (wordListFoldOpen.InList(token.content.c_str())) {
              numFoldOpen++;
            } else if (wordListFoldClose.InList(token.content.c_str())) {
              numFoldClose++;
            } else if (lexerData->settings.enableFoldMiddle && wordListFoldMiddle.InList(token.content.c_str())) {
              hasFoldMiddle = true;
            }
          }
        }

        // Skip the lines that have matching start and end keywords
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
          .tokenType = IDENTIFIER,
          .startPos = index
        };
        while (isalnum(ch) || ch == '_') {
          token.content.push_back(tolower(ch));
          ch = getNextChar(accessor, index, indexNext);
        }
        tokens.push_back(token);
      } else if (isdigit(ch) || ch == '-') {
        Token token {
          .tokenType = NUMERIC,
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

        // In the case it's a single '-', correct token type
        if (token.content.at(0) == '-' && token.content.size() == 1) {
          token.tokenType = SPECIAL;
        }
        tokens.push_back(token);
      } else {
        Token token {
          .tokenType = SPECIAL,
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

    styleContext.SetState(state);
    styleContext.Forward(token.content.size());
  }

  bool Lexer::isComment(int style) const {
    return style == COMMENT || style == COMMENTMULTILINE || style == COMMENTDOC;
  }

  int Lexer::getNextChar(Accessor& accessor, Sci_Position& index, Sci_Position& indexNext) const {
    index = indexNext;
    if (accessor.Encoding() != enc8bit) {
      Sci_Position length {};
      int ch = accessor.MultiByteAccess()->GetCharacterAndWidth(index, &length);
      indexNext = index + length;
      return ch;
    } else {
      indexNext = index + 1;
      return accessor.SafeGetCharAt(index);
    }
  }

  void Lexer::restyleDocument() const {
    if (isUsable()) {
      restyleDocument(MAIN_VIEW);
      restyleDocument(SUB_VIEW);
    }
  }

  void Lexer::restyleDocument(npp_view_t view) const {
    // Ask Scintilla to restyle urrent document on the given view, but only when it is using this lexer
    HWND handle = (view == MAIN_VIEW ? lexerData->nppData._scintillaMainHandle : lexerData->nppData._scintillaSecondHandle);
    npp_index_t viewDocIndex = static_cast<npp_index_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, static_cast<LPARAM>(view)));
    if (viewDocIndex != -1) {
      npp_buffer_t viewBufferID = static_cast<npp_buffer_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, static_cast<WPARAM>(viewDocIndex), static_cast<LPARAM>(view)));
      if (viewBufferID != 0) {
        if (lexerData->scriptLangID == static_cast<npp_buffer_t>(::SendMessage(lexerData->nppData._nppHandle, NPPM_GETBUFFERLANGTYPE, static_cast<WPARAM>(viewBufferID), 0))) {
          ::SendMessage(handle, SCI_COLOURISE, 0, -1);
        }
      }
    }
  }

} // namespace
