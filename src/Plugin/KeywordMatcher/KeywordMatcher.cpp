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

#include "KeywordMatcher.hpp"

#include "..\Common\Utility.hpp"
#include "..\Lexer\Lexer.hpp"

#include "..\..\external\gsl\include\gsl\util"
#include "..\..\external\npp\Common.h"

namespace papyrus {

  using SavedSearch = KeywordMatcher::SavedSearch;

  // Internal static variables
  namespace {
    word_list_t emptyWords;
    word_list_t otherFlowControlHighlightingWords {
      "Else",
      "ElseIf"
    };
  }

  SavedSearch::SavedSearch(HWND handle)
    : handle(handle) {
    startPos = ::SendMessage(handle, SCI_GETTARGETSTART, 0, 0);
    endPos = ::SendMessage(handle, SCI_GETTARGETEND, 0, 0);
    flags = static_cast<int>(::SendMessage(handle, SCI_GETSEARCHFLAGS, 0, 0));
  }

  SavedSearch::~SavedSearch() {
    ::SendMessage(handle, SCI_SETTARGETSTART, startPos, 0);
    ::SendMessage(handle, SCI_SETTARGETEND, endPos, 0);
    ::SendMessage(handle, SCI_SETSEARCHFLAGS, flags, 0);
  }

  KeywordMatcher::KeywordMatcher(const KeywordMatcherSettings& settings)
   : settings(settings) {
    // Subscribe to settings changes
    KeywordMatcherSettings& subscribableSettings = const_cast<KeywordMatcherSettings&>(settings);
    subscribableSettings.enableKeywordMatching.subscribe([&](auto eventData) { match(); });
    subscribableSettings.enabledKeywords.subscribe([&](auto eventData) { match(); });
    subscribableSettings.indicatorID.subscribe([&](auto eventData) { changeIndicator(eventData.oldValue); });
    subscribableSettings.matchedIndicatorStyle.subscribe([&](auto eventData) { if (handle != 0 && matched) { setupIndicator(); } });
    subscribableSettings.matchedIndicatorForegroundColor.subscribe([&](auto eventData) { if (handle != 0 && matched) { setupIndicator(); } });
    subscribableSettings.unmatchedIndicatorStyle.subscribe([&](auto eventData) { if (handle != 0 && !matched) { setupIndicator(); } });
    subscribableSettings.unmatchedIndicatorForegroundColor.subscribe([&](auto eventData) { if (handle != 0 && !matched) { setupIndicator(); } });
   }

  void KeywordMatcher::match(HWND scintillaHandle) {
    handle = scintillaHandle;
    match();
  }

  void KeywordMatcher::clear() {
    if (handle != 0) {
      docLength = static_cast<Sci_PositionCR>(::SendMessage(handle, SCI_GETLENGTH, 0, 0));
      ::SendMessage(handle, SCI_SETINDICATORCURRENT, settings.indicatorID, 0);
      ::SendMessage(handle, SCI_INDICATORCLEARRANGE, 0, docLength);
    }
  }

  // Private methods
  //

  void KeywordMatcher::match() {
    if (handle != 0) {
      // Clear existing matches
      clear();

      if (settings.enableKeywordMatching && settings.enabledKeywords != KEYWORD_NONE) {
        // Get current word at caret
        npp_position_t currentPos = ::SendMessage(handle, SCI_GETCURRENTPOS, 0, 0);
        npp_position_t currentWordStart = ::SendMessage(handle, SCI_WORDSTARTPOSITION, currentPos, true);
        npp_position_t currentWordEnd = ::SendMessage(handle, SCI_WORDENDPOSITION, currentPos, true);
        if (currentWordEnd > currentWordStart) {
          int style = static_cast<int>(::SendMessage(handle, SCI_GETSTYLEAT, currentWordStart, 0));
          bool isKeyword = Lexer::isKeyword(style);
          bool isFlowControl = Lexer::isFlowControl(style);
          if (isKeyword || isFlowControl) {
            char* word = new char[currentWordEnd - currentWordStart + 1];
            auto autoCleanup = gsl::finally([&] { delete[] word; });

            Sci_TextRange textRange {
              .chrg = {
                .cpMin = static_cast<Sci_PositionCR>(currentWordStart),
                .cpMax = static_cast<Sci_PositionCR>(currentWordEnd)
              },
              .lpstrText = word
            };
            ::SendMessage(handle, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&textRange));

            std::string currentWord(word);
            if (isKeyword) {
              if (utility::compare(currentWord, "Function")) {
                if (settings.enabledKeywords & KEYWORD_FUNCTION) {
                  matchKeyword(textRange.chrg, { "EndFunction", "Native" });
                }
              } else if (utility::compare(currentWord, "EndFunction") || utility::compare(currentWord, "Native")) {
                if (settings.enabledKeywords & KEYWORD_FUNCTION) {
                  matchKeyword(textRange.chrg, { "Function" }, false);
                }
              } else if (utility::compare(currentWord, "Struct")) {
                if (settings.enabledKeywords & KEYWORD_STRUCT) {
                  matchKeyword(textRange.chrg, { "EndStruct" });
                }
              } else if (utility::compare(currentWord, "EndStruct")) {
                if (settings.enabledKeywords & KEYWORD_STRUCT) {
                  matchKeyword(textRange.chrg, { "Struct" }, false);
                }
              } else if (utility::compare(currentWord, "Property")) {
                if (settings.enabledKeywords & KEYWORD_PROPERTY) {
                  matchKeyword(textRange.chrg, { "EndProperty", "Auto", "AutoReadOnly" });
                }
              } else if (utility::compare(currentWord, "EndProperty") || utility::compare(currentWord, "Auto") || utility::compare(currentWord, "AutoReadOnly")) {
                if (settings.enabledKeywords & KEYWORD_PROPERTY) {
                  matchKeyword(textRange.chrg, { "Property" }, false);
                }
              } else if (utility::compare(currentWord, "Group")) {
                if (settings.enabledKeywords & KEYWORD_GROUP) {
                  matchKeyword(textRange.chrg, { "EndGroup" });
                }
              } else if (utility::compare(currentWord, "EndGroup")) {
                if (settings.enabledKeywords & KEYWORD_GROUP) {
                  matchKeyword(textRange.chrg, { "Group" }, false);
                }
              } else if (utility::compare(currentWord, "State")) {
                if (settings.enabledKeywords & KEYWORD_STATE) {
                  matchKeyword(textRange.chrg, { "EndState" });
                }
              } else if (utility::compare(currentWord, "EndState")) {
                if (settings.enabledKeywords & KEYWORD_STATE) {
                  matchKeyword(textRange.chrg, { "State" }, false);
                }
              } else if (utility::compare(currentWord, "Event")) {
                if (settings.enabledKeywords & KEYWORD_EVENT) {
                  matchKeyword(textRange.chrg, { "EndEvent" });
                }
              } else if (utility::compare(currentWord, "EndEvent")) {
                if (settings.enabledKeywords & KEYWORD_EVENT) {
                  matchKeyword(textRange.chrg, { "Event" }, false);
                }
              }
            } else { // isFlowControl
              if (utility::compare(currentWord, "While")) {
                if (settings.enabledKeywords & KEYWORD_WHILE) {
                  matchFlowControl(textRange.chrg, "While", "EndWhile", {});
                }
              } else if (utility::compare(currentWord, "EndWhile")) {
                if (settings.enabledKeywords & KEYWORD_WHILE) {
                  matchFlowControl(textRange.chrg, "EndWhile", "While", {}, false);
                }
              } else if (utility::compare(currentWord, "If")) {
                if (settings.enabledKeywords & KEYWORD_IF) {
                  matchFlowControl(textRange.chrg, "If", "EndIf", (settings.enabledKeywords & KEYWORD_ELSE) ? otherFlowControlHighlightingWords : emptyWords);
                }
              } else if (utility::compare(currentWord, "EndIf")) {
                if (settings.enabledKeywords & KEYWORD_IF) {
                  matchFlowControl(textRange.chrg, "EndIf", "If", (settings.enabledKeywords & KEYWORD_ELSE) ? otherFlowControlHighlightingWords : emptyWords, false);
                }
              } else if (utility::compare(currentWord, "Else") || utility::compare(currentWord, "ElseIf")) {
                if ((settings.enabledKeywords & KEYWORD_IF) && (settings.enabledKeywords & KEYWORD_ELSE)) {
                  matchFlowControl(textRange.chrg, "If", "EndIf", otherFlowControlHighlightingWords);
                  matchFlowControl(textRange.chrg, "EndIf", "If", otherFlowControlHighlightingWords, false);
                }
              }
            }
          }
        }
      }
    }
  }

  void KeywordMatcher::matchKeyword(Sci_CharacterRange currentWordPos, word_list_t matchingWords, bool searchForward) {
    SavedSearch savedSearch(handle);
    Sci_PositionCR searchStart = searchForward ? currentWordPos.cpMax : currentWordPos.cpMin;
    Sci_PositionCR searchEnd = searchForward ? docLength : 0;
    Sci_PositionCR matchedStart = searchEnd;
    Sci_PositionCR matchedEnd = searchEnd;

    // Find the one closest to current word in matching words list
    for (const auto& matchingWord : matchingWords) {
      auto found = findText(matchingWord, searchStart, searchEnd, SearchWordType::Keyword, searchForward);
      if (found.cpMin != -1) {
        if (searchForward) {
          matchedStart = min(matchedStart, found.cpMin);
          matchedEnd = min(matchedEnd, found.cpMax);
        } else {
          matchedStart = max(matchedStart, found.cpMin);
          matchedEnd = max(matchedEnd, found.cpMax);
        }
      }
    }

    matched = (matchedStart != searchEnd);

    setupIndicator();
    ::SendMessage(handle, SCI_INDICATORFILLRANGE, currentWordPos.cpMin, currentWordPos.cpMax - currentWordPos.cpMin);
    if (matched) {
      ::SendMessage(handle, SCI_INDICATORFILLRANGE, matchedStart, matchedEnd - matchedStart);
    }
  }

  void KeywordMatcher::matchFlowControl(Sci_CharacterRange currentWordPos, const char* currentWord, const char* matchingWord, word_list_t otherWords, bool searchForward) {
    SavedSearch savedSearch(handle);
    result_list_t otherWordsPosList;
    auto found = matchFlowControl(currentWordPos, currentWord, matchingWord, otherWords, otherWordsPosList, searchForward);
    matched = (found.cpMin != -1);

    setupIndicator();
    ::SendMessage(handle, SCI_INDICATORFILLRANGE, currentWordPos.cpMin, currentWordPos.cpMax - currentWordPos.cpMin);
    for (const auto& pos : otherWordsPosList) {
      ::SendMessage(handle, SCI_INDICATORFILLRANGE, pos.cpMin, pos.cpMax - pos.cpMin);
    }

    if (matched) {
      ::SendMessage(handle, SCI_INDICATORFILLRANGE, found.cpMin, found.cpMax - found.cpMin);
    }
  }

  Sci_CharacterRange KeywordMatcher::matchFlowControl(Sci_CharacterRange currentWordPos, const char* currentWord, const char* matchingWord, word_list_t otherWords, result_list_t& otherWordsPosList, bool searchForward) {
    Sci_PositionCR searchStart = searchForward ? currentWordPos.cpMax : currentWordPos.cpMin;
    Sci_PositionCR searchEnd = searchForward ? docLength : 0;

    while ((searchForward && searchStart < searchEnd) || (!searchForward && searchStart > searchEnd)) {
      // Find the matching flow control closest to current word
      auto foundMatchingWord = findText(matchingWord, searchStart, searchEnd, SearchWordType::FlowControl, searchForward);
      if (foundMatchingWord.cpMin == -1) {
        // Can't find matching flow control
        return foundMatchingWord;
      }

      // Find the closest flow control of current word
      auto foundComparison = findText(currentWord, searchStart, searchEnd, SearchWordType::FlowControl, searchForward);

      // Check if the found matching word is closer
      if (foundComparison.cpMin == -1 || (searchForward && foundMatchingWord.cpMin < foundComparison.cpMin) || (!searchForward && foundMatchingWord.cpMin > foundComparison.cpMin)) {
        // In this case, the found matching flow control is the true match. Add other highlighting words from current pos to matching word pos
        findWords(searchStart, searchForward ? foundMatchingWord.cpMin : foundMatchingWord.cpMax, otherWords, otherWordsPosList, SearchWordType::FlowControl, searchForward);
        return foundMatchingWord;
      }

      // Nested flow control. Use recursive method to find the end position of nested block.
      // Add any other highlighting words from current pos to start of nested block, and skip highlighting other words inside nested block
      findWords(searchStart, searchForward ? foundComparison.cpMin : foundComparison.cpMax, otherWords, otherWordsPosList, SearchWordType::FlowControl, searchForward);
      auto nestedFlowControlEnd = matchFlowControl(foundComparison, currentWord, matchingWord, {}, otherWordsPosList, searchForward);

      // If nested control is open-ended, return as well
      if (nestedFlowControlEnd.cpMin == -1) {
        return nestedFlowControlEnd;
      }

      // Use nested flow control block's end postion as new start search position 
      searchStart = searchForward ? nestedFlowControlEnd.cpMax : nestedFlowControlEnd.cpMin;
    }

    // Didn't find anything
    return Sci_CharacterRange { .cpMin = -1 };
  }

  Sci_CharacterRange KeywordMatcher::findText(const char* text, Sci_PositionCR start, Sci_PositionCR end, SearchWordType searchWordType, bool searchForward, int searchFlags) {
    Sci_TextToFind search {
      .chrg = {
        .cpMin = start,
        .cpMax = end
      },
      .lpstrText = text
    };
    while (::SendMessage(handle, SCI_FINDTEXT, searchFlags, reinterpret_cast<LPARAM>(&search)) != -1) {
      // Search result has to be either keyword or flow control, depending on search word type
      int style = static_cast<int>(::SendMessage(handle, SCI_GETSTYLEAT, search.chrgText.cpMin, 0));
      if ((searchWordType == SearchWordType::Keyword && !Lexer::isKeyword(style)) || (searchWordType == SearchWordType::FlowControl && !Lexer::isFlowControl(style))) {
        // Not in correct style, likely a comment or string, search again
        search.chrg.cpMin = searchForward ? search.chrgText.cpMax : search.chrgText.cpMin;
      } else {
        // Found
        return search.chrgText;
      }
    }

    // Didn't find
    search.chrgText.cpMin = -1;
    return search.chrgText;
  }

  void KeywordMatcher::findWords(Sci_PositionCR start, Sci_PositionCR end, word_list_t words, result_list_t& foundPosList, SearchWordType searchWordType, bool searchForward) {
    for (const auto& word : words) {
      Sci_PositionCR searchStart = start;
      Sci_PositionCR searchEnd = end;
      while (true) {
        auto found = findText(word, searchStart, searchEnd, searchWordType, searchForward);
        if (found.cpMin == -1) {
          // No more matches
          break;
        }
        foundPosList.push_back(found);

        // Search again
        searchStart = searchForward ? found.cpMax : found.cpMin;
      }
    }
  }

  void KeywordMatcher::setupIndicator() {
    ::SendMessage(handle, SCI_INDICSETFORE, settings.indicatorID, matched ? settings.matchedIndicatorForegroundColor : settings.unmatchedIndicatorForegroundColor);
    ::SendMessage(handle, SCI_SETINDICATORCURRENT, settings.indicatorID, 0);
    settings.enableKeywordMatching ? showIndicator() : hideIndicator();
  }

  void KeywordMatcher::showIndicator() {
    ::SendMessage(handle, SCI_INDICSETSTYLE, settings.indicatorID, matched ? settings.matchedIndicatorStyle : settings.unmatchedIndicatorStyle);
  }

  void KeywordMatcher::hideIndicator() {
    ::SendMessage(handle, SCI_INDICSETSTYLE, settings.indicatorID, INDIC_HIDDEN);
  }

  void KeywordMatcher::changeIndicator(int oldIndicator) {
    if (handle != 0) {
      ::SendMessage(handle, SCI_SETINDICATORCURRENT, oldIndicator, 0);
      ::SendMessage(handle, SCI_INDICATORCLEARRANGE, 0, docLength);
      match();
    }
  }

} // namespace
