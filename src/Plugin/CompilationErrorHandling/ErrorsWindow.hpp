/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2016 - 2017 Tschilkroete <tschilkroete@gmail.com> (original author)
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

#include "..\..\external\npp\DockingDlgInterface.h"
#include "..\..\external\npp\PluginInterface.h"

#include <string>
#include <vector>

#include <windows.h>

namespace papyrus {

  class ErrorsWindow : public DockingDlgInterface {
    public:
      ErrorsWindow(HINSTANCE instance, HWND parent, HWND pluginMessageWindow);

      void show(const std::vector<Error>& compilationErrors);
      inline void hide() { display(false); }
      void clear();

    protected:
      INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

    private:
      void resize() const;

      // Private members
      //
      HWND pluginMessageWindow;
      HWND listView;
      std::vector<Error> errors;
  };

} // namespace
