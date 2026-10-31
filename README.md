# ClaudeReader

ClaudeReader is a desktop reader for comics, manga and webtoons. It finds the panels on each page and shows them one at a time, at a readable size. Manga are read in Japanese order, right to left. Webtoons scroll continuously.

It also keeps your comics in a library with folders, tags and reading progress. A built-in store finds and downloads manga into that library.

ClaudeReader runs on Windows and Linux. It is based on [BDReader](http://bdreader.sourceforge.net/) by Vincent Crocher and is free software under the GPL v3 or later.

## Installation

### Windows

ClaudeReader for Windows is a single file, `ClaudeReader.exe`, with nothing to install.

1. Download the Windows package: `ClaudeReader-windows-x64` from the latest successful run of the **Build** workflow (Actions tab).
2. Unzip it anywhere, for example in `Documents\ClaudeReader`.
3. Run `ClaudeReader.exe`. To open comics by double-clicking them, choose ClaudeReader in *Open with* for `.cbz`, `.cbr` or `.pdf` files.

Settings, reading positions and thumbnails are kept in `%APPDATA%\ClaudeReader`. To uninstall, delete the program folder and that folder.

### Linux

A ready-made build for Ubuntu 24.04 is in the `ClaudeReader-linux-x86_64` artifact of the **Build** workflow. Unpack it and run `./ClaudeReader`. It uses the system's OpenCV, libarchive, curl and gumbo libraries; installing the packages listed under *Building from source* provides them.

On other distributions, build it from source (below). Settings are kept in `~/.claudereader`.

### Building from source

You need a C++17 compiler, CMake 3.20 or newer, and these libraries:
- FLTK 1.4
- OpenCV 4 or 5
- MuPDF
- libarchive
- libcurl
- gumbo
- Lua 5.4 or newer, as a static library

**Windows (MSYS2).** Install [MSYS2](https://www.msys2.org), then run this in the *UCRT64* shell:

```sh
pacman -S mingw-w64-ucrt-x86_64-{toolchain,cmake,ninja,pkgconf,fltk,opencv,mupdf,libarchive,curl,gumbo-parser,lua}
./build.sh                  # build/ClaudeReader.exe and the DLLs it needs
deps/build-opencv.sh        # once: a small static OpenCV, for the single exe
./build.sh --static         # build-static/ClaudeReader.exe, no DLLs
```

**Linux (Debian, Ubuntu).** FLTK 1.4 is required. If your distribution only has 1.3, build [FLTK](https://github.com/fltk/fltk) first. Then:

```sh
sudo apt install build-essential cmake ninja-build pkg-config libopencv-dev libmupdf-dev \
    libmujs-dev libjbig2dec0-dev libopenjp2-7-dev libjpeg-dev libfreetype-dev libharfbuzz-dev \
    zlib1g-dev libarchive-dev libcurl4-openssl-dev libgumbo-dev liblua5.4-dev
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=<FLTK 1.4 install folder>
cmake --build build
sudo cmake --install build  # optional: program, menu entry and icon
```

Every push builds and tests both versions; see [`.github/workflows/build.yml`](.github/workflows/build.yml).

## Features

### Reading
- **Panel by panel**: the panels of each page are found automatically and shown one after the other, enlarged to fit the window. You can see the panel alone or highlighted in its page. Settings let you choose the highlight: a frame, a line, corners, or dimming the rest of the page, in any thickness and color.
- **Manga mode**: right-to-left reading order, starting with the top right panel and reading each row right to left. It is remembered for each comic.
- **Four reading modes**:
  - **Panels**: one panel at a time.
  - **Page**: the whole page.
  - **Strip**: rows of panels, for very tall pages.
  - **Scroll**: the pages one under the other, scrolled continuously, for webtoons and manhwa.
- **Comfortable viewing**:
  - fullscreen
  - a side bar with the previous, current and next pages
  - a magnifier (hold the left mouse button)
  - zoom and maximum-zoom options
  - page rotation
- **End page**: after the last page, read the next comic of the folder, start again, go back to the library, or open another comic.
- **Large pages**: very large pages and long strips open quickly without using much memory.

### Formats
- Comic archives: CBZ, CBR, CB7 and CBT, and the plain ZIP, RAR, 7Z and TAR equivalents.
- Documents: PDF, EPUB, XPS, FB2 and MOBI.
- Folders of images: JPEG, PNG, WebP, TIFF, BMP and JPEG 2000.

File names in any language are supported.

### Library
- Your comics as cards with covers and reading progress. Opening a comic goes back to where you left it.
- Sub-folders appear as folders, so you can sort comics into categories, with an Explorer-like path bar. A button switches to one list of all your comics.
- **Tags**:
  - Create, rename, recolor and delete tags in Settings.
  - Tag comics by right-clicking them.
  - Filter by tag, or search for `#tag`.
- Search, filter by read or unread, and sort.

### Store
- Search and download manga from **MangaDex**, **Mangapill** and **Nyaa**. Nyaa gives magnet links for your torrent client.
- Downloads go straight to the library, as CBZ, PDF, ZIP or a folder of images. You can queue several chapters, cancel downloads, and *Read* a chapter directly (it is downloaded first if needed).
- Covers, pages and searches are cached so they load faster the next time.
- New stores can be added as small Lua scripts. See the `README.txt` in the stores folder, which Settings → Store opens.

### Command line
```sh
ClaudeReader comic.cbz                                # open a comic
ClaudeReader -r -e comic.cbz exported/ .jpg           # export every panel as an image (-r: manga order)
ClaudeReader --store list                             # the stores
ClaudeReader --store mangadex search "yotsuba"
ClaudeReader --store mangadex chapters <manga id>
ClaudeReader --store mangadex download <manga id> <chapter id> cbz <library folder>
```

## Keyboard shortcuts

| Key | Action |
|---|---|
| Right / Left | Next / previous panel (next / previous page in Page and Scroll modes) |
| Down / Up | Next / previous page (scroll down / up in Scroll mode) |
| Home / End | First / last page |
| Ctrl+G | Go to a page |
| C, P, S, W | Panels, Page, Strip, Scroll mode |
| M | Manga mode (right to left) |
| Space, right mouse button | Zoom out for a moment (Panels mode) |
| + / - | Zoom on the panel (column width in Scroll mode) |
| Ctrl+I, Ctrl+B | Panel alone or in its page, panel highlight on/off |
| Ctrl+T | Pages side bar |
| F11 or Ctrl+F, Esc | Fullscreen, leave fullscreen |
| Ctrl+L | Library / back to the comic |
| Ctrl+Shift+S | Store |
| Ctrl+, | Settings |
| Ctrl+O, Ctrl+Shift+O | Open a comic, open a folder of images |
| F1 | Help |

## License

ClaudeReader is free software under the GNU General Public License version 3 or later; see [doc/License.txt](doc/License.txt). It is based on BDReader, Copyright (C) 2014-2016 Vincent Crocher.
