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
#include "ReaderBar.h"
#include "Page.h" //READINGMODE
#include <FL/fl_draw.H>

//------------------------------------Tool button------------------------

Fl_Tool_Button::Fl_Tool_Button(int X, int Y, int W, int H, const char *text, int icon):Fl_Button(X, Y, W, H),
    IconDim(NULL), IconBright(NULL), Hover(false), Toggled(false)
{
    copy_label(text);
    if(icon>=0)
    {
        IconDim=ThemeIcon((THEME_ICON)icon, THEME_FG_DIM, 16);
        IconBright=ThemeIcon((THEME_ICON)icon, THEME_FG_BRIGHT, 16);
    }
    box(FL_NO_BOX);
    labelsize(12);
    clear_visible_focus(); //Keep the arrow keys for the navigation
}

Fl_Tool_Button::~Fl_Tool_Button()
{
    delete IconDim;
    delete IconBright;
}

int Fl_Tool_Button::handle(int event)
{
    switch(event)
    {
        case FL_ENTER:
            Hover=true;
            redraw();
            break;
        case FL_LEAVE:
            Hover=false;
            redraw();
            break;
    }
    return Fl_Button::handle(event);
}

void Fl_Tool_Button::draw()
{
    fl_color(parent() ? parent()->color() : THEME_BAR_BG);
    fl_rectf(x(), y(), w(), h());

    bool on=Toggled && active_r();
    if(on || (Hover && active_r()) || (value() && active_r()))
    {
        fl_color(on ? fl_rgb_color(4, 57, 94) : THEME_HOVER);
        fl_rectf(x()+2, y()+4, w()-4, h()-8);
    }
    if(on)
    {
        fl_color(THEME_ACCENT);
        fl_rectf(x()+2, y()+h()-6, w()-4, 2);
    }

    bool bright=active_r() && (on || Hover);
    Fl_SVG_Image *icon=bright ? IconBright : IconDim;
    const char *text=label() ? label() : "";
    fl_font(FL_HELVETICA, labelsize());
    int tw=text[0] ? (int)fl_width(text) : 0;
    int iw=icon ? icon->w() : 0;
    int total=iw+(iw && tw ? 6 : 0)+tw;
    int cx=x()+(w()-total)/2;
    if(icon)
    {
        icon->draw(cx, y()+(h()-icon->h())/2);
        cx+=iw+(tw ? 6 : 0);
    }
    if(tw)
    {
        fl_color(!active_r() ? fl_inactive(THEME_FG_DIM) : bright ? THEME_FG_BRIGHT : THEME_FG);
        fl_draw(text, cx, y(), tw, h(), FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    }
}


//------------------------------------Reader bar------------------------

Fl_Reader_Bar::Fl_Reader_Bar(int X, int Y, int W):Fl_Group(X, Y, W, READER_BAR_H)
{
    box(FL_FLAT_BOX);
    color(THEME_EDITOR_BG);

    int h=READER_BAR_H, x=X+6;
    LibraryButton=new Fl_Tool_Button(x, Y, 92, h, "Library", ICON_LIBRARY);
    LibraryButton->tooltip("Back to the library (Ctrl+L): the comic stays open where you left it");
    x+=LibraryButton->w()+6;

    //Title and position: take the free space
    int right_w=4*64+8+118+8+2*32+6;
    Title=new Fl_Box(x, Y, W-(x-X)-right_w-150, h);
    Title->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    Title->labelsize(13);
    Title->labelcolor(THEME_FG_BRIGHT);
    Position=new Fl_Box(Title->x()+Title->w(), Y, 150, h);
    Position->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    Position->labelsize(12);
    Position->labelcolor(THEME_FG_DIM);

    //Reading modes: one of them on
    x=X+W-right_w;
    const char *mode_names[]={"Panels", "Page", "Strip", "Scroll"};
    const char *mode_tips[]={"Panel by panel (C)", "Whole page (P)", "Rows of panels, for the long pages (S)",
                             "Continuous scrolling, pages one under the other: webtoons, manhwa (W)"};
    int modes[]={CASE, PAGE, STRIP, SCROLL};
    for(int i=0; i<4; i++)
    {
        Fl_Tool_Button *b=new Fl_Tool_Button(x, Y, 64, h, mode_names[i]);
        b->tooltip(mode_tips[i]);
        ModeButtons.push_back(b);
        Modes.push_back(modes[i]);
        x+=64;
    }
    x+=8;
    DirectionButton=new Fl_Tool_Button(x, Y, 118, h, "Right to left", ICON_MANGA);
    DirectionButton->tooltip("Manga reading order: panels and pages from right to left (M)");
    x+=118+8;
    PagesButton=new Fl_Tool_Button(x, Y, 32, h, "", ICON_PAGES);
    PagesButton->tooltip("Pages side bar (Ctrl+T)");
    x+=32;
    FullscreenButton=new Fl_Tool_Button(x, Y, 32, h, "", ICON_FULLSCREEN);
    FullscreenButton->tooltip("Fullscreen (F11 or Ctrl+F, Esc to leave)");

    Fl_Widget *buttons[]={LibraryButton, DirectionButton, PagesButton, FullscreenButton};
    for(int i=0; i<4; i++)
        buttons[i]->callback(Button_cb, this);
    for(size_t i=0; i<ModeButtons.size(); i++)
        ModeButtons[i]->callback(Button_cb, this);
    end();
    resizable(Title);
}

void Fl_Reader_Bar::SetPosition(const std::string &title, int page, int pages, int panel, int panels)
{
    std::string position="Page "+std::to_string(page)+" / "+std::to_string(pages);
    //Panel 0: the whole page is shown before its first panel
    if(panels>0 && panel>0)
        position+="   \xc2\xb7   Panel "+std::to_string(panel)+" / "+std::to_string(panels);
    else if(panels>0)
        position+="   \xc2\xb7   "+std::to_string(panels)+(panels>1 ? " panels" : " panel");
    if(title!=TitleText)
    {
        TitleText=title;
        Title->copy_label(title.c_str());
        Title->tooltip(Title->label());
        Title->redraw();
    }
    if(position!=PositionText)
    {
        PositionText=position;
        Position->copy_label(position.c_str());
        Position->redraw();
    }
}

void Fl_Reader_Bar::SetModes(int mode, bool right_to_left, bool pages_shown)
{
    for(size_t i=0; i<ModeButtons.size(); i++)
        ModeButtons[i]->SetToggled(Modes[i]==mode);
    DirectionButton->SetToggled(right_to_left);
    PagesButton->SetToggled(pages_shown);
}

void Fl_Reader_Bar::Button_cb(Fl_Widget *w, void *p)
{
    Fl_Reader_Bar *bar=(Fl_Reader_Bar*)p;
    if(w==bar->LibraryButton && bar->OnLibrary)
        bar->OnLibrary();
    else if(w==bar->DirectionButton && bar->OnDirection)
        bar->OnDirection();
    else if(w==bar->PagesButton && bar->OnPages)
        bar->OnPages();
    else if(w==bar->FullscreenButton && bar->OnFullscreen)
        bar->OnFullscreen();
    else
        for(size_t i=0; i<bar->ModeButtons.size(); i++)
            if(w==bar->ModeButtons[i] && bar->OnMode)
                bar->OnMode(bar->Modes[i]);
}
