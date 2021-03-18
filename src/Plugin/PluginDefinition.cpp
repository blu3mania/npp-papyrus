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

#include "Plugin.hpp"

namespace papyrus {

  const TCHAR* getName() {
    return papyrusPlugin.name();
  }

  BOOL isUnicode() {
    return papyrusPlugin.useUnicode();
  }

  void setInfo(NppData nppData) {
    papyrusPlugin.setNppData(nppData);
  }

  FuncItem* getFuncsArray(int* count) {
    *count = papyrusPlugin.numFuncs();
    return papyrusPlugin.getFuncs();
  }

  void beNotified(SCNotification* notification) {
    papyrusPlugin.onNotification(notification);
  }

  LRESULT messageProc(UINT message, WPARAM wParam, LPARAM lParam) {
    return papyrusPlugin.handleNppMessage(message, wParam, lParam);
  }

} // namespace
