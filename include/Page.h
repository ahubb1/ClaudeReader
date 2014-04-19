#ifndef PAGE_H
#define PAGE_H

#include <FL/Fl.H>          //For fl::wait
extern "C" {
#include <mupdf/fitz.h>
//#include <mupdf/img.h>
}

#include <stlplus3/ntree.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace stlplus;

enum READINGMODE { PAGE, STRIP, CASE };

class Page
{
    public:
        Page(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, fz_context *ctx, fz_document *doc);
        ~Page();

        void DetectCases();
        Mat * RenderCase();

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


    protected:
    private:
        clock_t begin_t;    //TODO A ENLEVER
        vector<Rect> Cases;
        int NbCases;
        int PageNb;
        int NbPages;
        READINGMODE ReadingMode;
        bool PreviewPage;
        int ActiveCase;
        fz_context *FzCtx;
        fz_document *FzDoc;
        fz_page *FzPage;

        Mat PageRenderCv;
        Mat ImgRender;
};

#endif // PAGE_H
