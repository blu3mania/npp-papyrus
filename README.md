# Papyrus Plugin for Notepad++
[![GPL v3 License](https://img.shields.io/badge/License-GPL%20v3-blue)](https://raw.githubusercontent.com/blu3mania/npp-papyrus/main/LICENSE)
[![Visual Studio 2022](https://img.shields.io/badge/Visual%20Studio-2022-blue?logo=visual-studio)](https://visualstudio.microsoft.com/downloads/)
[![Visual Studio Code](https://img.shields.io/badge/Visual%20Studio%20Code-grey?logo=visual-studio-code)](https://code.visualstudio.com/)
[![C++](https://img.shields.io/badge/c++-blue.svg?&logo=c%2B%2B)](https://www.open-std.org/jtc1/sc22/wg21/docs/standards)
[![Notepad++ Plugin](https://img.shields.io/badge/Notepad++-Plugin-blue.svg?&logo=notepad%2B%2B)](https://notepad-plus-plus.org/)

[![Build Status](https://github.com/blu3mania/npp-papyrus/workflows/Build/badge.svg?branch=main)](https://github.com/blu3mania/npp-papyrus/actions/workflows/build.yml)
[![CodeQL Status](https://github.com/blu3mania/npp-papyrus/workflows/CodeQL/badge.svg?branch=main)](https://github.com/blu3mania/npp-papyrus/actions/workflows/codeql-analysis.yml)
[![Microsoft C++ Code Analysis Status](https://github.com/blu3mania/npp-papyrus/workflows/Microsoft%20C++%20Code%20Analysis/badge.svg?branch=main)](https://github.com/blu3mania/npp-papyrus/actions/workflows/msvc-analysis.yml)
[![Latest Release](https://img.shields.io/github/v/release/blu3mania/npp-papyrus)](https://github.com/blu3mania/npp-papyrus/releases/latest)

This plugin adds support for [Bethesda](https://bethesdagamestudios.com/)'s *Papyrus* scripting language to
[Notepad++](https://notepad-plus-plus.org/).
It provides syntax highlighting with automatic recognition of ***class names/functions/properties***,
supports ***keywords matching*** and ***hyperlinks to referenced scripts***.
It also comes with a compiler that can provide ***compilation errors*** in a separate list window, as well
as ***inline annotation*** and ***indication*** where errors are reported, plus ***anonymization*** of
compiled *.pex* files.

This plugin is derived from the original [PapyrusPlusPlus](https://github.com/tschilkroete/PapyrusPlusPlus)
plugin created by [tschilkroete](https://www.nexusmods.com/skyrim/users/20418429), with many bug fixes,
enhancements, and made to work with the latest Notepad++ release.


## Changes from original work
### Bug fixes
- **[Lexer]** Syntax highlighting with lexer now properly works with the latest Notepad++ version, so you no
  longer need to use a separate user-defined language XML, which defeats the purpose of a *lexer*.
- **[Compiler]** Can now handle huge compilation error list (this usually happens when a referenced script
  has errors or referenced script source does not exist), so that status message no longer gets stuck at
  *"Compiling..."*.
- **[Compiler]** In error list window, clicking on an error from a file that has not been opened yet will now
  correctly move cursor to the error line.
- **[Compiler]** Any exceptions while trying to compile the script are now properly shown to user (exceptions
  likely won't happen anyway).
- **[Lexer]** Operators are now correctly styled with defined *Operator* style, instead of the wrong *Type*
  style.
- **[Lexer]** Proper syntax highlighting with strings that contain double quote escapes.
- **[Lexer]** Proper syntax highlighting with integer literals that start with minus sign.
- **[Lexer]** Proper syntax highlighting with float literals that contain decimal point.
- **[Lexer]** Proper syntax highlighting with comments where "/" appears before/after ";" with spaces in
  between.
- **[Lexer]** Word *"property"* in comments is now correctly excluded from property handling.
- **[Lexer]** Correctly detect properties in edge cases like copying property lines and then editing.
- **[Lexer]** White spaces are now styled correctly.
- **[Compiler]** Only run compiler if active file is using Papyrus Script lexer. Configurable behavior,
  default on.
- **[Compiler]** Properly release process handle after compilation.

### Improvements
- **[Lexer]** Upgrade to support Scintilla's *ILexer5*.
- **[Lexer]** Support folding on properties. Original plugin likely omitted properties from folding since it
  could not exclude those that have definitions done in a single line.
- **[Lexer]** Support *"folding in code, middle"* so that *Else* and *ElseIf* can be folded as well.
  Configurable behavior, default on.
- **[Compiler]** Compilation error list window will be hidden when starting a new compilation.
- **[Compiler]** Compilation error list window will not contain duplicate error messages.
- **[Compiler]** Support more compilation flags:
  - -optimize (all games)
  - -release and -final (*Fallout 4*)
- **[Compiler]** Handle the rare compilation error cases with "-op" flag when errors are reported on *.pas*
  files.
- **[Compiler]** Handle generic compilation errors that are not reported on source files or *.pas* files, e.g.
  when one of the import directories is invalid.
- **[Lexer]** Separate the list of Papyrus language defined keywords into 2, so *Parent/Self/True/False/None*
  can be styled differently.
- **[Compiler]** Status bar shows the game name for current Papyrus script file.
- **[Compiler]** Status bar shows compiling status if switching to another file and back while compiling.
- **[Compiler]** When compilation succeeds or fails, status bar shows the file name alongside result message,
  if current file window is not the same as the one that got compiled.
- **[Lexer]** Slightly better performance in syntax highlighting with property name caching, especially with
  big script files.
- **[Lexer]** In addition, class name caching can be turned on in Settings menu. However, there is a caveat.
  See [configuration guide](Configuration.md#class-names-caching) for details. Configurable behavior, default
  off.
- **[Settings]** No more forced setup on startup.
- **[Settings]** Revamped UI with many more settings now configurable.

### New features
- **[Compiler]** Anonymize compiled *.pex* file. In case you are not aware, when you use PapyrusCompiler to
  compile any script your user account and machine name are stored inside the generated *.pex* file, so it's
  a big **privacy concern**.
- **[Annotator]** Show annotation below error lines, and/or show indications where errors are. Configurable
  behavior, default on.
- **[Compiler]** *Skyrim SE/AE* and *Fallout 4* support.
- **[Compiler]** Auto detection of game/compiler settings to be used based on source script file location.
- **[Lexer]** Support of new Papyrus syntax/keywords of *Fallout 4*.
- **[Lexer]** Syntax highlighting of function names.
- **[Lexer]** Class names can be styled as links to open the script files. FO4's namespace support is included.
  Configurable behavior, default on (Ctrl + double click).
- **[Matcher]** Highlight on matching keywords.
- **[Lexer]** A new *Advanced* submenu with:
  - *Show langID* - can be used to find out internal langID assigned to Papyrus Script lexer, which is useful
    if you need to manually configure Notepad++'s functionList feature.
  - *Install auto completion support* - provides auto-completion support for functions defined in base game,
    *SKSE*, and even *SkyUI*.
  - *Install function list support* - allows using *View -> Function List* menu to show all defined functions
    in a Papyrus script file.

### Future plan
- **[Lexer/Compiler]** [Papyrus Projects (PPJ)](https://www.creationkit.com/fallout4/index.php?title=Papyrus_Projects)
  support.


## Download
Get the latest release from [here](https://github.com/blu3mania/npp-papyrus/releases/latest).


## Installation
Please find [installation guide here](Installation.md).

**WARNING:**
- Do not install versions prior to v0.3.0 if you are using Notepad++ v8.3+.
- Do not install v0.3.0+ if you are using a Notepad++ version prior to v8.3.


## Configuration
Please find [configuration guide here](Configuration.md).


## Building
The project comes with the needed Scintilla/Lexilla and Notepad++ files for building. It also references
[GSL](https://github.com/microsoft/GSL) and [TinyXML2](https://github.com/leethomason/tinyxml2) as submodules,
which means if you clone the repository, you should specify *--recurse-submodules* to also get these modules
in your local repository.

- To build the project in *Visual Studio 2022*, just open the solution file in VS2022 and build.
- For those who use *Visual Studio Code*, a *.vscode* folder is provided at src level, with tasks defined and
  the default build task uses MSBuild to generate the *Release|x64* output. **Note**, you need to download and
  install *Build Tools for Visual Studio 2022* from [this page](https://visualstudio.microsoft.com/downloads/).
  Make sure to include *Desktop development with C++* workload during installation.

  Launch VSCode from *Developer Command Prompt for VS 2022* by running *"code ."* from src directory, so that
  environment needed by *MSBuild* is set up properly.
- The third option is cmake. A *CMakeLists.txt* file is provided in src directory. It is recommended to use a
  separate build directory at top level. For example, "cmake -S src -B build" creates a build directory at top
  level and prepares the build environment, then, "cmake --build build --config Release" builds the project in
  release mode.


## Code Structure
```
├── .github - GitHub related files
│   └── workflows - GitHub action workflows
├── .vscode - configuration files for VS Code
├── dist - output folder, used by build script to create the release package
│   └── extras - extra configuration files that can be used in Notepad++
│       ├── autoCompletion - auto completion configuration file for Papyrus scripts
│       ├── functionList - function list configuration file for Papyrus scripts
│       └── userDefineLangs - user-defined Papyrus language instead of this plugin's lexer
└── src - source code
    ├── external - source files from external projects (may be modified)
    │   ├── gsl - references GSL as submodule
    │   ├── lexilla - Lexilla source files
    │   ├── npp - Notepad++ source files
    │   ├── scintilla - Scintilla source files
    │   ├── tinyxml2 - references TinyXML2 as submodule
    │   └── XMessageBox - adopted and modified XMessageBox to provide dark mode support
    └── Plugin - source files of this plugin
        ├── Common - common definitions and utilities shared by all modules
        ├── CompilationErrorHandling - show/annotate compilation errors
        ├── Compiler - invoke Papyrus compiler in a separate thread
        ├── Lexer - Papyrus script lexer that provides syntax highlighting
        ├── KeywordMatcher - matching keywords highlighter
        ├── Settings - read/write Papyrus.ini and provide configuration support to other modules
        └── UI - other UI dialogs, such as About dialog
```


## Disclaimer
Both original work and this plugin are licensed under GPL v3, so make sure you read and understand it if you
are creating derived work. Most importantly, you **cannot** modify the code and only publish binary output
without making the modified code also publicly available.
