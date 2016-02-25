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
#include "Fl_Help_Window.h"

Fl_Help_Window::Fl_Help_Window(int W, int H, const char *L):Fl_Double_Window(W, H, L)
{
    //Create a help_view on the window
    begin();
        HelpView=new Fl_Help_View(0, 0, w(), h());
    end();
    HelpView->textsize(14);
    resizable(HelpView);
    HelpView->color(FL_BACKGROUND2_COLOR, FL_BACKGROUND2_COLOR);
    HelpView->box(FL_FLAT_BOX);
    show();
}

void Fl_Help_Window::value(const char *html)
{
    //Assign the value
    HelpView->value(html);
}

