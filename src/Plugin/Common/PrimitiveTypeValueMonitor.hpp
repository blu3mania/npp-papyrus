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

#include <functional>
#include <vector>

namespace utility {

  template <class T>
  class PrimitiveTypeValueMonitor {
    public:
      using callback_t = std::function<void(T, T)>;

      PrimitiveTypeValueMonitor() noexcept {}
      PrimitiveTypeValueMonitor(const T& value) noexcept : value(value) {}

      operator T() const noexcept { return value; }

      // Only assignment operator is monitored
      const PrimitiveTypeValueMonitor& operator=(const T& newValue) {
        if (value != newValue) {
          T oldValue = std::exchange(value, newValue);
          for (const auto& watcher : watchers) {
            watcher(oldValue, value);
          }
        }
        return *this;
      }

      // Add a watcher (i.e. callback) over value change
      void addWatcher(callback_t watcher) noexcept {
        watchers.push_back(watcher);
      }

      // Remove a watcher, which needs to be the exact instance
      void removeWatcher(callback_t watcher) noexcept {
        auto iter = watchers.find(watchers.begin(), watchers.end(), watcher);
        if (iter != watchers.end()) {
          watchers.erase(iter);
        }
      }

    private:
      T value {};
      std::vector<callback_t> watchers;
  };

} // namespace
