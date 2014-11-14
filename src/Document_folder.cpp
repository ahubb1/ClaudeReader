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
#include "Document_folder.h"

//Initialize the list of supported formats
const int NbSupportedFolderImgExtensions=22;
string SupportedFolderImgExtensions[NbSupportedFolderImgExtensions] = {".jpg", ".JPG", ".jpeg", ".JPEG", ".jpe", ".JPE", ".jp2", ".JP2", ".tiff", ".TIFF", ".tif", ".TIF", ".png", ".PNG", ".bmp", ".BMP", ".pbm", ".PBM", ".ppm", ".PPM", ".pgm", ".PGM"};

//! Image folder document constructor.
//!First call the default Document constructor and then open and close the folder (to retrieve number of pages and get the pages order).
Document_folder::Document_folder(char *dirname):Document(dirname)
{
    //Open the file
    Open();

    //Order and get the number of pages (images)
    Order();

    //Close the file
    Close();
}

//! Try to open the image folder.
//! \return 0 if everything ok
//! \return -1 if unable to open the file (no images found or directory doesn't exist)
int Document_folder::Open()
{
    //Check if the directory exists
    if(fl_filename_isdir(Filename)!=0)
        return 0;
    else
        return -1;
}

//! Close the folder.
void Document_folder::Close()
{

}


//! Get the number of images (pages) in the folder and keep track of their order for
//!further access. Also select only the image files within the folder.
void Document_folder::Order()
{
    //List the images in the folder and sort it (alphanum)
    int nb_files=fl_filename_list(Filename, &FilesList,  fl_casenumericsort);

    int nbpages=0;

    //For each file in the dir
    for (int i=0; i< nb_files; i++)
    {
        #ifdef DEBUG
            printf("%s\n", FilesList[i]->d_name);
        #endif

        string currentFilename(FilesList[i]->d_name);


        //Check if it's a supported image format (by extension)
            bool SupportedFormat=false;
            for(int k=0; k<NbSupportedFolderImgExtensions; k++)
            {
                //Try to find each possible extension in the filename
                if(currentFilename.find(SupportedFolderImgExtensions[k], currentFilename.length()-5)!=string::npos)
                {
                    //This is a supported file extension, so break and say it's a valid image
                    SupportedFormat=true;
                    break;
                }
            }

        //if image format ok, add it to the list
        if(SupportedFormat)
        {
            char fullFilename[FL_PATH_MAX];
            #ifdef __WINDOWS__
                sprintf(fullFilename, "%s\\%s", Filename, FilesList[i]->d_name);
            #else
                sprintf(fullFilename, "%s/%s", Filename, FilesList[i]->d_name);
            #endif
            ImgNames.push_back(string(fullFilename));
            nbpages++;
        }
    }

    //Set the number of pages
    //At least two images should be present to make a valid comics
    if(nbpages<2)
        NbPages=0;
    else
        NbPages=nbpages;
}

//! Create a new Page_folder.
Page_folder *Document_folder::CreatePage(int pagenb)
{
    return new Page_folder(pagenb, NbPages, ReadingMode, PagePreview, DisplayRatio, ImgNames[pagenb-1]);
}
