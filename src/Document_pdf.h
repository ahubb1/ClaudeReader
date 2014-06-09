#ifndef DOCUMENT_PDF_H
#define DOCUMENT_PDF_H

#include <Document.h>
#include <Page_pdf.h>


class Document_pdf : public Document
{
    public:
        Document_pdf(char *filename);
    protected:
        int Open();
        void Close();
        Page_pdf *CreatePage(int pagenb);
    private:
        //mupdf variables
        fz_context *FzCtx;
        fz_document *FzDoc;
};

#endif // DOCUMENT_PDF_H
