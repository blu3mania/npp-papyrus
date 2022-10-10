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

#include "Error.hpp"
#include "ErrorAnnotatorSettings.hpp"

#include "..\Common\NotepadPlusPlus.hpp"

#include "..\..\external\npp\PluginInterface.h"

#include <list>
#include <map>
#include <string>

namespace papyrus {

  class ErrorAnnotator {
    public:
      ErrorAnnotator(const NppData& nppData, const ErrorAnnotatorSettings& settings);
      ~ErrorAnnotator();

      // Clear the whole map and all annotations/indications on both views
      void clear();

      // Annotate current buffer if it has errors
      void annotate(const std::vector<Error>& compilationErrors);
      void annotate(npp_view_t view, std::wstring filePath);

    private:
      struct LineError {
        int line;
        std::string message;
        std::list<int> columns;
      };

      using FileErrors = std::list<LineError>;

      // Get current file path on the given view, if it's a applicable
      std::wstring getApplicableFilePathOnView(npp_view_t view) const;

      // Annotate current buffer on a given view, if it has errors
      void annotate(npp_view_t view);

      void clearAnnotations(HWND handle) const;
      void clearIndications(HWND handle) const;
      void clearIndications(HWND handle, int indicator) const;

      void showAnnotations(HWND handle) const;
      void hideAnnotations(HWND handle) const;
      void showIndications(HWND handle) const;
      void hideIndications(HWND handle) const;

      void updateAnnotationStyle();
      void updateAnnotationStyle(npp_view_t view, HWND handle);

      void drawAnnotations(HWND handle, const LineError& lineError) const;

      // Change indiator ID.
      // Scintilla reserves indicator 8-31 for containers. Notepad++ itself uses 8, and SciLexher.h defines most
      // of IDs above 20, which NPP uses.
      // By default 18 is used ny this plugin, but other plugins could cause conflicts, e.g. DSpellCheck uses 19.
      void changeIndicator(int oldIndicator);

      void updateIndicatorStyle();
      void updateIndicatorStyle(HWND handle) const;
      void updateIndicatorStyleOnFile(HWND handle, const std::wstring& filePath);

      void drawIndications(HWND handle, const LineError& lineError) const;

      // Private members
      //
      const NppData& nppData;
      const ErrorAnnotatorSettings& settings;
      std::map<std::wstring, FileErrors> errors;

      int mainViewStyleAssigned {0};
      int secondViewStyleAssigned {0};
  };

} // namespace
