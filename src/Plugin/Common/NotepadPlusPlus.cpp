/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2022 blu3mania <blu3mania@hotmail.com>

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

#include "NotepadPlusPlus.hpp"

#include "StringUtil.hpp"

#include "..\..\external\gsl\include\gsl\util"
#include "..\..\external\npp\Notepad_plus_msgs.h"
#include "..\..\external\npp\PluginInterface.h"

namespace utility {

  std::wstring getFilePathFromBuffer(HWND nppHandle, npp_buffer_t bufferID) {
    if (bufferID != 0) {
      // Retrieve the length of the file path so a properly sized buffer can be allocated.
      npp_size_t filePathLength = static_cast<npp_size_t>(::SendMessage(nppHandle, NPPM_GETFULLPATHFROMBUFFERID, static_cast<WPARAM>(bufferID), 0));
      if (filePathLength > 0) {
        wchar_t* filePathCharArray = new wchar_t[filePathLength + 1];
        auto autoCleanup = gsl::finally([&] { delete[] filePathCharArray; });
        if (::SendMessage(nppHandle, NPPM_GETFULLPATHFROMBUFFERID, static_cast<WPARAM>(bufferID), reinterpret_cast<LPARAM>(filePathCharArray)) != -1) {
          return filePathCharArray;
        }
      }
    }

    return std::wstring();
  }

  npp_buffer_t getActiveBufferIdOnView(HWND nppHandle, npp_view_t view) {
    npp_buffer_t bufferID {0};
    // Check whether there is an active doc on the given view.
    npp_index_t docIndex = static_cast<npp_index_t>(::SendMessage(nppHandle, NPPM_GETCURRENTDOCINDEX, 0, static_cast<LPARAM>(view)));
    if (docIndex != -1) {
      bufferID = static_cast<npp_buffer_t>(::SendMessage(nppHandle, NPPM_GETBUFFERIDFROMPOS, static_cast<WPARAM>(docIndex), static_cast<LPARAM>(view)));
    }

    return bufferID;
  }

  std::wstring getApplicableFilePathOnView(HWND nppHandle, npp_view_t view) {
    // Make sure it is a Papyrus script
    std::wstring filePath = getActiveFilePathOnView(nppHandle, view);
    if (endsWith(filePath, L".psc") || endsWith(filePath, L".pas")) {
      return filePath;
    }

    return std::wstring();
  }

  void clearIndications(HWND handle, int indicatorID) {
    // Need to specify which indicator to be cleared.
    ::SendMessage(handle, SCI_SETINDICATORCURRENT, indicatorID, 0);
    npp_length_t docLength = ::SendMessage(handle, SCI_GETLENGTH, 0, 0);
    ::SendMessage(handle, SCI_INDICATORCLEARRANGE, 0, docLength);
  }

} // namespace
