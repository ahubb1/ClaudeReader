//--------------------------------------------------------------------------
//
//    Copyright (C) 2014 Vincent Crocher
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
 * \file Document_archive.h
 * \brief Declaration file of the Document_archive class
 * \author Vincent Crocher
 * \version 0.2
 * \date July 2014
 *
 *
 */
#ifndef DOCUMENT_ARCHIVE_H
#define DOCUMENT_ARCHIVE_H

#include "Document.h"
#include "Page_archive.h"


class Document_archive : public Document
{
    public:
        Document_archive(char *filename);
    protected:
        int Open();
        void Close();
        void Order();
        Page_archive *CreatePage(int pagenb);
    private:
        struct archive *Arch;
        struct archive_entry *Entry;
        vector<NameNb> ImgNames; //Store the files names and order in the archive
};

#endif // DOCUMENT_ARCHIVE_H
