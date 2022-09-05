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

#include "AboutDialog.hpp"

#include "..\Common\NotepadPlusPlus.hpp"
#include "..\Common\Utility.hpp"

namespace papyrus {

  AboutDialog::~AboutDialog() {
    homePageLink.destroy();
    gpl3Link.destroy();
    origAuthorEmail.destroy();
    authorEmail.destroy();
    nppLink.destroy();
    scintillaLink.destroy();
    tinyxmlLink.destroy();
    gslLink.destroy();
  }

  // Protected methods
  //

  void AboutDialog::initControls() {
    std::wstring versionText = getText(IDC_ABOUT_PRODUCT_VERSION);
    size_t macroIndex = versionText.find(L"[VERSION]");
    if (macroIndex != std::wstring::npos) {
      versionText.replace(macroIndex, 9, PLUGIN_VERSION);
    }
    macroIndex = versionText.find(L"[BUILD]");
    if (macroIndex != std::wstring::npos) {
      versionText.replace(macroIndex, 7, std::to_wstring(BUILD_NUMBER));
    }
    setText(IDC_ABOUT_PRODUCT_VERSION, versionText);

    homePageLink.init(getHinst(), getHSelf());
    homePageLink.create(getControl(IDC_ABOUT_HOMEPAGE_LINK));
    gpl3Link.init(getHinst(), getHSelf());
    gpl3Link.create(getControl(IDC_ABOUT_LICENSE_LINK), L"https://www.gnu.org/licenses/gpl-3.0.en.html");
    origAuthorEmail.init(getHinst(), getHSelf());
    origAuthorEmail.create(getControl(IDC_ABOUT_ORIG_AUTHOR_EMAIL));
    authorEmail.init(getHinst(), getHSelf());
    authorEmail.create(getControl(IDC_ABOUT_AUTHOR_EMAIL));
    nppLink.init(getHinst(), getHSelf());
    nppLink.create(getControl(IDC_ABOUT_LIBRARY_NPP_LINK), IDC_ABOUT_LIBRARY_NPP_LINK);
    scintillaLink.init(getHinst(), getHSelf());
    scintillaLink.create(getControl(IDC_ABOUT_LIBRARY_SCINTILLA_LINK));
    tinyxmlLink.init(getHinst(), getHSelf());
    tinyxmlLink.create(getControl(IDC_ABOUT_LIBRARY_TINYXML_LINK));
    gslLink.init(getHinst(), getHSelf());
    gslLink.create(getControl(IDC_ABOUT_LIBRARY_GSL_LINK));
  }

  INT_PTR AboutDialog::handleCommandMessage(WPARAM wParam, LPARAM lParam) {
    if (wParam == IDC_ABOUT_LIBRARY_NPP_LINK) {
      // Special link for Notepad++'s About dialog. Send a message to activate that menu
      ::SendMessage(getHParent(), NPPM_MENUCOMMAND, 0, IDM_ABOUT);
    }

    return FALSE;
  }

  INT_PTR AboutDialog::handleCloseMessage(WPARAM wParam, LPARAM lParam) {
    hide();

    return FALSE;
  }
} // namespace
