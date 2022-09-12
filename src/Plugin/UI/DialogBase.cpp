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

#include "..\..\external\npp\NppDarkMode.h"

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

          // Handle dark mode
          NppDarkMode::autoSubclassAndThemeChildControls(getHSelf());

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

        case WM_CTLCOLOREDIT: {
          if (NppDarkMode::isEnabled()) {
            return NppDarkMode::onCtlColorSofter(reinterpret_cast<HDC>(wParam));
          }
          break;
        }

        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC: {
          if (NppDarkMode::isEnabled()) {
            return NppDarkMode::onCtlColorDarker(reinterpret_cast<HDC>(wParam));
          }
          break;
        }

        case WM_PRINTCLIENT: {
          if (NppDarkMode::isEnabled()) {
            // Do not proceed in dark mode
            return TRUE;
          }
          break;
        }

        case WM_ERASEBKGND: {
          if (NppDarkMode::isEnabled()) {
            RECT clientRect {};
            getClientRect(clientRect);
            ::FillRect(reinterpret_cast<HDC>(wParam), &clientRect, NppDarkMode::getDarkerBackgroundBrush());
            return TRUE;
          }
          break;
        }
      }
    }

    return FALSE;
  }

  void DialogBase::initDropdownList(HWND hwnd, int controlID, const dropdown_options_t& options, int selectedIndex) const {
    for (const auto& option : options) {
      ::SendDlgItemMessage(hwnd, controlID, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(option));
    }
    if (selectedIndex >= 0) {
      selectedIndex = min(selectedIndex, static_cast<int>(options.size() - 1));
      setDropdownSelectedIndex(hwnd, controlID, selectedIndex);
    }
  }

  bool DialogBase::setDropdownSelectedText(HWND hwnd, int controlID, LPCWSTR text) const {
    int index = static_cast<int>(::SendDlgItemMessage(hwnd, controlID, CB_FINDSTRINGEXACT, 0, reinterpret_cast<LPARAM>(text)));
    if (index != CB_ERR) {
      setDropdownSelectedIndex(hwnd, controlID, index);
      return true;
    } else {
      return false;
    }
  }

  void DialogBase::initColorPicker(HWND hwnd, ColourPicker& colorPicker, int labelControlID, int width, int height, int xOffset, int yOffset) const {
    colorPicker.init(getHinst(), hwnd);
    auto label = ::GetDlgItem(hwnd, labelControlID);
    RECT rc {};
    ::GetWindowRect(label, &rc);
    POINT p {
      .x = rc.right,
      .y = rc.top
    };
    ::ScreenToClient(hwnd, &p);
    ::MoveWindow(colorPicker.getHSelf(), p.x + xOffset, p.y + yOffset, width, height, TRUE);
  }

  HWND DialogBase::createToolTip(HWND hwnd, int controlID, LPCWSTR toolTip, int delayTime) const {
    auto control = getControl(hwnd, controlID);
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

  std::wstring DialogBase::getText(HWND hwnd, int controlID) const {
    auto control = getControl(hwnd, controlID);
    std::wstring content(static_cast<size_t>(::GetWindowTextLength(control)) + 1, L' ');
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
