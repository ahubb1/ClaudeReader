#include "Page_pdf.h"

//! Constructor of the pdf page.
//!Initialize, load a page, render it and call the cases detection if needed (not a
//!first or last page).
Page_pdf::Page_pdf(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, fz_context *fzctx, fz_document *fzdoc):Page(page_nb, nb_pages, rd_mode, page_p)
{
    //Load the page=> to an OpenCv Mat
    Load(fzctx, fzdoc);

    //Case detection on the page (test is done later for first or last page)
    DetectCases();
}


//! Load the page from the pdf document (render it and convert it to an OpenCv Mat)
void Page_pdf::Load(fz_context *fzctx, fz_document *fzdoc)
{
    //Load it (numbering from 0, not 1)
    fz_page *fzpage = fz_load_page(fzdoc, PageNb-1);

    //Transform of the display
    int rotation=0, zoom=300;
    fz_matrix transform;
    fz_rotate(&transform, rotation);
    fz_pre_scale(&transform, zoom / 100.0f, zoom / 100.0f);

    //Page bound
    fz_rect bounds;
    fz_bound_page(fzdoc, fzpage, &bounds);
    fz_transform_rect(&bounds, &transform);

    //Prepare a pixmap
    fz_irect bbox;
    fz_round_rect(&bbox, &bounds);
    fz_pixmap *pix = fz_new_pixmap_with_bbox(fzctx, fz_device_bgr(fzctx), &bbox);
    fz_clear_pixmap_with_value(fzctx, pix, 0xff);

    fz_device *dev = fz_new_draw_device(fzctx, pix);
    fz_run_page_contents(fzdoc, fzpage, dev, &transform, NULL);
    fz_free_device(dev);
    fz_free_page(fzdoc, fzpage);

    //Convert to opencv
    PageRenderCv=Mat::zeros(fz_pixmap_height(fzctx, pix), fz_pixmap_width(fzctx, pix), CV_8UC4);
    memcpy(PageRenderCv.data, fz_pixmap_samples(fzctx, pix), (fz_pixmap_width(fzctx, pix)*fz_pixmap_height(fzctx, pix)*4));

    //Free the pixmap
    fz_drop_pixmap(fzctx, pix);
}
