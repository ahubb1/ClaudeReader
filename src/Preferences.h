//--------------------------------------------------------------------------
//
//    Copyright (C) 2014  Vincent Crocher
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
//    along with BDReader.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Preferences.h
 * \brief Declaration file of the Preferences class
 * \author Vincent Crocher
 * \version 0.3
 * \date November 2014
 *
 *
 */
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "Document.h"

#define MAX_NB_RECENT 25

struct ZoomOption
{
    const char *caption;
    float value;
};


//! Class managing the user preferences (saving in files, loading, applying to the current document)
class Preferences
{
    public:
        Preferences();
        ~Preferences();

        int Load(const char prefFile[255], const char windir[2048]);
        int Apply(Document *doc);
        void Save(Document *doc);

        void CopyLocalTo(Preferences *pref);


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
        bool GetInContext() { return InContext; }
        void SetInContext(bool val) { Border = val; }
        bool GetBorder() { return Border; }
        void SetBorder(bool val) { Border = val; }
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
        char *GetLastDoc() { return LastDoc; }
        int GetLastDocPageNb() { return LastDocPageNb; }
        void SetLastDocPageNb(int val) { LastDocPageNb = val; }
        int GetNbRecentDocs() { return NbRecentDocs; }
        char *GetRecentDocs(int i) { return RecentDocs[i]; }

        ZoomOption GetMaxZoomOption(int i) { return MaxZoomOptionsList[i]; }
        ZoomOption GetAdditionnalZoomOption(int i) { return AdditionnalZoomOptionsList[i]; }

    protected:
    private:
        char Filename[255];
        READINGMODE ReadingMode;
        bool PagePreview;
        bool Thumbnails;
        bool InContext;
        bool Border;
        bool SavePref, LocalPref;
        int WinW, WinH;
        int WinX, WinY;
        int BkgColor;
        char LastDoc[2048];
        int LastDocPageNb;
        int NbRecentDocs;
        char RecentDocs[MAX_NB_RECENT][2048];
        int MaxZoomIdx;
        int AdditionnalZoomIdx;
        ZoomOption MaxZoomOptionsList[5];
        ZoomOption AdditionnalZoomOptionsList[8];
};

#endif // PREFERENCES_H
