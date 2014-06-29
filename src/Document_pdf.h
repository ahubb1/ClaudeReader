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
#ifndef DOCUMENT_PDF_H
#define DOCUMENT_PDF_H

#include "Document.h"
#include "Page_pdf.h"


class Document_pdf : public Document
{
    public:
        Document_pdf(char *filename);
    protected:
        int Open();
        void Close();
        Page_pdf *CreatePage(int pagenb);
    private:
        //mupdf variables
        fz_context *FzCtx;
        fz_document *FzDoc;
};

#endif // DOCUMENT_PDF_H
