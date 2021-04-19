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

#include <windows.h>

namespace utility {

  using timer_callback_t = std::function<void()>;

  class TimerQueue {
    public:
      // Should not to be discarded immediately
      [[nodiscard]] inline TimerQueue() { queueHandle = ::CreateTimerQueue(); }

      // Disable all copy/move constructors/assignment operators
      TimerQueue(TimerQueue&& other) = delete;

      // Destructor will release timer queue resource
      ~TimerQueue();

      // Convenient conversion operator to return queue handle
      inline operator HANDLE() const noexcept { return queueHandle; }

    private:
      HANDLE queueHandle;
  };

  // Timer allows you to get a callback after some specific time. Naturally, the callback happens on a
  // separate thread so the client function should take synchronization between threads into consideration.
  //
  // Due to the nature of this class, it usually should not be discarded before timer callback is invoked,
  // as that will trigger the destructor and cause unexpected behavior. If the actions defined in the callback
  // are no longer needed, the returned value can be disposed at that time. In such a case it is recommended
  // to call cancel() to make the behavior explicit.
  //
  class Timer {
    public:
      [[nodiscard]] Timer(int interval, timer_callback_t func, bool shortExecution = false, bool onlyOnce = true) noexcept;

      // Disable all copy/move constructors/assignment operators
      Timer(Timer&& other) = delete;

      inline ~Timer() { cancel(); }

      // Only a valid timer will trigger callback
      inline bool isValid() const noexcept { return valid; }

      // Cancel the timer and release timer resource
      void cancel() noexcept;

    private:
      static void CALLBACK callback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

      // Private members
      //
      HANDLE timerID;
      int interval;
      timer_callback_t func;
      bool onlyOnce;
      bool valid;
  };

  // Convenience functions to start a timer. Since Timer disables assignment but there are certainly cases
  // where a member/static variable is reused to keep track of timers of the same kind, smart pointer is
  // returned to make it possible, and ease the client from managing the disposal of previous timer pointers
  // that are no longer used.
  //
  [[nodiscard]] std::unique_ptr<Timer> startTimer(int interval, timer_callback_t&& func, bool shortExecution = false, bool onlyOnce = true) noexcept;

} // namespace
