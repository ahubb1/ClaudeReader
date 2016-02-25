//--------------------------------------------------------------------------
//
//    Copyright (C) 2014-2016 Vincent Crocher
//
//	  This file is part of BDReader.
//
//    BDReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    BDReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with BDReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Page_folder.h
 * \brief Declaration file of the Page_folder class
 * \author Vincent Crocher
 * \version 0.1
 * \date August 2014
 *
 *
 */
#ifndef PAGE_FOLDER_H
#define PAGE_FOLDER_H

#include <FL/filename.H>

#include "Page.h"

//! Derived from class Page. Intended to load a page (an image) from an image file from a folder in accordance with Document_folder.
class Page_folder : public Page
{
    public:
        Page_folder(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, int rotation, bool contrast, string imgname);
    protected:
        void Load(string imgname);
};

#endif // PAGE_FOLDER_H
