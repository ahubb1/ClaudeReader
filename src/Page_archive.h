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
 * \file Page_archive.h
 * \brief Declaration file of the Page_archive class
 * \author Vincent Crocher
 * \version 0.2
 * \date July 2014
 *
 *
 */
#ifndef PAGE_ARCHIVE_H
#define PAGE_ARCHIVE_H

//#define LIBARCHIVE_STATIC
//#define LIBZ_STATIC
#include <archive.h>
#include <archive_entry.h>

#include "Page.h"


struct NameNb
{
	string name;
	int nb;
};

//! Derived from class Page. Intended to load a page (an image) from an archive file in accordance with Document_archive.
class Page_archive : public Page
{
    public:
        Page_archive(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, int rotation, bool contrast, bool rtl, archive *arch, archive_entry *entry, int img_idx);
    protected:
        void Load(archive *arch, archive_entry *entry, int img_idx);
};

#endif // PAGE_ARCHIVE_H
