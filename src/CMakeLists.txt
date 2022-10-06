cmake_minimum_required(VERSION 3.20)

# set the project name
project(PapyrusPlugin)

# compile with C++ standard 23
set(CMAKE_CXX_STANDARD 23)

# null terminate resource strings
set(CMAKE_RC_FLAGS "/n")

# use Unicode chars
add_definitions(-DUNICODE -D_UNICODE)

# add source files
file(GLOB dllmain_source_files CONFIGURE_DEPENDS DllMain.cpp Exports.def)
file(GLOB tinyxml_source_files CONFIGURE_DEPENDS external/tinyxml2/tinyxml2.cpp)
file(GLOB_RECURSE scintilla_source_files CONFIGURE_DEPENDS external/scintilla/*.cxx)
file(GLOB_RECURSE lexilla_source_files CONFIGURE_DEPENDS external/lexilla/*.cxx)
file(GLOB_RECURSE npp_source_files CONFIGURE_DEPENDS external/npp/*.cpp)
file(GLOB_RECURSE plugin_source_files CONFIGURE_DEPENDS Plugin/*.cpp Plugin/*.rc)

include_directories(external/gsl/include external/scintilla external/lexilla external/npp)

# add output DLL
add_library(Papyrus SHARED ${dllmain_source_files} ${tinyxml_source_files} ${scintilla_source_files} ${lexilla_source_files} ${npp_source_files} ${plugin_source_files})
target_link_libraries(Papyrus Shlwapi.lib)
