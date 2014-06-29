//--------------------------------------------------------------------------
//
//    Copyright (C) 2014 Vincent Crocher
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
 * \file Page_pdf.h
 * \brief Declaration file of the Page_pdf class
 * \author Vincent Crocher
 * \version 0.1
 * \date June 2014
 *
 *
 */

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
