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

#include "ErrorsWindow.hpp"

#include "..\Common\Resources.hpp"

#include "..\..\external\npp\Notepad_plus_msgs.h"

#include <filesystem>

#include <commctrl.h>

namespace papyrus {

  ErrorsWindow::ErrorsWindow(HINSTANCE instance, HWND parent, HWND pluginMessageWindow)
   : DockingDlgInterface(IDD_ERRORS_WINDOW), pluginMessageWindow(pluginMessageWindow) {
    DockingDlgInterface::init(instance, parent);
    tTbData data {
      .pszName = L"Papyrus Script Errors",
      .dlgID = -1,
      .uMask = DWS_DF_CONT_BOTTOM,
      .pszModuleName = L"Papyrus.dll"
    };
    create(&data);
    ::SendMessage(parent, NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));
    display(false);
    listView = ::GetDlgItem(getHSelf(), IDC_ERRORS_LIST);
    ListView_SetExtendedListViewStyle(listView, LVS_EX_FULLROWSELECT);
    LVCOLUMN column {
      .mask = LVCF_WIDTH | LVCF_TEXT,
      .cx = 180,
      .pszText = const_cast<LPWSTR>(L"File")
    };
    ListView_InsertColumn(listView, 0, &column);
    column.cx = 100;
    column.pszText = const_cast<LPWSTR>(L"Message");
    ListView_InsertColumn(listView, 1, &column);
    column.cx = 45;
    column.pszText = const_cast<LPWSTR>(L"Line");
    ListView_InsertColumn(listView, 2, &column);
    column.cx = 45;
    column.pszText = const_cast<LPWSTR>(L"Col");
    ListView_InsertColumn(listView, 3, &column);
    resize();
  }

  void ErrorsWindow::show(const std::vector<Error>& compilationErrors) {
    errors = compilationErrors;
    for (int i = 0; i < static_cast<int>(errors.size()); ++i) {
      std::wstring filename = std::filesystem::path(errors[i].file).filename();
      LVITEM item {
        .mask = LVIF_TEXT,
        .iItem = i,
        .pszText = const_cast<LPWSTR>(filename.c_str())
      };
      ListView_InsertItem(listView, &item);
      item.iSubItem = 1;
      item.pszText = const_cast<LPWSTR>(errors[i].message.c_str());
      ListView_SetItem(listView, &item);
      item.iSubItem = 2;
      std::wstring line = std::to_wstring(errors[i].line);
      item.pszText = const_cast<LPWSTR>(line.c_str());
      ListView_SetItem(listView, &item);
      item.iSubItem = 3;
      std::wstring column = std::to_wstring(errors[i].column);
      item.pszText = const_cast<LPWSTR>(column.c_str());
      ListView_SetItem(listView, &item);
    }
    display();
  }

  // Protected methods
  //

  INT_PTR CALLBACK ErrorsWindow::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
      case WM_SIZE: {
        resize();
        return 0;
      }

      case WM_NOTIFY: {
        NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(lParam);
        if (item->hdr.hwndFrom == listView && item->hdr.code == NM_DBLCLK) {
          if (item->iItem != -1) {
            Error error = errors[item->iItem];
            ::SendMessage(pluginMessageWindow, PPM_JUMP_TO_ERROR, reinterpret_cast<WPARAM>(&error), 0);
          }
          return true;
        } else {
          return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
        }
      }

      default: {
        return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
      }
    }
  }

  // Private methods
  //

  void ErrorsWindow::resize() const {
    RECT windowSize {};
    ::GetClientRect(getHSelf(), &windowSize);
    ::SetWindowPos(listView, HWND_TOP, 2, 2, windowSize.right - windowSize.left - 4, windowSize.bottom - windowSize.top - 2, 0);
    int width = ListView_GetColumnWidth(listView, 0) + ListView_GetColumnWidth(listView, 2) + ListView_GetColumnWidth(listView, 3) + 8;
    LONG messageColWidth = windowSize.right - windowSize.left - width;
    ListView_SetColumnWidth(listView, 1, messageColWidth);
  }

  void ErrorsWindow::clear() {
    ListView_DeleteAllItems(listView);
    errors.clear();
  }

} // namespace
