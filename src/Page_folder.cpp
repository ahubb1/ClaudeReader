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
#include "Page_folder.h"

//! Constructor of the image folder page.
//!Initialize, load a page, render it and call the cases detection if needed (not a
//!first or last page).
Page_folder::Page_folder(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, int rotation, bool contrast, string imgname):Page(page_nb, nb_pages, rd_mode, page_p, display_ratio, rotation, contrast)
{
    //Load the page=> to an OpenCv Mat
    Load(imgname);

    //Apply image transformation if required
    Transform();

    //Case detection on the page (test is done later for first or last page)
    DetectCases();
}


//! Load the page from the archive (render it and convert it to an OpenCv Mat)
void Page_folder::Load(string imgname)
{
	PageRenderCv = imread(imgname.c_str(), CV_LOAD_IMAGE_COLOR);
	if(!PageRenderCv.data )                              // Check for invalid input
    {
        LoadErrorPage();
	    Failed=true;
    }
}
