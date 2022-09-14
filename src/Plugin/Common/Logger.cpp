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

#include "Logger.hpp"

#include "Utility.hpp"

#include "..\..\external\gsl\include\gsl\util"

#include <fstream>
#include <sstream>

namespace utility {

  Logger logger;

  Logger::~Logger() {
#ifdef _DEBUG
    if (logFile.is_open()) {
      logFile.close();
    }
#endif
  }

#ifdef _DEBUG
  void Logger::init(const std::wstring& filePath) {
    if (fileExists(filePath)) {
      logFile.open(filePath, std::ios::out | std::ios::app);
    } else {
      logFile.open(filePath, std::ios::out);
    }
  }
#else
  void Logger::init(const std::wstring&) {
  }
#endif

#ifdef _DEBUG
  void Logger::log(const std::wstring& message) {
    if (logFile.is_open() && !logFile.fail()) {
      logFile << message << std::endl;
    }
  }
#else
  void Logger::log(const std::wstring&) {
  }
#endif

} // namespace
