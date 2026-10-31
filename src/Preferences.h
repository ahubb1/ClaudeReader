//--------------------------------------------------------------------------
//
//    Copyright (C) 2014-2016  Vincent Crocher
//
//	  This file is part of BDReader.
//
//    BDReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    BDReaderFoobar is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with BDReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Preferences.h
 * \brief Declaration file of the Preferences class
 * \author Vincent Crocher
 * \version 0.6
 * \date June 2016
 *
 *
 */
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <FL/filename.H>
#include <FL/Fl_Preferences.H>
//To get default user documents path:
#ifdef __WINDOWS__
    #include <windows.h>
    #include <shlobj.h>
    #include <time.h>
#endif


#include "Document.h"
#include "PanelHighlight.h"

#define MAX_NB_RECENT 25

struct ZoomOption
{
    const char *caption;
    float value;
};

struct IntOption
{
    const char *caption;
    int value;
};


//! Per-user folder where preferences and thumbnails are stored (created if needed):
//!%APPDATA%\ClaudeReader on Windows, ~/.claudereader otherwise (the BDReader ones are moved there)
const char *GetUserDataDir();

//! Class managing the user preferences (saving in files, loading, applying to the current document)
class Preferences
{
    public:
        Preferences(const char windir[FL_PATH_MAX]);
        ~Preferences();

        int Load(const char filename[FL_PATH_MAX]);
        int LoadCompat(const char prefFile[255], const char windir[FL_PATH_MAX]);
        int Apply(Document *doc);
        void Save(Document *doc);
        void ForceSave();
        void SaveCompat(Document *doc);

        void CopyLocalTo(Preferences *pref);

        bool GetUseLibrary() { return UseLibrary; }
        void SetUseLibrary(bool val) { UseLibrary = val; }
        READINGMODE GetReadingMode() { return ReadingMode; }
        void SetReadingMode(READINGMODE val) { ReadingMode = val; }
        bool GetPagePreview() { return PagePreview; }
        void SetPagePreview(bool val) { PagePreview = val; }
        bool GetThumbnails() { return Thumbnails; }
        void SetThumbnails(bool val) { Thumbnails = val; }
        int GetMaxZoomIdx() { return MaxZoomIdx; }
        void SetMaxZoomIdx(int i) { MaxZoomIdx = i; }
        int GetAdditionnalZoomIdx() { return AdditionnalZoomIdx; }
        void SetAdditionnalZoomIdx(int i) {AdditionnalZoomIdx = i; }
        int GetRotationIdx() { return RotationIdx; }
        void SetRotationIdx(int i) { RotationIdx = i; }
        bool GetInContext() { return InContext; }
        void SetInContext(bool val) { InContext = val; }
        bool GetBorder() { return Border; }
        void SetBorder(bool val) { Border = val; }
        bool GetRightToLeft() { return RightToLeft; }
        void SetRightToLeft(bool val) { RightToLeft = val; }
        bool GetSavePref() { return SavePref; }
        void SetSavePref(bool val) { SavePref = val; }
        bool GetLocalPref() { return LocalPref; }
        void SetLocalPref(bool val) { LocalPref = val; }
        int GetWinW() { return WinW; }
        void SetWinW(int val) { WinW = val; }
        int GetWinH() { return WinH; }
        void SetWinH(int val) { WinH = val; }
        int GetWinX() { return WinX; }
        void SetWinX(int val) { WinX = val; }
        int GetWinY() { return WinY; }
        void SetWinY(int val) { WinY = val; }
        int GetBkgColor() { return BkgColor; }
        void SetBkgColor(int val) { BkgColor = val; }
        char *GetShortFilename() { return ShortFilename; }
        char *GetLibraryFolder() { return LibraryFolder; }
        void SetLibraryFolder(char *lib_folder) { snprintf(LibraryFolder, FL_PATH_MAX, "%s", lib_folder); }
        const PanelHighlight &GetHighlight() { return Highlight; }
        void SetHighlight(const PanelHighlight &val) { Highlight = val; }
        char *GetLastDoc() { return LastDoc; }
        int GetLastPageNb() { return LastPageNb; }
        void SetLastPageNb(int val) { LastPageNb = val; }
        int GetNbPages() { return NbPages; }
        void SetNbPages(int val) { NbPages = val; }
        struct tm *GetLastDate() { return (&LastDate); }
        void SetLastDate(struct tm d) { LastDate=d; }
        int GetNbRecentDocs() { return NbRecentDocs; }
        char *GetRecentDocs(int i) { return RecentDocs[i]; }

        ZoomOption GetMaxZoomOption(int i) { return MaxZoomOptionsList[i]; }
        ZoomOption GetAdditionnalZoomOption(int i) { return AdditionnalZoomOptionsList[i]; }

        IntOption GetRotationOption(int i) { return RotationOptionsList[i]; }

    private:
        void ConvertPreferences();

        Fl_Preferences *GlobalPrefs;
        Fl_Preferences *DocPrefs;
        char LibraryFolder[FL_PATH_MAX];
        char Filename[FL_PATH_MAX];
        char ShortFilename[FL_PATH_MAX];
        char PrefDir[FL_PATH_MAX];
        bool UseLibrary;
        PanelHighlight Highlight;   //!< Global only
        READINGMODE ReadingMode;
        bool PagePreview;
        bool Thumbnails;
        bool InContext;
        bool Border;
        bool RightToLeft;   //!< Manga reading order: panels ordered from right to left
        bool SavePref, LocalPref;
        int WinW, WinH;
        int WinX, WinY;
        int BkgColor;
        char LastDoc[FL_PATH_MAX];
        int LastPageNb;
        int NbPages;
        int NbRecentDocs;
        char RecentDocs[MAX_NB_RECENT][FL_PATH_MAX];
        int MaxZoomIdx;
        int AdditionnalZoomIdx;
        int RotationIdx;
        struct tm LastDate;
        ZoomOption MaxZoomOptionsList[5];
        ZoomOption AdditionnalZoomOptionsList[8];
        IntOption RotationOptionsList[4];
};

#endif // PREFERENCES_H
