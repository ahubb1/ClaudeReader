cmake_minimum_required(VERSION 3.20)
project(ClaudeReader VERSION 1.2.0 LANGUAGES CXX)

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
pkg_check_modules(MUPDF IMPORTED_TARGET mupdf)
set(MUPDF_TARGET PkgConfig::MUPDF)
if(NOT WIN32 AND (NOT MUPDF_FOUND OR MUPDF_LINK_LIBRARIES MATCHES "[.]a(;|$)"))
    # Debian and Ubuntu ship MuPDF as a static library whose mupdf.pc (when there is one) lists none of the
    # libraries it uses: the static libraries and those system libraries
    find_path(MUPDF_INCLUDE_DIR mupdf/fitz.h REQUIRED)
    find_library(MUPDF_LIBRARY NAMES libmupdf.a mupdf REQUIRED)
    find_library(MUPDF_THIRD_LIBRARY NAMES mupdf-third mupdfthird)
    find_library(MUJS_LIBRARY NAMES mujs)
    pkg_check_modules(MUPDF_DEPS REQUIRED IMPORTED_TARGET freetype2 harfbuzz libopenjp2 libjpeg zlib)
    find_library(JBIG2DEC_LIBRARY NAMES jbig2dec REQUIRED)
    add_library(mupdf_libraries INTERFACE)
    target_include_directories(mupdf_libraries INTERFACE ${MUPDF_INCLUDE_DIR})
    target_link_libraries(mupdf_libraries INTERFACE ${MUPDF_LIBRARY}
        $<$<BOOL:${MUPDF_THIRD_LIBRARY}>:${MUPDF_THIRD_LIBRARY}> $<$<BOOL:${MUJS_LIBRARY}>:${MUJS_LIBRARY}>
        PkgConfig::MUPDF_DEPS ${JBIG2DEC_LIBRARY} m)
    set(MUPDF_TARGET mupdf_libraries)
endif()
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)
# Store: HTTP(S), HTML parsing and the Lua sources scripts (Lua built in: static library)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(GUMBO REQUIRED IMPORTED_TARGET gumbo)
# "lua" in MSYS2 and Arch, "lua5.4" in Debian and Ubuntu
pkg_search_module(LUA REQUIRED lua lua5.5 lua5.4 lua-5.5 lua-5.4)
find_library(LUA_STATIC_LIBRARY NAMES liblua.a liblua5.5.a liblua5.4.a liblua-5.4.a
             HINTS ${LUA_LIBRARY_DIRS} REQUIRED)
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
    set(DEPENDENCIES_LIBS ${MUPDF_TARGET} PkgConfig::LIBARCHIVE PkgConfig::CURL PkgConfig::GUMBO)
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
    src/ReaderBar.cpp
    src/WebtoonView.cpp
    src/AnnTags.cpp
    src/AnnDialog.cpp
    src/Updater.cpp
)
if(WIN32)
    list(APPEND SOURCES src/WinRC.rc)
endif()
if(CLAUDEREADER_STATIC)
    list(APPEND SOURCES src/StaticImports.cpp)
endif()

find_package(Threads REQUIRED)  # Store downloads and covers (std::thread)

add_executable(ClaudeReader ${SOURCES})
target_include_directories(ClaudeReader PRIVATE src ${LUA_INCLUDE_DIRS} ${DEPENDENCIES_INCLUDES})
target_compile_definitions(ClaudeReader PRIVATE CLAUDEREADER_VERSION="${PROJECT_VERSION}")
target_compile_options(ClaudeReader PRIVATE ${DEPENDENCIES_CFLAGS})
# MuPDF first: with a static libmupdf.a, the shared libraries it uses must come after it
# (the linker drops a shared library seen before anything needs it, e.g. libjpeg through FLTK on Ubuntu)
target_link_libraries(ClaudeReader PRIVATE
    ${DEPENDENCIES_LIBS}
    fltk::images fltk::fltk
    ${OpenCV_LIBS}
    ${LUA_STATIC_LIBRARY} ${CMAKE_DL_LIBS}   # Lua loads C modules with dlopen on Linux
    Threads::Threads
    $<$<PLATFORM_ID:Windows>:dwmapi>
    $<$<PLATFORM_ID:Windows>:bcrypt>     # SHA-256 of the downloaded updates
)
if(TARGET mupdf_libraries)
    # The static libmupdf.a of Debian/Ubuntu and the shared libraries used by it and by OpenCV (libjpeg, libopenjp2...):
    # keep every listed library instead of dropping those seen before something needs them
    target_link_options(ClaudeReader PRIVATE -Wl,--no-as-needed)
endif()
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
