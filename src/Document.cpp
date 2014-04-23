//--------------------------------------------------------------------------
//
//    Copyright (C) 2014  Vincent Crocher
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
//    along with BDReader.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
#include "Document.h"


//! Create a new document object according to the given filename.
//!The constructor will initialize the object and try to open the pdf file. This can be
//!tested further using the NbPages variable. A value greater than zero means the document
//!has been successfully openned.
Document::Document(char *filename)
{
    //Init
    sprintf(Filename, "%s", filename);
    ActivePage=1;
    ReadingMode=PAGE;
    CurrentPage=NULL;
    ImgRender=NULL;
    FzCtx=NULL;
    FzDoc=NULL;
    PagePreview=true;

    //Init and open the pdf file
    // Create a context to hold the exception stack and various caches.
	FzCtx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	// Open the PDF, XPS or CBZ document.
	fz_try(FzCtx)
	{
	    FzDoc = fz_open_document(FzCtx, Filename);
	}
	fz_catch(FzCtx)
	{
	    FzDoc=NULL;
	    NbPages = 0;
	    return;
	}

    // Retrieve the number of pages
	NbPages = fz_count_pages(FzDoc);

    //Close the doc and free the context
	if(FzDoc)
        fz_close_document(FzDoc);
    if(FzCtx)
        fz_free_context(FzCtx);
}


//! Main destructor.
//!Free the Pages objects created if some and release free the mudpf context
//!and document if some still open.
Document::~Document()
{
    if(CurrentPage)
        delete CurrentPage;
    if(FzDoc)
        fz_close_document(FzDoc);
    if(FzCtx)
        fz_free_context(FzCtx);
}

//! Load the page designated by the ActivePage variable.
//!Basically call make a call to the LoadPage(int p) method with the ActivePage value
//!in parameter.
//! \return the Loadpage(ActivePage) return value
int Document::LoadPage()
{
    return LoadPage(ActivePage);
}


//! Load the page p.
//!The method will first re-create a mupdf context and try to open the document. Then
//!create a Page object with the given page number.
//! \return 0 if the page is succesfully created.
//! \return -1 if p is less than 1 (no Page has been created).
//! \return -2 if p is greater than the number of pages of the document (no Page has been created).
//! \return -3 if the document couldn't be opened (NbPages is then 0, no page has been created).
int Document::LoadPage(int p)
{
    if(p>0 && p<=NbPages)
    {
        // Create a context to hold the exception stack and various caches.
        FzCtx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
        // Open the PDF, XPS or CBZ document.
        fz_try(FzCtx)
        {
            FzDoc = fz_open_document(FzCtx, Filename);
        }
        fz_catch(FzCtx)
        {
            fz_free_context(FzCtx);
            FzCtx=NULL;
            FzDoc=NULL;
            NbPages = 0;
            return -3;
        }
        //Change the current page
        if(CurrentPage)
            delete CurrentPage;

        ActivePage=p;
        CurrentPage=new Page(ActivePage, NbPages, ReadingMode, PagePreview, FzCtx, FzDoc);
        ImgRender=CurrentPage->RenderCase();

        //Close the doc and free the context
        if(FzDoc)
            fz_close_document(FzDoc);
        if(FzCtx)
            fz_free_context(FzCtx);


        return 0;
    }

    if(p<1)
       return -1;

    if(p>NbPages)
       return -2;

    return -4;
}


void Document::NextCase()
{
    if(CurrentPage->NextCase()<0)
    {
        NextPage();
        LoadPage();
    }
}

void Document::PreviousCase()
{
    if(CurrentPage->PreviousCase()<0)
    {
        PreviousPage();
        LoadPage();
    }
}

//! Increment the active page by one, if possible (in the range 0 NbPages)
//! \return the new active page number
//! \return -1 if new page is negative
//! \return -2 if new page is greater than NbPages
int Document::NextPage()
{
    if((ActivePage+1)<1)
       return -1;

    if((ActivePage+1)>NbPages)
       return -2;

    ActivePage++;
    return ActivePage;
}

//! Decrement the active page by one, if possible (in the range 0 NbPages)
//! \return the new active page number
//! \return -1 if new page is negative
//! \return -2 if new page is greater than NbPages
int Document::PreviousPage()
{
    if((ActivePage-1)<1)
       return -1;

    if((ActivePage-1)>NbPages)
       return -2;

    ActivePage--;
    return ActivePage;
}
