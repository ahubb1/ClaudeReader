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


#ifdef __WINDOWS__
    int strptime(const char * lastdate_s, const char *format, struct tm *date)
    {
        //Minimalist version accepting only one given format...
        if(strcmp(format, "%d/%m/%Y-%H:%M:%S")!=0)
        {
            return -1;
        }
        else
        {
            int d, m, Y, H, M, S;
            if(sscanf(lastdate_s, "%2d/%2d/%4d-%2d:%2d:%2d", &d, &m, &Y, &H, &M, &S)==6)
            {
                date->tm_isdst=-1; //no dayligth saving thingy
                date->tm_year=Y-1900;
                date->tm_mon=m-1;
                date->tm_mday=d;
                date->tm_hour=H;
                date->tm_min=M;
                date->tm_sec=S;
                mktime (date); //Fill other fields of date
                return 0;
            }
            else
            {
                return -2;
            }
        }
    }
#endif



//! Constructor.
Preferences::Preferences(const char windir[FL_PATH_MAX]):
    NbRecentDocs(0),
    LastPageNb(0),
    NbPages(0),
    PagePreview(true),
    Thumbnails(true),
    MaxZoomIdx(0),
    AdditionnalZoomIdx(4),
    RotationIdx(0),
    ReadingMode(CASE),
    InContext(true),
    Border(true),
    SavePref(true),
    LocalPref(true),
    WinW(1000),
    WinH(600)
{
    //Init to nothing
    LastDoc[0]='\0';
    LibraryFolder[0]='\0';

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


//! Back compatibility function: open all old preferences files
//!and convert them to the new preference system (single fltk preference file)
void Preferences::ConvertPreferences()
{
    printf("Converting preferences to new format...\n");
    Preferences tmpPref(PrefDir);

    //Get all previous preferences files: *.pref in PrefDir
    dirent **dir_listing;
    int nb_files=fl_filename_list(PrefDir, &dir_listing, fl_casenumericsort);

    //For each file
    for (int i=0; i<nb_files; i++)
    {
        //Check extension
        if(strcmp(fl_filename_ext(dir_listing[i]->d_name), ".pref")==0)
        {
            //Open it
            printf("%s\n", dir_listing[i]->d_name);
            char fullfilename[FL_PATH_MAX];
            snprintf(fullfilename, FL_PATH_MAX, "%s/%s", PrefDir, dir_listing[i]->d_name);
            tmpPref.LoadCompat(dir_listing[i]->d_name, PrefDir);

            //And create an instance in new preference system from it
                char short_filename[FL_PATH_MAX];//new pref system does not use file extension (nor path)
                snprintf(short_filename, FL_PATH_MAX, "%s", fl_filename_name(dir_listing[i]->d_name));fl_filename_setext(short_filename, FL_PATH_MAX, "");
                DocPrefs = new Fl_Preferences(GlobalPrefs, short_filename);
                    DocPrefs->set("ReadingMode", tmpPref.GetReadingMode());
                    DocPrefs->set("Thumbnails", tmpPref.GetThumbnails());
                    DocPrefs->set("PagePreview", tmpPref.GetPagePreview());
                    DocPrefs->set("MaxZoomIdx", tmpPref.GetMaxZoomIdx());
                    DocPrefs->set("AdditionnalZoomIdx", tmpPref.GetAdditionnalZoomIdx());
                    DocPrefs->set("InContext", tmpPref.GetInContext());
                    DocPrefs->set("Border", tmpPref.GetBorder());
                    DocPrefs->set("RotationIdx", tmpPref.GetRotationIdx());
                    //Use last modified time of pref file as the last read time:
                        struct stat info_pref;
                        stat(fullfilename, &info_pref);
                        struct tm * time_info=localtime(&(info_pref.st_mtime));
                        char time_s[20];
                        strftime(time_s, 150, "%d/%m/%Y-%H:%M:%S", time_info);
                    DocPrefs->set("LastDate", time_s);
                    DocPrefs->set("LastPageNb", tmpPref.GetLastPageNb());
                    DocPrefs->set("NbPages", -1); //We don't know that
                delete DocPrefs;

            //Delete the old pref file
            remove(fullfilename);
        }
        //Delete the old general pref file
        char fullfilename[FL_PATH_MAX];
        snprintf(fullfilename, FL_PATH_MAX, "%s/preferences", PrefDir);
        remove(fullfilename);
    }
    printf("Done.\n");
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


//! Load preferences for the given files from the FLTK style preference file
//! \param filename: the short comics filename (without path nor extension) or empty for general preferences
//! \return 0 if the preferences file has been successfully loaded
//! \return -1 if the preferences file was not found
//! \return -2 if the preferences file had a wrong format
int Preferences::Load(const char filename[FL_PATH_MAX])
{
    int ret=-1;
    snprintf(ShortFilename, FL_PATH_MAX, "%s", fl_filename_name(filename));fl_filename_setext(ShortFilename, FL_PATH_MAX, "");
    GlobalPrefs = new Fl_Preferences(PrefDir, "bdreader", "bdreader");

    //For back compatibility: check if new preferences exists
    char new_pref_file[FL_PATH_MAX];
    snprintf(new_pref_file, FL_PATH_MAX, "%s/bdreader.prefs", PrefDir);
    struct stat info_pref;
    if(stat(new_pref_file, &info_pref)!=0)
    {
        ConvertPreferences();
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
            int r=(int)ReadingMode;GlobalPrefs->get("ReadingMode", r, r);ReadingMode=(READINGMODE)r;
            int t=(int)Thumbnails;GlobalPrefs->get("Thumbnails", t, t);Thumbnails=(bool)t;
            int p=(int)PagePreview;GlobalPrefs->get("PagePreview", p, p);PagePreview=(bool)p;
            GlobalPrefs->get("MaxZoomIdx", MaxZoomIdx, MaxZoomIdx);
            GlobalPrefs->get("AdditionnalZoomIdx", AdditionnalZoomIdx, AdditionnalZoomIdx);
            int n=(int)InContext;GlobalPrefs->get("InContext", n, n);InContext=(bool)n;
            int b=(int)Border;GlobalPrefs->get("Border", b, b);Border=(bool)b;
            GlobalPrefs->get("RotationIdx", RotationIdx, RotationIdx);
    }

    //Get doc specific pref if any (and overwrite global ones if needed)
    if(GlobalPrefs->groupExists(ShortFilename)!=0)
    {
        DocPrefs = new Fl_Preferences(GlobalPrefs, ShortFilename);
            int r=(int)ReadingMode;DocPrefs->get("ReadingMode", r, r);ReadingMode=(READINGMODE)r;
            int t=(int)Thumbnails;DocPrefs->get("Thumbnails", t, t);Thumbnails=(bool)t;
            int p=(int)PagePreview;DocPrefs->get("PagePreview", p, p);PagePreview=(bool)p;
            DocPrefs->get("MaxZoomIdx", MaxZoomIdx, MaxZoomIdx);
            DocPrefs->get("AdditionnalZoomIdx", AdditionnalZoomIdx, AdditionnalZoomIdx);
            int n=(int)InContext;DocPrefs->get("InContext", n, n);InContext=(bool)n;
            int b=(int)Border;DocPrefs->get("Border", b, b);Border=(bool)b;
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

    //Load previously openned page
    doc->LoadPage(LastPageNb);
    loaded=1;

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
            if(doc)
            {
                if(strcmp(doc->GetFilename(), RecentDocs[0])!=0)
                {
                    GlobalPrefs->set(Fl_Preferences::Name("RecentDocs%d", 0), fl_filename_name(doc->GetFilename()));
                }
            }
            //then the loaded ones
            for(int i=0; i<NbRecentDocs; i++)
            {
                GlobalPrefs->set(Fl_Preferences::Name("RecentDocs%d", i), fl_filename_name(RecentDocs[i]));
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
        DocPrefs = new Fl_Preferences(GlobalPrefs, ShortFilename);
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
