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

#include "DialogBase.hpp"

#include "..\Common\Resources.hpp"

#include <uxtheme.h>

namespace papyrus {

  void DialogBase::doDialog() {
    if (!isCreated()) {
      create(dialogID);
    }
    display();
  }

  // Protected methods
  //

  INT_PTR CALLBACK DialogBase::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) {
    if (!initializing) {
      switch (message) {
        case WM_INITDIALOG: {
          initializing = true;
          goToCenter();

          auto EnableDlgTheme = reinterpret_cast<ETDTProc>(::SendMessage(getHParent(), NPPM_GETENABLETHEMETEXTUREFUNC, 0, 0));
          if (EnableDlgTheme != nullptr) {
            EnableDlgTheme(getHSelf(), ETDT_ENABLETAB);
          }

          initControls();
          initializing = false;
          break;
        }

        case WM_COMMAND: {
          return handleCommandMessage(wParam, lParam);
        }

        case WM_NOTIFY: {
          return handleNotifyMessage(wParam, lParam);
        }

        case WM_CLOSE: {
          return handleCloseMessage(wParam, lParam);
        }

        default: {
          break;
        }
      }
    }

    return FALSE;
  }

  HWND DialogBase::getControl(int controlID) const {
    return ::GetDlgItem(getHSelf(), controlID);
  }

  void DialogBase::initDropdownList(int controlID, const dropdown_options_t& options, int selectedIndex) const {
    for (const auto& option : options) {
      ::SendDlgItemMessage(getHSelf(), controlID, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(option));
    }
    if (selectedIndex >= 0) {
      selectedIndex = min(selectedIndex, static_cast<int>(options.size() - 1));
      setDropdownSelectedIndex(controlID, selectedIndex);
    }
  }

  void DialogBase::initColorPicker(ColourPicker& colorPicker, int labelControlID, int width, int height, int xOffset, int yOffset) const {
    colorPicker.init(getHinst(), getHSelf());
    auto label = ::GetDlgItem(getHSelf(), labelControlID);
    RECT rc {};
    ::GetWindowRect(label, &rc);
    POINT p {
      .x = rc.right,
      .y = rc.top
    };
    ::ScreenToClient(getHSelf(), &p);
    ::MoveWindow(colorPicker.getHSelf(), p.x + xOffset, p.y + yOffset, width, height, TRUE);
  }

  HWND DialogBase::createToolTip(int controlID, LPCWSTR toolTip, int delayTime) const {
    auto control = getControl(controlID);
    if (!control) {
      return nullptr;
    }

    // Create the tooltip. g_hInst is the global instance handle
    HWND hwndToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, getHSelf(), nullptr, getHinst(), nullptr);
    if (!hwndToolTip) {
      return nullptr;
    }

    // Associate the tooltip with the tool
    TOOLINFO toolInfo {
      .cbSize = sizeof(toolInfo),
      .uFlags = TTF_IDISHWND | TTF_SUBCLASS,
      .hwnd = getHSelf(),
      .uId = reinterpret_cast<UINT_PTR>(control),
      .lpszText = const_cast<LPWSTR>(toolTip)
    };
    if (!::SendMessage(hwndToolTip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&toolInfo))) {
      DestroyWindow(hwndToolTip);
      return nullptr;
    }

    ::SendMessage(hwndToolTip, TTM_ACTIVATE, TRUE, 0);
    ::SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 200);
    
    // Convert delay time to milliseconds
    delayTime *= 1000;
    ::SendMessage(hwndToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM((delayTime), (0)));

    return hwndToolTip;
  }

  std::wstring DialogBase::getText(int controlID) const {
    auto control = getControl(controlID);
    std::wstring content(::GetWindowTextLength(control) + 1, L' ');
    ::GetWindowText(control, const_cast<LPWSTR>(content.c_str()), static_cast<int>(content.size()));
    content.pop_back(); // Remove trailing NULL
    return content;
  }

  LPCWSTR DialogBase::loadResourceString(int stringID) const {
    LPCWSTR str;
    ::LoadString(getHinst(), stringID, reinterpret_cast<LPWSTR>(&str), 0);
    return str;
  }

} // namespace
