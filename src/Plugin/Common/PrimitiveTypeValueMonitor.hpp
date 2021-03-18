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

#include "Topic.hpp"

#include <functional>

namespace utility {

  template <class T>
  class PrimitiveTypeValueMonitor {
    public:
      template <class T>
      struct ValueChangeEventData {
        T oldValue;
        T newValue;
      };

      using event_data_t = ValueChangeEventData<T>;
      using callback_t = std::function<void(const event_data_t&)>;
      using topic_t = Topic<event_data_t>;
      using subscription_t = topic_t::subscription_t;

      PrimitiveTypeValueMonitor() {}
      PrimitiveTypeValueMonitor(const T& value) noexcept : value(value) {}

      operator T() const noexcept { return value; }

      // Only assignment operator is monitored
      const PrimitiveTypeValueMonitor& operator=(const T& newValue) {
        if (value != newValue) {
          event_data_t eventData {
            std::exchange(value, newValue),
            newValue
          };
          topic.publish(eventData);
        }
        return *this;
      }

      subscription_t subscribe(callback_t&& watcher) noexcept {
        return topic.subscribe(std::forward<callback_t>(watcher));
      }

      bool unsubscribe(subscription_t& watcher) noexcept {
        return watcher.unsubscribe();
      }

    private:
      T value {};
      topic_t topic;
  };

} // namespace
