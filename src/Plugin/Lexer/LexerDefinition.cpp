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

#include "Lexer.hpp"

#include "..\..\external\scintilla\LexerModule.h"

namespace papyrus {

  int SCI_METHOD GetLexerCount() {
    return 1;
  }

  void SCI_METHOD GetLexerName(int index, char* name, int length) {
    // From NPP's Parameters.h:
    // #define MAX_EXTERNAL_LEXER_NAME_LEN 16
    switch (index) {
      case 0: {
        strncpy_s(name, length, Lexer::name(), _TRUNCATE);
        break;
      }
    }
  }

  // Not required anymore, but kept for compatibility with Notepad++ 8.3 - 8.3.3
  void SCI_METHOD GetLexerStatusText(int index, TCHAR* text, int length) {
    // From NPP's Parameters.h:
    // #define MAX_EXTERNAL_LEXER_DESC_LEN 32
    switch (index) {
      case 0: {
        wcsncpy_s(text, length, Lexer::statusText(), _TRUNCATE);
        break;
      }
    }
  }

  // Not required anymore, but kept for compatibility with Notepad++ 8.3 - 8.3.3
  LexerFactoryFunction SCI_METHOD GetLexerFactory(int index) {
    switch (index) {
      case 0: {
        return Lexer::factory;
      }
    }
    return nullptr;
  }

  ILexer* SCI_METHOD CreateLexer(const char* name) {
    if (strcmp(name, Lexer::name()) == 0) {
      return Lexer::factory();
    }
    return nullptr;
  }

} // namespace
