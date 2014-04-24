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
 * \version 0.1
 * \date April 2014
 *
 *
 */
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "Document.h"

//! Class managing the user preferences (saving in files, loading, applying to the current document)
class Preferences
{
    public:
        Preferences();
        ~Preferences();

        int Load(const char prefFile[255]);
        int Apply(Document *doc);
        void Save(Document *doc);


        READINGMODE GetReadingMode() { return ReadingMode; }
        void SetReadingMode(READINGMODE val) { ReadingMode = val; }
        bool GetPagePreview() { return PagePreview; }
        void SetPagePreview(bool val) { PagePreview = val; }
        int GetBkgColor() { return BkgColor; }
        void SetBkgColor(int val) { BkgColor = val; }
        char *GetLastDoc() { return LastDoc; }
        //void SetLastDoc[1024](char val) { LastDoc[1024] = val; }
        int GetLastDocPageNb() { return LastDocPageNb; }
        void SetLastDocPageNb(int val) { LastDocPageNb = val; }
        char *GetRecentDocs(int i) { return RecentDocs[i]; }
        //void SetRecentDocs[5][1024](char val) { RecentDocs[5][1024] = val; }
    protected:
    private:
        char Filename[255];
        READINGMODE ReadingMode;
        bool PagePreview;
        int BkgColor;
        char LastDoc[1024];
        int LastDocPageNb;
        int NbRecentDocs;
        char RecentDocs[5][1024];
};

#endif // PREFERENCES_H
