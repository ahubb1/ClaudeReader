cmake_minimum_required(VERSION 3.20)
project(ClaudeReader VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Single exe: every library linked statically, OpenCV from a static build in deps/opencv-static (see build.sh --static)
option(CLAUDEREADER_STATIC "Link every library statically" OFF)
if(CLAUDEREADER_STATIC)
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/deps/opencv-static)
    set(OpenCV_STATIC ON)
endif()

find_package(FLTK 1.4 CONFIG REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)
if(OpenCV_VERSION_MAJOR GREATER_EQUAL 5)
    find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs geometry)
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(MUPDF REQUIRED IMPORTED_TARGET mupdf)
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)
# Store: HTTP(S), HTML parsing and the Lua sources scripts (Lua built in: static library)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(GUMBO REQUIRED IMPORTED_TARGET gumbo)
pkg_check_modules(LUA REQUIRED lua)
find_library(LUA_STATIC_LIBRARY NAMES liblua.a HINTS ${LUA_LIBRARY_DIRS} REQUIRED)
if(CLAUDEREADER_STATIC)
    pkg_check_modules(TIFF REQUIRED libtiff-4)
    find_library(OPENJP2_STATIC_LIBRARY NAMES libopenjp2.a REQUIRED)
    # OpenCV and FLTK refer to some codecs by their import library or shared target: use the static ones
    foreach(target opencv_core opencv_imgproc opencv_imgcodecs opencv_geometry opencv_flann fltk::images)
        if(TARGET ${target})
            get_target_property(deps ${target} INTERFACE_LINK_LIBRARIES)
            list(TRANSFORM deps REPLACE "[.]dll[.]a$" ".a")
            list(TRANSFORM deps REPLACE "^openjp2$" "${OPENJP2_STATIC_LIBRARY}")
            set_target_properties(${target} PROPERTIES INTERFACE_LINK_LIBRARIES "${deps}")
        endif()
    endforeach()
    # MuJS is inside libmupdf.a in MSYS2
    # MuPDF compresses with Brotli too (the .pc lists only the decoder)
    list(REMOVE_ITEM MUPDF_STATIC_LDFLAGS -lmujs)
    list(APPEND MUPDF_STATIC_LDFLAGS -lbrotlienc -lbrotlicommon)
    set(DEPENDENCIES_LIBS ${MUPDF_STATIC_LDFLAGS} ${LIBARCHIVE_STATIC_LDFLAGS} ${CURL_STATIC_LDFLAGS} ${GUMBO_STATIC_LDFLAGS} ${TIFF_STATIC_LDFLAGS})
    set(DEPENDENCIES_CFLAGS ${MUPDF_STATIC_CFLAGS_OTHER} ${LIBARCHIVE_STATIC_CFLAGS_OTHER} ${CURL_STATIC_CFLAGS_OTHER})
    set(DEPENDENCIES_INCLUDES ${MUPDF_STATIC_INCLUDE_DIRS} ${LIBARCHIVE_STATIC_INCLUDE_DIRS} ${CURL_STATIC_INCLUDE_DIRS} ${GUMBO_STATIC_INCLUDE_DIRS})
else()
    set(DEPENDENCIES_LIBS PkgConfig::MUPDF PkgConfig::LIBARCHIVE PkgConfig::CURL PkgConfig::GUMBO)
endif()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/Preferences.cpp
    src/Document.cpp
    src/Page.cpp
    src/Document_pdf.cpp
    src/Page_pdf.cpp
    src/Document_archive.cpp
    src/Page_archive.cpp
    src/Document_folder.cpp
    src/Page_folder.cpp
    src/Export_Document.cpp
    src/Fl_ViewerCV.cxx
    src/Fl_Overlay_Arrow_Button.cxx
    src/Fl_Help_Window.cpp
    src/Fl_Books_Library.cpp
    src/md5.cpp
    src/Theme.cpp
    src/Fl_Path_Bar.cpp
    src/Net.cpp
    src/HtmlQuery.cpp
    src/LuaSource.cpp
    src/StoreSources.cpp
    src/StoreDownload.cpp
    src/StoreCli.cpp
    src/FileUtf8.cpp
    src/Async.cpp
    src/StoreView.cpp
    src/EndPage.cpp
    src/Tags.cpp
    src/SettingsView.cpp
)
if(WIN32)
    list(APPEND SOURCES src/WinRC.rc)
endif()
if(CLAUDEREADER_STATIC)
    list(APPEND SOURCES src/StaticImports.cpp)
endif()

add_executable(ClaudeReader ${SOURCES})
target_include_directories(ClaudeReader PRIVATE src ${LUA_INCLUDE_DIRS} ${DEPENDENCIES_INCLUDES})
target_compile_options(ClaudeReader PRIVATE ${DEPENDENCIES_CFLAGS})
target_link_libraries(ClaudeReader PRIVATE
    fltk::images fltk::fltk
    ${OpenCV_LIBS}
    ${DEPENDENCIES_LIBS}
    ${LUA_STATIC_LIBRARY}
    $<$<PLATFORM_ID:Windows>:dwmapi>
)
if(CLAUDEREADER_STATIC)
    # -static: the .a of every -l library, the GCC and C++ runtimes included; -s: no symbols in the release exe
    target_link_options(ClaudeReader PRIVATE -static $<$<CONFIG:Release>:-s>)
endif()

if(WIN32)
    target_compile_definitions(ClaudeReader PRIVATE __WINDOWS__ NOMINMAX WIN32_LEAN_AND_MEAN)
    set_target_properties(ClaudeReader PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
    if(NOT CLAUDEREADER_STATIC)
        # Ship a self-contained build: copy the MinGW runtime DLLs next to the exe
        add_custom_command(TARGET ClaudeReader POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ClaudeReader> $<TARGET_RUNTIME_DLLS:ClaudeReader>
            COMMAND_EXPAND_LISTS)
    endif()
endif()

install(TARGETS ClaudeReader RUNTIME DESTINATION bin)
if(UNIX)
    install(FILES share/bdreader.desktop DESTINATION share/applications RENAME claudereader.desktop)
    install(FILES share/bdreader.xpm DESTINATION share/pixmaps RENAME claudereader.xpm)
    install(FILES share/bdreader32.xpm DESTINATION share/pixmaps RENAME claudereader32.xpm)
    install(FILES doc/bdreader.1 DESTINATION share/man/man1 RENAME claudereader.1)
endif()
