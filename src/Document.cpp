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
    currentPage=NULL;
    nextPage=NULL;
    previousPage=NULL;
    ImgRender=NULL;
    NbPages=0;
    PagePreview=true;
    DisplayRatio=1.2;
}

//! Main destructor.
//!Free the Pages objects created if some and release free the mudpf context
//!and document if some still open.
Document::~Document()
{
    if(currentPage)
        delete currentPage;
    if(nextPage)
        delete nextPage;
    if(previousPage)
        delete previousPage;
}

//! Load the page designated by the ActivePage variable.
//!Basically make a call to the LoadPage(int p) method with the ActivePage value
//!in parameter.
//! \return the Loadpage(ActivePage) return value
int Document::LoadPage()
{
    return LoadPage(ActivePage);
}

//! Load the page p.
//!The method will first open the document. Then create a Page object with the given page number.
//! \return 0 if the page is succesfully created
//! \return -1 if p is less than 1 (no Page has been created)
//! \return -2 if p is greater than the number of pages of the document (no Page has been created)
//! \return -3 if the document couldn't be opened (NbPages is then 0, no page has been created)
int Document::LoadPage(int p)
{
    if(p>0 && p<=NbPages)
    {
        //Open the document and create the context...
        if(Open()<0)
            return -3;

        //Set the new active page number
        ActivePage=p;

        //If the page asked is already loaded as the NextPage
        if(nextPage && ActivePage==nextPage->GetPageNb())
        {
            //If there is a Current page created
            Page* tmp=NULL;
            if(currentPage)
                tmp=currentPage;

            //If there is a next page already created
            if(nextPage)
                currentPage=nextPage; //Use it as the new current page
            else
                currentPage=CreatePage(ActivePage); //Otherwise create it

            //Delete the old previous page if one
            if(previousPage)
                delete previousPage;
            //Update the previous page: it's now the old current page...
            previousPage=tmp;

            //If the new current page is not ready to be displayed, just wait
            while(!currentPage->IsLoaded())
            {
                Fl::wait(1);
            }
        }
        else
        {
            //If the page asked is already loaded as the PreviousPage
            if(previousPage && ActivePage==previousPage->GetPageNb())
            {
                //If there is a Current page created
                Page* tmp=NULL;
                if(currentPage)
                    tmp=currentPage;

                //If there is a previous page already created
                if(previousPage)
                    currentPage=previousPage; //Use it as the new current page
                else
                    currentPage=CreatePage(ActivePage); //Otherwise create it

                //Delete the old next page if one
                if(nextPage)
                    delete nextPage;
                //Update the next page: it's now the old current page...
                nextPage=tmp;

                //If the new current page is not ready to be displayed, just wait
                while(!currentPage->IsLoaded())
                {
                    Fl::wait(1);
                }

            }
            //If it's a completely new page
            else
            {
                //Change the current page
                if(currentPage)
                    delete currentPage;

                //Create the new page
                currentPage=CreatePage(ActivePage);
            }
        }

        //Anyway get the new rendering from the new current page
        ImgRender=currentPage->RenderCase();

        //Close the document, free the context..
        Close();

        return 0;
    }


    if(p<1)
       return -1;

    if(p>NbPages)
       return -2;

    return -4;
}

//! Preload the surrounding pages (previous and next one if they exists).
//!The method creates the previousPage and nextPage if it's needed: if they are not
//!already created and if they exist in the document. This method is intended to
//!be called after the LoadPage one.
void Document::PreloadPages()
{
    //Open the document and create the context...
    if(Open()<0)
        return;

    //If next page is not already the one loaded (or there is currently no next page)
    if( (!nextPage) || (nextPage && nextPage->GetPageNb()!=(ActivePage+1)) )
    {
        //and if this page exists in the pdf
        if( (ActivePage+1)<=NbPages )
        {
            //Load the new next page
            nextPage=CreatePage(ActivePage+1);
        }
        else
        {
            //otherwise the page doesn't exist
            nextPage=NULL;
        }
    }

    //If previous page is not already the one loaded (or there is currently no previous page)
    if( (!previousPage) || (previousPage && previousPage->GetPageNb()!=(ActivePage-1)) )
    {
        //and if this page exists in the pdf
        if( (ActivePage-1)>0 )
        {
            //Load the new previous page
            previousPage=CreatePage(ActivePage-1);
        }
        else
        {
            //otherwise the page doesn't exist
            previousPage=NULL;
        }
    }

    //Close the document, free the context..
    Close();
}

//! Reload the surrounding pages (previous and next): apply them the Document parameters
//!(ReadingMode and Display ratio) and call DetectCases on it. An updating equivalent
//!to PreloadPages().
void Document::UpdatePreloadedPages()
{
    if(previousPage)
    {
        previousPage->SetReadingMode(ReadingMode);
        previousPage->SetDisplayRatio(DisplayRatio);
        previousPage->DetectCases();
    }
    if(nextPage)
    {
        nextPage->SetReadingMode(ReadingMode);
        nextPage->SetDisplayRatio(DisplayRatio);
        nextPage->DetectCases();
    }
}

//! Return the current Page thumnails view: the whole page view.
Mat *Document::ThumbnailsRender()
{
    return ThumbnailsImgRender=currentPage->RenderPage();
}


//! Go to the next case of the page, or to next page if no more case.
//! \return 0 if a next case exists
//! \return -1 otherwise (a new page has been loaded)
int Document::NextCase()
{
    if(currentPage->NextCase()<0 || ReadingMode==PAGE)
    {
        if(NextPage()>0)
            LoadPage();
        return -1;
    }
    return 0;
}

//! Go to the previous case of the page, or to previous page if no more case.
//! \return 0 if a previous case exists
//! \return -1 otherwise (a new page has been loaded)
int Document::PreviousCase()
{
    if(currentPage->PreviousCase()<0 || ReadingMode==PAGE)
    {
        if(PreviousPage()>0)
            LoadPage();
        return -1;
    }
    return 0;
}

//! Increment the active page by one, if possible (in the range 0 NbPages).
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

//! Decrement the active page by one, if possible (in the range 0 NbPages).
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

//! Change the Document reading mode and apply it to the linked Pages.
//!In order to properly re-set the reading mode to each pages (current, previous and
//! next), the DetectCases() is called for each of them.
void Document::SetReadingMode(READINGMODE val)
{
    //Set the document reading mode
    ReadingMode=val;

    //Set it to each pages and call the DetectCases() since its behaviour depends on the reading mode
    currentPage->SetReadingMode(ReadingMode);
    currentPage->DetectCases();
}


//! Apply a new display ratio to the current Page and call DetectCases() on it.
//! \param val: the new aspect ratio.
void Document::SetDisplayRatio(float val)
{
    //Document value (strored for next pages)
    DisplayRatio=val;

    //Set it to each pages and call the DetectCases() since its behaviour depends on the reading mode
    currentPage->SetDisplayRatio(DisplayRatio);
    currentPage->DetectCases();
}

//! Change the Document page preview mode.
void Document::SetPagePreview(bool val)
{
    //Set it to the document
    PagePreview=val;
    //will be applied to the next Pages loaded
}
