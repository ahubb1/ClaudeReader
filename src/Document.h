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
/**
 * \file Document.h
 * \brief Declaration file of the Document class
 * \author Vincent Crocher
 * \version 0.2
 * \date June 2014
 *
 *
 */

#ifndef DOCUMENT_H_INCLUDED
#define DOCUMENT_H_INCLUDED

#include <Page.h>


//! Class managing the openned pdf document and creating the different Page objects when needed
class Document
{
    public:
        Document(char *filename);
        ~Document();

        virtual int Open() =0;
        virtual void Close() =0;

        virtual Page *CreatePage(int pagenb) =0;
        int LoadPage();
        int LoadPage(int p);
        void PreloadPages();
        void UpdatePreloadedPages();

        int NextCase();
        int PreviousCase();
        int NextPage();
        int PreviousPage();

        //Mat *Render() {return ImgRender=currentPage->RenderCase();}
        Rect *Render() {return RectRender=currentPage->RenderCase();}
        Mat *RenderPage() {return ImgRender=currentPage->RenderPage();}
        Mat *ThumbnailsRenderCurrent();
        Mat *ThumbnailsRenderPrevious();
        Mat *ThumbnailsRenderNext();


        char *GetFilename() {return Filename;}
        void SetReadingMode(READINGMODE val);
        READINGMODE GetReadingMode() {return ReadingMode;}
        int GetNbOfPages() {return NbPages;}
        int GetCurrentPageNb() {return ActivePage;}
        void SetPagePreview(bool val);
        bool IsPagePreview() {return PagePreview;}
        void SetDisplayRatio(float val);

        Page *currentPage, *nextPage, *previousPage;

    protected:
        char Filename[2048];
        int NbPages;
        int ActivePage;
        bool PagePreview;
        READINGMODE ReadingMode;
        float DisplayRatio;
    private:
        Mat *ImgRender;
        Rect *RectRender;
        Mat *ThumbnailsImgRenderCurrent;
};


#endif // DOCUMENT_H_INCLUDED
