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
 * \version 0.1
 * \date April 2014
 *
 *
 */

#ifndef DOCUMENT_H_INCLUDED
#define DOCUMENT_H_INCLUDED

#include <Page.h>



class Document
{
    public:
        Document(char *filename);
        ~Document();

        int LoadPage();
        int LoadPage(int p);
        void NextCase();
        void PreviousCase();
        int NextPage();
        int PreviousPage();
        Mat *Render() {return ImgRender=CurrentPage->RenderCase();}

        char *GetFilename() {return Filename;}
        void SetReadingMode(READINGMODE val) {ReadingMode=val; CurrentPage->SetReadingMode(ReadingMode);}
        READINGMODE GetReadingMode() {return ReadingMode;}
        int GetNumberOfPages() {return NbPages;}
        int GetCurrentPageNb() {return ActivePage;}
        void SetPagePreview(bool val) {PagePreview=val;}
        bool IsPagePreview() {return PagePreview;}

        Page *CurrentPage;


    private:
        char Filename[1024];
        fz_context *FzCtx;
        fz_document *FzDoc;
        Mat *ImgRender;
        int NbPages;
        int ActivePage;
        bool PagePreview;
        READINGMODE ReadingMode;
};


#endif // DOCUMENT_H_INCLUDED
