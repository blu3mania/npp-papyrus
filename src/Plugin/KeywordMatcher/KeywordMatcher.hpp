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

#pragma once

#include "KeywordMatcherSettings.hpp"

#include "..\Common\NotepadPlusPlus.hpp"

#include "..\..\external\npp\PluginInterface.h"

#include <string>
#include <vector>

namespace papyrus {

  using word_list_t = std::vector<const char*>;
  using result_list_t = std::vector<Sci_CharacterRange>;

  class KeywordMatcher {
    public:
      struct SavedSearch {
        SavedSearch(HWND handle);
        ~SavedSearch();

        HWND handle;
        npp_position_t startPos;
        npp_position_t endPos;
        int flags;
      };

      KeywordMatcher(const NppData& nppData, const KeywordMatcherSettings& settings);

      bool match(HWND scintillaHandle);
      inline void goToMatchedPos() const {
        if (handle != 0 && matched) {
          ::SendMessage(handle, SCI_GOTOPOS, matchedPos, 0);
        }
      }
      void clear();

    private:
      enum class SearchWordType {
        Keyword,
        FlowControl
      };

      void match();
      void matchKeyword(Sci_CharacterRange currentWordPos, const char* currentWord, word_list_t matchingWords, bool searchForward = true);
      void matchFlowControl(Sci_CharacterRange currentWordPos, const char* currentWord, const char* matchingWord, word_list_t otherWords, bool searchForward = true);
      Sci_CharacterRange matchFlowControl(Sci_CharacterRange currentWordPos, const char* currentWord, const char* matchingWord, word_list_t otherWords, result_list_t& otherWordsPosList, bool searchForward = true);
      Sci_CharacterRange findText(const char* text, Sci_PositionCR start, Sci_PositionCR end, SearchWordType searchWordType, bool searchForward = true, int searchFlags = SCFIND_WHOLEWORD);
      void findWords(Sci_PositionCR start, Sci_PositionCR end, word_list_t words, result_list_t& foundPosList, SearchWordType searchWordType, bool searchForward = true);

      void setupIndicator();
      void showIndicator();
      void hideIndicator();

      // Change indicator ID.
      // Scintilla reserves indicator 8-31 for containers. Notepad++ itself uses 8, and SciLexher.h defines most of IDs above 20, which NPP uses.
      // By default 17 is used for keyword matcher, but other plugins could cause conflicts, e.g. DSpellCheck uses 19. It is recommended to auto allocate.
      void changeIndicator();

      // Private members
      //
      const NppData& nppData;
      const KeywordMatcherSettings& settings;
      HWND handle {0};
      Sci_PositionCR docLength {0};

      int indicatorID {0};
      int allocatedIndicatorID {0};

      bool matched {false};
      Sci_PositionCR matchedPos {0};
  };

} // namespace
