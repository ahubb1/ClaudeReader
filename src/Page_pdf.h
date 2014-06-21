#ifndef PAGE_PDF_H
#define PAGE_PDF_H

extern "C" {
#include <mupdf/fitz.h>
}
#include "Page.h"


class Page_pdf : public Page
{
    public:
        Page_pdf(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, fz_context *fzctx, fz_document *fzdoc);
    protected:
        void Load(fz_context *fzctx, fz_document *fzdoc);
};

#endif // PAGE_PDF_H
