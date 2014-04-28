#ifndef FL_TRANS_BUTTON_H
#define FL_TRANS_BUTTON_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>

#include <stdio.h>

const Fl_Color FL_TRANS = 0xffffffff;

class Fl_Trans_Button : public Fl_Button
{
    public:
        Fl_Trans_Button(int X, int Y, int W, int H, const char *L=0, float R=.0, float Wa=1., float Ha=1.):Fl_Button(X, Y, W, H, L){r=R;w_a=Wa;h_a=Ha;is_visible=false;};
        void draw();
    protected:
        int handle(int E);
    private:
        bool is_visible;

        //Rotation angle (in degrees)
        float r;
        //Arrow position/size (percentage)
        float w_a, h_a;
};

#endif // FL_TRANS_BUTTON_H
