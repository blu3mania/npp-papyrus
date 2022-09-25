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

#include "DateTimeUtil.hpp"

#include <chrono>

namespace utility {

  using Clock = std::chrono::system_clock;

  int currentYear() noexcept {
    struct tm time {};
    auto now = Clock::to_time_t(Clock::now());
    localtime_s(&time, &now);
    return time.tm_year + 1900;
  }

} // namespace
