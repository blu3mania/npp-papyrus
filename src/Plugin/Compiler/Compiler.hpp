/*
This file is part of Papyrus Plugin for Notepad++.

Copyright (C) 2016 Tschilkroete <tschilkroete@gmail.com> (original author)
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

#include "CompilationRequest.hpp"
#include "CompilerSettings.hpp"

#include "..\CompilationErrorHandling\Error.hpp"

#include <thread>
#include <vector>

#include <windows.h>

namespace papyrus {

  class Compiler {
    public:
      Compiler(HWND messageWindow, const CompilerSettings& settings);

      void start(const CompilationRequest& request);

    private:
      // Compile the given script file in a separate thread
      void compile(CompilationRequest request);

      // Anonymize generated PEX script
      bool anonymizeOutput(const std::wstring& outputFile, std::wstring& errorMsg);

      // Anonymize a field that is at current location. This feild can be "Script path", or "User name", or "Host name"
      void anonymizeCurrentField(std::fstream& file, bool isBigEndian);

      // Read size of a field from PEX header. Skyrim & SSE use big endian, FO4 uses little endian
      int readSize(std::fstream& file, bool isBigEndian);

      // Parse compilation errors and send the result back to plugin message window
      void parseErrors(const std::wstring& errorText, const CompilerSettings::GameSettings& gameSettings, const std::wstring& outputDirectory);

      // Close compilation process
      void closeProcess(const PROCESS_INFORMATION& processInfo, const STARTUPINFO& startupInfo);

      // Send any unexpected "other error message" to plugin main processor, along with last error code from Win32 API
      void sendOtherErrorMessage(const wchar_t* msg);

      // Private members
      //
      const HWND messageWindow;
      const CompilerSettings& settings;
      std::thread compilationThread;
  };

} // namespace
