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
