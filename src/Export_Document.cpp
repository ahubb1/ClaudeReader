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
#include "Export_Document.h"

//Initialize the list of supported formats
const int NbSupportedWritingImgExtensions=4;    //! Number of image extensions (format) supported.
const char *SupportedWritingImgExtensions[NbSupportedWritingImgExtensions] = {".jpg", ".tif", ".png", ".bmp"};  //! List of supported image extensions (formats).

//! Main constructor: initialization of export object
Export_Document::Export_Document(Document *doc, const char *path, const char *fileext, bool panelmode, bool pagepreview)
{
    //Cpy the given values
    Doc=doc;
    strcpy(PathToExport, path);
    sprintf(PathToExport, "%s//", PathToExport);
    strcpy(FilesExtension, fileext);
    PanelMode=panelmode;
    PagePreview=pagepreview;

    //Init
    SavingInProgress=false;
    Canceled=false;
}

Export_Document::~Export_Document()
{
}

//! Main method: process to the export as images files: go through the document and
//!generate the images in the given format.
//! \return 0 when export successfull
//! \return -1 if the path to export is invalid (not a directory or doesn't exist)
//! \return -2 if the image format (extensions) is not supported
//! \return -3 if the export has failed (not reached the last page of the document) or has been canceled
int Export_Document::Export()
{
    //Check if exporting path exists (and is a directory)
        if(fl_filename_isdir(PathToExport)==0)
        {
            return -1;
        }

    //Check file extension (is it a supported image format ?)
        bool SupportedFormat=false;
        for(int k=0; k<NbSupportedWritingImgExtensions; k++)
        {
            //Try to find each possible extension in the filename
            if(strstr(FilesExtension, SupportedWritingImgExtensions[k])!=NULL)
            {
                //This the file extension, so break and say it's a valid image
                SupportedFormat=true;
                break;
            }
        }
        if(!SupportedFormat)
        {
            return -2;
        }


    //Say we are working on it
    SavingInProgress=true;


    //Create proper base filename
        const char *BaseFilename;
        char FullFilename[FL_PATH_MAX];
        strcpy(FullFilename, Doc->GetFilename());
        //Remove the original extension of the document
        fl_filename_setext(FullFilename, sizeof(FullFilename), "");
        //Keep only the filename (w/o the path)
        BaseFilename = fl_filename_name(FullFilename);


    //Set the document READING mode: page or panel
        if(PanelMode)
        {
            Doc->SetReadingMode(CASE);
            //Set the page preview (useless in page mode)
            Doc->SetPagePreview(PagePreview);
        }
        else
        {
            Doc->SetReadingMode(PAGE);
        }

    //Go to first page of the document
    Doc->LoadPage(1);

    //For each page, includng the first one
    int nbimages=0;
    do
    {
        //Build the page number (long version: w/ 00 in front of)
            char longpagenb[5];
            if(Doc->GetCurrentPageNb()<10)
            {
                sprintf(longpagenb, "000%d", Doc->GetCurrentPageNb());
            }
            else if(Doc->GetCurrentPageNb()<100)
            {
                sprintf(longpagenb, "00%d", Doc->GetCurrentPageNb());
            }
            else if(Doc->GetCurrentPageNb()<1000)
            {
                sprintf(longpagenb, "0%d", Doc->GetCurrentPageNb());
            }
            else
            {
                sprintf(longpagenb, "%d", Doc->GetCurrentPageNb());
            }

        //Load the document page
        Doc->LoadPage();

        //Locally render it
            //If in panel mode and detection has not failed
            if(PanelMode && !Doc->currentPage->HasDetectionFailed())
            {
                 int casenb=0;
                 //For each panel, including the first one
                 do
                 {
                    //put the panel number at the end of the filename and the extension to filename
                    char filename[2048];
                    //Append a 0 in front of case number if less than 10
                    if(casenb<10)
                    {
                        sprintf(filename, "%s%s_%s0%d%s", PathToExport, BaseFilename, longpagenb, casenb, FilesExtension);
                    }
                    else
                    {
                        sprintf(filename, "%s%s_%s%d%s", PathToExport, BaseFilename, longpagenb, casenb, FilesExtension);
                    }

                    //Render
                    Mat rendering(*Doc->Render());

                    //Write it (extension already choosen)
                    imwrite(filename, rendering);
                    nbimages++;

                    casenb++;
                 }while(Doc->NextCase()==0 && !Canceled);

                 //Need to go a page backward since NextCase go to next page automattically, except for last page
                 if(Doc->GetCurrentPageNb()!=Doc->GetNbOfPages())
                 {
                     Doc->PreviousPage();
                 }
            }
            //Render only once, the page
            else
            {
                //Build the filename (page nb and extension)
                char filename[2048];
                if(PanelMode)
                {
                    //Add zeroes since panels in between two pages
                    sprintf(filename, "%s%s_%s00%s", PathToExport, BaseFilename, longpagenb, FilesExtension);
                }
                else
                {
                    //Just use the long page number
                    sprintf(filename, "%s%s_%d%s", PathToExport, BaseFilename, Doc->GetCurrentPageNb(), FilesExtension);
                }

                //Render
                Mat rendering(*Doc->Render());

                //Write it (extension already choosen)
                imwrite(filename, rendering);
                nbimages++;

                //If we are in Panel but failed, double the page: easier for a user on another reader
                if(PanelMode)
                {
                    sprintf(filename, "%s%s_%s01%s", PathToExport, BaseFilename, longpagenb, FilesExtension);
                    imwrite(filename, rendering);
                    nbimages++;
                }
            }

        //Update the progression
        CurrentPage=Doc->GetCurrentPageNb();

        //Give some time to fltk for refreshing
        Fl::check();
    }while(Doc->NextPage()>0 && !Canceled);

    //Done
    SavingInProgress=false;

    if(CurrentPage==Doc->GetNbOfPages() && !Canceled)
        return nbimages;
    else
        return -3;
}


