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

#include "..\..\external\lexilla\Accessor.h"
#include "..\..\external\lexilla\WordList.h"
#include "..\..\external\scintilla\ILexer.h"
#include "..\..\external\scintilla\Scintilla.h"

#include <vector>

#include <windows.h>

namespace papyrus {

  using namespace Lexilla;
  using namespace Scintilla;

  using ILexer = ILexer5;

  class SimpleLexerBase : public ILexer {
    public:
      inline SimpleLexerBase(const char* name, int id) : name(name), id(id) {}
      SimpleLexerBase() = delete;

      inline virtual ~SimpleLexerBase() {}

      // ILexer4 interface
      inline virtual int SCI_METHOD Version() const override { return lvRelease5; }
      inline virtual void SCI_METHOD Release() override { delete this; }
      inline virtual const char * SCI_METHOD PropertyNames() override { return ""; }
      inline virtual int SCI_METHOD PropertyType(const char* name) override { return 0; }
      inline virtual const char * SCI_METHOD DescribeProperty(const char* name) override { return ""; }
      inline virtual Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override { return -1; }
      inline virtual const char * SCI_METHOD DescribeWordListSets() override { return ""; }
      virtual Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;
      virtual void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess) = 0;
      virtual void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess) = 0;
      inline virtual void * SCI_METHOD PrivateCall(int operation, void* pointer) { return nullptr; }
      inline virtual int SCI_METHOD LineEndTypesSupported() override { return SC_LINE_END_TYPE_DEFAULT; }
      inline virtual int SCI_METHOD AllocateSubStyles(int styleBase, int numberStyles) override { return -1; }
      inline virtual int SCI_METHOD SubStylesStart(int styleBase) override { return -1; }
      inline virtual int SCI_METHOD SubStylesLength(int styleBase) override { return 0; }
      inline virtual int SCI_METHOD StyleFromSubStyle(int subStyle) override { return subStyle; }
      inline virtual int SCI_METHOD PrimaryStyleFromStyle(int style) override { return style; }
      inline virtual void SCI_METHOD FreeSubStyles() override {}
      inline virtual void SCI_METHOD SetIdentifiers(int style, const char* identifiers) override {}
      inline virtual int SCI_METHOD DistanceToSecondaryStyles() override { return 0; }
      virtual const char * SCI_METHOD GetSubStyleBases() override;
      inline virtual int SCI_METHOD NamedStyles() override { return 0; }
      inline virtual const char * SCI_METHOD NameOfStyle(int style) override { return ""; }
      inline virtual const char * SCI_METHOD TagsOfStyle(int style) override { return ""; }
      inline virtual const char * SCI_METHOD DescriptionOfStyle(int style) override { return ""; }

      // ILexer5 methods
      inline virtual const char * SCI_METHOD GetName() override { return name; }
      inline virtual int SCI_METHOD  GetIdentifier() override { return id; }
      inline virtual const char * SCI_METHOD PropertyGet(const char *key) override { return ""; }

    protected:
      // Whether current lexer is usable.
      inline virtual bool isUsable() const { return true; }

      // A list of WordList pointers for instre1 & 2. If not all instre word lists are supported, just return a partial list (e.g. empty). If a list is skipped, use nullptr.
      virtual const std::vector<WordList*>& getInstreWordLists() const = 0;

      // A list of WordList pointers for type1 - 7. If not all instre word lists are supported, just return a partial list (e.g. type1 - 4). If a list is skipped, use nullptr.
      virtual const std::vector<WordList*>& getTypeWordLists() const = 0;

    private:
      const char* const name;
      const int id;
  };

} // namespace
