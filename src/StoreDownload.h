//--------------------------------------------------------------------------
//
//    This file is part of ClaudeReader (based on BDReader, Copyright (C) 2014-2016  Vincent Crocher).
//
//    ClaudeReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ClaudeReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ClaudeReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file StoreDownload.h
 * \brief Download of the store chapters and export as CBZ, ZIP, PDF or images
 *
 */
#ifndef STOREDOWNLOAD_H
#define STOREDOWNLOAD_H

#include <string>
#include <atomic>
#include <functional>
#include "LuaSource.h"

enum STORE_FORMAT { FORMAT_CBZ=0, FORMAT_ZIP, FORMAT_PDF, FORMAT_IMAGES };

//! File extension of a format (".cbz"...), empty for the images folder
const char *StoreFormatExtension(STORE_FORMAT format);
//! Format from its name (cbz, zip, pdf, images), FORMAT_CBZ if unknown
STORE_FORMAT StoreFormatFromName(const std::string &name);

//! Name usable as a file name (forbidden characters replaced, length limited)
std::string SafeFileName(const std::string &name);

//! Output of a chapter, without extension: library_dir/Store/<source>/<manga>/<chapter>
std::string ChapterOutputBase(const std::string &library_dir, const LuaSource *source, const StoreEntry &manga, const StoreEntry &chapter);

//! Existing download of a chapter (any format), empty if none
std::string FindDownloadedChapter(const std::string &output_base);

//! Download the pages of a chapter (in parallel, through the cache) and export them.
/** \param progress: called (from the download threads) with the number of pages done and the total
 \param cancel: set to true to stop
 \param output_path: the file (or folder) written
 \return false and an error message on failure (nothing is left half written)*/
bool DownloadChapter(LuaSource *source, const StoreEntry &chapter, const std::string &output_base, STORE_FORMAT format,
                     std::function<void(int, int)> progress, std::atomic<bool> *cancel, std::string *output_path, std::string *error);

#endif // STOREDOWNLOAD_H
