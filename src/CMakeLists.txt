cmake_minimum_required(VERSION 3.20)

# set the project name
project(PapyrusPlugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RC_FLAGS "/n")
add_definitions(-DUNICODE -D_UNICODE)

file(GLOB dllmain_source_files CONFIGURE_DEPENDS DllMain.cpp Exports.def)
file(GLOB scintilla_source_files CONFIGURE_DEPENDS external/scintilla/Accessor.cxx external/scintilla/PropSetSimple.cxx external/scintilla/WordList.cxx)
file(GLOB npp_source_files CONFIGURE_DEPENDS external/npp/ColourPicker.cpp external/npp/ColourPopup.cpp external/npp/Common.cpp external/npp/StaticDialog.cpp external/npp/URLCtrl.cpp)
file(GLOB tinyxml_source_files CONFIGURE_DEPENDS external/tinyxml2/tinyxml2.cpp)
file(GLOB_RECURSE plugin_source_files CONFIGURE_DEPENDS Plugin/*.cpp Plugin/*.rc)

include_directories(external/gsl/include external/npp external/scintilla)

# add the executable
add_library(Papyrus SHARED ${dllmain_source_files} ${scintilla_source_files} ${npp_source_files} ${tinyxml_source_files} ${plugin_source_files})
target_link_libraries(Papyrus Shlwapi.lib)
