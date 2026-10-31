//--------------------------------------------------------------------------
//
//    Copyright (C) 2014-2016  Vincent Crocher
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
 * \version 0.9
 * \date June 2015
 *
 *
 */
#ifndef PAGE_H
#define PAGE_H

#ifdef _WIN32
    #include <windows.h> //Before the "using namespace": avoid std::byte and cv::ACCESS_MASK conflicts
#endif
#include <FL/Fl.H>          //For fl::wait
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#if CV_VERSION_MAJOR >= 5
    #include <opencv2/geometry.hpp> //findContours, boundingRect
#endif
#include <vector>
#include <string>
#include <algorithm>

using namespace cv;
using namespace std;

//! Node of the tree used to order the cases of a page (see Page::DetectionOrganize)
struct CaseNode
{
    Rect rect;
    int parent;             //!< Index of the parent node, -1 for the root
    vector<int> children;   //!< Indexes of the children nodes, ordered
};
typedef vector<CaseNode> CasesTree;


//! Pages larger than this (pixels) are reduced for the panels detection (memory)
#define DETECTION_MAX_PIXELS 12000000

enum READINGMODE { PAGE, STRIP, CASE };

//! Class managing document pages: loading, rendering and panels detection. Interface between original image format and OpenCv format.
/** NOT INTENDED TO BE USED DIRECTLY, USE DERIVED CLASSES INSTEAD (Page_archive, Page_pdf...)*/
class Page
{
    public:
        Page(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, int rotation=0, bool contrast=false, bool rtl=false);
        ~Page();

        void LoadErrorPage();

        void Transform();

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
        void SetRightToLeft(bool val) {RightToLeft=val;}
        bool IsLoaded() {return Loaded;}
        void SetDisplayRatio(float val) {DisplayRatio=val;};

    private:
        void DetectionDetect(vector<Rect> *CasesTmp, Mat* src);
        bool DetectionCheck(vector<Rect> *CasesTmp, Mat* src, int nbdetections, const Mat &gray);
        bool DetectionOrganize(vector<Rect> *CasesTmp, Mat* src);
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
        unsigned int Rotation;
        bool ContrastEnhancement;
        bool RightToLeft;           //!< Manga: panels read from right to left

        Mat PageRenderCv;
        Mat ImgRender;
        Rect RectRender;
};

#endif // PAGE_H
