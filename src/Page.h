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
 * \file Page.h
 * \brief Declaration file of the Page class
 * \author Vincent Crocher
 * \version 0.1
 * \date April 2014
 *
 *
 */
#ifndef PAGE_H
#define PAGE_H

#include <FL/Fl.H>          //For fl::wait
#include <stlplus3/ntree.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace stlplus;


enum READINGMODE { PAGE, STRIP, CASE };

//! Class managing document pages: loading, rendering and case detection
//! NOT INTENDED TO BE USED DIRECTLY, USE DERIVED CLASSES INSTEAD
class Page
{
    public:
        Page(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p);
        ~Page();

        void DetectCases();
        Mat * RenderCase();
        Mat * RenderPage();

        int NextCase();
        int PreviousCase();

        int GetNbCases() { return NbCases; }
        void SetNbCases(int val) { NbCases = val; }
        int GetPageNb() { return PageNb; }
        void SetPageNb(int val) { PageNb = val; }
        int GetActiveCase() { return ActiveCase; }
        int SetActiveCase(int val);
        READINGMODE GetReadingMode() {return ReadingMode;}
        void SetReadingMode(READINGMODE val) {ReadingMode=val;}
        bool IsLoaded() {return Loaded;}


    protected:
        vector<Rect> Cases;
        int NbCases;
        int PageNb;
        int NbPages;
        READINGMODE ReadingMode;
        bool PreviewPage;
        int ActiveCase;
        bool Loaded;

        Mat PageRenderCv;
        Mat ImgRender;
};

#endif // PAGE_H
