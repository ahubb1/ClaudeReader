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
//    BDReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with BDReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
#include "Preferences.h"

//! Constructor.
Preferences::Preferences(const char windir[FL_PATH_MAX])
{
    //Init to nothing
    NbRecentDocs=0;
    LastDoc[0]='\0';
    LastPageNb=0;
    NbPages=0;
    LibraryFolder[0]='\0';
    PagePreview=true;
    Thumbnails=true;
    MaxZoomIdx=0;
    AdditionnalZoomIdx=4;
    RotationIdx=0;
    InContext=true;
    Border=true;
    SavePref=true;
    LocalPref=true;
    WinW=600;
    WinH=600;

    //Max zoom possible values
    MaxZoomOptionsList[0].caption="None (Default)\t";
    MaxZoomOptionsList[0].value=100;
    MaxZoomOptionsList[1].caption="80%\t";
    MaxZoomOptionsList[1].value=0.8;
    MaxZoomOptionsList[2].caption="100% (PP2)\t";
    MaxZoomOptionsList[2].value=1.;
    MaxZoomOptionsList[3].caption="150%\t";
    MaxZoomOptionsList[3].value=1.5;
    MaxZoomOptionsList[4].caption="200%\t";
    MaxZoomOptionsList[4].value=2.;

    //Additionnal zoom possible values
    AdditionnalZoomOptionsList[0].caption="x0.5\t";
    AdditionnalZoomOptionsList[0].value=0.5;
    AdditionnalZoomOptionsList[1].caption="x0.6\t";
    AdditionnalZoomOptionsList[1].value=0.6;
    AdditionnalZoomOptionsList[2].caption="x0.7\t";
    AdditionnalZoomOptionsList[2].value=0.7;
    AdditionnalZoomOptionsList[3].caption="x0.8\t";
    AdditionnalZoomOptionsList[3].value=0.8;
    AdditionnalZoomOptionsList[4].caption="x0.9 (Default)\t";
    AdditionnalZoomOptionsList[4].value=0.9;
    AdditionnalZoomOptionsList[5].caption="x1.0 (None)\t";
    AdditionnalZoomOptionsList[5].value=1.0;
    AdditionnalZoomOptionsList[6].caption="x1.1\t";
    AdditionnalZoomOptionsList[6].value=1.1;
    AdditionnalZoomOptionsList[7].caption="x1.2\t";
    AdditionnalZoomOptionsList[7].value=1.2;

    //Rotations possible values
    RotationOptionsList[0].caption="None (Default)\t";
    RotationOptionsList[0].value=0;
    RotationOptionsList[1].caption="+90 (Clockwise)\t";
    RotationOptionsList[1].value=1;
    RotationOptionsList[2].caption="180 (Flip vertcial)\t";
    RotationOptionsList[2].value=2;
    RotationOptionsList[3].caption="-90 (Counter-clockwise)\t";
    RotationOptionsList[3].value=3;

    //Default library folder
    #ifdef __WINDOWS__
        HRESULT hres = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, LibraryFolder);
        if(hres!=S_OK)
        {
            printf("Error retrieving path. Using: ");
            snprintf(LibraryFolder, FL_PATH_MAX, "%s", getenv("HOME"));
        }
    #else
        snprintf(LibraryFolder, FL_PATH_MAX, "%s", getenv("HOME"));
    #endif


    //Where to store pref:
    #ifdef __WINDOWS__
        sprintf(PrefDir, "%s", windir);
    #else
        //if the file (and the folder .bdreader) doesn't exist yet, create it
        snprintf(PrefDir, FL_PATH_MAX, "%s/.bdreader", getenv("HOME"));
        mkdir(PrefDir, 755);
    #endif

}

//! Destructor (nothing).
Preferences::~Preferences()
{
}

//! Load preferences from the given file
//! \return 0 if the preferences file has been successfully loaded
//! \return -1 if the preferences file was not found
//! \return -2 if the preferences file had a wrong format
int Preferences::LoadCompat(const char prefFilename[255], const char windir[FL_PATH_MAX])
{
    //First remember the preferences file filename
    #ifdef __WINDOWS__
        sprintf(Filename, "%s/%s", windir, prefFilename);
    #else
        sprintf(Filename, "%s/.bdreader/%s", getenv("HOME"), prefFilename);
        //if the file (and the folder .bdreader) doesn't exist yet, create it
        char dir[2048];
        snprintf(dir, 2048, "%s/.bdreader", getenv("HOME"));
        mkdir(dir, 755);
    #endif

    //Open the file
    FILE *prefFile=fopen(Filename, "r");
    if(prefFile)
    {
        //The library folder
        if(fscanf(prefFile, "lib %s ", &LibraryFolder)!=1)
        {
            //Error but dont't return for back compat, use default instead
            printf("Using default library folder: ");
            #ifdef __WINDOWS__
                HRESULT hres = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, LibraryFolder);
                if(hres!=S_OK)
                {
                    printf("Error retrieving path. Using: ");
                    snprintf(LibraryFolder, FL_PATH_MAX, "%s", getenv("HOME"));
                }
            #else
                snprintf(LibraryFolder, FL_PATH_MAX, "%s", getenv("HOME"));
            #endif
        }
        printf("%s\n", LibraryFolder);

        //The reading mode, thumbnails option and preview page option
        int thumb_int, preview_int;
        if(fscanf(prefFile, "%d %d %d ", &ReadingMode, &thumb_int, &preview_int)!=3)
            return -2;

        Thumbnails=thumb_int;
        PagePreview=preview_int;

        //The preferences preferences (!)
        int savepref, localpref;
        if(fscanf(prefFile, "%d %d ", &savepref, &localpref)!=2)
        {
            return -2;
        }
        else
        {
            SavePref=savepref;
            LocalPref=localpref;
        }

        //The window size
        int win_w=600, win_h=600;
        if(fscanf(prefFile, "%d %d ", &win_w, &win_h)!=2)
        {
            return -2;
        }
        else
        {
            WinW=win_w;
            WinH=win_h;
        }

        //The window position
        int win_x=0, win_y=0;
        if(fscanf(prefFile, "%d %d ", &win_x, &win_y)!=2)
        {
            return -2;
        }
        else
        {
            WinX=win_x;
            WinY=win_y;
        }

        //The max and additionall zoom
        int max_z=MaxZoomIdx, add_z=AdditionnalZoomIdx;
        if(fscanf(prefFile, "%d %d ", &max_z, &add_z)!=2)
        {
            //return -2  //Don't return: for back compatibility;
        }
        else
        {
            MaxZoomIdx=max_z;
            AdditionnalZoomIdx=add_z;
        }

        //The last document filename
        fgets(LastDoc, 2048, prefFile);
        (*strchr(LastDoc,'\n'))='\0'; //replace the last \n by a \0

        //The last doc current page
        if(fscanf(prefFile, "%d ", &LastPageNb)!=1)
            return -2;

        //The recent docs
        int i=0;
        while(i<MAX_NB_RECENT && !feof(prefFile))
        {
            fgets(RecentDocs[i], 2048, prefFile);
            //replace the last \n by a \0
            char *c = strchr(RecentDocs[i],'\n');
            if(c)
                *c='\0';
            i++;
        }
        NbRecentDocs=i;

        //Close the file
        fclose(prefFile);

        return 0;
    }

    return -1;
}


int Preferences::Load(const char filename[FL_PATH_MAX])
{
    int ret=-1;
    GlobalPrefs = new Fl_Preferences(PrefDir, "bdreader", "bdreader");

    //For back compatibility: check if new preferences exists
    if(GlobalPrefs->groupExists("./")==0)
    {
        printf("NO NEW PREF\n");exit(-1);
        //ConvertPref();
    }


    //Load global param first
    if(GlobalPrefs->groupExists(".")!=0)
    {
        GlobalPrefs->get("LibraryFolder", LibraryFolder, LibraryFolder, 2047);
        GlobalPrefs->get("WinW", WinW, WinW);
        GlobalPrefs->get("WinH", WinH, WinH);
        GlobalPrefs->get("WinX", WinX, WinX);
        GlobalPrefs->get("WinY", WinY, WinY);
        GlobalPrefs->get("LastDoc", LastDoc, LastDoc, 2047);
        //Recent docs
        int i=0;
        for(i=0; i<MAX_NB_RECENT; i++)
        {
            if(GlobalPrefs->get(Fl_Preferences::Name("RecentDocs%d", i), RecentDocs[i], "\0", 2047)==0)
                break;
        }
        NbRecentDocs=i;

        //And doc specific: global ones used as default
            GlobalPrefs->get("ReadingMode", (int&)ReadingMode, ReadingMode);
            GlobalPrefs->get("Thumbnails", (int&)(Thumbnails), Thumbnails);
            GlobalPrefs->get("PagePreview", (int&)(PagePreview), PagePreview);
            GlobalPrefs->get("MaxZoomIdx", MaxZoomIdx, MaxZoomIdx);
            GlobalPrefs->get("AdditionnalZoomIdx", AdditionnalZoomIdx, AdditionnalZoomIdx);
            GlobalPrefs->get("InContext", (int&)(InContext), InContext);
            GlobalPrefs->get("Border", (int&)(Border), Border);
            GlobalPrefs->get("RotationIdx", RotationIdx, RotationIdx);
    }

    //Get doc specific pref if any (and overwrite global ones if needed)
    if(GlobalPrefs->groupExists(filename)!=0)
    {
        DocPrefs = new Fl_Preferences(GlobalPrefs, filename);
            DocPrefs->get("ReadingMode", (int&)ReadingMode, ReadingMode);
            DocPrefs->get("Thumbnails", (int&)(Thumbnails), Thumbnails);
            DocPrefs->get("PagePreview", (int&)(PagePreview), PagePreview);
            DocPrefs->get("MaxZoomIdx", MaxZoomIdx, MaxZoomIdx);
            DocPrefs->get("AdditionnalZoomIdx", AdditionnalZoomIdx, AdditionnalZoomIdx);
            DocPrefs->get("InContext", (int&)(InContext), InContext);
            DocPrefs->get("Border", (int&)(Border), Border);
            DocPrefs->get("RotationIdx", RotationIdx, RotationIdx);
            char lastdate_s[20];
            DocPrefs->get("LastDate", lastdate_s, "01/01/1980-00:00:00", 20);
            strptime(lastdate_s, "%d/%m/%Y-%H:%M:%S", &LastDate);
            DocPrefs->get("LastPageNb", LastPageNb, LastPageNb);
            DocPrefs->get("NbPages", NbPages, NbPages);
        delete DocPrefs;

        ret=0;
    }
    else
        ret=-1;

    delete GlobalPrefs;
    return ret;
}

//! Apply the loaded parameters to the document
//! \return 1 if a page has been loaded
//! \return 0 otherwise
int Preferences::Apply(Document *doc)
{
    int loaded=0;

    //The reading mode
    doc->SetReadingMode(ReadingMode);

    //The preview/not preview
    doc->SetPagePreview(PagePreview);

    //Check if it was previously open
    if(strcmp(LastDoc, doc->GetFilename())==0)
    {
        //if yes, load the last page
        doc->LoadPage(LastPageNb);
        loaded=1;
    }

    return loaded;
}

//! Save the current preferences to the preference file
void Preferences::SaveCompat(Document *doc)
{
    //If a doc exists (otherwise no new preferences are saved)
    if(doc)
    {
        //Open the preferences file
        FILE *prefFile=fopen(Filename, "w");
        if(prefFile)
        {
            //The library folder
            fprintf(prefFile, "lib %s\n", LibraryFolder);

            //The reading mode, background color and preview page option
            fprintf(prefFile, "%d\n%d\n%d\n", doc->GetReadingMode(), Thumbnails, doc->IsPagePreview());

            //The preferences preferences (!)
            fprintf(prefFile, "%d\n%d\n", SavePref, LocalPref);

            //The window size
            fprintf(prefFile, "%d\n%d\n", WinW, WinH);

            //The window position
            fprintf(prefFile, "%d\n%d\n", WinX, WinY);

            //The max and additionall zoom
            fprintf(prefFile, "%d\n%d\n", MaxZoomIdx, AdditionnalZoomIdx);

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


void Preferences::Save(Document *doc)
{

    //Global pref first
    GlobalPrefs = new Fl_Preferences(PrefDir, "bdreader", "bdreader");
        GlobalPrefs->set("LibraryFolder", LibraryFolder);
        GlobalPrefs->set("SavePref", SavePref);
        GlobalPrefs->set("LocalPref", LocalPref);
        GlobalPrefs->set("WinW", WinW);
        GlobalPrefs->set("WinH", WinH);
        GlobalPrefs->set("WinX", WinX);
        GlobalPrefs->set("WinY", WinY);
        GlobalPrefs->set("LastDoc", LastDoc);
        //The recent docs
            //first the current one (if not already)
            if(strcmp(doc->GetFilename(), RecentDocs[0])!=0)
            {
                GlobalPrefs->set(Fl_Preferences::Name("RecentDocs%d", 0), doc->GetFilename());
            }
            //then the loaded ones
            for(int i=0; i<NbRecentDocs; i++)
            {
                GlobalPrefs->set(Fl_Preferences::Name("RecentDocs%d", i), RecentDocs[i]);
            }
        GlobalPrefs->set("ReadingMode", ReadingMode);
        GlobalPrefs->set("Thumbnails", Thumbnails);
        GlobalPrefs->set("PagePreview", PagePreview);
        GlobalPrefs->set("MaxZoomIdx", MaxZoomIdx);
        GlobalPrefs->set("AdditionnalZoomIdx", AdditionnalZoomIdx);
        GlobalPrefs->set("InContext", InContext);
        GlobalPrefs->set("Border", Border);
        GlobalPrefs->set("RotationIdx", RotationIdx);


    //If a doc exists write specific prefs
    if(doc)
    {
        DocPrefs = new Fl_Preferences(GlobalPrefs, doc->GetFilename());
            DocPrefs->set("ReadingMode", ReadingMode);
            DocPrefs->set("Thumbnails", Thumbnails);
            DocPrefs->set("PagePreview", PagePreview);
            DocPrefs->set("MaxZoomIdx", MaxZoomIdx);
            DocPrefs->set("AdditionnalZoomIdx", AdditionnalZoomIdx);
            DocPrefs->set("InContext", InContext);
            DocPrefs->set("Border", Border);
            DocPrefs->set("RotationIdx", RotationIdx);
            char time_s[150];time_t now=time(0);strftime(time_s, 150, "%d/%m/%Y-%H:%M:%S", localtime(&now));
            DocPrefs->set("LastDate", time_s);
            DocPrefs->set("LastPageNb", doc->GetCurrentPageNb());
            DocPrefs->set("NbPages", doc->GetNbOfPages());
        delete DocPrefs;
    }

    delete GlobalPrefs;
}


//! Copy all local preferences from a Preferences object to another (no "global only" preferences)
void Preferences::CopyLocalTo(Preferences *pref)
{
     pref->ReadingMode=ReadingMode;
     pref->PagePreview=PagePreview;
     pref->Thumbnails=Thumbnails;
     pref->MaxZoomIdx=MaxZoomIdx;
     pref->RotationIdx=RotationIdx;
     pref->AdditionnalZoomIdx=AdditionnalZoomIdx;
     pref->InContext=InContext;
     pref->Border=Border;
}
