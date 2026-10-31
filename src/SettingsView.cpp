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
#include "Async.h"
#include "FileUtf8.h"
#include <thread>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Color_Chooser.H>
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

//! Row of a widget in a list of rebuilt rows (per_row widgets each), -1 if not found.
//! (Not argument(): it is the user data of the callback, replaced by callback(cb, this))
static int RowOf(const std::vector<Fl_Widget*> &rows, Fl_Widget *w, int per_row)
{
    for(size_t i=0; i<rows.size(); i++)
        if(rows[i]==w)
            return (int)i/per_row;
    return -1;
}

//! Choice styled as the inputs
static Fl_Choice *SettingsChoice()
{
    Fl_Choice *c=new Fl_Choice(0, 0, 160, 26);
    c->box(THEME_INPUT_BOX);
    c->color(THEME_INPUT_BG);
    c->textcolor(THEME_FG);
    c->textsize(13);
    c->down_box(FL_FLAT_BOX);
    c->clear_visible_focus();
    return c;
}

static Fl_Color RgbColor(unsigned int rgb)
{
    return fl_rgb_color((rgb>>16)&0xFF, (rgb>>8)&0xFF, rgb&0xFF);
}

//! Highlight color choice: a round dot, ringed when it is the current color
class Fl_Color_Swatch : public Fl_Button
{
    public:
        Fl_Color_Swatch(unsigned int rgb):Fl_Button(0, 0, 24, 26), Rgb(rgb), Selected(false) { box(FL_FLAT_BOX); clear_visible_focus(); }
        void draw()
        {
            fl_color(THEME_EDITOR_BG);
            fl_rectf(x(), y(), w(), h());
            int cx=x()+w()/2, cy=y()+h()/2;
            if(Selected)
            {
                fl_color(THEME_FG_BRIGHT);
                fl_pie(cx-10, cy-10, 20, 20, 0, 360);
                fl_color(THEME_EDITOR_BG);
                fl_pie(cx-8, cy-8, 16, 16, 0, 360);
            }
            fl_color(RgbColor(Rgb));
            fl_pie(cx-7, cy-7, 14, 14, 0, 360);
            //Dark colors stay visible on the dark background
            fl_color(THEME_INPUT_BORDER);
            fl_arc(cx-7, cy-7, 14, 14, 0, 360);
        }
        unsigned int Rgb;
        bool Selected;
};

//! A small page with three panels, the second one highlighted as in the reader
class Fl_Highlight_Preview : public Fl_Widget
{
    public:
        Fl_Highlight_Preview(const PanelHighlight *h):Fl_Widget(0, 0, 220, 150), Highlight(h) {}
        void draw()
        {
            fl_color(THEME_INPUT_BG);
            fl_rectf(x(), y(), w(), h());
            bool dim=Highlight->Style==HIGHLIGHT_DIM;
            //Page and panels: darkened but the current one when dimming
            auto shade=[dim](uchar v, bool current) { return (uchar)(dim && !current ? v*0.35 : v); };
            int px=x()+(w()-100)/2, py=y()+10, pw=100, ph=h()-20;
            fl_color(fl_rgb_color(shade(240, false), shade(238, false), shade(232, false)));
            fl_rectf(px, py, pw, ph);
            int m=6, half=(pw-3*m)/2;
            int top_h=(ph-3*m)/2;
            int panels[3][4]={{px+m, py+m, pw-2*m, top_h}, {px+m, py+2*m+top_h, half, ph-3*m-top_h}, {px+2*m+half, py+2*m+top_h, half, ph-3*m-top_h}};
            for(int i=0; i<3; i++)
            {
                bool current=i==1;
                fl_color(fl_rgb_color(shade(150, current), shade(170, current), shade(195, current)));
                fl_rectf(panels[i][0], panels[i][1], panels[i][2], panels[i][3]);
            }
            //The highlight, as the viewer draws it
            int t=std::max(1, std::min(Highlight->Thickness, HIGHLIGHT_MAX_THICKNESS));
            int o=t/2+1;
            int ax=panels[1][0]-o, ay=panels[1][1]-o, aw=panels[1][2]+2*o, ah=panels[1][3]+2*o;
            if(Highlight->Style==HIGHLIGHT_FRAME)
            {
                fl_color(fl_rgb_color(24, 24, 24));
                fl_line_style(FL_SOLID|FL_CAP_SQUARE|FL_JOIN_MITER, 9);
                fl_rect(ax-o-4, ay-o-4, aw+2*o+8, ah+2*o+8);
            }
            fl_color(RgbColor(Highlight->Color));
            fl_line_style(FL_SOLID|FL_CAP_SQUARE|FL_JOIN_MITER, t);
            if(Highlight->Style==HIGHLIGHT_CORNERS)
            {
                int l=std::max(8, std::min(aw, ah)/5);
                fl_line(ax, ay+l, ax, ay, ax+l, ay);
                fl_line(ax+aw-l, ay, ax+aw, ay, ax+aw, ay+l);
                fl_line(ax+aw, ay+ah-l, ax+aw, ay+ah, ax+aw-l, ay+ah);
                fl_line(ax+l, ay+ah, ax, ay+ah, ax, ay+ah-l);
            }
            else
                fl_rect(ax, ay, aw, ah);
            fl_line_style(0);
        }
        const PanelHighlight *Highlight;
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

            //Panel highlight
            HighlightTitle=Section("Panel highlight", "How the panel being read is shown in panel mode. "
                                   "Ctrl+B shows or hides it while reading.");
            StyleLabel=Text(13, THEME_FG);
            StyleLabel->label("Style");
            StyleChoice=SettingsChoice();
            for(int i=0; i<HIGHLIGHT_STYLES; i++)
                StyleChoice->add(PanelHighlight::StyleName(i));
            StyleChoice->callback(Highlight_cb, this);
            ThicknessLabel=Text(13, THEME_FG);
            ThicknessLabel->label("Thickness");
            ThicknessChoice=SettingsChoice();
            for(int i=1; i<=HIGHLIGHT_MAX_THICKNESS; i++)
                ThicknessChoice->add((std::to_string(i)+" px").c_str());
            ThicknessChoice->callback(Highlight_cb, this);
            ColorLabel=Text(13, THEME_FG);
            ColorLabel->label("Color");
            {
                //Accent, the tags palette, white and black
                std::vector<unsigned int> colors={0x0078D4};
                for(int i=0; i<Tags::PaletteSize(); i++)
                {
                    uchar r, g, b;
                    Fl::get_color(Tags::Color(i), r, g, b);
                    colors.push_back((r<<16)|(g<<8)|b);
                }
                colors.push_back(0xFFFFFF);
                colors.push_back(0x000000);
                for(size_t i=0; i<colors.size(); i++)
                {
                    Fl_Color_Swatch *s=new Fl_Color_Swatch(colors[i]);
                    s->callback(HighlightColor_cb, this);
                    ColorSwatches.push_back(s);
                }
            }
            CustomColorButton=SettingsButton("Custom...", false);
            CustomColorButton->tooltip("Any color");
            CustomColorButton->callback(HighlightCustom_cb, this);
            HighlightPreview=new Fl_Highlight_Preview(&Highlight);

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
            StoreTitle=Section("Stores", "Each store is a tab of the Store view. Add stores from the catalog, "
                               "from a .lua file or from a link to one.");
            StoresLabel=Text(12, THEME_FG, true);
            StoresLabel->label("Installed");
            AddFileButton=SettingsButton("Add from file...", false);
            AddFileButton->tooltip("Install a store script (.lua) from your computer");
            AddFileButton->callback(StoreAddFile_cb, this);
            AddLinkButton=SettingsButton("Add from link...", false);
            AddLinkButton->tooltip("Install a store script from its web address");
            AddLinkButton->callback(StoreAddLink_cb, this);
            SourcesButton=SettingsButton("Open folder", false);
            SourcesButton->tooltip("The folder of your stores (see its README.txt to write one)");
            SourcesButton->callback(SourcesFolder_cb, this);
            StoresStatus=Text(12, THEME_FG_DIM);
            CatalogLabel=Text(12, THEME_FG, true);
            CatalogLabel->label("Catalog");
            CatalogButton=SettingsButton("Browse catalog", true);
            CatalogButton->tooltip("Stores ready to install, from the ClaudeReader repository");
            CatalogButton->callback(Catalog_cb, this);
            CatalogStatus=Text(12, THEME_FG_DIM);
            CacheInfo=Text(13, THEME_FG);
            ClearCacheButton=SettingsButton("Clear cache", false);
            ClearCacheButton->callback(ClearCache_cb, this);
            CatalogShown=false;
            Installing=false;
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
    if(GetHighlight)
        Highlight=GetHighlight();
    StyleChoice->value(std::max(0, std::min(Highlight.Style, HIGHLIGHT_STYLES-1)));
    ThicknessChoice->value(std::max(1, std::min(Highlight.Thickness, HIGHLIGHT_MAX_THICKNESS))-1);
    for(size_t i=0; i<ColorSwatches.size(); i++)
        ((Fl_Color_Swatch*)ColorSwatches[i])->Selected=((Fl_Color_Swatch*)ColorSwatches[i])->Rgb==Highlight.Color;
    if(GetLibraryFolder)
        LibraryText=GetLibraryFolder();
    LibraryPath->copy_label(LibraryText.c_str());
    if(GetUseLibrary)
        UseLibraryCheck->value(GetUseLibrary());
    char size[64];
    snprintf(size, sizeof(size), "Cache of the covers, pages and searches: %.1f MB", Net::CacheSize()/1048576.);
    CacheInfo->copy_label(size);
    BuildStoreRows();
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

    section(HighlightTitle);
    {
        int rows_y=cy;
        StyleLabel->resize(cx, cy, 70, 26);
        StyleChoice->resize(cx+76, cy, 150, 26);
        cy+=ROW_H+6;
        ThicknessLabel->resize(cx, cy, 70, 26);
        ThicknessChoice->resize(cx+76, cy, 150, 26);
        cy+=ROW_H+6;
        ColorLabel->resize(cx, cy, 70, 26);
        int sx=cx+76;
        for(size_t i=0; i<ColorSwatches.size(); i++)
        {
            //Two rows of swatches
            if(i==6)
            {
                sx=cx+76;
                cy+=28;
            }
            ColorSwatches[i]->resize(sx, cy, 24, 26);
            sx+=26;
        }
        CustomColorButton->resize(sx+8, cy, 80, 26);
        cy+=ROW_H+6;
        //The preview at the right of the controls
        HighlightPreview->resize(cx+std::max(330, cw-220), rows_y, 220, std::max(150, cy-rows_y-6));
        cy=std::max(cy, rows_y+156);
    }
    cy+=40;

    section(LibraryTitle);
    LibraryPath->resize(cx, cy, cw-130, 26);
    LibraryButton->resize(cx+cw-120, cy, 120, 26);
    cy+=36;
    UseLibraryCheck->resize(cx, cy, cw, 26);
    cy+=76;

    section(StoreTitle);
    StoresLabel->resize(cx, cy, cw, 20);
    cy+=24;
    for(size_t i=0; i+2<StoreRows.size(); i+=3)
    {
        StoreRows[i]->resize(cx, cy, 220, 26);              //Name
        StoreRows[i+1]->resize(cx+228, cy, cw-228-90, 26);  //Version and origin
        StoreRows[i+2]->resize(cx+cw-80, cy, 80, 26);       //Remove
        cy+=ROW_H;
    }
    cy+=8;
    AddFileButton->resize(cx, cy, 130, 26);
    AddLinkButton->resize(cx+138, cy, 130, 26);
    SourcesButton->resize(cx+276, cy, 110, 26);
    cy+=32;
    StoresStatus->resize(cx, cy, cw, 22);
    cy+=34;

    CatalogLabel->resize(cx, cy, cw-140, 26);
    CatalogButton->resize(cx+cw-130, cy, 130, 26);
    cy+=32;
    CatalogStatus->resize(cx, cy, cw, CatalogStatus->label() && *CatalogStatus->label() ? 22 : 0);
    cy+=CatalogStatus->h() ? 26 : 0;
    for(size_t i=0; i+3<CatalogRows.size(); i+=4)
    {
        CatalogRows[i]->resize(cx, cy, 160, 26);                    //Name
        CatalogRows[i+1]->resize(cx+164, cy, 60, 26);               //Version
        CatalogRows[i+2]->resize(cx+228, cy, cw-228-100, 26);       //Description
        CatalogRows[i+3]->resize(cx+cw-90, cy, 90, 26);             //Install
        cy+=ROW_H+2;
    }
    cy+=24;

    CacheInfo->resize(cx, cy, cw-130, 26);
    ClearCacheButton->resize(cx+cw-120, cy, 120, 26);
    cy+=36+SETTINGS_PAD;

    Content->size(Content->w(), cy-Content->y());
    Scroll->redraw();
}


//------------------------------------Callbacks------------------------

void Fl_Settings_View::HighlightChanged()
{
    for(size_t i=0; i<ColorSwatches.size(); i++)
    {
        Fl_Color_Swatch *s=(Fl_Color_Swatch*)ColorSwatches[i];
        s->Selected=s->Rgb==Highlight.Color;
        s->redraw();
    }
    HighlightPreview->redraw();
    if(OnHighlight)
        OnHighlight(Highlight);
}

void Fl_Settings_View::Highlight_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    v->Highlight.Style=v->StyleChoice->value();
    v->Highlight.Thickness=v->ThicknessChoice->value()+1;
    v->HighlightChanged();
}

void Fl_Settings_View::HighlightColor_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    v->Highlight.Color=((Fl_Color_Swatch*)w)->Rgb;
    v->HighlightChanged();
}

void Fl_Settings_View::HighlightCustom_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    uchar r=(v->Highlight.Color>>16)&0xFF, g=(v->Highlight.Color>>8)&0xFF, b=v->Highlight.Color&0xFF;
    if(fl_color_chooser("Panel highlight color", r, g, b, 1))
    {
        v->Highlight.Color=(r<<16)|(g<<8)|b;
        v->HighlightChanged();
    }
}

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
        (CenterNextMessage(), fl_alert)("A tag named \"%s\" already exists.", name->value());
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
    int row=RowOf(v->TagRows, w, 4);
    const TagDef *tag=row<0 ? NULL : Tags::Get(((Fl_Tag_Swatch*)v->TagRows[row*4])->Id);
    if(!tag)
        return;
    if((CenterNextMessage(), fl_choice)("Delete the tag \"%s\"?\nThe comics keep everything else.", "Cancel", "Delete", NULL, tag->Name.c_str())!=1)
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

void Fl_Settings_View::SetStoresStatus(const std::string &text, bool error)
{
    StoresStatus->copy_label(text.c_str());
    StoresStatus->labelcolor(error ? fl_rgb_color(241, 76, 76) : THEME_FG_DIM);
    StoresStatus->redraw();
}

//! The installed stores: name, version and origin, Remove for the user ones
void Fl_Settings_View::BuildStoreRows()
{
    for(size_t i=0; i<StoreRows.size(); i++)
    {
        Content->remove(StoreRows[i]);
        Fl::delete_widget(StoreRows[i]);
    }
    StoreRows.clear();
    std::vector<std::string> errors;
    Stores=ListStores(&errors);
    Content->begin();
    for(size_t i=0; i<Stores.size(); i++)
    {
        const InstalledStore &s=Stores[i];
        Fl_Box *name=Text(13, THEME_FG);
        name->copy_label(s.Name.c_str());
        std::string info=(s.Version.empty() ? std::string("") : "version "+s.Version+"   \xc2\xb7   ")+
                         (s.BuiltIn ? "built-in" : s.ReplacesBuiltIn ? "your version of a built-in store" : "added by you");
        Fl_Box *details=Text(12, THEME_FG_DIM);
        details->copy_label(info.c_str());
        Fl_Widget *action;
        if(s.BuiltIn)
            action=Text(12, THEME_FG_DIM);
        else
        {
            Fl_Button *remove=SettingsButton(s.ReplacesBuiltIn ? "Restore" : "Remove", false);
            remove->tooltip(s.ReplacesBuiltIn ? "Remove your version: back to the built-in store" : "Remove this store");
            remove->callback(StoreRemove_cb, this);
            action=remove;
        }
        StoreRows.push_back(name);
        StoreRows.push_back(details);
        StoreRows.push_back(action);
    }
    Content->end();
    if(!errors.empty())
    {
        std::string text="Not loaded: "+errors[0];
        if(errors.size()>1)
            text+=" (and "+std::to_string(errors.size()-1)+" more)";
        SetStoresStatus(text, true);
    }
}

//! The stores of the catalog: Install, Update or Installed
void Fl_Settings_View::BuildCatalogRows()
{
    for(size_t i=0; i<CatalogRows.size(); i++)
    {
        Content->remove(CatalogRows[i]);
        Fl::delete_widget(CatalogRows[i]);
    }
    CatalogRows.clear();
    Content->begin();
    for(size_t i=0; i<Catalog.size(); i++)
    {
        const StoreCatalogEntry &c=Catalog[i];
        const InstalledStore *installed=NULL;
        for(size_t k=0; k<Stores.size(); k++)
            if(Stores[k].Id==c.Id)
                installed=&Stores[k];
        Fl_Box *name=Text(13, THEME_FG);
        name->copy_label(c.Name.empty() ? c.Id.c_str() : c.Name.c_str());
        Fl_Box *version=Text(12, THEME_FG_DIM);
        version->copy_label(c.Version.c_str());
        Fl_Box *description=Text(12, THEME_FG_DIM);
        description->copy_label(c.Description.c_str());
        description->tooltip(description->label());
        Fl_Widget *action;
        bool update=installed && !installed->BuiltIn && CompareVersions(installed->Version, c.Version)<0;
        if(installed && !update && !installed->BuiltIn)
        {
            Fl_Box *done=Text(12, THEME_FG_DIM);
            done->label("\xe2\x9c\x93 Installed");
            action=done;
        }
        else
        {
            Fl_Button *install=SettingsButton(update ? "Update" : installed ? "Replace" : "Install", !installed || update);
            install->tooltip(update ? "A newer version is available" : installed ? "Use this version instead of the built-in store" : "Add this store");
            install->callback(CatalogInstall_cb, this);
            action=install;
        }
        CatalogRows.push_back(name);
        CatalogRows.push_back(version);
        CatalogRows.push_back(description);
        CatalogRows.push_back(action);
    }
    Content->end();
}

void Fl_Settings_View::InstallScript(const std::string &script, const std::string &from)
{
    if(Installing)
        return;
    Installing=true;
    SetStoresStatus("Checking the store from "+from+"...");
    //The script runs while it is checked: in a thread (its time and memory are limited)
    std::thread([this, script]()
    {
        InstalledStore installed;
        bool replaced=false;
        std::string error;
        bool ok=InstallStoreScript(script, &installed, &replaced, &error);
        RunInUiThread([this, ok, installed, replaced, error]()
        {
            Installing=false;
            if(!ok)
            {
                SetStoresStatus(error, true);
                return;
            }
            SetStoresStatus((replaced ? "Updated: " : "Installed: ")+installed.Name+
                            (installed.Version.empty() ? "" : " "+installed.Version)+". It is a tab of the Store view.");
            BuildStoreRows();
            if(CatalogShown)
                BuildCatalogRows();
            Layout();
            redraw();
            if(OnStoresChanged)
                OnStoresChanged();
        });
    }).detach();
}

void Fl_Settings_View::InstallFromUrl(const std::string &url)
{
    if(Installing)
        return;
    Installing=true;
    SetStoresStatus("Downloading "+url+"...");
    std::thread([this, url]()
    {
        NetResponse r=Net::Get(url);
        RunInUiThread([this, url, r]()
        {
            Installing=false;
            if(!r.Ok())
                SetStoresStatus("Unable to download "+url+": "+r.Error, true);
            else
                InstallScript(r.Body, url);
        });
    }).detach();
}

void Fl_Settings_View::StoreRemove_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    int index=RowOf(v->StoreRows, w, 3);
    if(index<0 || index>=(int)v->Stores.size())
        return;
    InstalledStore store=v->Stores[index];
    if((CenterNextMessage(), fl_choice)("Remove the store \"%s\"?\n%s", "Cancel", "Remove", NULL, store.Name.c_str(),
                 store.ReplacesBuiltIn ? "The built-in version is used again." : "Its file is deleted.")!=1)
        return;
    if(!FileUtf8::Remove(store.Origin))
    {
        v->SetStoresStatus("Unable to delete "+store.Origin, true);
        return;
    }
    v->SetStoresStatus("Removed: "+store.Name);
    v->BuildStoreRows();
    if(v->CatalogShown)
        v->BuildCatalogRows();
    v->Layout();
    v->redraw();
    if(v->OnStoresChanged)
        v->OnStoresChanged();
}

void Fl_Settings_View::StoreAddFile_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    Fl_Native_File_Chooser chooser;
    chooser.title("Add a store");
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    chooser.filter("Store scripts\t*.lua");
    if(chooser.show()!=0 || !chooser.filename())
        return;
    std::string path=chooser.filename(), script;
    if(!FileUtf8::Read(path, &script))
    {
        v->SetStoresStatus("Unable to read "+path, true);
        return;
    }
    v->InstallScript(script, fl_filename_name(path.c_str()));
}

void Fl_Settings_View::StoreAddLink_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    CenterNextMessage();
    const char *url=fl_input("Address of the store script (.lua):", "https://");
    if(!url)
        return;
    std::string address=url;
    address.erase(0, address.find_first_not_of(" \t"));
    address.erase(address.find_last_not_of(" \t")+1);
    if(address.compare(0, 8, "https://")!=0 && address.compare(0, 7, "http://")!=0)
    {
        v->SetStoresStatus("The address must start with https:// (or http://)", true);
        return;
    }
    //GitHub page of a file: its raw content
    const std::string blob="https://github.com/";
    size_t b=address.find("/blob/");
    if(address.compare(0, blob.size(), blob)==0 && b!=std::string::npos)
        address="https://raw.githubusercontent.com/"+address.substr(blob.size(), b-blob.size())+"/"+address.substr(b+6);
    v->InstallFromUrl(address);
}

void Fl_Settings_View::Catalog_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    v->CatalogShown=true;
    v->CatalogButton->label("Refresh catalog");
    v->CatalogStatus->label("Loading the catalog...");
    v->Layout();
    v->redraw();
    std::thread([v]()
    {
        std::string url=StoreCatalogUrl();
        NetResponse r=Net::Get(url);
        std::vector<StoreCatalogEntry> entries;
        std::string error;
        bool ok=r.Ok() && ParseStoreCatalog(r.Body, url, &entries, &error);
        if(!r.Ok())
            error="Unable to load the catalog: "+r.Error;
        RunInUiThread([v, ok, entries, error]()
        {
            v->Catalog=entries;
            if(!ok)
                v->CatalogStatus->copy_label(error.c_str());
            else
                v->CatalogStatus->copy_label(entries.empty() ? "The catalog is empty." : "");
            v->BuildCatalogRows();
            v->Layout();
            v->redraw();
        });
    }).detach();
}

void Fl_Settings_View::CatalogInstall_cb(Fl_Widget *w, void *p)
{
    Fl_Settings_View *v=(Fl_Settings_View*)p;
    int index=RowOf(v->CatalogRows, w, 4);
    if(index>=0 && index<(int)v->Catalog.size())
        v->InstallFromUrl(v->Catalog[index].Url);
}

void Fl_Settings_View::SourcesFolder_cb(Fl_Widget *w, void *p)
{
    std::string uri="file:///"+UserSourcesDir();
    for(size_t i=0; i<uri.size(); i++)
        if(uri[i]=='\\')
            uri[i]='/';
    fl_open_uri(uri.c_str());
}
