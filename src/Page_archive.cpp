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
#include "Page_archive.h"

//! Constructor of the archive page.
//!Initialize, load a page, render it and call the cases detection if needed (not a
//!first or last page).
Page_archive::Page_archive(int page_nb, int nb_pages, READINGMODE rd_mode, bool page_p, float display_ratio, int rotation, bool contrast, bool rtl, archive *arch, archive_entry *entry, int img_idx):Page(page_nb, nb_pages, rd_mode, page_p, display_ratio, rotation, contrast, rtl)
{
    //Load the page=> to an OpenCv Mat
    Load(arch, entry, img_idx);

    //Apply image transformation if required
    Transform();

    //Case detection on the page (test is done later for first or last page)
    DetectCases();
}


//! Load the page from the archive (render it and convert it to an OpenCv Mat)
void Page_archive::Load(archive *arch, archive_entry *entry, int img_idx)
{
	//Skip the pages until the one we want to load
	int i=0;
	bool found=false;
	while(archive_read_next_header(arch, &entry) == ARCHIVE_OK)
	{
		if(i==img_idx)
		{
			found=true;
			break;
		}
		archive_read_data_skip(arch);
		i++;
	}

	//Read the whole page: archive_read_data can return less than the entry size per call
	std::vector<unsigned char> cv_buff;
	la_ssize_t size=-1;
	if(found)
	{
		unsigned char chunk[65536];
		while((size = archive_read_data(arch, chunk, sizeof(chunk))) > 0)
			cv_buff.insert(cv_buff.end(), chunk, chunk+size);
	}

	//Decode the buffer into a cvMat
	if(size==0 && !cv_buff.empty())
		PageRenderCv = imdecode(cv_buff, IMREAD_COLOR);

	//The file should be valid and size>0 but parsing or decoding errors are possible
	if(PageRenderCv.empty())
	{
	    #ifdef DEBUG
            printf("%d %s\n", (int)size, archive_error_string(arch));
        #endif
	    LoadErrorPage();
	    Failed=true;
	}
}
