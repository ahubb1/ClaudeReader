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
 * \file FileUtf8.h
 * \brief Thread safe file functions with UTF-8 paths.
 *
 * The FLTK ones (fl_fopen, fl_stat, fl_make_path...) convert the paths in a shared static buffer on Windows:
 * they must only be used from the user interface thread. These ones can be used from any thread.
 */
#ifndef FILEUTF8_H
#define FILEUTF8_H

#include <stdio.h>
#include <time.h>
#include <string>
#include <vector>

namespace FileUtf8
{
    FILE *Open(const std::string &path, const char *mode);
    //! File information, false if it does not exist
    bool Stat(const std::string &path, long long *size=NULL, time_t *mtime=NULL, bool *is_dir=NULL);
    bool Exists(const std::string &path);
    bool IsDir(const std::string &path);
    //! Create a folder and its parents
    bool MakeDirs(const std::string &path);
    bool Remove(const std::string &path);           //!< File
    bool RemoveDir(const std::string &path);        //!< Empty folder
    //! Rename, replacing an existing file
    bool Rename(const std::string &from, const std::string &to);
    //! Names of the entries of a folder (without "." and "..")
    std::vector<std::string> List(const std::string &dir);
    //! Whole file content
    bool Read(const std::string &path, std::string *data);
    bool Write(const std::string &path, const std::string &data);
}

#endif // FILEUTF8_H
