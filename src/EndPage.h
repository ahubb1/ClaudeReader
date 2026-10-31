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
 * \file EndPage.h
 * \brief Page shown after the last page of a comic, in the reading area: what to read next
 *
 */
#ifndef ENDPAGE_H
#define ENDPAGE_H

#include <string>
#include <functional>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

//! The end of comic page (covers the display): next comic, restart, library, open another one
class Fl_End_Page : public Fl_Group
{
    public:
        Fl_End_Page(int X, int Y, int W, int H);

        //! Fill the page: comic title, number of pages and the next comic suggested (empty if none)
        void Set(const std::string &title, int pages, const std::string &next_title);
        void resize(int X, int Y, int W, int H);

        std::function<void()> OnNext, OnRestart, OnLibrary, OnOpen;

    private:
        void Layout();
        static void Button_cb(Fl_Widget *w, void *p);

        Fl_Box *Over, *Title, *Info, *BackHint;
        Fl_Button *NextButton, *RestartButton, *LibraryButton, *OpenButton;
        std::string NextLabel;
};

#endif // ENDPAGE_H
