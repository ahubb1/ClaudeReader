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
#include "Fl_Books_Library.h"



const char SAFE[256] =
{
    /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
    /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

    /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
    /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

    /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

std::string UriEncode(const std::string & sSrc)
{
   const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
   const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
   const int SRC_LEN = sSrc.length();
   unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
   unsigned char * pEnd = pStart;
   const unsigned char * const SRC_END = pSrc + SRC_LEN;

   for (; pSrc < SRC_END; ++pSrc)
   {
	  if (SAFE[*pSrc] || *pSrc=='/' || *pSrc=='.' || *pSrc=='_' || *pSrc=='-' || *pSrc=='(' || *pSrc==')' || *pSrc=='!')
		 *pEnd++ = *pSrc;
	  else
	  {
		 // escape this char
		 *pEnd++ = '%';
		 *pEnd++ = DEC2HEX[*pSrc >> 4];
		 *pEnd++ = DEC2HEX[*pSrc & 0x0F];
	  }
   }

   std::string sResult((char *)pStart, (char *)pEnd);
   delete [] pStart;
   return sResult;
}



//-------------------------Sort comparison functions--------------------------
bool ElementsNameSort(Fl_Library_Element* a, Fl_Library_Element* b)
{
    if(a->GetAlphaNumPosition()<b->GetAlphaNumPosition())
    //if(a->GetFilename()<b->GetFilename())
        return true;
    else
        return false;
}

bool ElementsTypeSort(Fl_Library_Element* a, Fl_Library_Element* b)
{
    if(strcmp(fl_filename_ext(a->GetFilename()),fl_filename_ext(b->GetFilename()))<0)
        return true;
    else
        return false;
}

bool ElementsLastTimeSort(Fl_Library_Element* a, Fl_Library_Element* b)
{
    if(a->GetLastTime()>b->GetLastTime())
        return true;
    else
        return false;
}

bool ElementsCreationTimeSort(Fl_Library_Element* a, Fl_Library_Element* b)
{
    if(a->GetCreationTime()>b->GetCreationTime())
        return true;
    else
        return false;
}

bool ElementsAccessTimeSort(Fl_Library_Element* a, Fl_Library_Element* b)
{
    if(a->GetAccessTime()>b->GetAccessTime())
        return true;
    else
        return false;
}
//----------------------------------------------------------------------------



Fl_Library_Element::Fl_Library_Element(int xx, int yy, int ww, int hh, const char *l, const char *abs_lib_dir, const char * relative_path, const char *filename, const char *windir):Fl_Button(xx, yy, ww, hh, l)
{
    strncpy(Filename, filename, FL_PATH_MAX);
    //Cut filename
    snprintf(ShortFilename, sizeof(ShortFilename), "%s", filename);
    char * last_pt = strrchr(ShortFilename, '.');//locate last point
    if(last_pt)
        *last_pt='\0'; //remove extension (everything after last pt)
    Fl_Button::copy_label(ShortFilename);
    //Keep lower case version for search
    int i=0;
    for(i=0; ShortFilename[i]; i++)
        ShortFilenameLC[i] = tolower(ShortFilename[i]);
    ShortFilenameLC[i]='\0';
    //Absolute one
    snprintf(AbsoluteFilename, FL_PATH_MAX, "%s%s%s", abs_lib_dir, relative_path, Filename);

    //Style
    Fl_Button::labelsize(13);
    //Fl_Button::labeltype(FL_EMBOSSED_LABEL);
    Fl_Button::box(FL_EMBOSSED_BOX);
    Fl_Button::align(FL_ALIGN_CENTER | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_CLIP );
    Fl_Button::color(FL_BACKGROUND_COLOR, FL_BACKGROUND2_COLOR);

    //Thumbnail:
        //Get md5sum and retrieve thumbnail if exists in .cache/thumbnails/large or .cache/thumbnails/normal
        std::string fileURI_s=UriEncode(std::string(AbsoluteFilename));fileURI_s="file://"+fileURI_s;
        #ifndef __WINDOWS__
        gchar *md5_hash=g_compute_checksum_for_string(G_CHECKSUM_MD5, fileURI_s.c_str(), fileURI_s.length());
        sprintf(ThumbnailName, "%s/.cache/thumbnails/large/%s.png", getenv("HOME"), md5_hash); //AND TEST ALSO: "%s/.cache/thumbnails/normal/%s.png"
        //apply it to box
        Img = new Fl_PNG_Image(ThumbnailName);
        //Id doesn't exist try folder one
        //TODO: create one:
        //open document using bdreader (other instance and command line ?)
        //reopen...
        //and on Windows: check in exe folder (as preferences) and create one with hash if not already...
        if(Img->w()==0)
        {
            //Fallback to folder.jpg
            delete Img;
            sprintf(ThumbnailName, "%s%sfolder.jpg", abs_lib_dir, relative_path);
            Img = new Fl_JPEG_Image(ThumbnailName);
        }
        #else
            /*std::wstring wc(strlen(AbsoluteFilename), L'#');
            mbstowcs(&wc[0], AbsoluteFilename, strlen(AbsoluteFilename));
            wchar_t *pwstr = (wchar_t*)wc.c_str();
            LPOLESTR lpDesc(pwstr);
            HGLOBAL hglobal=OleGetIconOfFile(lpDesc, FALSE);

            LPVOID WINAPI memPtr = GlobalLock(hglobal);

            METAFILEPICT mfp;
            memcpy((void*)&mfp, (void*)memPtr, sizeof(mfp));
            HMETAFILE hMF;
            hMF = mfp.hMF;
            GlobalUnlock(hglobal);

            int n = GetMetaFileBitsEx(hMF, 0, 0); //just get nb
            void* mfdata;
            if(n>0)
            {
                mfdata=(void*)malloc(n-1);
                GetMetaFileBitsEx(hMF, n, mfdata);
                Img = new Fl_RGB_Image ((uchar *)mfdata, mfp.xExt, mfp.yExt);
            }
            else
            {
                Img = new Fl_PNG_Image("");
            }
            GlobalFree(hglobal);
            Img = new Fl_RGB_Image ((uchar *)mfdata, mfp.xExt, mfp.yExt);*/

            /*C#:
            ShellFile shellFile = ShellFile.FromFilePath(pathToYourFile);
            Bitmap shellThumb = shellFile.Thumbnail.ExtraLargeBitmap;*/


//            // Get the image list index of the icon
//            SHFILEINFO sfi;
//            std::string wc(AbsoluteFilename);
//            LPCTSTR pszPath((LPCTSTR)AbsoluteFilename);
//            if (!SHGetFileInfo(pszPath, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX))
//            {
//                //error
//            }
//
//            // Get the jumbo image list
//            HIMAGELIST* piml;
//            if (FAILED(SHGetImageList(2, IID_IImageList, (void**)&piml))) //2 is for extra-large icons, use 4 for even large (jumbos)
//            {
//                //error
//            }
//            /*//OR
//            IImageList *piml;
//            if ( FAILED(SHGetImageList(2, IID_PPV_ARGS(&piml))) )//2 is for extra-large icons, use 4 for even large (jumbos)
//            {
//                //error
//            }*/
//
//
//            // Extract an icon
//            HICON hico;
//            ((IImageList*)piml)->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hico);
//
//            // Clean up
//            ((IImageList*)piml)->Release();
//
//            //hico is the ICON...
//            //HBITMAP bmp = hico->hbmColor; //HICON not HICONINFO....
//            ICONINFO iconinfo;
//            GetIconInfo(hico, &iconinfo);
//            HBITMAP hBitmap = iconinfo.hbmColor;

            Img = new Fl_PNG_Image("");
        #endif
        image(Img);

    //Preference file:
        Preferences pref(windir);
        if(pref.Load(Filename)<0)
        {
            PrefFilename[0]='\0';
            LastTime=0;
        }
        struct tm *time_info=pref.GetLastDate();
        LastTime = ((((((time_info->tm_year-80)*365)+time_info->tm_yday)*24+time_info->tm_hour)*60)+time_info->tm_min)*60+time_info->tm_sec; //Time in seconds since 1st january 1980...
        LastPage=pref.GetLastPageNb();
        NbPages=pref.GetNbPages();//Can be -1 if unknown


    //File info
        //Creation date (actually last modif but...)
        struct stat info_file;
        stat(fileURI_s.c_str(), &info_file);
        time_info=localtime(&(info_file.st_mtime));
        CreationTime = ((((((time_info->tm_year-80)*365)+time_info->tm_yday)*24+time_info->tm_hour)*60)+time_info->tm_min)*60+time_info->tm_sec; //Time in seconds since 1st january 1900...
        //Last access time: closer to the actual creation date on this filesystem
        time_info=localtime(&(info_file.st_atime));
        AccessTime = ((((((time_info->tm_year-80)*365)+time_info->tm_yday)*24+time_info->tm_hour)*60)+time_info->tm_min)*60+time_info->tm_sec; //Time in seconds since 1st january 1900...

    //Tooltip contains full filename and some infos
        char tmp[FL_PATH_MAX];
        sprintf(tmp, "%s%s\n", relative_path, Filename);
        time_t now = time(0);
        struct tm *nowt=localtime(&now);
        long int NowDay = round((((nowt->tm_year-80)*365)+nowt->tm_yday)+(nowt->tm_hour+nowt->tm_min/60.)/24.);
        int d=NowDay-round(LastTime/3600./24.);
        if(LastTime!=0)
        {
            if(d>0)
            {
                sprintf(tmp, "%sLast read %d days ago", tmp, d);
            }
            else
            {
                if(d==0)
                    sprintf(tmp, "%sLast read today", tmp, d);
                else
                {
                    sprintf(tmp, "%sNot read yet", tmp);
                    LastTime=-1;
                }
            }

            if(NbPages>0)
                sprintf(tmp, "%s (Page %d/%d).", tmp, LastPage, NbPages);
            else
                sprintf(tmp, "%s (Page %d/??).", tmp, LastPage);
        }
        else
        {
            sprintf(tmp, "%sNot read yet", tmp);
        }
        Fl_Button::copy_tooltip(tmp);
}




Fl_Books_Library::Fl_Books_Library(int ww, int hh, const char *l, const char * lib_dir, const char *wind, unsigned int child_ww, unsigned int child_hh, unsigned int sp=0):Fl_Double_Window(ww, hh, l)
{
    nb_visible_elem=0;
    child_w=child_ww;
    child_h=child_hh;
    _spacing=sp;

    //Top bar
    MenuBar = new Fl_Group(0, 0, w(), 25);
        //Space
        Fl_Box emptyspace(0,0,10,30);
        Main_Group = new Fl_Group(10, 3, 110+w()/6.+50+80+40+80, 20);
            //Lib folder
            Fl_Group * grp = new Fl_Group(10, 3, 110+w()/6., 20);
                Fl_Box * label = new Fl_Box(grp->x(), grp->y(), 110, 20, "Library folder:");label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
                LibFolder_Input = new Fl_Input(label->x()+label->w(), grp->y(), w()/6., 20, "");
                LibFolder_Input->value("");
                LibFolder_Input->callback(Fl_Books_Library::LibFolder_cb, this);
            grp->end();
            grp->resizable(LibFolder_Input);
            //Filter
            Fl_Group * grpp2 = new Fl_Group(grp->x()+grp->w()+5, grp->y(), 50+80, 20);
                Fl_Box * label2 = new Fl_Box(grpp2->x(), grpp2->y(), 50, 20, "Filter:");label2->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
                Filter_Choice = new Fl_Choice(label2->x()+label2->w(), grpp2->y(), 80, 20, "");
                Filter_Choice->add("All");
                Filter_Choice->add("Unread");
                Filter_Choice->add("Read");
                Filter_Choice->value(0);
                Filter_Choice->callback(Fl_Books_Library::Filter_cb, this);
            grpp2->end();
            grpp2->resizable(NULL);
            //Sort
            Fl_Group * grpp3 = new Fl_Group(grpp2->x()+grpp2->w()+5, grp->y(), 40+80, 20);
                Fl_Box * label3 = new Fl_Box(grpp3->x(), grpp3->y(), 40, 20, "Sort:");label3->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
                Sort_Choice = new Fl_Choice(label3->x()+label3->w(), grpp3->y(), 80, 20, "");
                Sort_Choice->add("Name");
                Sort_Choice->add("Type");
                Sort_Choice->add("Date");
                Sort_Choice->add("Read");
                Sort_Choice->value(3);
                Sort_Choice->callback(Fl_Books_Library::Sort_cb, this);
            grpp3->end();
            grpp3->resizable(NULL);
        Main_Group->end();
        Main_Group->resizable(grp);
        //Search
        Search_Group = new Fl_Group(w()-(60+w()/6.+100), grp->y(), 60+w()/6.+100, 20);
            Fl_Box * label4 = new Fl_Box(Search_Group->x(), Search_Group->y(), 60, 20, "Search:");label4->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
            Search_Input = new Fl_Input(w()-Search_Group->w()+label4->w(), Search_Group->y(), w()/6., 20, "");
            Search_Input->value("");
            Search_Input->callback(Fl_Books_Library::Search_cb, this);
            Search_Input->when(FL_WHEN_CHANGED);
            Search_Label = new Fl_Box(Search_Input->x()+Search_Input->w()-100, 3, 100, 20, "0 results");
            Search_Label->align(FL_ALIGN_RIGHT);
        Search_Group->end();
        Search_Group->resizable(Search_Input);
        //Space
        Fl_Box * space = new Fl_Box(grpp3->x()+grpp3->w(), grp->y(), Search_Group->x()-(grpp3->x()+grpp3->w()), 20, "");
    MenuBar->end();
    MenuBar->resizable(space);

    Main_Scroll = new Fl_Scroll(0, MenuBar->y()+MenuBar->h(), w(), h()-(MenuBar->y()+MenuBar->h()));
        emptyboxtop = new Fl_Box(Main_Scroll->x(), Main_Scroll->y(), Main_Scroll->w()-20, 10); //Margin at the top of the main scroll
        emptyboxtop->box(FL_NO_BOX);
        int box_w=180;
        int box_h=box_w*1.5333;
        Main_Table = new Fl_Pack(Main_Scroll->x(), Main_Scroll->y()+emptyboxtop->h(), Main_Scroll->w()-20, Main_Scroll->h()-2*emptyboxtop->h(),"");
        Main_Table->end();
        Main_Table->spacing(_spacing);
        Main_Table->box(FL_NO_BOX);
        Main_Table->type(Fl_Pack::VERTICAL);
        Main_Table->color(FL_SELECTION_COLOR, FL_BACKGROUND2_COLOR);
        _cols=(Main_Table->w()-spacing())/((child_w+spacing()));
        emptyboxbottom = new Fl_Box(Main_Scroll->x(), Main_Scroll->y()+Main_Scroll->h()-emptyboxtop->h(), Main_Scroll->w()-20, 5); //Margin at the top of the main scroll
        emptyboxbottom->box(FL_NO_BOX);
        Main_Table->add(emptyboxbottom);
    Main_Scroll->end();
    Main_Scroll->align(Fl_Scroll::VERTICAL_ALWAYS);//no effect...
    Main_Scroll->resizable(Main_Table);
    Main_Scroll->color(FL_SELECTION_COLOR, FL_BACKGROUND2_COLOR);

    end();
    resizable(Main_Scroll);
    //size_range(800, 300, 0, 0, 5, 5, 0);
    color(FL_BACKGROUND_COLOR);
    //Set modal if independant window
    if(Fl_Double_Window::parent()==NULL)
        set_modal();
    else
        clear_modal_states();

    //Assign library directory
    set_dir(lib_dir, wind);

    elem_callback=NULL;
    elem_callback_param=NULL;

    //Go through directory
    reload();
}


Fl_Books_Library::~Fl_Books_Library()
{
    delete MenuBar;

    //Delete elements individually first since some of them are not in any group (not visible)
    for(int i=0; i<elements.size(); i++)
    {
        delete elements[i];
    }
    elements.clear();
    delete Main_Scroll; //Contains all subpacks etc...
    sub_packs.clear();
}


//!Add a new subpack container (a row) to the library (to store/show) elements
void Fl_Books_Library::add_subpack()
{
    //Add fl_pack under last one
    Fl_Pack *new_sub_pack=new Fl_Pack(x(), 0, Main_Table->w(), child_h);
    new_sub_pack->end();
    sub_packs.push_back(new_sub_pack);
    Main_Table->add(new_sub_pack);
    new_sub_pack->type(Fl_Pack::HORIZONTAL);
    new_sub_pack->spacing(_spacing);
    new_sub_pack->add(new Fl_Box(x(), 0, (Main_Table->w()-_cols*(child_w+spacing())-spacing())/2., child_h)); //extra empty box on the left
}

//!Add a new Fl_Library_Element to the Library if appropriate (i.e. if it has a correct extension)
//! return true if element has been added, false otherwise
bool Fl_Books_Library::add_element(const char *relative_path, const char *filename, const char *windir)
{
    //List only 'true' comics files (not archives or folders)
    if( strstr(fl_filename_ext(filename), ".pdf")!=NULL ||
        strstr(fl_filename_ext(filename), ".cbz")!=NULL ||
        strstr(fl_filename_ext(filename), ".cbr")!=NULL ||
        strstr(fl_filename_ext(filename), ".cbt")!=NULL ||
        strstr(fl_filename_ext(filename), ".cb7")!=NULL ||
        strstr(fl_filename_ext(filename), ".epub")!=NULL)
    {
        //Create a new element
        nb_visible_elem++;//visible by default
        Fl_Library_Element* new_elem = new Fl_Library_Element(0, 0, child_w, child_h, "", abs_lib_dir, relative_path, filename, windir);
        elements.push_back(new_elem);
        new_elem->SetAlphaNumPosition(elements.size()-1);//Keep track of it's position in original alphanum sorting
        new_elem->callback(elem_callback, elem_callback_param);
        return true;
    }
    else
    {
        return false;
    }
}


//!Determine wich widget to show/hid based on filters and sorts
void Fl_Books_Library::refresh()
{
    //Be patient
    cursor(FL_CURSOR_WAIT);

    //Just refresh what should be visible or not and re-organize (columns...)


    //Delete existing sub_packs (w/o deleting their widget)
    for(int i=0; i<sub_packs.size(); i++)
    {
        //Remove all elements from the pack before deleting it
        for(int j=sub_packs[i]->children(); j>-1; j--) //The reverse trick !!!
        {
            sub_packs[i]->remove(j);
        }
        delete sub_packs[i]; //now useless and empty: delete
    }
    sub_packs.clear();//delete references from vector


    //If the filter is 'Unread', sort can't be 'read':
    if(Filter_Choice->value()==1)
    {
        if(Sort_Choice->value()==3)
        {
            Sort_Choice->value(0);
        }
        ((Fl_Menu_Item*)Sort_Choice->find_item("Read"))->deactivate();
    }
    else
    {
        ((Fl_Menu_Item*)Sort_Choice->find_item("Read"))->activate();
    }


    //Apply elements sorting
    switch(Sort_Choice->value())
    {
        //By name
        case 0:
            std::sort(elements.begin(), elements.end(), ElementsNameSort);
            break;

        //By type
        case 1:
            std::sort(elements.begin(), elements.end(), ElementsTypeSort);
            break;

        //By creation date
        case 2:
            std::sort(elements.begin(), elements.end(), ElementsCreationTimeSort);
            break;

        //By reading date
        case 3:
            std::sort(elements.begin(), elements.end(), ElementsLastTimeSort);
            break;

        default:
            break;
    }


    //Lower case before searching to be case insensitive
    char searchtermlc[1024];
    snprintf(searchtermlc, 1024, "%s", Search_Input->value());
    for(int i=0; searchtermlc[i]; i++)
        searchtermlc[i] = tolower(searchtermlc[i]);

    //Iterate through vector and test each elem with filters...:
    nb_visible_elem=0;
    for(int i=0; i<elements.size(); i++)
    {
        if((strstr(elements[i]->GetShortFilenameLC(), searchtermlc)!=NULL || strlen(searchtermlc)==0) && //search filter
           ( Filter_Choice->value()==0 || //Filter by: All
            (Filter_Choice->value()==1 && elements[i]->GetLastPage()==0)|| //Filter by Unread (only)
            (Filter_Choice->value()==2 && elements[i]->GetLastPage()>0) //Filter by Read (only)
           )
          )
        {
            nb_visible_elem++;
            //create a new subpack if needed
            if(nb_visible_elem>rows()*cols())
            {
                add_subpack();
            }
            //insert elem in the last subpack
            //printf("item: %d %d %d %s\n", i, sub_packs[sub_packs.size()-1]->h(), elements[i]->h(), elements[i]->label());
            sub_packs[sub_packs.size()-1]->add(elements[i]);
            //show it
            elements[i]->show();
        }
        else
        {
            elements[i]->hide();
        }
        /*redraw();
        Fl::check(); Rigolo mais lent*/
    }
    //Add a box at bottom to make a margin
    /*if(sub_packs.size()>0)
    {
        Main_Table->remove(emptyboxbottom);
        Main_Table->add(emptyboxbottom);
        emptyboxbottom->resize(Main_Table->x(), sub_packs[sub_packs.size()-1]->y()+sub_packs[sub_packs.size()-1]->h(), Main_Table->w()-20, 5); //Margin at the top of the main scroll
    }*/
    //Show nb of results
    char nb_txt[50];
    sprintf(nb_txt, "%d results", nb_visible_elem);
    Search_Label->copy_label(nb_txt);

    //Reinit scroll position
    Main_Scroll->scroll_to(0, 0);

    //Redraw everything !
    redraw();

    cursor(FL_CURSOR_DEFAULT);
}


//!Reload everything based on directory and then call Refresh for filters, sort etc...
void Fl_Books_Library::reload()
{
    //Be patient...
    Fl_Window waiting_window(300, 80, "BDReader");
    waiting_window.set_modal();
    waiting_window.callback((void (*) (Fl_Widget*))NULL);//no callback to prevent closing
    Fl_Box label(20, 10, 150, 20, "Loading comics library...");
    Fl_Progress *progress_bar = new Fl_Progress(10, 35, 280, 30);
    progress_bar->minimum(0);progress_bar->maximum(1);
    progress_bar->selection_color(FL_SELECTION_COLOR);
    waiting_window.end();
    waiting_window.show();
    cursor(FL_CURSOR_WAIT);
    deactivate();
    redraw();

    //Clear everything
    for(int i=0; i<sub_packs.size(); i++)
    {
        //IS IT NEEDED SINCE CALL MAIN_TABLE CLEAR AFTER ???? Sub-children ???
        sub_packs[i]->clear();
    }
    Main_Table->clear();
    sub_packs.clear();
    elements.clear();
    nb_visible_elem=0;


    //Browse new directory
    dirent **dir_listing;
    int nb_files=fl_filename_list(abs_lib_dir, &dir_listing, fl_casenumericsort);

    //For each file/folder in library
    for (int i=0; i<nb_files; i++)
    {
        char full_filename[FL_PATH_MAX];
        sprintf(full_filename, "%s%s", abs_lib_dir, dir_listing[i]->d_name);
        //If it'a dir, then do it again
        if(dir_listing[i]->d_name[0]!='.' && fl_filename_isdir(full_filename)!=0)
        {
            dirent **sub_dir_listing;
            char abs_sub_dir[FL_PATH_MAX];
            sprintf(abs_sub_dir, "%s%s", abs_lib_dir, dir_listing[i]->d_name);
            int nb_files2=fl_filename_list(abs_sub_dir, &sub_dir_listing, fl_casenumericsort);
            for (int j=2; j<nb_files2; j++)
            {
                //If it'a dir again, then do it again
                sprintf(full_filename, "%s%s", abs_sub_dir, sub_dir_listing[j]->d_name);
                if(sub_dir_listing[j]->d_name[0]!='.' &&  fl_filename_isdir(sub_dir_listing[j]->d_name)!=0)
                {
                    dirent **sub_sub_dir_listing;
                    char abs_sub_dir[FL_PATH_MAX];
                    snprintf(abs_sub_dir, FL_PATH_MAX, "%s%s%s", abs_lib_dir, dir_listing[i]->d_name, sub_dir_listing[j]->d_name);
                    int nb_files3=fl_filename_list(abs_sub_dir, &sub_sub_dir_listing, fl_casenumericsort);
                    for (int k=0; k<nb_files3; k++)
                    {
                        char relativepath[FL_PATH_MAX];
                        snprintf(relativepath, FL_PATH_MAX, "%s%s", dir_listing[i]->d_name, sub_dir_listing[j]->d_name);
                        add_element(relativepath, sub_sub_dir_listing[k]->d_name, windir);
                    }
                    fl_filename_free_list(&sub_sub_dir_listing, nb_files3);
                }
                add_element(dir_listing[i]->d_name, sub_dir_listing[j]->d_name, windir);
            }
            fl_filename_free_list(&sub_dir_listing, nb_files2);
        }
        else
        {
            add_element("", dir_listing[i]->d_name, windir);

        }
        Fl::check();
        progress_bar->value(i/(float)nb_files);
    }
    fl_filename_free_list(&dir_listing, nb_files);
    Fl::check();
    printf("%d comics found.\n", elements.size());


    refresh();
    cursor(FL_CURSOR_DEFAULT);
    activate();
    redraw();
}


//!Set new library directory but do not reload: see apply_dir
//! return true if new directory is used (i.e. is a directory), false otherwise
bool Fl_Books_Library::set_dir(const char * lib_dir, const char *wind)
{
    if(fl_filename_isdir(lib_dir)!=0)
    {
        //Copy lib dir as absolute path
        char tmp_abs_lib_dir[FL_PATH_MAX];
        fl_filename_absolute(tmp_abs_lib_dir, FL_PATH_MAX, lib_dir);
        snprintf(abs_lib_dir, FL_PATH_MAX, "%s/", tmp_abs_lib_dir);
        //Windir if needed
        snprintf(windir, FL_PATH_MAX, "%s", wind);

        //Make sure we have the trailing /
        if(abs_lib_dir[strlen(abs_lib_dir)-1]!='/')
        {
            abs_lib_dir[strlen(abs_lib_dir)]='/';
            abs_lib_dir[strlen(abs_lib_dir)+1]='\0';
        }
        //and that we have only one
        else
        {
            if(abs_lib_dir[strlen(abs_lib_dir)-2]=='/')
            {
                abs_lib_dir[strlen(abs_lib_dir)-1]='\0';
            }
        }
        LibFolder_Input->value(abs_lib_dir);
        LibFolder_Input->redraw();
        Search_Input->value("");
        Search_Input->redraw();

        return true;
    }
    else
    {   char msg[FL_PATH_MAX+25];
        snprintf(msg, FL_PATH_MAX+25, "Invalid directory (%s) !", lib_dir);
        fl_alert(msg);
        return false;
    }
}

//!Set new library directory (optionnal) and reload/refresh the widget accordingly
void Fl_Books_Library::apply_dir(const char * lib_dir, const char *wind)
{
    bool new_valid_dir=true;
    if(lib_dir!=0)
    {
        if(!set_dir(lib_dir, wind))
        {
            new_valid_dir=false;
        }
    }

    //Reload only when directory is valid
    if(new_valid_dir)
        reload();
}


//! Get the filename of the file listed after the one in parameter if similar name (i.e. same series)
/** \param current_filename: the full (absolute) filename of the current file
    \param suggestion_filename: the full (absolute) filename of the suggested file to open
    \param suggestion_short_filename: the short (file only) filename of the suggested file to open
    \return -1 if no suggestion found, the index of the suggestion otherwise */
int Fl_Books_Library::GetSuggestionNext(char current_filename[2048], char suggestion_filename[2048], char suggestion_short_filename[2048])
{
    //Sort elements by name
    std::sort(elements.begin(), elements.end(), ElementsNameSort);

    //Find current file in the element list
    unsigned int i=0;
    for(i=0; i<elements.size(); i++)
    {
        if(strcmp(elements[i]->GetAbsoluteFilename(), current_filename)==0)
            break;
    }

    //Not found or last one
    if(i>=elements.size()-1)
    {
        return -1;
    }
    else
    {
        //Copy suggestion name
        strncpy(suggestion_filename, elements[i+1]->GetAbsoluteFilename(), 2048);
        strncpy(suggestion_short_filename, elements[i+1]->GetFilename(), 2048);

        //Check if looks like to be in the same series: no more than 5 characters of difference
        int l=std::min(strlen(elements[i]->GetFilename()), strlen(suggestion_short_filename));
        int diff=0;
        for(int n=0; n<l; n++)
        {
            if(elements[i]->GetFilename()[n]!=suggestion_short_filename[n])
                diff++;
        }
        if(diff>5)
            return -1;
        else
            return i+1;
    }
}
