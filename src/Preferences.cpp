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
//    BDReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with BDReader.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
#include "Preferences.h"

//! Constructor (nothing).
Preferences::Preferences()
{
    NbRecentDocs=0;
}

//! Destructor (nothing).
Preferences::~Preferences()
{
    //dtor
}

//! Load preferences from the given file
int Preferences::Load(const char prefFilename[255])
{
    //First remember the preferences file filename
    sprintf(Filename, "%s", prefFilename);

    //Open the file
    FILE *prefFile=fopen(Filename, "r");
    if(prefFile)
    {
        //The reading mode, background color and preview page option
        if(fscanf(prefFile, "%d %d %d ", &ReadingMode, &BkgColor, &PagePreview)!=3)
            return -1;

        //The last document filename
        fgets(LastDoc, 1024, prefFile);
        (*strchr(LastDoc,'\n'))='\0'; //replace the last \n by a \0

        //The last doc current page
        if(fscanf(prefFile, "%d ", &LastDocPageNb)!=1)
            return -1;

        //The recent docs
        int i=0;
        while(i<5 && !feof(prefFile))
        {
            fgets(RecentDocs[i], 1024, prefFile);
            //replace the last \n by a \0
            char *c = strchr(RecentDocs[i],'\n');
            if(c)
                *c='\0';
            i++;
        }
        NbRecentDocs=i;

        /*printf("%d %d %d %s %d %d\n", ReadingMode, BkgColor, PagePreview, LastDoc, LastDocPageNb, NbRecentDocs);
        //The recent docs
        for(int j=0; j<NbRecentDocs; j++)
            printf("%d %s\n", j, RecentDocs[j]);
        printf("\n");*/

        //Close the file
        fclose(prefFile);
    }

    return 0;
}

//! Apply the loaded parameters to the document
//! \return 1 if a page has been loaded
//! \return 0 otherwise
int Preferences::Apply(Document *doc)
{
    int loaded=0;

    //Check if it was previously open
    if(strcmp(LastDoc, doc->GetFilename())==0)
    {
        //if yes, load the last page
        doc->LoadPage(LastDocPageNb);
        //and preload
        //doc->PreloadPages();
        loaded=1;
    }

    //The reading mode
    doc->SetReadingMode(ReadingMode);

    //The preview/not preview
    doc->SetPagePreview(PagePreview);

    return loaded;
}

//! Save the current preferences to the preference file
void Preferences::Save(Document *doc)
{
    //If a doc exists
    if(doc)
    {
        //Open the preferences file
        FILE *prefFile=fopen(Filename, "w");
        if(prefFile)
        {
            //The reading mode, background color and preview page option
            fprintf(prefFile, "%d\n%d\n%d\n", doc->GetReadingMode(), BkgColor, doc->IsPagePreview());

            //The last document filename
            fprintf(prefFile, "%s\n", doc->GetFilename());

            //The last doc current page
            fprintf(prefFile, "%d", doc->GetCurrentPageNb());

            //The recent docs
            //first the current one (if not already)
            if(strcmp(doc->GetFilename(), RecentDocs[0])!=0)
            {
                fprintf(prefFile, "\n%s", doc->GetFilename());
            }
            //then the loaded ones
            for(int i=0; i<NbRecentDocs; i++)
            {
                fprintf(prefFile, "\n%s", RecentDocs[i]);
            }

            //Close the file
            fclose(prefFile);
        }
    }
}
