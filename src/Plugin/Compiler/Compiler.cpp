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

#include "Compiler.hpp"

#include "..\Common\Logger.hpp"
#include "..\Common\Resources.hpp"
#include "..\Common\StringUtil.hpp"
#include "..\Lexer\Lexer.hpp"

#include "..\..\external\gsl\include\gsl\util"
#include "..\..\external\npp\Common.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace papyrus {

  constexpr DWORD STDOUT_PIPE_SIZE = 10 * 1024 * 1024;  // Allow up to 10 MiB data to be returned from stdout
  constexpr DWORD STDERR_PIPE_SIZE = 500 * 1024 * 1024; // Allow up to 500 MiB data to be returned from stderr

  Compiler::Compiler(HWND messageWindow, const CompilerSettings& settings)
   : messageWindow(messageWindow), settings(settings) {
  }

  void Compiler::start(const CompilationRequest& request) {
    try {
      if (!compilationThread.joinable()) {
        compilationThread = std::thread([=]() { compile(request); }); // Capture the request by value due to asynchronous nature of thread
      } else {
        ::SendMessage(messageWindow, PPM_OTHER_ERROR, reinterpret_cast<WPARAM>(L"Compilation thread unusable."), reinterpret_cast<LPARAM>(L"Compilation aborted."));
      }
    } catch (const std::system_error&) {
      ::SendMessage(messageWindow, PPM_OTHER_ERROR, reinterpret_cast<WPARAM>(L"Starting compiler in thread failed."), reinterpret_cast<LPARAM>(L"Compilation stopped."));
    }
  }

  // Private methods
  //

  void Compiler::compile(CompilationRequest request) {
    try {
      const CompilerSettings::GameSettings& gameSettings = settings.gameSettings(request.game);
      std::wstring path = gameSettings.compilerPath;
      if (std::ifstream(path).good()) {
        // Determine output file directory
        std::wstring outputDirectory = gameSettings.outputDirectory;
        if (request.useAutoModeOutputDirectory) {
          if (std::filesystem::path(settings.autoModeOutputDirectory).is_absolute()) {
            outputDirectory = settings.autoModeOutputDirectory;
          } else {
            outputDirectory = std::filesystem::path(request.filePath).parent_path() / settings.autoModeOutputDirectory;
          }
        }

        // Determine PapyrusCompiler's working directory
        std::filesystem::path filePath = std::filesystem::path(request.filePath);
        auto scriptName = Lexer::getScriptName(request.bufferID);
        auto scriptNameComponents = utility::split(scriptName, ":");
        for (size_t i = 0; i < scriptNameComponents.size(); ++i) {
          filePath = filePath.parent_path();
        }
        LPCWSTR workingDirectory {filePath.c_str()};

        // Define compiler process.
        std::wstring commandLine =
          L"\"" + path + L"\"" +
          L" \"" + request.filePath + L"\"" +
          L" -i=\"" + gameSettings.importDirectories + L"\"" +
          L" -o=\"" + outputDirectory + L"\"" +
          L" -f=\"" + gameSettings.flagFile + L"\"" +
          (gameSettings.optimizeFlag ? L" -op" : L"") +
          (gameSettings.releaseFlag ? L" -r" : L"") +
          (gameSettings.finalFlag ? L" -final" : L"") +
          L" " + gameSettings.additionalArguments;
        STARTUPINFO startupInfo {
          .dwFlags = STARTF_USESTDHANDLES
        };

        // Setup error output pipe.
        HANDLE outputReadHandle {};
        HANDLE errorReadHandle {};
        SECURITY_ATTRIBUTES attr {};
        attr.bInheritHandle = TRUE;
        if (::CreatePipe(&outputReadHandle, &startupInfo.hStdOutput, &attr, STDOUT_PIPE_SIZE) && ::CreatePipe(&errorReadHandle, &startupInfo.hStdError, &attr, STDERR_PIPE_SIZE)) {
          // Run the process.
          PROCESS_INFORMATION compilationProcess {};
          if (::CreateProcess(nullptr, const_cast<LPWSTR>(commandLine.c_str()), nullptr, nullptr, TRUE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, nullptr, workingDirectory, &startupInfo, &compilationProcess)) {
            if (::WaitForSingleObject(compilationProcess.hProcess, INFINITE) != WAIT_FAILED) {
              DWORD size {};
              if (::PeekNamedPipe(errorReadHandle, nullptr, 0, nullptr, &size, nullptr)) {
                // Check if there are error reported by compiler on stderr.
                if (size > 0) {
                  // Read the error output from pipe.
                  std::vector<char> data(size);
                  if (::ReadFile(errorReadHandle, &data[0], size, nullptr, nullptr)) {
                    std::wstring errorOutput(data.begin(), data.end());
                    parseErrors(errorOutput, gameSettings, outputDirectory);
                  } else {
                    sendOtherErrorMessage(L"ReadFile failed. Compilation stopped.");
                    closeProcess(compilationProcess, startupInfo);
                    return;
                  }
                } else {
                  // Check stdout as well. This is for the rare case that compilation passed but somehow the compiler chokes at .pas file, when optimize flag is used.
                  if (::PeekNamedPipe(outputReadHandle, nullptr, 0, nullptr, &size, nullptr)) {
                    bool hasError = false;
                    if (size > 0) {
                      std::vector<char> data(size);
                      if (::ReadFile(outputReadHandle, &data[0], size, nullptr, nullptr)) {
                        std::wstring stdOutput(data.begin(), data.end());
                        if (stdOutput.find(L"compilation failed") != std::string::npos) {
                          hasError = true;
                          parseErrors(stdOutput, gameSettings, outputDirectory);
                        }
                      }
                    }

                    if (!hasError) {
                      // No error, check if anonymization is needed.
                      if (gameSettings.anonynmizeFlag) {
                        // Output file has the same name as script name (relative path is determined by namepsace), with file extension set as ".pex".
                        std::filesystem::path outputFile = std::filesystem::path(outputDirectory);
                        for (const auto& scriptNameComponent : scriptNameComponents) {
                          outputFile /= scriptNameComponent;
                        }
                        outputFile += ".pex";

                        std::wstring errorMsg;
                        if (anonymizeOutput(outputFile, errorMsg)) {
                          ::SendMessage(messageWindow, PPM_COMPILATION_DONE, PARAM_COMPILATION_WITH_ANONYMIZATION, 0);
                        } else {
                          ::SendMessage(messageWindow, PPM_ANONYMIZATION_FAILED, reinterpret_cast<WPARAM>(&errorMsg), 0);
                        }
                      } else {
                        ::SendMessage(messageWindow, PPM_COMPILATION_DONE, PARAM_COMPILATION_ONLY, 0);
                      }
                    }
                  } else {
                    sendOtherErrorMessage(L"PeekNamedPipe failed on stdout. Compilation stopped.");
                    closeProcess(compilationProcess, startupInfo);
                    return;
                  }
                }
              } else {
                sendOtherErrorMessage(L"PeekNamedPipe failed on stderr. Compilation stopped.");
                closeProcess(compilationProcess, startupInfo);
                return;
              }
            } else {
              sendOtherErrorMessage(L"WaitForSingleObject failed. Compilation stopped.");
              closeProcess(compilationProcess, startupInfo);
              return;
            }

            // Always close child process handles.
            closeProcess(compilationProcess, startupInfo);
          } else {
            sendOtherErrorMessage(L"CreateProcess failed. Compilation stopped.");
            return;
          }
        } else {
          sendOtherErrorMessage(L"CreatePipe failed. Compilation stopped.");
          return;
        }
      } else {
        ::SendMessage(messageWindow, PPM_COMPILER_NOT_FOUND, 0, 0);
      }
    } catch (...) {
      // In case of any exception
      ::SendMessage(messageWindow, PPM_OTHER_ERROR, reinterpret_cast<WPARAM>(L"Running compiler in thread failed."), reinterpret_cast<LPARAM>(L"Compilation stopped."));
    }
    compilationThread.detach();
  }

  bool Compiler::anonymizeOutput(const std::wstring& outputFile, std::wstring& errorMsg) {
    bool noError = true;
    std::fstream file;
    file.open(outputFile, std::ios::binary | std::ios::in | std::ios::out);
    auto autoCleanup = gsl::finally([&] { file.close(); });

    if (file.fail()) {
      wchar_t errorMsgBuffer[512];
      _wcserror_s(errorMsgBuffer, errno);
      errorMsg = std::wstring(errorMsgBuffer);
      noError = false;
    } else {
      // PEX file format (Skyrim & SSE in big endian, FO4 in little endian):
      //   Signature:         4 bytes. Value: 0xDEC057FA (Skyrim & Skyrim SE) or 0xFA57C0DE (Fallout 4)
      //   Major version:     1 byte.
      //   Minor version:     1 byte.
      //   Game ID:           2 bytes.
      //   Compilation time:  8 bytes.
      //   Script path size:  2 bytes.
      //   Script path:       n bytes.
      //   User name size:    2 bytes.
      //   User name:         n bytes.
      //   Host name size:    2 bytes.
      //   Host name:         n bytes.
      unsigned __int32 signature {};
      file.read(reinterpret_cast<char *>(&signature), 4);
      if (signature == 0xDEC057FA || signature == 0xFA57C0DE) {
        bool isBigEndian = (signature == 0xDEC057FA);

        // Skip to "Script path size".
        file.seekg(16);

        // Anonymize "Script path".
        anonymizeCurrentField(file, isBigEndian);

        // Anonymize "User name".
        anonymizeCurrentField(file, isBigEndian);

        // Anonymize "Host name".
        anonymizeCurrentField(file, isBigEndian);
      } else {
        errorMsg = L"Unknown PEX file format: " + outputFile;
        noError = false;
      }
    }

    return noError;
  }

  void Compiler::anonymizeCurrentField(std::fstream& file, bool isBigEndian) {
    // First read current field's size.
    int size = readSize(file, isBigEndian);
    if (size > 0) {
      // Switch to write mode.
      file.seekp(file.tellg());

      // Overwrite with dashes.
      char* buffer = new char[size];
      auto autoCleanup = gsl::finally([&] { delete[] buffer; });
      memset(buffer, '-', size);
      file.write(buffer, size);

      // Switch back to read mode.
      file.seekg(file.tellp());
    }
  }

  int Compiler::readSize(std::fstream& file, bool isBigEndian) {
    std::int16_t size {};
    char* bytes = reinterpret_cast<char*>(&size);
    file.read(bytes, 2);

    if (isBigEndian)
    {
      std::swap(bytes[0], bytes[1]);
    }
    return size;
  }

  void Compiler::parseErrors(const std::wstring& errorText, const CompilerSettings::GameSettings& gameSettings, const std::wstring& outputDirectory) {
    bool hasUnparsableLines = false;
    std::vector<Error> errors;
    std::wstringstream output(errorText);
    std::wstring line;
    while (std::getline(output, line)) {
      try {
        std::wstring lineError = line;
        Error error;
        bool isScriptError = false;
        if (utility::startsWith(lineError, L"<unknown>")) {
          error.file = L"<unknown>";
          lineError.erase(0, 10);
        } else {
          size_t fileExtIndex = utility::indexOf(lineError, L".psc(");
          if (fileExtIndex == std::string::npos && gameSettings.optimizeFlag) {
            fileExtIndex = utility::indexOf(lineError, L".pas(");
            isScriptError = true;
          }

          if (fileExtIndex != std::string::npos) {
            error.file = lineError.substr(0, fileExtIndex + 4);
            if (isScriptError) {
              error.file = std::filesystem::path(outputDirectory) / error.file; // Papyrus compiler doesn't provide full path for .pas files
            }
            lineError.erase(0, fileExtIndex + 5);
          }
        }

        if (!error.file.empty()) {
          if (!isScriptError) { // .psc
            size_t indexComma = lineError.find_first_of(L',');
            error.line = std::stoi(lineError.substr(0, indexComma));

            size_t indexParenthesis = lineError.find_first_of(L')');
            error.column = std::stoi(lineError.substr(indexComma + 1, indexParenthesis - (indexComma - 1)));
            error.message = lineError.substr(indexParenthesis + 3);
          } else { // .pas
            size_t indexParenthesis = lineError.find_first_of(L')');
            error.line = std::stoi(lineError.substr(0, indexParenthesis));
            error.column = 1; // Papyrus compiler doesn't provide column info for .pas files
            error.message = lineError.substr(indexParenthesis + 4);
          }

          // Discard duplicate errors.
          auto iter = std::find_if(errors.begin(), errors.end(),
            [&](const auto& comparisionError) {
              return comparisionError.file == error.file
                && comparisionError.message == error.message
                && comparisionError.line == error.line
                && comparisionError.column == error.column;
            }
          );
          if (iter == errors.end()) {
            errors.push_back(error);
          }
        }
      } catch (...) {
        //log(line);
        hasUnparsableLines = true;
      }
    }

    if (errors.empty()) {
      // In the rare case when error cannot be parsed (likely some errors dumped on stdout that are not related to specific files), send the whole output to error window.
      errors.push_back(Error {
        .message = errorText
      });
    }
    ::SendMessage(messageWindow, PPM_COMPILATION_FAILED, reinterpret_cast<WPARAM>(&errors), hasUnparsableLines);
  }

  void Compiler::closeProcess(const PROCESS_INFORMATION& processInfo, const STARTUPINFO& startupInfo) {
    // Properly close child process handles, i.e. hProcess and hThread
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    // Close pipes assigned to child process.
    CloseHandle(startupInfo.hStdOutput);
    CloseHandle(startupInfo.hStdError);
  }

  void Compiler::sendOtherErrorMessage(const wchar_t* msg) {
    std::wstring errorMsg(L"Error code: " + std::to_wstring(::GetLastError()));
    ::SendMessage(messageWindow, PPM_OTHER_ERROR, reinterpret_cast<WPARAM>(errorMsg.c_str()), reinterpret_cast<LPARAM>(msg));
  }

} // namespace
