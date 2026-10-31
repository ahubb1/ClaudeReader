//--------------------------------------------------------------------------
//
//    This file is part of ClaudeReader (based on BDReader, Copyright (C) 2014-2016  Vincent Crocher).
//
//    ClaudeReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ClaudeReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ClaudeReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
#include "SettingsView.h"
#include "Tags.h"
#include "Net.h"
#include "LuaSource.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <algorithm>

#define SETTINGS_PAD    32
#define SETTINGS_W      640     //Content column width
#define ROW_H           30

//! Flat button of the settings
static Fl_Button *SettingsButton(const char *label, bool accent)
{
    Fl_Button *b=new Fl_Button(0, 0, 120, 26, label);
    b->box(FL_FLAT_BOX);
    b->color(accent ? THEME_ACCENT : THEME_INPUT_BG, accent ? fl_rgb_color(2, 106, 192) : THEME_HOVER);
    b->labelcolor(accent ? FL_WHITE : THEME_FG);
    b->labelsize(12);
    b->clear_visible_focus();
    return b;
}

static Fl_Box *Text(int size, Fl_Color color, bool bold=false)
{
    Fl_Box *b=new Fl_Box(0, 0, 10, 10);
    b->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP|FL_ALIGN_CLIP);
    b->labelsize(size);
    b->labelcolor(color);
    if(bold)
        b->labelfont(FL_HELVETICA_BOLD);
    return b;
}

//! Color swatch of a tag: a round dot, click to change the color
class Fl_Tag_Swatch : public Fl_Button
{
    public:
        Fl_Tag_Swatch(int id):Fl_Button(0, 0, 26, 26), Id(id) { box(FL_FLAT_BOX); clear_visible_focus(); }
        void draw()
        {
            fl_color(THEME_EDITOR_BG);
            fl_rectf(x(), y(), w(), h());
            const TagDef *tag=Tags::Get(Id);
            fl_color(tag ? Tags::Color(tag->Color) : THEME_FG_DIM);
            fl_pie(x()+(w()-14)/2, y()+(h()-14)/2, 14, 14, 0, 360);
        }
        int Id;
};

//! Name of a tag: renamed with Enter or when leaving the field
class Fl_Tag_Name : public Fl_Input
{
    public:
        Fl_Tag_Name(int id):Fl_Input(0, 0, 200, 26), Id(id) {}
        int Id;
};

Fl_Settings_View::Fl_Settings_View(int X, int Y, int W, int H):Fl_Group(X, Y, W, H)
{
    box(FL_FLAT_BOX);
    color(THEME_EDITOR_BG);

    Scroll=new Fl_Scroll(X, Y, W, H);
        Content=new Fl_Group(X, Y, W, H);
            Title=Text(26, THEME_FG_BRIGHT);
            Title->label("Settings");

            //Tags
            TagsTitle=Section("Tags", "Tags organize your comics: right-click a comic in the library to tag it, "
                              "filter the library with the Tag choice or search #tag.");
            NoTags=Text(12, THEME_FG_DIM);
            NoTags->label("No tag yet.");
            NewTagInput=new Fl_Hint_Input(0, 0, 200, 26);
            NewTagInput->Hint="New tag name";
            NewTagInput->box(THEME_INPUT_BOX);
            NewTagInput->color(THEME_INPUT_BG);
            NewTagInput->textcolor(THEME_FG);
            NewTagInput->textsize(13);
            NewTagInput->when(FL_WHEN_ENTER_KEY_ALWAYS);
            NewTagInput->callback(TagAdd_cb, this);
            NewTagButton=SettingsButton("Add tag", true);
            NewTagButton->callback(TagAdd_cb, this);

            //Library
            LibraryTitle=Section("Library", "The folder of your comics (its sub-folders are shown as folders in the library). "
                                 "The store downloads go there too, in Store.");
            LibraryPath=Text(13, THEME_FG);
            LibraryButton=SettingsButton("Change...", false);
            LibraryButton->callback(LibraryFolder_cb, this);
            UseLibraryCheck=new Fl_Check_Button(0, 0, 400, 26, " Open the library when ClaudeReader starts");
            UseLibraryCheck->labelsize(13);
            UseLibraryCheck->labelcolor(THEME_FG);
            UseLibraryCheck->selection_color(THEME_ACCENT);
            UseLibraryCheck->clear_visible_focus();
            UseLibraryCheck->callback(UseLibrary_cb, this);

            //Store
            StoreTitle=Section("Store", "Covers, pages and searches are kept in a cache to read and download again faster.");
            CacheInfo=Text(13, THEME_FG);
            ClearCacheButton=SettingsButton("Clear cache", false);
            ClearCacheButton->callback(ClearCache_cb, this);
            SourcesInfo=Text(13, THEME_FG);
            SourcesInfo->label("Your own stores: Lua scripts in the stores folder (see its README.txt).");
            SourcesButton=SettingsButton("Open folder", false);
            SourcesButton->callback(SourcesFolder_cb, this);
        Content->end();
        Content->resizable(NULL);
    Scroll->end();
    Scroll->type(Fl_Scroll::VERTICAL);
    Scroll->box(FL_FLAT_BOX);
    Scroll->color(THEME_EDITOR_BG);
    Scroll->scrollbar.color(THEME_EDITOR_BG, fl_rgb_color(79, 79, 79));
    Scroll->scrollbar.box(FL_FLAT_BOX);
    end();
    resizable(Scroll);
}

//! Section title (and its description, returned in TagsHelp... through the next widget)
Fl_Box *Fl_Settings_View::Section(const char *title, const char *description)
{
    Fl_Box *t=Text(11, THEME_FG, true);
    std::string upper=title;
    for(size_t i=0; i<upper.size(); i++)
        upper[i]=(char)toupper((unsigned char)upper[i]);
    t->copy_label(upper.c_str());
    Fl_Box *d=Text(12, THEME_FG_DIM);
    d->copy_label(description);
    //The description follows its title in the children (see Layout)
    return t;
}

void Fl_Settings_View::resize(int X, int Y, int W, int H)
{
    Fl_Group::resize(X, Y, W, H);
    Layout();
}

void Fl_Settings_View::Refresh()
{
    BuildTagRows();
    if(GetLibraryFolder)
        LibraryText=GetLibraryFolder();
    LibraryPath->copy_label(LibraryText.c_str());
    if(GetUseLibrary)
        UseLibraryCheck->value(GetUseLibrary());
    char size[64];
    snprintf(size, sizeof(size), "Cache: %.1f MB", Net::CacheSize()/1048576.);
    CacheInfo->copy_label(size);
    Layout();
    redraw();
}

void Fl_Settings_View::BuildTagRows()
{
    for(size_t i=0; i<TagRows.size(); i++)
    {
        Content->remove(TagRows[i]);
        Fl::delete_widget(TagRows[i]);
    }
    TagRows.clear();

    const std::vector<TagDef> &all=Tags::All();
    Content->begin();
    for(size_t i=0; i<all.size(); i++)
    {
        Fl_Tag_Swatch *swatch=new Fl_Tag_Swatch(all[i].Id);
        swatch->tooltip("Click to change the color");
        swatch->callback(TagColor_cb, this);
        Fl_Tag_Name *name=new Fl_Tag_Name(all[i].Id);
        name->value(all[i].Name.c_str());
        name->box(THEME_INPUT_BOX);
        name->color(THEME_INPUT_BG);
        name->textcolor(THEME_FG);
        name->textsize(13);
        name->when(FL_WHEN_ENTER_KEY|FL_WHEN_RELEASE);
        name->tooltip("Rename: type and press Enter");
        name->callback(TagName_cb, this);
        Fl_Button *remove=SettingsButton("Delete", false);
        remove->argument(all[i].Id);
        remove->callback(TagDelete_cb, this);
        Fl_Box *spacer=Text(12, THEME_FG_DIM); //Keeps 4 widgets per row
        TagRows.push_back(swatch);
        TagRows.push_back(name);
        TagRows.push_back(remove);
        TagRows.push_back(spacer);
    }
    Content->end();
}

//! One column, VSCodium settings like: section title, description, controls
void Fl_Settings_View::Layout()
{
    int cx=x()+SETTINGS_PAD, cw=std::min(SETTINGS_W, w()-2*SETTINGS_PAD-Fl::scrollbar_size());
    int cy=y()+SETTINGS_PAD-Scroll->yposition();
    Content->resize(x(), y()-Scroll->yposition(), w()-Fl::scrollbar_size(), Content->h());

    Title->resize(cx, cy, cw, 40);
    cy+=60;

    //Section title and description: the description is the next child
    auto section=[&](Fl_Box *title)
    {
        title->resize(cx, cy, cw, 20);
        Fl_Widget *description=Content->child(Content->find(title)+1);
        description->resize(cx, cy+22, cw, 36);
        cy+=64;
    };

    section(TagsTitle);
    if(TagRows.empty())
    {
        NoTags->show();
        NoTags->resize(cx, cy, cw, 22);
        cy+=32;
    }
    else
        NoTags->hide();
    for(size_t i=0; i+3<TagRows.size(); i+=4)
    {
        TagRows[i]->resize(cx, cy, 26, 26);                 //Color
        TagRows[i+1]->resize(cx+34, cy, 260, 26);           //Name
        TagRows[i+2]->resize(cx+302, cy, 80, 26);           //Delete
        TagRows[i+3]->resize(cx+392, cy, cw-392, 26);       //Spacer
        cy+=ROW_H+4;
    }
    cy+=6;
    NewTagInput->resize(cx+34, cy, 260, 26);
    NewTagButton->resize(cx+302, cy, 80, 26);
    cy+=70;

    section(LibraryTitle);
    LibraryPath->resize(cx, cy, cw-130, 26);
    LibraryButton->resize(cx+cw-120, cy, 120, 26);
    cy+=36;
    UseLibraryCheck->resize(cx, cy, cw, 26);
    cy+=76;

    section(StoreTitle);
    CacheInfo->resize(cx, cy, cw-130, 26);
    ClearCacheButton->resize(cx+cw-120, cy, 120, 26);
    cy+=36;
    SourcesInfo->resize(cx, cy, cw-130, 26);
    SourcesButton->resize(cx+cw-120, cy, 120, 26);
    cy+=36+SETTINGS_PAD;

    Content->size(Content->w(), cy-Content->y());
    Scroll->redraw();
}


//------------------------------------Callbacks------------------------

void Fl_Settings_View::TagColor_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    int id=((Fl_Tag_Swatch*)w)->Id;
    const TagDef *tag=Tags::Get(id);
    if(!tag)
        return;
    Tags::SetColor(id, tag->Color+1);
    w->redraw();
    if(v->OnTagsChanged)
        v->OnTagsChanged();
}

void Fl_Settings_View::TagName_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    Fl_Tag_Name *name=(Fl_Tag_Name*)w;
    const TagDef *tag=Tags::Get(name->Id);
    if(!tag || tag->Name==name->value())
        return;
    int existing=Tags::Find(name->value());
    if(existing>0 && existing!=name->Id)
    {
        fl_alert("A tag named \"%s\" already exists.", name->value());
        name->value(tag->Name.c_str());
        return;
    }
    Tags::Rename(name->Id, name->value());
    if(v->OnTagsChanged)
        v->OnTagsChanged();
    //Sorted again (not now: the field is in use)
    Fl::add_timeout(0.0, [](void *view) { ((Fl_Settings_View*)view)->Refresh(); }, v);
}

void Fl_Settings_View::TagDelete_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    const TagDef *tag=Tags::Get((int)w->argument());
    if(!tag)
        return;
    if(fl_choice("Delete the tag \"%s\"?\nThe comics keep everything else.", "Cancel", "Delete", NULL, tag->Name.c_str())!=1)
        return;
    Tags::Remove(tag->Id);
    if(v->OnTagsChanged)
        v->OnTagsChanged();
    Fl::add_timeout(0.0, [](void *view) { ((Fl_Settings_View*)view)->Refresh(); }, v);
}

void Fl_Settings_View::TagAdd_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    if(v->NewTagInput->size()==0)
        return;
    if(Tags::Create(v->NewTagInput->value())<=0)
        return;
    v->NewTagInput->value("");
    if(v->OnTagsChanged)
        v->OnTagsChanged();
    v->Refresh();
}

void Fl_Settings_View::LibraryFolder_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    Fl_Native_File_Chooser chooser;
    chooser.title("Folder of your comics");
    chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    chooser.directory(v->LibraryText.c_str());
    if(chooser.show()!=0)
        return;
    if(v->OnLibraryFolder)
        v->OnLibraryFolder(chooser.filename());
    v->Refresh();
}

void Fl_Settings_View::UseLibrary_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    if(v->OnUseLibrary)
        v->OnUseLibrary(v->UseLibraryCheck->value()!=0);
}

void Fl_Settings_View::ClearCache_cb(Fl_Widget *w, void *p)
{
    Net::ClearCache();
    ((Fl_Settings_View*)p)->Refresh();
}

void Fl_Settings_View::SourcesFolder_cb(Fl_Widget *w, void *p)
{
    std::string uri="file:///"+UserSourcesDir();
    for(size_t i=0; i<uri.size(); i++)
        if(uri[i]=='\\')
            uri[i]='/';
    fl_open_uri(uri.c_str());
}
