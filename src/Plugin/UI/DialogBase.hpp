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

#include "..\..\external\npp\ColourPicker.h"
#include "..\..\external\npp\StaticDialog.h"

#include <string>
#include <vector>

#include <windows.h>

namespace papyrus {

  using dropdown_options_t = std::vector<LPCWSTR>;

  class DialogBase : public StaticDialog {
    public:
      inline DialogBase(int dialogID) : StaticDialog(), dialogID(dialogID) {}
      inline virtual ~DialogBase() {}

      virtual INT_PTR doDialog();

    protected:
      virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

      virtual void initControls() = 0;
      inline virtual INT_PTR handleCommandMessage(WPARAM wParam, LPARAM lParam) { return FALSE; }
      inline virtual INT_PTR handleNotifyMessage(WPARAM wParam, LPARAM lParam) { return FALSE; }
      inline virtual INT_PTR handleCloseMessage(WPARAM wParam, LPARAM lParam) { return FALSE; }

      inline virtual void hide() const { display(false); }

      virtual HWND getControl(int controlID) const;
      virtual void initDropdownList(int controlID, const dropdown_options_t& options) const;
      virtual void initColorPicker(ColourPicker& colorPicker, int labelControlID, int width = 30, int height = 30, int xOffset = 8, int yOffset = -8) const;
      virtual HWND createToolTip(int controlID, std::wstring toolTip, int delayTime = 15) const;

      inline virtual void setControlVisibility(int controlID, bool show) const { ::ShowWindow(getControl(controlID), show ? SW_SHOW : SW_HIDE); }
      inline virtual void showControl(int controlID) const { setControlVisibility(controlID, true); }
      inline virtual void hideControl(int controlID) const { setControlVisibility(controlID, false); }

      inline virtual void setControlEnabled(int controlID, bool enabled) const { ::EnableWindow(getControl(controlID), enabled); }
      inline virtual void enableControl(int controlID) const { setControlEnabled(controlID, true); }
      inline virtual void disableControl(int controlID) const { setControlEnabled(controlID, false); }

      inline virtual void setChecked(int controlID, bool checked) const { ::SendDlgItemMessage(getHSelf(), controlID, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0); }
      inline virtual bool getChecked(int controlID) const { return (::SendDlgItemMessage(getHSelf(), controlID, BM_GETCHECK, 0, 0) == BST_CHECKED); }

      inline virtual void setText(int controlID, std::wstring text) const { ::SetWindowText(getControl(controlID), text.c_str()); }
      inline virtual void setText(int controlID, LPCWSTR text) const { ::SetWindowText(getControl(controlID), text); }
      virtual std::wstring getText(int controlID) const;

      // Private members
      //
      int dialogID;
  };

} // namespace
