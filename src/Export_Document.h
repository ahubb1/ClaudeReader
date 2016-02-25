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
/**
 * \file Export_Document.h
 * \brief Declaration file of the Export_Document class
 * \author Vincent Crocher
 * \version 0.1
 * \date June 2014
 *
 *
 */
#ifndef EXPORT_DOCUMENT_H
#define EXPORT_DOCUMENT_H

#include <FL/Fl.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_File_Chooser.H>//For directory creation

#include "Document.h"

//! An export class. Meant to be used with an opened Document to export it as a series of images.
class Export_Document
{
    public:
        Export_Document(Document *doc, const char *path, const char *fileext, bool panelmode, bool pagepreview=false);
        ~Export_Document();
        int Export(Fl_Progress* progressbar=NULL);
        void Cancel() { Canceled=true; }

        Document *GetDocToExport() { return Doc; }
        void SetDocToExport(Document *val) { Doc = val; }
        char *GetPathToExport() { return PathToExport; }
        void SetPathToExport(char *val) { strcpy(PathToExport,val); }
        char *GetFilesExtension() { return FilesExtension; }
        void SetFilesExtension(char *val) { strcpy(FilesExtension, val); }
        int GetProgression() { return CurrentPage; }
        bool IsSaving() { return SavingInProgress; }
    protected:
    private:
        Document *Doc;               //! The Document to export.
        char PathToExport[2048];    //! The path to write the images.
        char FilesExtension[5];     //! Images extensions. See SupportedWritingImgExtensions for a list of supported extensions.
        bool PanelMode;             //! When true, one image will be created for each panel on the pages, otherwise one image for each page only.
        bool PagePreview;           //! When in PanelMode, if true, will create an image of the whole page plus one image for each panel of the page.
        int CurrentPage;            //! The page currently processed by the Export() method.
        bool SavingInProgress;      //! Flag indicating if the export process is running.
        bool Canceled;              //! Set to true when the user cancel the export while running.
};

#endif // EXPORT_DOCUMENT_H
