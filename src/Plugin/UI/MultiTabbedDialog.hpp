/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2022 blu3mania <blu3mania@hotmail.com>

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

#include "DialogBase.hpp"

#include <algorithm>
#include <list>
#include <map>

namespace papyrus {

  using tab_id_t = int;

  class MultiTabbedDialog : public DialogBase {

    public:
      inline MultiTabbedDialog(int dialogID, int tabsControlID) : DialogBase(dialogID), tabsControlID(tabsControlID) {}
      virtual ~MultiTabbedDialog();

      inline virtual void addTab(tab_id_t tab, int dialogID, std::wstring text, bool lazyInitialization = true) { addTabAt(tab, dialogID, text, tabs.end(), lazyInitialization); }
      virtual void addTabBefore(tab_id_t tab, int dialogID, std::wstring text, tab_id_t referenceTab, bool lazyInitialization = true) { addTabAt(tab, dialogID, text, findTab(referenceTab), lazyInitialization); };
      virtual void addTabAfter(tab_id_t tab, int dialogID, std::wstring text, tab_id_t referenceTab, bool lazyInitialization = true)  {
        auto iter = findTab(referenceTab);
        addTabAt(tab, dialogID, text, iter == tabs.end() ? iter : std::next(iter), lazyInitialization);
      };

      virtual bool removeTab(tab_id_t tab, bool destroy = false);

      virtual void showTab(tab_id_t tab);

      inline virtual bool isTabDialogCreated(tab_id_t tab) { return getTabHandle(tab) != nullptr; }

    protected:
      void initControls() override;
      INT_PTR handleNotifyMessage(WPARAM wParam, LPARAM lParam) override;

      inline virtual INT_PTR handleTabCommandMessage(tab_id_t tab, WPARAM wParam, LPARAM lParam) { return FALSE; }

      inline virtual HWND getControl(tab_id_t tab, int controlID) const { return ::GetDlgItem(getTabHandle(tab), controlID); };

      inline virtual void initDropdownList(tab_id_t tab, int controlID, const dropdown_options_t& options, int selectedIndex = -1) const { DialogBase::initDropdownList(getTabHandle(tab), controlID, options, selectedIndex); }
      inline virtual bool setDropdownSelectedText(tab_id_t tab, int controlID, LPCWSTR text) const { return DialogBase::setDropdownSelectedText(getTabHandle(tab), controlID, text); }
      inline virtual bool setDropdownSelectedText(tab_id_t tab, int controlID, std::wstring text) const { return DialogBase::setDropdownSelectedText(getTabHandle(tab), controlID, text); }
      inline virtual void setDropdownSelectedIndex(tab_id_t tab, int controlID, int index) const { DialogBase::setDropdownSelectedIndex(getTabHandle(tab), controlID, index); }
      inline virtual int getDropdownSelectedIndex(tab_id_t tab, int controlID) const { return DialogBase::getDropdownSelectedIndex(getTabHandle(tab), controlID); }
      inline virtual void clearDropdownList(tab_id_t tab, int controlID) const { DialogBase::clearDropdownList(getTabHandle(tab), controlID); }

      inline virtual void initColorPicker(tab_id_t tab, ColourPicker& colorPicker, int labelControlID, int width = 30, int height = 30, int xOffset = 8, int yOffset = -8) const { DialogBase::initColorPicker(getTabHandle(tab), colorPicker, labelControlID, width, height, xOffset, yOffset); }

      inline virtual HWND createToolTip(tab_id_t tab, int controlID, LPCWSTR toolTip, int delayTime = 15) const { return DialogBase::createToolTip(getTabHandle(tab), controlID, toolTip, delayTime); }
      inline virtual HWND createToolTip(tab_id_t tab, int controlID, std::wstring toolTip, int delayTime = 15) const { return DialogBase::createToolTip(getTabHandle(tab), controlID, toolTip, delayTime); }
      inline virtual HWND createToolTip(tab_id_t tab, int controlID, int tooltipStringID, int delayTime = 15) const { return DialogBase::createToolTip(getTabHandle(tab), controlID, tooltipStringID, delayTime); }

      inline virtual void setControlVisibility(tab_id_t tab, int controlID, bool show) const { DialogBase::setControlVisibility(getTabHandle(tab), controlID, show); }
      inline virtual void showControl(tab_id_t tab, int controlID) const { DialogBase::showControl(getTabHandle(tab), controlID); }
      inline virtual void hideControl(tab_id_t tab, int controlID) const { DialogBase::hideControl(getTabHandle(tab), controlID); }

      inline virtual void setControlEnabled(tab_id_t tab, int controlID, bool enabled) const { DialogBase::setControlEnabled(getTabHandle(tab), controlID, enabled); }
      inline virtual void enableControl(tab_id_t tab, int controlID) const { DialogBase::enableControl(getTabHandle(tab), controlID); }
      inline virtual void disableControl(tab_id_t tab, int controlID) const { DialogBase::disableControl(getTabHandle(tab), controlID); }

      inline virtual void setChecked(tab_id_t tab, int controlID, bool checked) const { DialogBase::setChecked(getTabHandle(tab), controlID, checked); }
      inline virtual bool getChecked(tab_id_t tab, int controlID) const { return DialogBase::getChecked(getTabHandle(tab), controlID); }

      inline virtual void setText(tab_id_t tab, int controlID, std::wstring text) const { DialogBase::setText(getTabHandle(tab), controlID, text); }
      inline virtual void setText(tab_id_t tab, int controlID, LPCWSTR text) const { DialogBase::setText(getTabHandle(tab), controlID, text); }
      inline virtual std::wstring getText(tab_id_t tab, int controlID) const { return DialogBase::getText(getTabHandle(tab), controlID); };

      inline virtual void onTabDialogCreated(tab_id_t tab) {}
      inline virtual void onTabDialogDestroyed(tab_id_t tab) {}
      inline virtual void onTabVisibilityChanged(tab_id_t tab, bool visible) {}

    private:
      struct Tab {
        tab_id_t tab {0};
        MultiTabbedDialog* multiTabbedDialog {nullptr};
      };

      using tab_list_t = std::list<Tab>;

      struct TabItem {
        HWND handle {NULL};
        int dialogID {0};
      };

      static INT_PTR CALLBACK tabDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

      inline HWND getTabHandle(tab_id_t tab) const { return tabItems.at(tab).handle; }

      inline void createTabDialog(tab_id_t tab);

      inline void setTabVisibility(tab_id_t tab, bool show) {
        ::ShowWindow(getTabHandle(tab), show ? SW_SHOW : SW_HIDE);
        onTabVisibilityChanged(tab, show);
      }

      inline tab_list_t::const_iterator findTab(tab_id_t tab) const {
        return std::find_if(tabs.begin(), tabs.end(),
          [&](const auto& tabInfo) {
            return tabInfo.tab == tab;
          }
        );
      }

      inline int getTabIndex(tab_id_t tab) const { return getTabIndex(findTab(tab)); }
      inline int getTabIndex(tab_list_t::const_iterator pos) const { return static_cast<int>(std::distance(tabs.begin(), pos)); }

      void addTabAt(tab_id_t tab, int dialogID, std::wstring text, tab_list_t::const_iterator pos, bool lazyInitialization);

      // Private members
      //
      const int tabsControlID;
      tab_list_t tabs;
      tab_list_t hiddenTabs;
      std::map<tab_id_t, TabItem> tabItems;
      tab_id_t currentTab {-1};

      RECT tabDialogRect {};
  };

} // namespace
