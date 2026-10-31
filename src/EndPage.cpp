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
#include "EndPage.h"
#include "Theme.h"

#define END_BUTTON_W 340
#define END_BUTTON_H 40
#define END_BUTTON_GAP 8

//! Wide flat button of the end page, the label on the left
static Fl_Button *EndButton(const char *label, bool accent)
{
    Fl_Button *b=new Fl_Button(0, 0, END_BUTTON_W, END_BUTTON_H, label);
    b->box(FL_FLAT_BOX);
    b->color(accent ? THEME_ACCENT : THEME_BAR_BG, accent ? fl_rgb_color(2, 106, 192) : THEME_HOVER);
    b->labelcolor(accent ? FL_WHITE : THEME_FG);
    b->labelsize(14);
    b->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    b->clear_visible_focus(); //Keep the arrow keys for the navigation
    return b;
}

Fl_End_Page::Fl_End_Page(int X, int Y, int W, int H):Fl_Group(X, Y, W, H)
{
    box(FL_FLAT_BOX);
    color(THEME_EDITOR_BG);

    Over=new Fl_Box(X, Y, W, 20, "END OF");
    Over->labelsize(11);
    Over->labelfont(FL_HELVETICA_BOLD);
    Over->labelcolor(THEME_FG_DIM);
    Title=new Fl_Box(X, Y, W, 60);
    Title->labelsize(26);
    Title->labelcolor(THEME_FG_BRIGHT);
    Title->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE|FL_ALIGN_WRAP|FL_ALIGN_CLIP);
    Info=new Fl_Box(X, Y, W, 20);
    Info->labelsize(13);
    Info->labelcolor(THEME_FG_DIM);

    NextButton=EndButton("", true);
    RestartButton=EndButton("     Restart from the beginning", false);
    LibraryButton=EndButton("     Return to library", false);
    OpenButton=EndButton("     Open another comic...", false);
    Fl_Button *buttons[]={NextButton, RestartButton, LibraryButton, OpenButton};
    for(int i=0; i<4; i++)
        buttons[i]->callback(Button_cb, this);

    BackHint=new Fl_Box(X, Y, W, 20, "\xe2\x86\x90   back to the last page");
    BackHint->labelsize(12);
    BackHint->labelcolor(THEME_FG_DIM);
    end();
    resizable(NULL);
    Layout();
}

void Fl_End_Page::Set(const std::string &title, int pages, const std::string &next_title)
{
    Title->copy_label(title.c_str());
    Info->copy_label((std::to_string(pages)+(pages>1 ? " pages" : " page")+"   \xc2\xb7   finished").c_str());
    if(next_title.empty())
        NextButton->hide();
    else
    {
        NextLabel="     Read next:  "+next_title;
        NextButton->label(NextLabel.c_str());
        NextButton->show();
    }
    Layout();
    redraw();
}

void Fl_End_Page::resize(int X, int Y, int W, int H)
{
    Fl_Group::resize(X, Y, W, H);
    Layout();
}

//! Column centered in the page
void Fl_End_Page::Layout()
{
    int buttons=NextButton->visible() ? 4 : 3;
    int height=20+64+24+28+buttons*(END_BUTTON_H+END_BUTTON_GAP)+24+20;
    int cy=y()+(h()-height)/2;
    int bx=x()+(w()-END_BUTTON_W)/2;
    int tw=std::min(w()-40, 700);

    Over->resize(x(), cy, w(), 20);
    Title->resize(x()+(w()-tw)/2, cy+20, tw, 64);
    Info->resize(x(), cy+84, w(), 24);
    int by=cy+84+24+28;
    Fl_Button *buttons_list[]={NextButton, RestartButton, LibraryButton, OpenButton};
    for(int i=0; i<4; i++)
    {
        if(!buttons_list[i]->visible())
            continue;
        buttons_list[i]->resize(bx, by, END_BUTTON_W, END_BUTTON_H);
        by+=END_BUTTON_H+END_BUTTON_GAP;
    }
    BackHint->resize(x(), by+24, w(), 20);
}

void Fl_End_Page::Button_cb(Fl_Widget *w, void *p)
{
    Fl_End_Page *page=(Fl_End_Page*)p;
    std::function<void()> action;
    if(w==page->NextButton) action=page->OnNext;
    else if(w==page->RestartButton) action=page->OnRestart;
    else if(w==page->LibraryButton) action=page->OnLibrary;
    else if(w==page->OpenButton) action=page->OnOpen;
    if(action)
        action();
}
