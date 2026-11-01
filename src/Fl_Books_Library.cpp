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
#include <FL/fl_utf8.h>
#include "Fl_Books_Library.h"
#include <map>
#include <FL/Fl_Menu_Item.H>



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
    snprintf(Filename, FL_PATH_MAX, "%s", filename);
    //A folder comic (listed with a final separator): its name as it is, no extension to remove
    size_t len=strlen(Filename);
    FolderComic=len>0 && (Filename[len-1]=='/' || Filename[len-1]=='\\');
    if(FolderComic)
        Filename[len-1]='\0';
    Title=Filename;
    if(!FolderComic)
    {
        size_t dot=Title.find_last_of('.');
        if(dot!=std::string::npos && dot>0)
            Title.erase(dot);
    }
    copy_tooltip(Title.c_str());
    //Cut filename
    snprintf(ShortFilename, sizeof(ShortFilename), "%s", Title.c_str());
    Fl_Button::copy_label(ShortFilename);
    //Keep lower case version for search: the whole title (the short one is cut at 24 characters)
    size_t i=0;
    for(i=0; i<Title.size() && i+1<sizeof(ShortFilenameLC); i++)
        ShortFilenameLC[i] = tolower((unsigned char)Title[i]);
    ShortFilenameLC[i]='\0';
    //Absolute one
    snprintf(AbsoluteFilename, FL_PATH_MAX, "%s%s%s", abs_lib_dir, relative_path, Filename);
    snprintf(RelativeDir, FL_PATH_MAX, "%s", relative_path);
    Library=NULL;
    TagIds=Tags::OfComic(TagKey());

    //Style (drawn as a card, see draw())
    Hover=false;
    Img=NULL;
    LastPage=0;
    NbPages=0;
    Fl_Button::box(FL_FLAT_BOX);
    Fl_Button::color(THEME_EDITOR_BG, THEME_HOVER);
    Fl_Button::clear_visible_focus();

    //Thumbnail:
        //Get md5sum and retrieve thumbnail if exists in .cache/thumbnails/large or .cache/thumbnails/normal
        std::string fileURI_s=UriEncode(std::string(AbsoluteFilename));fileURI_s="file://"+fileURI_s;
        string md5_hash = MD5(fileURI_s).hexdigest();
        #ifndef __WINDOWS__
            sprintf(ThumbnailName, "%s/.cache/thumbnails/large/%s.png", getenv("HOME"), md5_hash.c_str());
        #else
            //Own thumbnails in pref directory
            snprintf(ThumbnailName, FL_PATH_MAX, "%s\\thumbnails\\%s.png", GetUserDataDir(), md5_hash.c_str());
        #endif

        //apply it to box
        Img = new Fl_PNG_Image(ThumbnailName);
        //If does not exist
        if(Img->w()==0)
        {
            /*Fallback to folder.jpg
            delete Img;
            sprintf(ThumbnailName, "%s%sfolder.jpg", abs_lib_dir, relative_path);
            Img = new Fl_JPEG_Image(ThumbnailName);*/

            //Or create one
            if(Img->w()==0)
            {
                delete Img;
                Img=NULL;
                Document *doc=OpenPerExtension(AbsoluteFilename);
                #ifndef __WINDOWS__
                    sprintf(ThumbnailName, "%s/.cache/thumbnails/large/%s.png", getenv("HOME"), md5_hash.c_str());
                #else
                    //Own thumbnails in pref directory
                    snprintf(ThumbnailName, FL_PATH_MAX, "%s\\thumbnails", GetUserDataDir());
                    CreateDirectory(ThumbnailName, NULL);
                    snprintf(ThumbnailName, FL_PATH_MAX, "%s\\thumbnails\\%s.png", GetUserDataDir(), md5_hash.c_str());
                #endif
                if(doc && doc->WriteDocumentThumbnail(ThumbnailName, 180)>=0)
                {
                    Img = new Fl_PNG_Image(ThumbnailName);
                }
                delete doc;
            }
        }
        image(Img);

    //Preference file:
        #ifndef DEBUG2
            Preferences pref(windir);
            if(pref.Load(Filename)<0)
            {
                PrefFilename[0]='\0';
                LastTime=0;
            }
            else
            {
                //pref.ForceSave();
                struct tm *time_info=pref.GetLastDate();
                LastTime = ((((((time_info->tm_year-80)*365)+time_info->tm_yday)*24+time_info->tm_hour)*60)+time_info->tm_min)*60+time_info->tm_sec; //Time in seconds since 1st january 1980...
                LastPage=pref.GetLastPageNb();
                NbPages=pref.GetNbPages();//Can be -1 if unknown
            }
        #else
            PrefFilename[0]='\0';
            LastTime=0;
        #endif

    //File info
        //Creation date (actually last modif but...)
        struct stat info_file;
        fl_stat(fileURI_s.c_str(), &info_file);
        struct tm *time_info=localtime(&(info_file.st_mtime));
        CreationTime = ((((((time_info->tm_year-80)*365)+time_info->tm_yday)*24+time_info->tm_hour)*60)+time_info->tm_min)*60+time_info->tm_sec; //Time in seconds since 1st january 1900...
        //Last access time: closer to the actual creation date on this filesystem
        time_info=localtime(&(info_file.st_atime));
        AccessTime = ((((((time_info->tm_year-80)*365)+time_info->tm_yday)*24+time_info->tm_hour)*60)+time_info->tm_min)*60+time_info->tm_sec; //Time in seconds since 1st january 1900...

    //Tooltip contains full filename and some infos
        char tmp[FL_PATH_MAX];
        sprintf(tmp, "%s%s\n", relative_path, Filename);
        time_t now = time(0);
        struct tm *nowt=localtime(&now);
        long int NowHour = round((((nowt->tm_year-80)*365)+nowt->tm_yday)*24+nowt->tm_hour+nowt->tm_min/60.);
        int h=NowHour-round(LastTime/3600.);
        int d=round((NowHour-LastTime/3600.)/24.);
        if(LastTime!=0)
        {
            //Append to tmp (sprintf on its own buffer is undefined)
            if(d>0)
            {
                sprintf(tmp+strlen(tmp), "Last read %d days ago", d);
            }
            else
            {
                if(d==0)
                {
                    sprintf(tmp+strlen(tmp), "Last read %dh ago", h);
                }
                else
                {
                    sprintf(tmp+strlen(tmp), "Not read yet");
                    LastTime=-1;
                }
            }

            if(NbPages>0)
                sprintf(tmp+strlen(tmp), " (Page %d/%d).", LastPage, NbPages);
            else
                sprintf(tmp+strlen(tmp), " (Page %d/??).", LastPage);
        }
        else
        {
            sprintf(tmp+strlen(tmp), "Not read yet");
        }
        Fl_Button::copy_tooltip(tmp);
}



int Fl_Library_Element::handle(int event)
{
    if(event==FL_ENTER || event==FL_LEAVE)
    {
        Hover=(event==FL_ENTER);
        redraw();
    }
    //Right button: tags menu (not an opening), shown when the button is released as the Windows context menus
    if((event==FL_PUSH || event==FL_RELEASE) && Fl::event_button()==FL_RIGHT_MOUSE)
    {
        if(event==FL_RELEASE)
            ShowTagsMenu();
        return 1;
    }
    return Fl_Button::handle(event);
}

bool Fl_Library_Element::HasTag(int id)
{
    return std::find(TagIds.begin(), TagIds.end(), id)!=TagIds.end();
}

void Fl_Library_Element::ReloadTags()
{
    TagIds=Tags::OfComic(TagKey());
    redraw();
}

//! Label usable in a menu (& and / are special)
static std::string MenuText(const std::string &text)
{
    std::string out;
    for(size_t i=0; i<text.size(); i++)
    {
        if(text[i]=='&')
            out+='&';
        out+=text[i];
    }
    return out;
}

//! Tick or untick the tags of the comic, create a tag, manage the tags
void Fl_Library_Element::ShowTagsMenu()
{
    const std::vector<TagDef> all=Tags::All();
    std::vector<std::string> labels;
    for(size_t i=0; i<all.size(); i++)
        labels.push_back(MenuText(all[i].Name));
    std::vector<Fl_Menu_Item> menu(all.size()+3);
    memset(menu.data(), 0, menu.size()*sizeof(Fl_Menu_Item));
    for(size_t i=0; i<all.size(); i++)
    {
        menu[i].text=labels[i].c_str();
        menu[i].flags=FL_MENU_TOGGLE|(HasTag(all[i].Id) ? FL_MENU_VALUE : 0)|(i+1==all.size() ? FL_MENU_DIVIDER : 0);
    }
    menu[all.size()].text="New tag...";
    menu[all.size()+1].text="Manage tags...";

    const Fl_Menu_Item *chosen=menu[0].popup(Fl::event_x(), Fl::event_y());
    if(!chosen)
        return;
    size_t index=chosen-&menu[0];
    if(index<all.size())
        Tags::Toggle(TagKey(), all[index].Id);
    else if(index==all.size())
    {
        CenterNextMessage('?');
        const char *name=fl_input("New tag for \"%s\":", "", ShortFilename);
        if(!name || !*name)
            return;
        int id=Tags::Create(name);
        if(id>0 && !HasTag(id))
            Tags::Toggle(TagKey(), id);
    }
    else
    {
        if(Library && Library->OnManageTags)
            Library->OnManageTags();
        return;
    }
    ReloadTags();
    if(Library)
        Library->refresh_tags();
}

//! Card: thumbnail, reading progress bar and title
void Fl_Library_Element::draw()
{
    const int pad=8, title_h=34, bar_h=3, chips_h=16;

    //Background, lighter when hovered
    fl_color(Hover ? THEME_HOVER : THEME_EDITOR_BG);
    fl_rectf(x(), y(), w(), h());

    //Thumbnail, fitted in the image area
    int ix=x()+pad, iy=y()+pad, iw=w()-2*pad, ih=h()-2*pad-title_h-chips_h;
    if(HasThumbnail())
    {
        Img->scale(iw, ih, 1, 1);
        Img->draw(ix+(iw-Img->w())/2, iy+ih-Img->h());
    }
    else
    {
        fl_color(THEME_INPUT_BG);
        fl_rectf(ix, iy, iw, ih);
        fl_color(THEME_FG_DIM);
        fl_font(FL_HELVETICA, 12);
        fl_draw("No preview", ix, iy, iw, ih, FL_ALIGN_CENTER);
    }

    //Reading progress
    int by=iy+ih+4;
    if(NbPages>0 && LastPage>0)
    {
        fl_color(THEME_BORDER);
        fl_rectf(ix, by, iw, bar_h);
        fl_color(THEME_ACCENT);
        fl_rectf(ix, by, (int)(iw*fmin(1., LastPage/(double)NbPages)), bar_h);
    }

    //Title: one line, "..." when too long (the whole title in the tooltip)
    fl_color(Hover ? THEME_FG_BRIGHT : THEME_FG);
    fl_font(FL_HELVETICA, 12);
    std::string title=Title;
    if(fl_width(title.c_str())>iw)
    {
        const char *ellipsis="\xe2\x80\xa6";
        while(!title.empty() && fl_width((title+ellipsis).c_str())>iw)
        {
            //Remove the last UTF-8 character
            size_t cut=title.size()-1;
            while(cut>0 && ((unsigned char)title[cut]&0xC0)==0x80)
                cut--;
            title.erase(cut);
        }
        title+=ellipsis;
    }
    fl_push_clip(ix, by+bar_h+4, iw, title_h-bar_h-8);
    fl_draw(title.c_str(), ix, by+bar_h+4, iw, title_h-bar_h-8, FL_ALIGN_TOP_LEFT);
    fl_pop_clip();

    //Tags: colored dot and name
    int cx=ix, cy=y()+h()-pad-chips_h;
    fl_font(FL_HELVETICA, 11);
    fl_push_clip(ix, cy, iw, chips_h);
    for(size_t i=0; i<TagIds.size(); i++)
    {
        const TagDef *tag=Tags::Get(TagIds[i]);
        if(!tag)
            continue;
        int tw=(int)fl_width(tag->Name.c_str());
        fl_color(Tags::Color(tag->Color));
        fl_pie(cx, cy+(chips_h-8)/2, 8, 8, 0, 360);
        fl_color(THEME_FG_DIM);
        fl_draw(tag->Name.c_str(), cx+11, cy, tw+2, chips_h, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        cx+=11+tw+10;
    }
    fl_pop_clip();
}


//------------------------------------Folder card------------------------

Fl_Library_Folder::Fl_Library_Folder(int ww, int hh, const std::string &name, const std::string &relative_dir, int count):
    Fl_Button(0, 0, ww, hh),
    Name(name),
    RelativeDir(relative_dir),
    Count(count),
    Hover(false)
{
    box(FL_FLAT_BOX);
    clear_visible_focus();
    Icon=ThemeIcon(ICON_OPEN, THEME_FG_DIM, 96);
    IconHover=ThemeIcon(ICON_OPEN, THEME_FG_BRIGHT, 96);
    copy_tooltip(name.c_str());
}

Fl_Library_Folder::~Fl_Library_Folder()
{
    delete Icon;
    delete IconHover;
}

int Fl_Library_Folder::handle(int event)
{
    if(event==FL_ENTER || event==FL_LEAVE)
    {
        Hover=(event==FL_ENTER);
        redraw();
    }
    return Fl_Button::handle(event);
}

void Fl_Library_Folder::draw()
{
    const int pad=8, title_h=34, chips_h=16;
    fl_color(Hover ? THEME_HOVER : THEME_EDITOR_BG);
    fl_rectf(x(), y(), w(), h());

    //Icon in the image area (as the comics covers)
    int ix=x()+pad, iy=y()+pad, iw=w()-2*pad, ih=h()-2*pad-title_h-chips_h;
    fl_color(THEME_BAR_BG);
    fl_rectf(ix, iy, iw, ih);
    Fl_SVG_Image *icon=Hover ? IconHover : Icon;
    icon->draw(ix+(iw-icon->w())/2, iy+(ih-icon->h())/2);

    //Name and number of comics
    fl_font(FL_HELVETICA_BOLD, 12);
    fl_color(Hover ? THEME_FG_BRIGHT : THEME_FG);
    fl_push_clip(ix, iy+ih+7, iw, title_h-6);
    fl_draw(Name.c_str(), ix, iy+ih+7, iw, title_h-6, FL_ALIGN_TOP_LEFT|FL_ALIGN_WRAP);
    fl_pop_clip();
    fl_font(FL_HELVETICA, 11);
    fl_color(THEME_FG_DIM);
    std::string count=std::to_string(Count)+(Count>1 ? " comics" : " comic");
    fl_draw(count.c_str(), ix, y()+h()-pad-chips_h, iw, chips_h, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
}



void reload_cb(void * library)
{
    ((Fl_Books_Library*)library)->reload();
}

void SelectLibFolder_cb(Fl_Widget * w, void* p)
{
    Fl_Books_Library *bl = (Fl_Books_Library*)p;

    Fl_Native_File_Chooser chooser=Fl_Native_File_Chooser();
    chooser.title("Select library folder");
    chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    char folder[FL_PATH_MAX];
    chooser.directory(bl->abs_lib_dir);

    //Show the chooser window
    int ret=chooser.show();
    if(ret !=1 && ret!=-1)
    {
        bl->LibFolder_Input->value(chooser.filename());
        bl->stop_browsing();
        bl->apply_dir(bl->LibFolder_Input->value(), bl->windir);
        bl->LibFolder_Input->parent()->redraw();
    }
}

Fl_Books_Library::Fl_Books_Library(int xx, int yy, int ww, int hh, const char *l, const char * lib_dir, const char *wind, unsigned int child_ww, unsigned int child_hh, unsigned int sp=0):Fl_Group(xx, yy, ww, hh, l)//Fl_Double_Window(ww, hh, l)
{
    nb_visible_elem=0;
    child_w=child_ww;
    child_h=child_hh;
    _spacing=sp;

    //Top bar (see layout_toolbar)
    FoldersView=true;
    MenuBar = new Fl_Group(x(), y(), w(), 25);
        int main_w=4./5.*w()-15;
        Main_Group = new Fl_Group(x()+10, y()+3, main_w, 20);
            //Folders view or all the comics
            View_Button = new Fl_Activity_Button(x()+10, y()+3, 26, 20, ICON_OPEN, 16);
            View_Button->color(THEME_BAR_BG);
            View_Button->tooltip("Folders view (on) or all the comics in one list (off)");
            View_Button->SetToggled(true);
            View_Button->callback(View_cb, this);
            //Lib folder (what is left once the filters and sort are placed)
            Fl_Group * grp = new Fl_Group(x()+10+31, y()+3, main_w-(31+5+50+80+5+40+70+5+35+110), 20);
                //File explorer like path bar: folders, sub-folders menus, text edition and folder chooser
                Home_Button = new Fl_Button(grp->x(), grp->y(), 96, 20, "\xe2\x86\x90  My library");
                Home_Button->box(FL_FLAT_BOX);
                Home_Button->color(THEME_ACCENT, fl_rgb_color(2, 106, 192));
                Home_Button->labelcolor(FL_WHITE);
                Home_Button->labelsize(12);
                Home_Button->clear_visible_focus();
                Home_Button->tooltip("Back to your library folder");
                Home_Button->callback(Home_cb, this);
                Home_Button->hide();
                Fl_Path_Bar *path_bar = new Fl_Path_Bar(grp->x(), grp->y(), grp->w(), 20);
                PathBar=path_bar;
                path_bar->browse_button()->tooltip("Choose the folder containing your comics");
                path_bar->browse_button()->callback(SelectLibFolder_cb, (void*)this);
                LibFolder_Input = path_bar->input();
                LibFolder_Input->value("");
                LibFolder_Input->callback(Fl_Books_Library::LibFolder_cb, this);
            grp->end();
            grp->resizable(path_bar);
            //Filter
            Fl_Group * grpp2 = new Fl_Group(grp->x()+grp->w()+5, grp->y(), 50+80, 20);
                Fl_Box * label2 = new Fl_Box(grpp2->x(), grpp2->y(), 50, 20, "Filter:");label2->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
                Filter_Choice = new Fl_Choice(label2->x()+label2->w(), grpp2->y(), 80, 20, "");
                Filter_Choice->add("All");
                Filter_Choice->add("Unread");
                Filter_Choice->add("Read");
                Filter_Choice->value(0);
                Filter_Choice->callback(Fl_Books_Library::Filter_cb, this);
            grpp2->end();
            grpp2->resizable(NULL);
            //Sort
            Fl_Group * grpp3 = new Fl_Group(grpp2->x()+grpp2->w()+5, grp->y(), 40+70, 20);
                Fl_Box * label3 = new Fl_Box(grpp3->x(), grpp3->y(), 40, 20, "Sort:");label3->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
                Sort_Choice = new Fl_Choice(label3->x()+label3->w(), grpp3->y(), 70, 20, "");
                Sort_Choice->add("Name");
                Sort_Choice->add("Type");
                Sort_Choice->add("Date");
                Sort_Choice->add("Read");
                Sort_Choice->value(3);
                Sort_Choice->callback(Fl_Books_Library::Sort_cb, this);
            grpp3->end();
            grpp3->resizable(NULL);
            //Tag
            Fl_Group * grpp4 = new Fl_Group(grpp3->x()+grpp3->w()+5, grp->y(), 35+110, 20);
                Fl_Box * label4t = new Fl_Box(grpp4->x(), grpp4->y(), 35, 20, "Tag:");label4t->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
                Tag_Choice = new Fl_Choice(label4t->x()+label4t->w(), grpp4->y(), 110, 20, "");
                Tag_Choice->tooltip("Only the comics with this tag (in the whole library)");
                Tag_Choice->callback(Fl_Books_Library::Tag_cb, this);
            grpp4->end();
            grpp4->resizable(NULL);
        Main_Group->end();
        Main_Group->resizable(grp);
        //Search
        Search_Group = new Fl_Group(x()+10+main_w+10, y()+3, w()-main_w-30, 20);
            Search_Input = new Fl_Hint_Input(Search_Group->x(), Search_Group->y(), Search_Group->w()-80, 20);
            Search_Input->Hint="Search   (#tag: tags only)";
            //Backspace in the empty search: parent folder
            Search_Input->OnEmptyBackspace=[this]()
            {
                if(CurrentDir.empty())
                    return;
                std::string parent=CurrentDir.substr(0, CurrentDir.size()-1);
                size_t slash=parent.rfind('/');
                open_folder(slash==std::string::npos ? "" : parent.substr(0, slash+1));
            };
            Search_Input->value("");
            Search_Input->callback(Fl_Books_Library::Search_cb, this);
            Search_Input->when(FL_WHEN_CHANGED);
            Search_Label = new Fl_Box(Search_Group->x()+Search_Group->w()-75, Search_Group->y(), 75, 20, "0 comics");
            Search_Label->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
        Search_Group->end();
        Search_Group->resizable(Search_Input);
    MenuBar->end();
    MenuBar->resizable(NULL);

    Main_Scroll = new Fl_Scroll(xx, MenuBar->y()+MenuBar->h(), w(), h()-MenuBar->h());
        emptyboxtop = new Fl_Box(Main_Scroll->x(), Main_Scroll->y(), Main_Scroll->w()-20, 10); //Margin at the top of the main scroll
        emptyboxtop->box(FL_NO_BOX);
        int box_w=180;
        int box_h=box_w*1.5333;
        Main_Table = new Fl_Pack(Main_Scroll->x(), Main_Scroll->y()+emptyboxtop->h(), Main_Scroll->w()-20, Main_Scroll->h()-2*emptyboxtop->h(),"");
        Main_Table->end();
        Main_Table->spacing(_spacing);
        Main_Table->box(FL_NO_BOX);
        Main_Table->type(Fl_Pack::VERTICAL);
        Main_Table->color(THEME_EDITOR_BG, THEME_EDITOR_BG);
        _cols=(Main_Table->w()-spacing())/((child_w+spacing()));
        emptyboxbottom = new Fl_Box(Main_Scroll->x(), Main_Scroll->y()+Main_Scroll->h()-emptyboxtop->h(), Main_Scroll->w()-20, 5); //Margin at the top of the main scroll
        emptyboxbottom->box(FL_NO_BOX);
        Main_Table->add(emptyboxbottom);
    Main_Scroll->end();
    Main_Scroll->align(Fl_Scroll::VERTICAL_ALWAYS);//no effect...
    Main_Scroll->resizable(Main_Table);
    Main_Scroll->color(THEME_EDITOR_BG, THEME_EDITOR_BG);
    Main_Scroll->scrollbar.color(THEME_EDITOR_BG, fl_rgb_color(79, 79, 79));
    Main_Scroll->hscrollbar.color(THEME_EDITOR_BG, fl_rgb_color(79, 79, 79));
    Main_Scroll->scrollbar.box(FL_FLAT_BOX);
    Main_Scroll->hscrollbar.box(FL_FLAT_BOX);
    Main_Scroll->box(FL_FLAT_BOX);
    MenuBar->box(FL_FLAT_BOX);
    MenuBar->color(THEME_BAR_BG);
    Fl_Choice *choices[]={Filter_Choice, Sort_Choice, Tag_Choice};
    for(int i=0; i<3; i++)
    {
        choices[i]->box(THEME_INPUT_BOX);
        choices[i]->color(THEME_INPUT_BG);
        choices[i]->down_box(FL_FLAT_BOX);
        choices[i]->textcolor(THEME_FG);
    }
    Search_Label->labelcolor(THEME_FG_DIM);
    Search_Input->box(THEME_INPUT_BOX);
    Search_Input->textcolor(THEME_FG);
    Search_Input->textsize(12);
    fill_tag_choice();

    end();
    resizable(Main_Scroll);
    //size_range(800, 300, 0, 0, 5, 5, 0);
    color(THEME_EDITOR_BG);
    box(FL_FLAT_BOX);
    /*Set modal if independant window
    if(Fl_Double_Window::parent()==NULL)
        set_modal();
    else
        clear_modal_states();*/

    //Assign library directory
    set_dir(lib_dir, wind);

    elem_callback=NULL;
    elem_callback_param=NULL;

    //Go through directory but allow time to show windows...
    Fl::add_timeout(0.2, reload_cb, (void*)this);
    Fl::check();
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
bool Fl_Books_Library::add_element(const char *relative_path, const char *filename, const char *windir, bool folder_comic)
{
    //List only 'true' comics files (not archives) and the folders of images
    DOCUMENT_TYPE type=GetDocumentType(filename);
    if( folder_comic || type==DOC_MUPDF || (type==DOC_ARCHIVE && strncasecmp(fl_filename_ext(filename), ".cb", 3)==0) )
    {
        //Create a new element
        nb_visible_elem++;//visible by default
        Fl_Library_Element* new_elem = new Fl_Library_Element(0, 0, child_w, child_h, "", abs_lib_dir, relative_path, filename, windir);
        new_elem->Library=this;
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


//!Determine wich widget to show/hide based on filters and sorts
void Fl_Books_Library::refresh()
{
    //Be patient
    fl_cursor(FL_CURSOR_WAIT);

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


    //Search (case insensitive) in the names and the tags, or in the tags only with "#"
    std::string search=Search_Input->value();
    std::transform(search.begin(), search.end(), search.begin(), [](unsigned char c){ return (char)tolower(c); });
    bool tags_only=!search.empty() && search[0]=='#';
    if(tags_only)
        search=search.substr(1);
    int tag_filter=(Tag_Choice->value()>0 && Tag_Choice->value()-1<(int)TagChoiceIds.size()) ? TagChoiceIds[Tag_Choice->value()-1] : -1;
    //Folders view, except when searching or filtering by tag: the whole library
    bool flat=!FoldersView || !search.empty() || tags_only || tag_filter>0;

    //Folder cards (rebuilt): the sub-folders of the current folder with comics inside
    for(size_t i=0; i<folders.size(); i++)
        Fl::delete_widget(folders[i]);
    folders.clear();
    std::vector<Fl_Widget*> visible;
    if(!flat)
    {
        std::map<std::string, int> subs;
        for(size_t i=0; i<elements.size(); i++)
        {
            std::string rel=elements[i]->GetRelativeDir();
            if(rel.size()>CurrentDir.size() && rel.compare(0, CurrentDir.size(), CurrentDir)==0)
            {
                std::string rest=rel.substr(CurrentDir.size());
                subs[rest.substr(0, rest.find('/')+1)]++;
            }
        }
        std::vector< std::pair<std::string, int> > sorted(subs.begin(), subs.end());
        std::sort(sorted.begin(), sorted.end(), [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b)
        {
            return strcasecmp(a.first.c_str(), b.first.c_str())<0;
        });
        Fl_Group *current=Fl_Group::current();
        Fl_Group::current(NULL);
        for(size_t i=0; i<sorted.size(); i++)
        {
            std::string name=sorted[i].first.substr(0, sorted[i].first.size()-1);
            Fl_Library_Folder *folder=new Fl_Library_Folder(child_w, child_h, name, CurrentDir+sorted[i].first, sorted[i].second);
            folder->callback(Folder_cb, this);
            folders.push_back(folder);
            visible.push_back(folder);
        }
        Fl_Group::current(current);
    }

    //Comics
    int nb_comics=0;
    for(size_t i=0; i<elements.size(); i++)
    {
        Fl_Library_Element *e=elements[i];
        bool shown=flat || CurrentDir==e->GetRelativeDir();
        //Search
        if(shown && !search.empty())
        {
            bool found=!tags_only && strstr(e->GetShortFilenameLC(), search.c_str())!=NULL;
            for(size_t t=0; t<e->GetTags().size() && !found; t++)
            {
                const TagDef *tag=Tags::Get(e->GetTags()[t]);
                std::string name=tag ? tag->Name : "";
                std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return (char)tolower(c); });
                found=!name.empty() && name.find(search)!=std::string::npos;
            }
            shown=found;
        }
        //Read state
        shown=shown && (Filter_Choice->value()==0 ||
                        (Filter_Choice->value()==1 && e->GetLastTime()==0) ||
                        (Filter_Choice->value()==2 && e->GetLastTime()>0));
        //Tag
        shown=shown && (tag_filter<0 || e->HasTag(tag_filter));
        if(shown)
        {
            visible.push_back(e);
            nb_comics++;
        }
        else
            e->hide();
    }

    //In rows
    nb_visible_elem=0;
    for(size_t i=0; i<visible.size(); i++)
    {
        nb_visible_elem++;
        if(nb_visible_elem>rows()*cols())
            add_subpack();
        sub_packs[sub_packs.size()-1]->add(visible[i]);
        visible[i]->show();
    }

    //Show nb of results
    char nb_txt[50];
    snprintf(nb_txt, sizeof(nb_txt), nb_comics>1 ? "%d comics" : "%d comic", nb_comics);
    Search_Label->copy_label(nb_txt);

    //Reinit scroll position
    Main_Scroll->scroll_to(0, 0);

    //Redraw everything !
    redraw();

    fl_cursor(FL_CURSOR_DEFAULT);
}


//!Reload everything based on directory and then call Refresh for filters, sort etc...
//! Add a library entry: a comics file, or every comics in a folder (browsed recursively up to MAX_LIBRARY_DEPTH levels)
//! \param relative_dir: folder of the entry, relative to the library folder ("" or ending with a separator)
//! \param name: name of the entry, as listed by fl_filename_list (folders end with '/')
void Fl_Books_Library::scan_entry(const char *relative_dir, const char *name, int depth)
{
    //Hidden files, current and parent folders
    if(name[0]=='.')
        return;

    char full_name[FL_PATH_MAX];
    snprintf(full_name, FL_PATH_MAX, "%s%s%s", abs_lib_dir, relative_dir, name);
    if(fl_filename_isdir(full_name)==0)
    {
        add_element(relative_dir, name, windir);
        return;
    }

    //A folder with two page images or more is a comic, as when it is opened (issue #5): store downloads in
    //Images format, scans... Its sub-folders are not browsed
    {
        dirent **listing;
        int nb=fl_filename_list(full_name, &listing, fl_casenumericsort);
        int images=0;
        for(int i=0; i<nb && images<2; i++)
        {
            const char *entry=listing[i]->d_name;
            size_t len=strlen(entry);
            if(len>0 && entry[len-1]!='/' && IsPageImage(entry))
                images++;
        }
        if(nb>0)
            fl_filename_free_list(&listing, nb);
        if(images>=2)
        {
            //Listed with a final separator: a folder comic for the element
            std::string folder=name;
            if(folder.empty() || (folder.back()!='/' && folder.back()!='\\'))
                folder+="/";
            add_element(relative_dir, folder.c_str(), windir, true);
            return;
        }
    }

    if(depth>=MAX_LIBRARY_DEPTH)
        return;

    char sub_relative_dir[FL_PATH_MAX];
    snprintf(sub_relative_dir, FL_PATH_MAX, "%s%s", relative_dir, name);
    //Make sure the folder ends with a separator
    size_t len=strlen(sub_relative_dir);
    if(len>0 && sub_relative_dir[len-1]!='/' && sub_relative_dir[len-1]!='\\' && len+1<FL_PATH_MAX)
        strcat(sub_relative_dir, "/");

    dirent **listing;
    int nb=fl_filename_list(full_name, &listing, fl_casenumericsort);
    for(int i=0; i<nb; i++)
        scan_entry(sub_relative_dir, listing[i]->d_name, depth+1);
    if(nb>0)
        fl_filename_free_list(&listing, nb);
}

void Fl_Books_Library::mark_duplicate_names()
{
    //Comics of a folder by name (case insensitive)
    std::map<std::string, std::vector<Fl_Library_Element*> > names;
    for(size_t i=0; i<elements.size(); i++)
        names[std::string(elements[i]->GetRelativeDir())+"|"+elements[i]->GetShortFilenameLC()].push_back(elements[i]);
    for(std::map<std::string, std::vector<Fl_Library_Element*> >::iterator it=names.begin(); it!=names.end(); ++it)
    {
        if(it->second.size()<2)
            continue;
        for(size_t k=0; k<it->second.size(); k++)
        {
            Fl_Library_Element *e=it->second[k];
            std::string format;
            if(e->IsFolderComic())
                format="folder";
            else
            {
                const char *ext=fl_filename_ext(e->GetFilename());
                for(const char *c=ext[0]=='.' ? ext+1 : ext; *c; c++)
                    format+=(char)toupper((unsigned char)*c);
            }
            e->SetTitle(e->GetTitle()+" ("+format+")");
        }
    }
}

void Fl_Books_Library::reload()
{
    //The first scan, scheduled by the constructor, is not needed any more
    Fl::remove_timeout(reload_cb, this);
    //Asked while scanning (the scan handles the events: timers, clicks...): after this one, not inside it,
    //which listed the comics twice
    if(Reloading)
    {
        ReloadAgain=true;
        return;
    }
    Reloading=true;

    //Be patient...
    Fl_Window waiting_window(300, 80, "ClaudeReader");
    waiting_window.set_modal();
    waiting_window.callback((void (*) (Fl_Widget*))NULL);//no callback to prevent closing
    Fl_Box label(20, 10, 150, 20, "Loading comics library...");
    Fl_Progress *progress_bar = new Fl_Progress(10, 35, 280, 30);
    progress_bar->minimum(0);progress_bar->maximum(1);
    progress_bar->selection_color(FL_SELECTION_COLOR);
    waiting_window.end();
    waiting_window.show();
    fl_cursor(FL_CURSOR_WAIT);
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
        //Sub-folders are browsed recursively
        scan_entry("", dir_listing[i]->d_name, 0);
        progress_bar->value(i/(float)nb_files);
        /*Exact values are inacurate: nb of files is not nb of comics: folders, other files etc...
        char progresstxt[12];
        sprintf(progresstxt, "%d/%d", i, nb_files);
        progress_bar->label(progresstxt);*/
        Fl::check();
    }
    fl_filename_free_list(&dir_listing, nb_files);
    Fl::check();
    #ifdef DEBUG
        printf("%d comics found.\n", elements.size());
    #endif

    mark_duplicate_names();
    refresh();
    fl_cursor(FL_CURSOR_DEFAULT);
    activate();
    redraw();

    Reloading=false;
    if(ReloadAgain)
    {
        ReloadAgain=false;
        reload();
    }
}


//!Set new library directory but do not reload: see apply_dir
//! return true if new directory is used (i.e. is a directory), false otherwise
bool Fl_Books_Library::set_dir(const char * lib_dir, const char *wind)
{
    if(fl_filename_isdir(lib_dir)!=0)
    {
        char trailing_char;
        #ifdef __WINDOWS__
            trailing_char='\\';
        #else
            trailing_char='/';
        #endif // __WINDOWS__

        //Another library folder: back to its top
        std::string former_dir=abs_lib_dir;

        //Copy lib dir as absolute path
        char tmp_abs_lib_dir[FL_PATH_MAX];
        fl_filename_absolute(tmp_abs_lib_dir, FL_PATH_MAX, lib_dir);
        snprintf(abs_lib_dir, FL_PATH_MAX, "%s%c", tmp_abs_lib_dir, trailing_char);
        //Windir if needed
        snprintf(windir, FL_PATH_MAX, "%s", wind);

        //Make sure we have the trailing / or '\'
        if(abs_lib_dir[strlen(abs_lib_dir)-1]!=trailing_char)
        {
            abs_lib_dir[strlen(abs_lib_dir)]=trailing_char;
            abs_lib_dir[strlen(abs_lib_dir)+1]='\0';
        }
        //and that we have only one
        else
        {
            if(abs_lib_dir[strlen(abs_lib_dir)-2]==trailing_char)
            {
                abs_lib_dir[strlen(abs_lib_dir)-1]='\0';
            }
        }
        printf("%s\n", abs_lib_dir);
        if(former_dir!=abs_lib_dir)
            CurrentDir.clear();
        update_path_bar();
        Search_Input->value("");
        Search_Input->redraw();

        return true;
    }
    else
    {   char msg[FL_PATH_MAX+25];
        snprintf(msg, FL_PATH_MAX+25, "Invalid directory (%s) !", lib_dir);
        (CenterNextMessage(), fl_alert)(msg);
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
    update_path_bar();
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


//------------------------------------Folders, tags, toolbar------------------------

void Fl_Books_Library::resize(int xx, int yy, int ww, int hh)
{
    int old_w=w(), old_h=h();
    Fl_Group::resize(xx, yy, ww, hh);
    layout_toolbar();
    if(ww!=old_w || hh!=old_h)
    {
        emptyboxtop->resize(Main_Scroll->x(), Main_Scroll->y(), Main_Scroll->w()-20, 10);
        Main_Table->resize(Main_Scroll->x(), Main_Scroll->y()+emptyboxtop->h(), Main_Scroll->w()-20, Main_Scroll->h()-emptyboxtop->h());
        _cols=(Main_Table->w()-spacing())/((child_w+spacing()));
        refresh();
        Main_Scroll->init_sizes();
        Main_Scroll->scroll_to(0, 0);
    }
}

//! 4/5 of the width for the view, folder, filter, sort and tag, the rest for the search
void Fl_Books_Library::layout_toolbar()
{
    int main_w=4./5.*w()-15;
    MenuBar->resize(x(), y(), w(), 25);
    Main_Group->resize(x()+10, y()+3, main_w, 20);
    Search_Group->resize(x()+10+main_w+10, y()+3, w()-main_w-30, 20);
}

void Fl_Books_Library::update_path_bar()
{
    std::string path=std::string(abs_lib_dir)+CurrentDir;
    LibFolder_Input->value(path.c_str());
    LibFolder_Input->parent()->redraw();
}

//! Path bar: a folder in the library is shown, another one becomes the library folder
void Fl_Books_Library::path_chosen(const char *path)
{
    auto normalize=[](std::string text)
    {
        for(size_t i=0; i<text.size(); i++)
        {
            if(text[i]=='\\')
                text[i]='/';
            #ifdef _WIN32
                text[i]=(char)tolower((unsigned char)text[i]); //Case insensitive file system
            #endif
        }
        if(text.empty() || text[text.size()-1]!='/')
            text+='/';
        return text;
    };
    std::string chosen=path, root=normalize(abs_lib_dir);
    //While browsing: another folder is browsed too (or the library, when it is in it)
    if(is_browsing() && normalize(chosen).compare(0, root.size(), root)!=0)
    {
        browse(path);
        return;
    }
    if(normalize(chosen).compare(0, root.size(), root)==0)
    {
        std::string rel=chosen.size()>root.size() ? chosen.substr(root.size()) : "";
        std::replace(rel.begin(), rel.end(), '\\', '/');
        if(!rel.empty() && rel[rel.size()-1]!='/')
            rel+='/';
        open_folder(rel);
        return;
    }
    apply_dir(path, windir);
}

//! Same folder test as the path bar: separators and (on Windows) case
static std::string NormalizedDir(std::string text)
{
    for(size_t i=0; i<text.size(); i++)
    {
        if(text[i]=='\\')
            text[i]='/';
        #ifdef _WIN32
            text[i]=(char)tolower((unsigned char)text[i]);
        #endif
    }
    if(text.empty() || text[text.size()-1]!='/')
        text+='/';
    return text;
}

void Fl_Books_Library::browse(const char *dir)
{
    std::string home=home_dir(), folder=dir;
    std::string home_n=NormalizedDir(home), folder_n=NormalizedDir(folder);
    //In the library: the library, at this folder
    if(folder_n.compare(0, home_n.size(), home_n)==0)
    {
        if(is_browsing())
        {
            HomeDir.clear();
            apply_dir(home.c_str(), windir);
            update_browsing_ui();
        }
        std::string rel=folder.size()>home.size() ? folder.substr(home.size()) : "";
        std::replace(rel.begin(), rel.end(), '\\', '/');
        while(!rel.empty() && rel[0]=='/')
            rel.erase(0, 1);
        if(!rel.empty() && rel[rel.size()-1]!='/')
            rel+='/';
        open_folder(rel);
        return;
    }
    //Elsewhere: browsed, in reading order (natural sort: Vol 2 before Vol 10)
    if(!is_browsing())
        HomeDir=abs_lib_dir;
    Sort_Choice->value(0);
    apply_dir(dir, windir);
    update_browsing_ui();
}

void Fl_Books_Library::stop_browsing()
{
    HomeDir.clear();
    update_browsing_ui();
}

//! "My library" shown left of the path bar while browsing
void Fl_Books_Library::update_browsing_ui()
{
    Fl_Group *grp=PathBar->parent();
    if(is_browsing())
    {
        Home_Button->resize(grp->x(), grp->y(), 96, 20);
        Home_Button->show();
        PathBar->resize(grp->x()+100, grp->y(), grp->w()-100, 20);
    }
    else
    {
        Home_Button->hide();
        PathBar->resize(grp->x(), grp->y(), grp->w(), 20);
    }
    grp->init_sizes();
    grp->redraw();
}

void Fl_Books_Library::Home_cb(Fl_Widget *w, void *p)
{
    Fl_Books_Library *bl=(Fl_Books_Library*)p;
    std::string home=bl->HomeDir;
    bl->HomeDir.clear();
    bl->apply_dir(home.c_str(), bl->windir);
    bl->update_browsing_ui();
}

void Fl_Books_Library::open_folder(const std::string &relative_dir)
{
    CurrentDir=relative_dir;
    refresh();
    update_path_bar();
}

//! Backspace: parent folder
int Fl_Books_Library::handle(int event)
{
    if((event==FL_SHORTCUT || event==FL_KEYBOARD) && Fl::event_key()==FL_BackSpace && visible_r() && !CurrentDir.empty()
       && !(Fl::focus() && dynamic_cast<Fl_Input_*>(Fl::focus())))
    {
        std::string parent=CurrentDir.substr(0, CurrentDir.size()-1);
        size_t slash=parent.rfind('/');
        open_folder(slash==std::string::npos ? "" : parent.substr(0, slash+1));
        return 1;
    }
    return Fl_Group::handle(event);
}

//! Tag filter choices: "Any" and the tags
void Fl_Books_Library::fill_tag_choice()
{
    int selected=(Tag_Choice->value()>0 && Tag_Choice->value()-1<(int)TagChoiceIds.size()) ? TagChoiceIds[Tag_Choice->value()-1] : -1;
    Tag_Choice->clear();
    TagChoiceIds.clear();
    Tag_Choice->add("Any");
    int value=0;
    const std::vector<TagDef> &all=Tags::All();
    for(size_t i=0; i<all.size(); i++)
    {
        //Menu special characters
        std::string label;
        for(size_t c=0; c<all[i].Name.size(); c++)
        {
            char ch=all[i].Name[c];
            if(ch=='/' || ch=='\\' || ch=='_')
                label+='\\';
            else if(ch=='&')
                label+='&';
            label+=ch;
        }
        Tag_Choice->add(label.c_str());
        TagChoiceIds.push_back(all[i].Id);
        if(all[i].Id==selected)
            value=(int)i+1;
    }
    Tag_Choice->value(value);
}

void Fl_Books_Library::refresh_tags()
{
    for(size_t i=0; i<elements.size(); i++)
        elements[i]->ReloadTags();
    int former=Tag_Choice->value();
    fill_tag_choice();
    //The list changes only if it is filtered by tags (the chips are redrawn anyway)
    if(former>0 || Search_Input->size()>0)
        refresh();
    else
        redraw();
}
