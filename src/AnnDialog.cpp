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
#include "AnnDialog.h"
#include "Theme.h"
#include "Async.h"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <memory>
#include <thread>

namespace
{
    Fl_Button *DialogButton(int x, int y, int w, const char *label, bool accent)
    {
        Fl_Button *b=new Fl_Button(x, y, w, 28, label);
        b->box(FL_FLAT_BOX);
        b->color(accent ? THEME_ACCENT : THEME_INPUT_BG, accent ? fl_rgb_color(2, 106, 192) : THEME_HOVER);
        b->labelcolor(accent ? FL_WHITE : THEME_FG);
        b->labelsize(13);
        return b;
    }

    Fl_Box *DialogText(int x, int y, int w, int h, int size, Fl_Color color)
    {
        Fl_Box *b=new Fl_Box(x, y, w, h);
        b->align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_INSIDE|FL_ALIGN_WRAP|FL_ALIGN_CLIP);
        b->labelsize(size);
        b->labelcolor(color);
        return b;
    }

    //! Centered on the ClaudeReader window
    void CenterOnMain(Fl_Window *dialog)
    {
        Fl_Window *main=Fl::first_window();
        while(main && main->window())
            main=main->window();
        if(main && main!=dialog)
            dialog->position(main->x()+(main->w()-dialog->w())/2, main->y()+(main->h()-dialog->h())/2);
    }

    std::string Join(const std::vector<std::string> &list)
    {
        std::string text;
        for(size_t i=0; i<list.size(); i++)
            text+=(i ? ", " : "")+list[i];
        return text;
    }

    //! The find tags dialog: its widgets and state
    struct FindState
    {
        Fl_Double_Window *Window;
        Fl_Hint_Input *Input;
        Fl_Choice *Matches;
        Fl_Box *Status, *Genres, *Themes;
        Fl_Button *Link, *GenresButton, *ThemesButton, *BothButton;
        std::vector<AnnTitle> Titles;
        int Result=ANN_NONE;
        unsigned Generation=0;                  //!< Late answers of an older search are ignored
        std::shared_ptr<bool> Alive=std::make_shared<bool>(true);

        void SetStatus(const std::string &text)
        {
            Status->copy_label(text.c_str());
            Status->redraw();
        }

        //! The match chosen: its genres and themes (loaded if needed), the buttons
        void ShowMatch()
        {
            int index=Matches->value();
            GenresButton->deactivate();
            ThemesButton->deactivate();
            BothButton->deactivate();
            Link->hide();
            Genres->label("");
            Themes->label("");
            if(index<0 || index>=(int)Titles.size())
            {
                Window->redraw();
                return;
            }
            if(!Titles[index].Detailed)
            {
                SetStatus("Loading the details...");
                AnnTitle title=Titles[index];
                unsigned generation=Generation;
                std::shared_ptr<bool> alive=Alive;
                std::thread([this, title, index, generation, alive]()
                {
                    AnnTitle detailed=title;
                    std::string error;
                    bool ok=Ann::Details(&detailed, &error);
                    RunInUiThread([this, detailed, index, generation, alive, ok, error]()
                    {
                        if(!*alive || generation!=Generation || index>=(int)Titles.size())
                            return;
                        if(!ok)
                        {
                            SetStatus(error);
                            return;
                        }
                        Titles[index]=detailed;
                        ShowMatch();
                    });
                }).detach();
                return;
            }
            const AnnTitle &t=Titles[index];
            std::string genres="Genres:   "+(t.Genres.empty() ? std::string("none") : Join(t.Genres));
            std::string themes="Themes:   "+(t.Themes.empty() ? std::string("none") : Join(t.Themes));
            Genres->copy_label(genres.c_str());
            Themes->copy_label(themes.c_str());
            if(!t.Genres.empty())
                GenresButton->activate();
            if(!t.Themes.empty())
                ThemesButton->activate();
            if(!t.Genres.empty() || !t.Themes.empty())
                BothButton->activate();
            Link->show();
            SetStatus(Titles.size()>1 ? std::to_string(Titles.size())+" matches: choose the right one" : "");
            Window->redraw();
        }

        void Search()
        {
            std::string name=Input->value();
            Generation++;
            Titles.clear();
            Matches->clear();
            ShowMatch();
            SetStatus("Searching Anime News Network...");
            unsigned generation=Generation;
            std::shared_ptr<bool> alive=Alive;
            std::thread([this, name, generation, alive]()
            {
                std::vector<AnnTitle> titles;
                std::string error, searched;
                bool ok=Ann::Find(name, &titles, &error, &searched);
                RunInUiThread([this, titles, generation, alive, ok, error, name, searched]()
                {
                    if(!*alive || generation!=Generation)
                        return;
                    if(!ok)
                    {
                        SetStatus(error);
                        return;
                    }
                    Titles=titles;
                    for(size_t i=0; i<Titles.size(); i++)
                    {
                        //Menu text: no '/' (submenus) nor '&' (shortcuts)
                        std::string label=Titles[i].Name;
                        for(size_t k=0; k<label.size(); k++)
                            if(label[k]=='/')
                                label.insert(k++, "\\");
                            else if(label[k]=='&')
                                label.insert(k++, "&");
                        Matches->add(label.c_str());
                    }
                    Matches->value(0);
                    ShowMatch();
                    //Found with a word of the name only: say it
                    if(searched!=name)
                        SetStatus("Nothing named \""+name+"\": results for \""+searched+"\"");
                });
            }).detach();
        }
    };

    void Search_cb(Fl_Widget *, void *p) { ((FindState*)p)->Search(); }
    void Match_cb(Fl_Widget *, void *p) { ((FindState*)p)->ShowMatch(); }
    void Link_cb(Fl_Widget *, void *p)
    {
        FindState *s=(FindState*)p;
        int index=s->Matches->value();
        if(index>=0 && index<(int)s->Titles.size())
            fl_open_uri(s->Titles[index].Url().c_str());
    }
    void Choose_cb(Fl_Widget *w, void *p)
    {
        FindState *s=(FindState*)p;
        s->Result=w==s->GenresButton ? ANN_GENRES : w==s->ThemesButton ? ANN_THEMES : ANN_BOTH;
        s->Window->hide();
    }
    void Cancel_cb(Fl_Widget *w, void *p)
    {
        ((FindState*)p)->Result=ANN_NONE;
        ((FindState*)p)->Window->hide();
    }
}

int FindTagsDialog(const std::string &name, const std::string &scope, AnnTitle *chosen)
{
    FindState s;
    Fl_Double_Window win(620, 330, "Find tags online");
    s.Window=&win;
    win.color(THEME_EDITOR_BG);
    Fl_Box *title=DialogText(20, 14, 580, 24, 15, THEME_FG_BRIGHT);
    title->label("Tags from Anime News Network");
    Fl_Box *for_box=DialogText(20, 40, 580, 20, 12, THEME_FG_DIM);
    for_box->copy_label(("For: "+scope).c_str());

    Fl_Box *search_label=DialogText(20, 76, 60, 26, 13, THEME_FG);
    search_label->label("Manga");
    search_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    s.Input=new Fl_Hint_Input(84, 76, 410, 26);
    s.Input->box(THEME_INPUT_BOX);
    s.Input->color(THEME_INPUT_BG);
    s.Input->textcolor(THEME_FG);
    s.Input->textsize(13);
    s.Input->value(name.c_str());
    s.Input->when(FL_WHEN_ENTER_KEY_ALWAYS);
    s.Input->callback(Search_cb, &s);
    Fl_Button *search=DialogButton(502, 75, 98, "Search", false);
    search->callback(Search_cb, &s);

    Fl_Box *match_label=DialogText(20, 112, 60, 26, 13, THEME_FG);
    match_label->label("Match");
    match_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    s.Matches=new Fl_Choice(84, 112, 516, 26);
    s.Matches->box(THEME_INPUT_BOX);
    s.Matches->color(THEME_INPUT_BG);
    s.Matches->textcolor(THEME_FG);
    s.Matches->textsize(13);
    s.Matches->down_box(FL_FLAT_BOX);
    s.Matches->callback(Match_cb, &s);

    s.Genres=DialogText(20, 152, 580, 36, 13, THEME_FG);
    s.Themes=DialogText(20, 192, 580, 52, 13, THEME_FG);
    s.Link=DialogButton(20, 246, 230, "View on Anime News Network", false);
    s.Link->box(FL_NO_BOX);
    s.Link->labelcolor(fl_rgb_color(55, 148, 255));
    s.Link->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    s.Link->callback(Link_cb, &s);
    s.Link->tooltip("The encyclopedia entry of this manga (data from Anime News Network)");
    s.Status=DialogText(260, 250, 340, 22, 12, THEME_FG_DIM);
    s.Status->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);

    s.GenresButton=DialogButton(20, 286, 110, "Genres", false);
    s.ThemesButton=DialogButton(138, 286, 110, "Themes", false);
    s.BothButton=DialogButton(256, 286, 110, "Both", true);
    s.GenresButton->tooltip("Add the genres as tags");
    s.ThemesButton->tooltip("Add the themes as tags");
    s.BothButton->tooltip("Add the genres and the themes as tags");
    Fl_Button *cancel=DialogButton(490, 286, 110, "Cancel", false);
    Fl_Button *choices[]={s.GenresButton, s.ThemesButton, s.BothButton};
    for(int i=0; i<3; i++)
        choices[i]->callback(Choose_cb, &s);
    cancel->callback(Cancel_cb, &s);
    win.end();
    win.set_modal();
    CenterOnMain(&win);
    win.show();
    s.Search();
    while(win.shown())
        Fl::wait();
    *s.Alive=false;
    int index=s.Matches->value();
    if(s.Result!=ANN_NONE && index>=0 && index<(int)s.Titles.size())
        *chosen=s.Titles[index];
    else
        s.Result=ANN_NONE;
    return s.Result;
}

int AskTagKind(const std::string &message)
{
    int result=ANN_NONE;
    Fl_Double_Window win(520, 170, "Find tags online");
    win.color(THEME_EDITOR_BG);
    Fl_Box *title=DialogText(20, 14, 480, 24, 15, THEME_FG_BRIGHT);
    title->label("Tags from Anime News Network");
    Fl_Box *text=DialogText(20, 44, 480, 70, 13, THEME_FG);
    text->copy_label(message.c_str());
    Fl_Button *genres=DialogButton(20, 126, 100, "Genres", false);
    Fl_Button *themes=DialogButton(128, 126, 100, "Themes", false);
    Fl_Button *both=DialogButton(236, 126, 100, "Both", true);
    Fl_Button *cancel=DialogButton(400, 126, 100, "Cancel", false);
    struct Choice { int *Result; int Value; Fl_Window *Window; };
    Choice choices[4]={{&result, ANN_GENRES, &win}, {&result, ANN_THEMES, &win}, {&result, ANN_BOTH, &win}, {&result, ANN_NONE, &win}};
    Fl_Button *buttons[4]={genres, themes, both, cancel};
    for(int i=0; i<4; i++)
        buttons[i]->callback([](Fl_Widget *, void *p)
        {
            Choice *c=(Choice*)p;
            *c->Result=c->Value;
            c->Window->hide();
        }, &choices[i]);
    win.end();
    win.set_modal();
    CenterOnMain(&win);
    win.show();
    while(win.shown())
        Fl::wait();
    return result;
}
