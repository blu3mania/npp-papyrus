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

#include "SimpleLexerBase.hpp"

#include "..\..\external\scintilla\LexerModule.h"

#include <string>

namespace papyrus {

  // Internal static variables
  namespace {
    const char subStyleBases[] {0};
  }

  Sci_Position SCI_METHOD SimpleLexerBase::WordListSet(int n, const char *wl) {
    if (isUsable()) {
      WordList* wordList = nullptr;
      if (n == 0 || n == 1) {
        // instre1 and instre2
        auto lists = getInstreWordLists();
        if ((size_t)n < lists.size()) {
          wordList = lists.at(n);
        }
      } else if (n < 9) {
        // type1 - type7
        n -= 2;
        auto lists = getTypeWordLists();
        if ((size_t)n < lists.size()) {
          wordList = lists.at(n);
        }
      }

      if (wordList != nullptr) {
        WordList newList;
        newList.Set(wl);
        if (newList != *wordList) {
          wordList->Set(wl);
          return 0;
        }
      }
    }
    return -1;
  }

  const char * SCI_METHOD SimpleLexerBase::GetSubStyleBases() {
    return subStyleBases;
  }

} // namespace
