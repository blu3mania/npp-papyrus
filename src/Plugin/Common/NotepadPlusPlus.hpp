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

#include <string>

#include <windows.h>

using npp_view_t      = int;
using npp_lang_type_t = int;
using npp_index_t     = int32_t;
using npp_buffer_t    = intptr_t;
using npp_size_t      = size_t;
using npp_length_t    = intptr_t;
using npp_position_t  = intptr_t;
using npp_ptr_t       = void*;

// These definitions are copied from Notepad++'s menuCmdID.h.
// They are unlikely to change but make sure they are checked and updated as needed
// with each new Notepad++ releases.
#define IDM                       40000
#define IDM_LANG                  (IDM + 6000)
#define IDM_LANGSTYLE_CONFIG_DLG  (IDM_LANG + 1)
#define IDM_ABOUT                 (IDM  + 7000)

// These definitions are copied from Notepad++'s Parameters.h.
// They are unlikely to change but make sure they are checked and updated as needed
// with each new Notepad++ releases.
#define NB_MAX_EXTERNAL_LANG        30

namespace utility {

  // Retrieve the full file path of a document from its Notepad++ buffer ID
  std::wstring getFilePathFromBuffer(HWND nppHandle, npp_buffer_t bufferID);

  // Retrieve the Notepad++ buffer ID of the active document on a given view
  npp_buffer_t getActiveBufferIdOnView(HWND nppHandle, npp_view_t view);

  // Retrieve the full file path of the active document on a given view
  inline std::wstring getActiveFilePathOnView(HWND nppHandle, npp_view_t view) {
    npp_buffer_t bufferID = getActiveBufferIdOnView(nppHandle, view);
    return (bufferID != 0 ? getFilePathFromBuffer(nppHandle, bufferID) : std::wstring());
  }

  // Retrieve the full file path of the active document on a given view, if it is a Papyrus script
  std::wstring getApplicableFilePathOnView(HWND nppHandle, npp_view_t view);

  // Clear existing indications drawn with a given indicator
  void clearIndications(HWND handle, int indicatorID);

} // namespace
