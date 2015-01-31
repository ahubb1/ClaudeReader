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
 * \file Document_folder.h
 * \brief Declaration file of the Document_folder class
 * \author Vincent Crocher
 * \version 0.1
 * \date August 2014
 *
 *
 */

#ifndef DOCUMENT_FOLDER_H
#define DOCUMENT_FOLDER_H

#include "Document.h"
#include "Page_folder.h"

//! Derived from Document class. Meant to open and manage folders of images.
class Document_folder : public Document
{
    public:
        Document_folder(char *dirname);
    protected:
        int Open();
        void Close();
        void Order();
        Page_folder *CreatePage(int pagenb);
    private:
        dirent ** FilesList;
        vector<string> ImgNames; //Store the files names and order in the archive
};

#endif // DOCUMENT_FOLDER_H
