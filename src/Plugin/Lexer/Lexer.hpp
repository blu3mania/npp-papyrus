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

#include "SimpleLexerBase.hpp"

#include "LexerData.hpp"

#include "..\Common\NotepadPlusPlus.hpp"

#include "..\..\external\lexilla\Accessor.h"
#include "..\..\external\lexilla\StyleContext.h"
#include "..\..\external\lexilla\WordList.h"
#include "..\..\external\scintilla\ILexer.h"

#include <list>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <windows.h>

namespace papyrus {

  using names_cache_t = std::pair<std::set<std::string>, std::mutex>;

  constexpr char LEXER_NAME[] = "Papyrus Script";
  constexpr TCHAR LEXER_STATUS_TEXT[] = L"Papyrus Script"; // Not required anymore, but kept for compatibility with Notepad++ 8.3 - 8.3.3

  class Lexer : public SimpleLexerBase {
    public:
      // A class that helps with management of shared Lexer data, since the handling are all static, and not tied to a specific
      // Lexer instance. For example, restyle currently displayed document, regardless if it's lexed by current Lexer instance.
      class Helper {
        public:
          struct SavedScintillaSettings {
            bool saved {false};
            int hotspotActiveForegroundColor;
            int hotspotActiveBackgroundColor;
            bool hotspotActiveUnderline;
          };

          Helper();

          // Only when configuration file exists under Notepad++'s plugin config folder can this lexer be used
          inline bool isUsable() const { return (lexerData != nullptr && lexerData->usable); }

          // Get cached class/non-class names for a game
          names_cache_t& getClassNamesForGame(Game game);
          names_cache_t& getNonClassNamesForGame(Game game);

        private:
          // Get current buffer ID on the given view, if it's a applicable
          npp_buffer_t getApplicableBufferIdOnView(npp_view_t view) const;

          // Restyle currently displayed document, which includes Lex and Fold
          void restyleDocument() const;
          void restyleDocument(npp_view_t view) const;

          // Clear cached class/non-class names
          void clearClassNames();
          void clearNonClassNames();

          // Hotspot click handler
          void handleHotspotClick(HWND handle, Sci_Position position) const;
      };

      Lexer();
      ~Lexer();

      // Interface functions with Notepad++
      inline static char* name() { return const_cast<char*>(LEXER_NAME); }
      inline static TCHAR* statusText() { return const_cast<TCHAR*>(LEXER_STATUS_TEXT); }  // Not required anymore, but kept for compatibility with Notepad++ 8.3 - 8.3.3
      inline static ILexer* factory() { return new Lexer(); }

      // Lexer functions
      void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess) override;
      void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess) override;

      // Utility method to check whether a style (from StyleContext) is certain style type defined by this lexer
      inline static bool isKeyword(int style) {
        State styleState = static_cast<State>(style);
        return styleState == State::Keyword || styleState == State::Keyword2;
      }

      inline static bool isFlowControl(int style) {
        State styleState = static_cast<State>(style);
        return styleState == State::FlowControl;
      }

      inline static bool isComment(int style) {
        State styleState = static_cast<State>(style);
        return styleState == State::Comment || styleState == State::CommentMultiLine || styleState == State::CommentDoc;
      }

      // Utility method to retrieve full path of a class. It supports FO4's namespaces
      static std::wstring getClassFilePath(std::string className);

    protected:
      // Only when configuration file exists under Notepad++'s plugin config folder can this lexer be used
      bool isUsable() const override;

      // Get word lists pointers for instre1 & 2, type1 - 7
      inline const std::vector<WordList*>& getInstreWordLists() const override { return instreWordLists; }
      inline const std::vector<WordList*>& getTypeWordLists() const override { return typeWordLists; }

    private:
      // Lexer style states
      enum class State {
        Default,
        Operator,
        FlowControl,
        Type,
        Keyword,
        Keyword2,
        FoldOpen,
        FoldMiddle,
        FoldClose,
        Comment,
        CommentMultiLine,
        CommentDoc,
        Number,
        String,
        Property,
        Class,
        Function
      };

      // Defined properties in current Papyrus script
      struct Property {
        std::string name;
        Sci_Position line;
        bool needRecheck {false};
      };

      enum class TokenType {
        Identifier,
        Numeric,
        Special
      };

      struct Token {
        std::string content;
        TokenType tokenType;
        Sci_Position startPos;
      };

      // Parse a text line and tokenize each word/symbol, etc.
      std::vector<Token> tokenize(Accessor& accessor, Sci_Position line) const;

      // Colorize a word/symbol in StyleContext to a provided state based on the given token.
      void colorToken(StyleContext& styleContext, Token token, State state) const;

      // Get next character (wide char supported)
      int getNextChar(Accessor& accessor, Sci_Position& index, Sci_Position& indexNext) const;

      // Check whether a given name is in a names cache
      bool isNameInCache(const std::string& name, const std::set<std::string>& namesCache, std::mutex& mutex) const;

      // Add a given name to a names cache
      void addNameToCache(const std::string& name, std::set<std::string>& namesCache, std::mutex& mutex);

      // Content change handler. Update property list to make sure it's correct
      void handleContentChange(HWND handle, Sci_Position position, Sci_Position linesAdded);

      // Try to detect current document's Notepad++ buffer ID
      void detectBufferId();

      // Private members
      //

      // Word lists for different function groups
      WordList wordListOperators;   // instre1
      WordList wordListFlowControl; // instre2
      WordList wordListTypes;       // type1
      WordList wordListKeywords;    // type2
      WordList wordListKeywords2;   // type3
      WordList wordListFoldOpen;    // type4
      WordList wordListFoldMiddle;  // type5
      WordList wordListFoldClose;   // type6

      // Provide pointers to corresponding word lists to base class
      const std::vector<WordList*> instreWordLists;
      const std::vector<WordList*> typeWordLists;

      // Cache list of lines that define properties
      std::list<Property> propertyLines;

      // Cache property names defined in current file, for better performance
      std::set<std::string> propertyNames;

      // Current script's name
      std::string scriptName {};

      // Current document's buffer ID managed by Notepad++
      npp_buffer_t bufferID {0};

      // Subscriptions
      change_event_topic_t::subscription_t changeEventSubscription;
  };

} // namespace
