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
#include "Document_archive.h"

//Initialize the list of supported formats
const int NbSupportedImgExtensions=22;
string SupportedImgExtensions[NbSupportedImgExtensions] = {".jpg", ".JPG", ".jpeg", ".JPEG", ".jpe", ".JPE", ".jp2", ".JP2", ".tiff", ".TIFF", ".tif", ".TIF", ".png", ".PNG", ".bmp", ".BMP", ".pbm", ".PBM", ".ppm", ".PPM", ".pgm", ".PGM"};


//-----------------------Archive specific function---------------------
bool operator<(const NameNb &a, const NameNb &b)
{
    return a.name < b.name;
}
//---------------------------------------------------------------------



//! Archive document constructor.
//!First call the default Document constructor and then open and close the doc (to retrieve number of pages and get the pages order).
Document_archive::Document_archive(char *filename):Document(filename)
{
    //Open the file
    Open();

    //Order and get the number of pages (images)
    Order();

    //Close the file
    Close();
}

//! Try to open the archive file.
//! \return 0 if everything ok
//! \return -1 if unable to open the file
int Document_archive::Open()
{
    //Open the archive
	Arch = archive_read_new();
	archive_read_support_filter_all(Arch);
	archive_read_support_format_all(Arch);
	if (archive_read_open_filename(Arch, Filename, 2048) != ARCHIVE_OK)
        return -1;
    else
        return 0;
}

//! Close the archive.
void Document_archive::Close()
{
    archive_read_close(Arch);
    archive_read_free(Arch);
}


//! Get the number of images (pages) in the archive and keep track of their order for
//!further access. Also select only the image files within the archive.
void Document_archive::Order()
{
    int i=0, nbpages=0;
    ImgNames.clear();
	while (archive_read_next_header(Arch, &Entry) == ARCHIVE_OK)
	{
		//Files names
		string buff_s(archive_entry_pathname(Entry));
		//Check the size to know if valid file (and not directory or corrupted)
		int size = archive_entry_size(Entry);
		//If valid file and not a directory
		if(size>0)
		{
            //Then check if it's a supported image format (by extension)
            bool SupportedFormat=false;
            for(int k=0; k<NbSupportedImgExtensions; k++)
            {
                //Try to find each possible extension in the filename
                if(buff_s.find(SupportedImgExtensions[k], buff_s.length()-5)!=string::npos)
                {
                    //This is a supported file extension, so break and say it's a valid image
                    SupportedFormat=true;
                    break;
                }
            }

            //if image format ok, add it to the list
            if(SupportedFormat)
            {
                ImgNames.push_back(NameNb());
                ImgNames[nbpages].name=buff_s;
                ImgNames[nbpages].nb=i;
                nbpages++;
            }
		}
        i++;
	}

	//Sort the pages according to their name (should be properly named: blabla00.jpg, blabla01.jpg ...)
	sort(ImgNames.begin(), ImgNames.end());

    //Set the number of pages
	NbPages=nbpages;
}

//! Create a new Page_archive.
Page_archive *Document_archive::CreatePage(int pagenb)
{
    Page_archive *page;
    page=new Page_archive(pagenb, NbPages, ReadingMode, PagePreview, DisplayRatio, Rotation, ContrastEnhancement, Arch, Entry, ImgNames[pagenb-1].nb);
    //If page creation have failed try re-open the document and re-try the page creation
    if(page->HasLoadingFailed())
    {
        #ifdef DEBUG
            printf("%d Retry\n", pagenb);
        #endif
        delete page;
        archive_read_close(Arch);
        Open();
        page=new Page_archive(pagenb, NbPages, ReadingMode, PagePreview, DisplayRatio, Rotation, ContrastEnhancement, Arch, Entry, ImgNames[pagenb-1].nb);
    }
    return page;
}
