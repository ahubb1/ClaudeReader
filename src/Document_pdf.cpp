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
#include "Document_pdf.h"

//! Pdf document constructor.
//!First call the default Document constructor and then open and close the doc (to retrieve number of pages).
Document_pdf::Document_pdf(char *filename):Document(filename)
{
    FzCtx=NULL;
    FzDoc=NULL;

    //Init and open the file
    Open();

    //Close the file
    Close();
}

//! Try to open the pdf file, retrieve the number of pages and create the FzCtx and FzDoc.
//! \return 0 if everything ok
//! \return -1 if unable to open the file
int Document_pdf::Open()
{
    // Create a context to hold the exception stack and various caches.
    FzCtx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    // Open the PDF, XPS or CBZ document.
    fz_try(FzCtx)
    {
        FzDoc = fz_open_document(FzCtx, Filename);
        // Retrieve the number of pages
        NbPages = fz_count_pages(FzDoc);
    }
    fz_catch(FzCtx)
    {
        fz_free_context(FzCtx);
        FzCtx=NULL;
        FzDoc=NULL;
        NbPages = 0;
        return -1;
    }

    return 0;
}

//! Close the doc and free the context.
void Document_pdf::Close()
{
    //Close the doc and free the context
    if(FzDoc)
        //TODO: add a try catch to avoid double closing ????
        fz_close_document(FzDoc);
    if(FzCtx)
        fz_free_context(FzCtx);
}

//! Create a new Page_pdf
Page_pdf *Document_pdf::CreatePage(int pagenb)
{
    return new Page_pdf(pagenb, NbPages, ReadingMode, PagePreview, DisplayRatio, Rotation, ContrastEnhancement, FzCtx, FzDoc);
}
