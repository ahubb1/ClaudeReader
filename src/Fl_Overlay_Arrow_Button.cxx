//--------------------------------------------------------------------------
//
//    Copyright (C) 2014-2016  Vincent Crocher
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
#include "Fl_Overlay_Arrow_Button.h"
#include <FL/Fl_Group.H>
#include <string>
#include <algorithm>

int Fl_Overlay_Arrow_Button::handle(int event)
{
    int ret=0;
    if((Fl::belowmouse()==this && event==FL_MOVE) || event==FL_ENTER)
    {
        //Make visible (redraw only when appearing: the drawing is translucent)
        if(!is_visible)
        {
            is_visible=true;
            redraw();
        }
        //The event has been handled
        ret=1;
    }
    else
    {
        //If the mouse leave the widget
        if(event==FL_LEAVE)
        {
            //Hide the button
            is_visible=false;
            hide();
            //Redraw it: nothing is drawn
            redraw();
            //and show it again
            show();
            //The event has been handled
            ret=1;
        }
    }

    //Pressed state is drawn differently: redraw what is under the button first
    if((event==FL_PUSH || event==FL_RELEASE) && parent())
        parent()->damage(FL_DAMAGE_ALL, x(), y(), w(), h());

    //Call the classical button callback event handler (return non zero if the event has been handled)
    int ret2 = Fl_Button::handle(event);
    if(ret2!=0)
        return ret2;
    return ret;
}

//! Chevron pointing in the button direction (r: 0 right, 180 left, 90 up, -90 down) in a translucent dark circle
void Fl_Overlay_Arrow_Button::BuildIcons(int size)
{
    delete Icon;
    delete IconPressed;
    IconSize=size;

    //SVG rotation is clockwise, FLTK one is counter-clockwise
    char rotation[32];
    snprintf(rotation, sizeof(rotation), "rotate(%d 20 20)", (int)-r);

    for(int pressed=0; pressed<2; pressed++)
    {
        std::string svg="<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 40 40'>";
        svg+=pressed ? "<circle cx='20' cy='20' r='19' fill='#0078d4' fill-opacity='0.9'/>"
                     : "<circle cx='20' cy='20' r='19' fill='#181818' fill-opacity='0.78' stroke='#ffffff' stroke-opacity='0.14' stroke-width='1'/>";
        svg+="<path d='M17 12.5l7.5 7.5-7.5 7.5' fill='none' stroke='#e7e7e7' stroke-width='2.4' stroke-linecap='round' stroke-linejoin='round' transform='";
        svg+=rotation;
        svg+="'/></svg>";
        Fl_SVG_Image *img=new Fl_SVG_Image(NULL, svg.c_str());
        img->resize(size, size);
        if(pressed)
            IconPressed=img;
        else
            Icon=img;
    }
}

void Fl_Overlay_Arrow_Button::draw()
{
    //Draw only if really visible: if the mouse is above
    if(!is_visible)
        return;

    //Circle size fitted to the button (the buttons are thin strips along the display borders)
    int size=std::min(44, std::min(w(), h())-6);
    if(size<12)
        return;
    if(size!=IconSize)
        BuildIcons(size);

    Fl_SVG_Image *img=value() ? IconPressed : Icon;
    img->draw(x()+(w()-size)/2, y()+(h()-size)/2);
}
