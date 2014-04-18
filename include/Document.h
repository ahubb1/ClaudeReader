#ifndef DOCUMENT_H_INCLUDED
#define DOCUMENT_H_INCLUDED

#include <Page.h>



class Document
{
    public:
        Document(char *filename);
        ~Document();

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
