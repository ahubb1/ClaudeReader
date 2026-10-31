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
#include "Fl_Path_Bar.h"
#include <FL/fl_draw.H>
#include <FL/filename.H>
#include <FL/Fl_Menu_Item.H>
#include <string.h>

#define PATH_BAR_BROWSE_W 24    //Folder chooser button width
#define PATH_BAR_EMPTY_MIN 24   //Empty space always kept at the end (click to edit)
#define PATH_BAR_CHEVRON_W 16
#define PATH_BAR_FONT_SIZE 12

static const char *CHEVRON="\xe2\x80\xba";  //Single right-pointing angle quotation mark
static const char *OVERFLOW_MARK="\xc2\xab"; //Left-pointing double angle quotation mark


int Fl_Path_Input::handle(int event)
{
    if(event==FL_KEYBOARD && Fl::event_key()==FL_Escape)
    {
        Bar->EditMode(false);
        return 1;
    }
    int ret=Fl_Input::handle(event);
    if(event==FL_UNFOCUS)
        Bar->EditMode(false);
    return ret;
}


Fl_Path_Bar::Fl_Path_Bar(int X, int Y, int W, int H):
    Fl_Group(X, Y, W, H),
    HoverItem(-1),
    Editing(false)
{
    Input = new Fl_Path_Input(X+1, Y+1, W-PATH_BAR_BROWSE_W-1, H-2, this);
    Input->box(FL_FLAT_BOX);
    Input->color(THEME_INPUT_BG);
    Input->textsize(PATH_BAR_FONT_SIZE);
    Input->when(FL_WHEN_ENTER_KEY_ALWAYS);
    Input->hide();
    BrowseButton = new Fl_Activity_Button(X+W-PATH_BAR_BROWSE_W, Y+1, PATH_BAR_BROWSE_W-1, H-2, ICON_OPEN, 16);
    BrowseButton->color(THEME_INPUT_BG);
    BrowseButton->tooltip("Browse for a folder");
    end();
    resizable(Input);
    box(THEME_INPUT_BOX);
    color(THEME_INPUT_BG);
}

//! Split the path in folders and place them, hiding the first ones when there is not enough space
void Fl_Path_Bar::LayoutItems()
{
    Items.clear();

    //Folders of the path, with their full path
    std::string path=Input->value();
    for(size_t i=0; i<path.size(); i++)
        if(path[i]=='\\')
            path[i]='/';
    std::vector<Item> folders;
    size_t start=0;
    if(!path.empty() && path[0]=='/') //Unix root
    {
        folders.push_back(Item{0, 0, "/", "/", {}, ITEM_FOLDER});
        start=1;
    }
    while(start<path.size())
    {
        size_t end=path.find('/', start);
        if(end==std::string::npos)
            end=path.size();
        if(end>start)
            folders.push_back(Item{0, 0, path.substr(start, end-start), path.substr(0, end)+"/", {}, ITEM_FOLDER});
        start=end+1;
    }

    //Widths: a folder and its chevron
    fl_font(FL_HELVETICA, PATH_BAR_FONT_SIZE);
    std::vector<int> widths;
    int total=0;
    for(size_t i=0; i<folders.size(); i++)
    {
        widths.push_back((int)fl_width(folders[i].label.c_str())+12+PATH_BAR_CHEVRON_W);
        total+=widths.back();
    }

    //Hide the first folders if needed (always keep the last one)
    int available=w()-PATH_BAR_BROWSE_W-PATH_BAR_EMPTY_MIN-4;
    size_t first=0;
    std::vector<std::string> hidden;
    while(total>available && first+1<folders.size())
    {
        if(first==0)
            total+=24; //Overflow mark
        total-=widths[first];
        hidden.insert(hidden.begin(), folders[first].path);
        first++;
    }

    int cx=x()+3;
    if(first>0)
    {
        Items.push_back(Item{cx, 20, OVERFLOW_MARK, "", hidden, ITEM_OVERFLOW});
        cx+=24;
    }
    for(size_t i=first; i<folders.size(); i++)
    {
        int lw=widths[i]-PATH_BAR_CHEVRON_W;
        Items.push_back(Item{cx, lw, folders[i].label, folders[i].path, {}, ITEM_FOLDER});
        cx+=lw;
        Items.push_back(Item{cx, PATH_BAR_CHEVRON_W, CHEVRON, folders[i].path, {}, ITEM_CHEVRON});
        cx+=PATH_BAR_CHEVRON_W;
    }
}

int Fl_Path_Bar::ItemAt(int X)
{
    for(size_t i=0; i<Items.size(); i++)
        if(X>=Items[i].x && X<Items[i].x+Items[i].w)
            return (int)i;
    return -1;
}

void Fl_Path_Bar::draw()
{
    draw_box();
    if(!Editing)
    {
        LayoutItems();
        fl_push_clip(x()+1, y()+1, w()-PATH_BAR_BROWSE_W-1, h()-2);
        fl_font(FL_HELVETICA, PATH_BAR_FONT_SIZE);
        for(size_t i=0; i<Items.size(); i++)
        {
            const Item &it=Items[i];
            if((int)i==HoverItem)
            {
                fl_color(THEME_HOVER);
                fl_rectf(it.x, y()+2, it.w, h()-4);
            }
            if(it.kind==ITEM_FOLDER)
            {
                fl_color((int)i==HoverItem ? THEME_FG_BRIGHT : THEME_FG);
                fl_draw(it.label.c_str(), it.x+6, y(), it.w-6, h(), FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
            }
            else
            {
                fl_color((int)i==HoverItem ? THEME_FG_BRIGHT : THEME_FG_DIM);
                fl_draw(it.label.c_str(), it.x, y(), it.w, h(), FL_ALIGN_CENTER);
            }
        }
        fl_pop_clip();
    }
    draw_children();
}

int Fl_Path_Bar::handle(int event)
{
    //Children first: text field (edit mode) and folder chooser button
    if(Fl_Group::handle(event) && event!=FL_ENTER && event!=FL_MOVE)
        return 1;
    if(Editing)
        return 0;

    switch(event)
    {
        case FL_ENTER:
        case FL_MOVE:
        {
            int hover=Fl::event_inside(BrowseButton) ? -1 : ItemAt(Fl::event_x());
            if(hover!=HoverItem)
            {
                HoverItem=hover;
                redraw();
            }
            return 1;
        }
        case FL_LEAVE:
            HoverItem=-1;
            redraw();
            return 1;
        case FL_PUSH:
        {
            if(Fl::event_button()!=FL_LEFT_MOUSE)
                return 0;
            int i=ItemAt(Fl::event_x());
            if(i<0)
            {
                EditMode(true);
                return 1;
            }
            Item item=Items[i]; //Copy: the items can change while a menu is open
            switch(item.kind)
            {
                case ITEM_FOLDER:
                    Navigate(item.path);
                    break;
                case ITEM_CHEVRON:
                    PopupSubFolders(item);
                    break;
                case ITEM_OVERFLOW:
                    PopupHidden(item);
                    break;
            }
            return 1;
        }
        default:
            return 0;
    }
}

void Fl_Path_Bar::EditMode(bool on)
{
    if(on==Editing)
        return;
    Editing=on;
    HoverItem=-1;
    if(on)
    {
        Input->show();
        Input->take_focus();
        Input->insert_position(0, Input->size()); //Select everything
    }
    else
    {
        Input->hide();
    }
    redraw();
}

void Fl_Path_Bar::Navigate(const std::string &path)
{
    Input->value(path.c_str());
    Input->do_callback();
    redraw();
}

//! Escape the menu special characters of a folder name
static std::string MenuLabel(const std::string &name)
{
    std::string label;
    for(size_t i=0; i<name.size(); i++)
    {
        if(name[i]=='&')
            label+='&';
        label+=name[i];
    }
    return label;
}

//! Show the sub-folders of the chevron folder and go to the one chosen
void Fl_Path_Bar::PopupSubFolders(const Item &item)
{
    std::vector<std::string> names;
    dirent **listing;
    int nb=fl_filename_list(item.path.c_str(), &listing, fl_casenumericsort);
    for(int i=0; i<nb; i++)
    {
        std::string name=listing[i]->d_name;
        if(name.empty() || name[0]=='.' || name[name.size()-1]!='/') //Hidden, current/parent or not a folder
            continue;
        names.push_back(name.substr(0, name.size()-1));
    }
    if(nb>0)
        fl_filename_free_list(&listing, nb);

    std::vector<std::string> labels;
    for(size_t i=0; i<names.size(); i++)
        labels.push_back(MenuLabel(names[i]));
    std::vector<Fl_Menu_Item> menu(labels.size()+2);
    memset(menu.data(), 0, menu.size()*sizeof(Fl_Menu_Item));
    if(labels.empty())
    {
        menu[0].text="(No sub-folders)";
        menu[0].flags=FL_MENU_INACTIVE;
    }
    for(size_t i=0; i<labels.size(); i++)
        menu[i].text=labels[i].c_str();

    const Fl_Menu_Item *chosen=menu[0].popup(item.x, y()+h());
    if(chosen && !labels.empty())
        Navigate(item.path+names[chosen-&menu[0]]+"/");
}

//! Show the folders hidden by lack of space (deepest first) and go to the one chosen
void Fl_Path_Bar::PopupHidden(const Item &item)
{
    std::vector<std::string> labels;
    for(int i=(int)item.hidden.size()-1; i>=0; i--)
    {
        //Folder name: last part of its path ("C:/Users/" -> "Users")
        std::string name=item.hidden[i].substr(0, item.hidden[i].size()-1);
        size_t slash=name.rfind('/');
        if(slash!=std::string::npos && slash+1<name.size())
            name=name.substr(slash+1);
        labels.push_back(MenuLabel(name));
    }
    std::vector<Fl_Menu_Item> menu(labels.size()+1);
    memset(menu.data(), 0, menu.size()*sizeof(Fl_Menu_Item));
    for(size_t i=0; i<labels.size(); i++)
        menu[i].text=labels[i].c_str();

    const Fl_Menu_Item *chosen=menu[0].popup(item.x, y()+h());
    if(chosen)
        Navigate(item.hidden[item.hidden.size()-1-(chosen-&menu[0])]);
}
