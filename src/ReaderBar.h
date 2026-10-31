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
/**
 * \file ReaderBar.h
 * \brief Tool bar above the reading area: back to the library, comic title and position, reading mode, direction and view toggles
 *
 */
#ifndef READERBAR_H
#define READERBAR_H

#include <string>
#include <vector>
#include <functional>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include "Theme.h"

#define READER_BAR_H 34

//! Flat button of a tool bar: optional icon and text, highlighted when toggled on
class Fl_Tool_Button : public Fl_Button
{
    public:
        Fl_Tool_Button(int X, int Y, int W, int H, const char *text, int icon=-1);
        ~Fl_Tool_Button();

        int handle(int event);
        void draw();

        void SetToggled(bool val) {if(Toggled!=val) {Toggled=val; redraw();}}

    private:
        Fl_SVG_Image *IconDim, *IconBright;
        bool Hover, Toggled;
};

class Fl_Reader_Bar : public Fl_Group
{
    public:
        Fl_Reader_Bar(int X, int Y, int W);

        //! Comic shown: title and position ("Page 3/48", "Panel 2/7" in panel mode, panels<=0 otherwise)
        void SetPosition(const std::string &title, int page, int pages, int panel, int panels);
        //! Current reading mode (READINGMODE of Page.h), reading direction and side bar state
        void SetModes(int mode, bool right_to_left, bool pages_shown);

        std::function<void()> OnLibrary, OnDirection, OnPages, OnFullscreen;
        std::function<void(int)> OnMode;        //!< A READINGMODE

    private:
        static void Button_cb(Fl_Widget *w, void *p);

        Fl_Tool_Button *LibraryButton, *DirectionButton, *PagesButton, *FullscreenButton;
        Fl_Box *Title, *Position;
        std::vector<Fl_Tool_Button*> ModeButtons;    //!< Panels, Page, Strip, Scroll
        std::vector<int> Modes;                 //!< READINGMODE of each mode button
        std::string TitleText, PositionText;
};

#endif // READERBAR_H
