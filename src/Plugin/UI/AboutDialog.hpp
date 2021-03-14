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

#include "..\Common\Resources.hpp"
#include "..\UI\DialogBase.hpp"

#include "..\..\external\npp\URLCtrl.h"

namespace papyrus {

  class AboutDialog : public DialogBase {
    public:
      inline AboutDialog() : DialogBase(IDD_ABOUT_DIALOG) {}
      ~AboutDialog();

    protected:
      void initControls() override;
      INT_PTR handleCommandMessage(WPARAM wParam, LPARAM lParam) override;

    private:
      URLCtrl homePageLink;
      URLCtrl gpl3Link;
      URLCtrl origAuthorEmail;
      URLCtrl authorEmail;
      URLCtrl nppLink;
      URLCtrl scintillaLink;
      URLCtrl tinyxmlLink;
      URLCtrl gslLink;
  };

} // namespace
