//--------------------------------------------------------------------------
//
//    Copyright (C) 2014  Vincent Crocher
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
//    along with BDReader.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Fl_TransButton.h
 * \brief Declaration file of the Fl_Trans_Button class
 * \author Vincent Crocher
 * \version 0.1
 * \date April 2014
 *
 *
 */
#ifndef FL_TRANS_BUTTON_H
#define FL_TRANS_BUTTON_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>

#include <stdio.h> //TODO: to remove

const Fl_Color FL_TRANS = 0xffffffff;

//! Fl_Trans_Button is a Fl_Button drawing a "stylistic" arrow and behave such that it only appears when overlayed by the mouse cursor
//!The class is inherited from the Fl_Button class. It reproduces the classical behaviour of a Fl_Button (callback on click) but
//!has the particularity to dissapear when not overlaid by the mouse cursor and to draw an arrow on a transparent background instead
//!of displaying the classical button label. The label is not used anymore. For a description of arrow style and widget parameters
//!see the constructor description.
class Fl_Trans_Button : public Fl_Button
{
    public:
        //! Constructor.
        //!\param X, Y, W, H, L: see Fl_Button
        //!\param R: the rotation angle defining the arrow direction (0: to the left, 180: to the right, 90: up...)
        //!\param Wa: the proportion of the arrow width regarding the widget width (default is 1: the arrow has the widget width)
        //!\param Ha: the proportion of the arrow height regarding the widget height (default is 1: the arrow has the widget height)
        //!\param S: the arrow style. Three styles are provided: 0(default): classic straight arrow, 1: sharp more complex shape, 2: a rounded arrow
        Fl_Trans_Button(int X, int Y, int W, int H, const char *L=0, float R=.0, float Wa=1., float Ha=1., int S=0):Fl_Button(X, Y, W, H, L){r=R;w_a=Wa;h_a=Ha;style=S;is_visible=false;};
    protected:
        int handle(int E);
        void draw();
    private:
        //Say if the widget should be drawn or stay empty
        bool is_visible;

        //Rotation angle (in degrees)
        float r;
        //Arrow size (percentage of the widget size)
        float w_a, h_a;

        //Arrow style: 0 (classic arrow, straight), 1 (straight lines but more complex shape, more sharp) or 2 (rounded arrow)
        int style;
};

#endif // FL_TRANS_BUTTON_H
