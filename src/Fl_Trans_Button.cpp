#include "Fl_Trans_Button.h"


int Fl_Trans_Button::handle(int event)
{
    int ret=0;
    if(Fl::belowmouse()==this)
    {
        //Make visible
        is_visible=true;
        //and redraw
        redraw();
        //Pass the event to other widget if needed
        ret=0;
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

    //Call the classical button callback event handler (return non zero if the event have been handled)
    int ret2 = Fl_Button::handle(event);
    if(ret2!=0)
        return ret2;

    return ret;
}

void Fl_Trans_Button::draw()
{
    //Draw only if really visible: if the mouse is above
    if(is_visible)
    {
        /*"Semi transparent" background color
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
        }*/

        //Draw the arrow
        //Transformation matrix (for rotation)
        fl_push_matrix();
            //Apply the rotation
            fl_translate(x()+w()/2., y()+h()/2.);//go to the center
            fl_rotate(r);//rotate
            fl_translate(-x()-w()/2., -y()-h()/2.);//go back
            //Draw
            int realxa=x()+0.5*w()*(1.-w_a);
            int realya=y()+0.5*h()*(1.-h_a);
            int realwa=w()*(float)w_a;
            int realha=h()*(float)h_a;
            /*int realxa=x()+w()*(1.-x_a);
            int realya=y()+h()*(1.-y_a);
            int realwa=w()*(float)w_a;
            int realha=h()*(float)h_a;*/
            //inside
            if(labelcolor()!=FL_TRANS)
            {
                fl_color(labelcolor());
                //fl_polygon(realxa, realya, realxa+realwa, realya+realha/2., realxa, realya+realha, realxa+realwa*0.5, realya+realha/2.);
                fl_begin_complex_polygon();
                    fl_curve(realxa, realya, 0.6*realxa+ 0.4*(realxa+realwa), 0.75*realya+0.25*(realya+realha/2.), 0.25*realxa+0.75*(realxa+realwa), 0.3*realya+0.7*(realya+realha/2.), realxa+realwa, realya+realha/2.);
                    fl_curve(realxa+realwa, realya+realha/2., 0.25*realxa+0.75*(realxa+realwa), 0.3*(realya+realha)+0.7*(realya+realha/2.), 0.6*realxa+0.4*(realxa+realwa), 0.75*(realya+realha)+0.25*(realya+realha/2.), realxa, realya+realha);
                    fl_curve(realxa, realya+realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, realxa, realya);
                fl_end_complex_polygon();
            }
            //line
            if(selection_color()!=FL_TRANS)
            {
                fl_color(selection_color());
                fl_line_style(0, 2, 0);
                //fl_loop(realxa, realya, realxa+realwa, realya+realha/2., realxa, realya+realha, realxa+realwa*0.5, realya+realha/2.);
                fl_begin_line();
                    fl_curve(realxa, realya, 0.6*realxa+ 0.4*(realxa+realwa), 0.75*realya+0.25*(realya+realha/2.), 0.25*realxa+0.75*(realxa+realwa), 0.3*realya+0.7*(realya+realha/2.), realxa+realwa, realya+realha/2.);
                    fl_curve(realxa+realwa, realya+realha/2., 0.25*realxa+0.75*(realxa+realwa), 0.3*(realya+realha)+0.7*(realya+realha/2.), 0.6*realxa+0.4*(realxa+realwa), 0.75*(realya+realha)+0.25*(realya+realha/2.), realxa, realya+realha);
                    fl_curve(realxa, realya+realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, 0.6*realxa+ 0.4*(realxa+realwa), realya+0.5*realha, realxa, realya);
                fl_end_line();
                fl_line_style(0);
            }
        fl_pop_matrix();
    }
}
