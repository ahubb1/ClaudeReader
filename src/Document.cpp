#include "Document.h"


Document::Document(char *filename)
{
    //Init
    sprintf(Filename, "%s", filename);
    ActivePage=1;
    ReadingMode=PAGE;
    CurrentPage=NULL;
    ImgRender=NULL;
    PagePreview=true;

    //Init and open the pdf file
    // Create a context to hold the exception stack and various caches.
	FzCtx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	// Open the PDF, XPS or CBZ document.
	FzDoc = fz_open_document(FzCtx, Filename);
    // Retrieve the number of pages
	NbPages = fz_count_pages(FzDoc);
}


Document::~Document()
{
    if(CurrentPage)
        delete CurrentPage;
    fz_close_document(FzDoc);
	fz_free_context(FzCtx);
}

int Document::LoadPage()
{
    LoadPage(ActivePage);
}

int Document::LoadPage(int p)
{
    if(p>0 && p<=NbPages)
    {
        //Change the current page
        if(CurrentPage)
            delete CurrentPage;

        ActivePage=p;
        CurrentPage=new Page(ActivePage, NbPages, ReadingMode, PagePreview, FzCtx, FzDoc);
        ImgRender=CurrentPage->RenderCase();

        return 0;
    }

    if(p<1)
       return -1;

    if(p>NbPages)
       return -2;

    return -3;
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
//! /return: The new active page, -1 if new page is negative, -2 if new page is superir to NbPages
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
//! /return: The new active page, -1 if new page is negative, -2 if new page is superir to NbPages
int Document::PreviousPage()
{
    if((ActivePage-1)<1)
       return -1;

    if((ActivePage-1)>NbPages)
       return -2;

    ActivePage--;
    return ActivePage;
}
