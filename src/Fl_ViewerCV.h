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
/*
Based on Fl_Opencv programmed by BlueKid
http://derindelimavi.blogspot.com/ bluekid70@gmail.com
*/
/**
 * \file Fl_ViewerCV.h
 * \brief Declaration file of the Fl_ViewerCV class
 * \author Vincent Crocher
 * \version 0.1
 * \date Sept 2014
 *
 *
 */

#ifndef Fl_ViewerCV_H
#define Fl_ViewerCV_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <opencv2/opencv.hpp>


using namespace cv;


IplImage *fl_LoadImage(char*filename,int iscolor);
bool fl_SaveImage(char*filename,IplImage *image);

//Convert the fltk mouse event into opencv ones (to be able to use open cv callback functions and events)
int fl_event2cv_event(int fl_event);



class FL_EXPORT Fl_ViewerCV : public Fl_Widget
{
    public:

        Fl_ViewerCV(int X, int Y, float mzoom=1.);
        Fl_ViewerCV(int X,int Y,int W,int H, float mzoom=1.);
        ~Fl_ViewerCV();

        //Assign a new image to draw (in Mat Opencv format)
        void SetImage(Mat *nimage, Rect disprect=Rect(0,0,0,0));

        void SetDisplayRect(Rect disprect)
        {
            if(image)
            {
                DisplayRect=disprect;
                UpdateView();
            }
        }

        void UpdateView();


        int handle(int event);
        void resize(int x, int y, int ww, int hh);

        void SetHandleKeys(bool val){HandleKeys=val;}
        void SetCVMouseCallback(CvMouseCallback on_mouse, void * param=0)
        {
          mouse_cb=on_mouse;
          CVcb_param=param;
        }

        friend void ResizingTimer_cb(void *flopencv);
        void SetResizing_cb(void (*cb)(Fl_Widget*, void *), void * param=NULL)
        {
          Resizing_cb=cb;
          Resizing_cb_param=param;
        }

        void SetOnClick_cb(void (*cb)(Fl_Widget*, void *), void * param=NULL)
        {
          OnClick_cb=cb;
          OnClick_cb_param=param;
        }

        void SetOnRelease_cb(void (*cb)(Fl_Widget*, void *), void * param=NULL)
        {
          OnRelease_cb=cb;
          OnRelease_cb_param=param;
        }


        bool IsInContext(){return InContext;}
        void SetInContext(bool val){InContext=val;}
        bool IsBorder(){return Border;}
        void SetBorder(bool val){Border=val;}
        float GetScalingFactor(){return Scale;}
        void SetMaxZoom(float val){MaxZoom=val;}
        float GetMaxZoom(){return MaxZoom;}
        void SetCustomZoom(float val){fmin(fmax(CustomZoom=val, 0.8), 2.0);}
        float GetCustomZoom(){return CustomZoom;}
        float GetCurrentZoom(){return 1./Scale;}


    protected:
        //Draw the image
        void draw(int,int,int,int);
        void draw(){
          draw(Fl_Widget::x(),Fl_Widget::y(),w(),h());
        }

        //Mat fimage;
        int iscolor;
        bool fit;
        bool InContext;
        bool Border;
        float Scale;
        float MaxZoom;
        float CustomZoom;
        //Callbacks
        bool HandleKeys;
        void (*Resizing_cb)(Fl_Widget *, void *);
        void *Resizing_cb_param;
        void (*OnClick_cb)(Fl_Widget *, void *);
        void *OnClick_cb_param;
        void (*OnRelease_cb)(Fl_Widget *, void *);
        void *OnRelease_cb_param;
        CvMouseCallback mouse_cb;
        void *CVcb_param;

        Rect DisplayRect;
        Mat *image, fimage;
        Fl_RGB_Image *FlImg;
};

#endif


