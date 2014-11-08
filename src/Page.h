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
//    along with BDReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Page.h
 * \brief Declaration file of the Page class
 * \author Vincent Crocher
 * \version 0.8
 * \date June 2014
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
        Page(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio);
        ~Page();

        void LoadErrorPage();

        void DetectCases();

        Mat * RenderCaseMat();
        Rect * RenderCase();
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
        bool HasLoadingFailed() {return Failed;}
        bool HasDetectionFailed() {return DetectionFailed;}
        bool IsLoaded() {return Loaded;}
        void SetDisplayRatio(float val) {DisplayRatio=val;};

    private:
        void DetectionDetect(vector<Rect> *CasesTmp, Mat* src);
        bool DetectionCheck(vector<Rect> *CasesTmp, Mat* src, int nbdetections);
        void DetectionOrganize(vector<Rect> *CasesTmp, Mat* src);
        void DetectionStripCutting();


    protected:
        vector<Rect> Cases;
        Rect WithinRect;
        int NbCases;
        int PageNb;
        int NbPages;
        READINGMODE ReadingMode;
        bool PreviewPage;
        int ActiveCase;
        bool Failed;
        bool Loaded;
        bool DetectionFailed;
        float DisplayRatio;

        Mat PageRenderCv;
        Mat ImgRender;
        Rect RectRender;
};

#endif // PAGE_H
