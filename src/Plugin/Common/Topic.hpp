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
#include <list>
#include <memory>

namespace utility {

  template <class T>
  class Topic {
    public:
      using handler_t = std::function<void(const T&)>;

      // Represents a subscription on the topic
      template <class T>
      class Subscription {
        friend class Topic<T>;

        public:
          using topic_t = Topic<T>;
          using handler_t = topic_t::handler_t;

          [[nodiscard]] inline Subscription(topic_t& topic, handler_t&& func) noexcept : topic(topic), handler(func), subscribed(true) {}
          inline ~Subscription() { unsubscribe(); }

          // Message from subscribed topic
          inline void notify(const T& message) {
            if (subscribed) {
              handler(message);
            }
          }

          // Unsubscribe from topic
          bool unsubscribe() noexcept {
            if (subscribed) {
              if (topic.unsubscribe(this)) {
                return true;
              } else {
                // Somehow not registered with topic. Mark as unsubscribed
                subscribed = false;
              }
            }
            return false;
          }

        private:
          topic_t& topic;
          handler_t handler;
          bool subscribed {false};
      };

      using subscription_t = std::shared_ptr<Subscription<T>>;

      [[nodiscard]] inline Topic() {}

      // Disable all copy/move constructors/assignment operators
      Topic(Topic&& other) = delete;

      inline ~Topic() {
        // Detach all subscriptions
        for (const auto& subscription : subscriptions) {
          subscription->subscribed = false;
        }
      }

      // Overload assignment operator as a convenient way for publisher
      inline const Topic& operator=(const T& value) {
        publish(value);
        return *this;
      }

      inline subscription_t subscribe(handler_t&& func) noexcept {
        subscription_t subscription(new Subscription<T>(*this, std::forward<handler_t>(func)));
        subscriptions.push_back(subscription);
        return subscription;
      }

      bool unsubscribe(Subscription<T>* subscriptionToRemove) noexcept {
        auto iter = std::find_if(subscriptions.begin(), subscriptions.end(),
          [&](const auto& subscription) {
            return subscription.get() == subscriptionToRemove;
          }
        );
        if (iter != subscriptions.end()) {
          (*iter)->subscribed = false;
          subscriptions.erase(iter);
          return true;
        }

        return false;
      }

      // Completion notification from topic
      inline void publish(const T& message) {
        for (const auto& subscription : subscriptions) {
          subscription->notify(message);
        }
      }

    private:
      std::list<subscription_t> subscriptions;
  };

} // namespace
