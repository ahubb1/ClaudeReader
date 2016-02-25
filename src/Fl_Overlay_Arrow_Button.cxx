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


int Fl_Overlay_Arrow_Button::handle(int event)
{
    int ret=0;
    if((Fl::belowmouse()==this && event==FL_MOVE) || event==FL_ENTER)
    {
        //Make visible
        is_visible=true;
        //and redraw
        redraw();
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

    //Call the classical button callback event handler (return non zero if the event has been handled)
    int ret2 = Fl_Button::handle(event);
    if(ret2!=0)
        return ret2;

    return ret;
}


void Fl_Overlay_Arrow_Button::draw()
{
    //Draw only if really visible: if the mouse is above
    if(is_visible)
    {
        //"Semi transparent" background color: fake transparency diplaying some pixels of the background color
        if(color()!=FL_TRANS)
        {
            fl_color(color());
            for(int i=0; i<w();i+=2)
            {
                for(int j=0; j<h(); j+=4)
                {
                    fl_point(x()+i, y()+j);
                    fl_point(1+x()+i, y()+j+2);
                }
            }
        }

        //Draw the arrow
        //Transformation matrix (for rotation)
        fl_push_matrix();
            //Apply the rotation
            fl_translate(x()+w()/2., y()+h()/2.);//go to the center
            fl_rotate(r);//rotate
            fl_translate(-x()-w()/2., -y()-h()/2.);//go back

            //Compute the arrow coordinates and dimensions
            int signha=1, signwa=1;
            //Apply an horizontal symetrie to the arrow (=>the arrow way)
            if( (r>90 && r<=180) || (r<=-90 && r>=-180) )
            {
                signwa=-1;
            }
            int realwa=signwa*fl_transform_dx(w()*(float)w_a, h()*(float)h_a);
            int realha=signha*fl_transform_dy(w()*(float)w_a, h()*(float)h_a);
            int realxa=x()+0.5*(w()-realwa);
            int realya=y()+0.5*(h()-realha);

            //inside
            if(labelcolor()!=FL_TRANS)
            {
                fl_color(labelcolor());
                fl_begin_complex_polygon(); //(complex polygon is needed instead of simple polygon since, depending on signwa, the shape can be not convex)
                    //Draw the arrow according to the style choosen
                    switch(style)
                    {
                        case 1:
                            fl_vertex(realxa, realya);
                            fl_vertex(realxa+realwa, realya+realha/2.);
                            fl_vertex(realxa, realya+realha);
                            fl_vertex(realxa+realwa*0.3, realya+realha/2.);
                            break;

                        case 2:
                            fl_curve(realxa, realya, 0.6*realxa+ 0.4*(realxa+realwa), 0.75*realya+0.25*(realya+realha/2.), 0.25*realxa+0.75*(realxa+realwa), 0.3*realya+0.7*(realya+realha/2.), realxa+realwa, realya+realha/2.);
                            fl_curve(realxa+realwa, realya+realha/2., 0.25*realxa+0.75*(realxa+realwa), 0.3*(realya+realha)+0.7*(realya+realha/2.), 0.6*realxa+0.4*(realxa+realwa), 0.75*(realya+realha)+0.25*(realya+realha/2.), realxa, realya+realha);
                            fl_curve(realxa, realya+realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, realxa, realya);
                            break;

                        //O is the default one
                        default:
                            fl_vertex(realxa, realya);
                            fl_vertex(realxa+realwa, realya+realha/2.);
                            fl_vertex(realxa, realya+realha);
                            break;
                    }
                fl_end_complex_polygon();
            }
            //line
            if(selection_color()!=FL_TRANS)
            {
                fl_color(selection_color());
                fl_line_style(0, 2, 0);
                fl_begin_loop();
                    //Draw the arrow according to the style choosen
                    switch(style)
                    {
                        case 1:
                            fl_vertex(realxa, realya);
                            fl_vertex(realxa+realwa, realya+realha/2.);
                            fl_vertex(realxa, realya+realha);
                            fl_vertex(realxa+realwa*0.3, realya+realha/2.);
                            break;

                        case 2:
                            fl_curve(realxa, realya, 0.6*realxa+ 0.4*(realxa+realwa), 0.75*realya+0.25*(realya+realha/2.), 0.25*realxa+0.75*(realxa+realwa), 0.3*realya+0.7*(realya+realha/2.), realxa+realwa, realya+realha/2.);
                            fl_curve(realxa+realwa, realya+realha/2., 0.25*realxa+0.75*(realxa+realwa), 0.3*(realya+realha)+0.7*(realya+realha/2.), 0.6*realxa+0.4*(realxa+realwa), 0.75*(realya+realha)+0.25*(realya+realha/2.), realxa, realya+realha);
                            fl_curve(realxa, realya+realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, realxa, realya);
                            break;

                        //O is the default one
                        default:
                            fl_vertex(realxa, realya);
                            fl_vertex(realxa+realwa, realya+realha/2.);
                            fl_vertex(realxa, realya+realha);
                            break;
                    }
                fl_end_loop();
                fl_line_style(0);
            }
        fl_pop_matrix();
    }
}
