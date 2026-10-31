//--------------------------------------------------------------------------
//
//    Copyright (C) 2014-2016 Vincent Crocher
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
#include "Page_pdf.h"

//! Constructor of the pdf page.
//!Initialize, load a page, render it and call the cases detection if needed (not a
//!first or last page).
Page_pdf::Page_pdf(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, int rotation, bool contrast, bool rtl, fz_context *fzctx, fz_document *fzdoc):Page(page_nb, nb_pages, rd_mode, page_p, display_ratio, rotation, contrast, rtl)
{
    //Load the page=> to an OpenCv Mat
    Load(fzctx, fzdoc);

    //Apply image transformation if required
    Transform();

    //Case detection on the page (test is done later for first or last page)
    DetectCases();
}


//! Load the page from the pdf document (render it and convert it to an OpenCv Mat)
void Page_pdf::Load(fz_context *fzctx, fz_document *fzdoc)
{
    if(!fzctx || !fzdoc)
    {
        LoadErrorPage();
        Failed=true;
        return;
    }

    float zoom = DEFAULT_DPI / 72.; //Compute zoom value to get the default_dpi value (300dpi)

    //Render the page (numbering from 0, not 1) into a BGR pixmap without alpha
    fz_pixmap *pix = NULL;
    fz_try(fzctx)
    {
        pix = fz_new_pixmap_from_page_number(fzctx, fzdoc, PageNb-1, fz_scale(zoom, zoom), fz_device_bgr(fzctx), 0);
    }
    fz_catch(fzctx)
    {
        #ifdef DEBUG
            printf("Page_pdf:: Error rendering: %s\n", fz_caught_message(fzctx));
        #endif
        LoadErrorPage();
        Failed=true;
        return;
    }

    //Convert to opencv (copy as the pixmap rows can be padded)
    Mat pixWrap(fz_pixmap_height(fzctx, pix), fz_pixmap_width(fzctx, pix), CV_8UC3, fz_pixmap_samples(fzctx, pix), fz_pixmap_stride(fzctx, pix));
    pixWrap.copyTo(PageRenderCv);

    //Free the pixmap
    fz_drop_pixmap(fzctx, pix);
}
