//--------------------------------------------------------------------------
//
//    Copyright (C) 2014 Vincent Crocher
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
#ifndef FL_HELP_WINDOW_H
#define FL_HELP_WINDOW_H

#include <FL/Fl_Help_View.H>

//! A help window derived from Fl_Window to display help text formatted in HTML thanks to a Fl_Help_View widget.
class Fl_Help_Window : public Fl_Window
{
    public:
        Fl_Help_Window(int W, int H, const char *L=0);

        void value(const char *html);
    protected:
    private:
        Fl_Help_View *HelpView;
};

#endif // FL_HELP_WINDOW_H
