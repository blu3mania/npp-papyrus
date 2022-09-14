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

#include "Timer.hpp"

namespace utility {

  // Static shared timer queue
  namespace {
    TimerQueue timerQueue;
  }

  TimerQueue::~TimerQueue() {
    if (queueHandle) {
      // Cancel all timers. Ignore return value as there is no good handling of errors.
      ::DeleteTimerQueueEx(queueHandle, nullptr);
      queueHandle = nullptr;
    }
  }

  Timer::Timer(int interval, timer_callback_t func, bool shortExecution, bool onlyOnce) noexcept
    : timerID(nullptr), interval(interval), func(std::move(func)), onlyOnce(onlyOnce), valid(false) {
    if ((HANDLE)timerQueue) {
      ULONG flags = (shortExecution ? WT_EXECUTEDEFAULT : WT_EXECUTELONGFUNCTION) | (onlyOnce ? WT_EXECUTEONLYONCE : WT_EXECUTEDEFAULT);
      if (::CreateTimerQueueTimer(&timerID, timerQueue, callback, this, interval, onlyOnce ? 0 : interval, flags)) {
        valid = true;
      }
    }
  }

  void Timer::cancel() noexcept {
    if (valid && timerID) {
      // Cancel timer. Ignore return value as there is no good handling of errors.
      ::DeleteTimerQueueTimer(nullptr, timerID, nullptr);
      valid = false;
      timerID = nullptr;
    }
  }

  void CALLBACK Timer::callback(PVOID lpParameter, BOOLEAN) {
    Timer* timer = static_cast<Timer*>(lpParameter);
    timer->func();
    if (timer->onlyOnce) {
      timer->cancel();
    }
  }

  // Convenience functions to start a timer
  std::unique_ptr<Timer>
  startTimer(int interval, timer_callback_t&& func, bool shortExecution, bool onlyOnce) noexcept {
    return std::make_unique<Timer>(interval, std::forward<timer_callback_t>(func), shortExecution, onlyOnce);
  }

} // namespace
