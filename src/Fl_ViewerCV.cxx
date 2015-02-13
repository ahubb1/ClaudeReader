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
/*
Based on Fl_Opencv programmed by BlueKid
http://derindelimavi.blogspot.com/ bluekid70@gmail.com
*/
#include "Fl_ViewerCV.h"




Fl_ViewerCV::Fl_ViewerCV(int X,int Y,int W,int H, float mzoom):Fl_Widget(X,Y,W,H,0)
{
  x(X);y(Y);w(W);h(H);
  image=NULL;
  FlImg=NULL;
  DisplayRect=Rect(0,0,0,0);
  fit=true;
  InContext=true;
  Border=true;
  Scale=1.0;
  MaxZoom=mzoom;
  CustomZoom=1.0;
  mouse_cb=NULL;
  HandleKeys=false;
  Resizing_cb=NULL;
  Resizing_cb_param=NULL;
  OnClick_cb=NULL;
  OnClick_cb_param=NULL;
  OnRelease_cb=NULL;
  OnRelease_cb_param=NULL;
}



void Fl_ViewerCV::draw(int xx, int yy, int ww, int hh)
{
    //If no image to draw, paint the widget with it's given color
    if(!FlImg || FlImg->w()<=0 || FlImg->h()<=0)
    {
        fl_color(color());
        fl_rectf(x(), y(), w(), h());
        return;
    }

    //Or draw the image
    FlImg->draw(xx, yy);
    return;
}


void Fl_ViewerCV::UpdateView()
{
    //If there is NO display rect
    if( !(DisplayRect.width>0 && DisplayRect.height>0 && DisplayRect.x>=0 && DisplayRect.y>=0) )
    {
        //Then set it to the whole page
        DisplayRect.width=image->cols;
        DisplayRect.height=image->rows;
        DisplayRect.x=0;
        DisplayRect.y=0;
    }

    //Compute the scaling factor such the image will best fit in the widget area, keeping the original aspect ratio
    float fx, fy;
    fx=(float)DisplayRect.width/w();
    fy=(float)DisplayRect.height/h();
    Scale=max(fx,fy);

    //Apply custom zoom if in Panel only (not when strip: page width or whole page: page width) and In context
    if(DisplayRect.width!=image->cols && InContext)
        Scale/=CustomZoom;

    //Impose a maximum zoom value (=>a minimum ratio value), defined by maxzoom
    Scale=fmax(Scale, 1./MaxZoom);


    //Real area to draw: centered on the DisplayRect but can be larger
        //Scale the original image to it's new size
        Mat image_roi;
        cv::resize(*image, image_roi, Size(cvFloor(image->cols/Scale), cvFloor(image->rows/Scale)), 0, 0, INTER_CUBIC);

        //Add a visual border around the current panel
        if(DisplayRect.width!=image->cols && Border)
        {
            Mat image_roi_rect;
            image_roi.copyTo(image_roi_rect);
            for (int i=1; i<12; i++)
                cv::rectangle(image_roi_rect, Rect(DisplayRect.x/Scale-i, DisplayRect.y/Scale-i, DisplayRect.width/Scale+2*i, DisplayRect.height/Scale+2*i), Scalar(0,0,22*i), 1);
            addWeighted(image_roi_rect, 0.5, image_roi, 0.5, 0.0, image_roi);
        }

        //Display in context or not
        if(!InContext && DisplayRect.width!=image->cols)
            image_roi(Rect(DisplayRect.x/Scale, DisplayRect.y/Scale, DisplayRect.width/Scale, DisplayRect.height/Scale)).copyTo(image_roi);

        //Center of the display in resized image
        int x_center=(DisplayRect.x+DisplayRect.width/2.)/Scale;
        int y_center=(DisplayRect.y+DisplayRect.height/2.)/Scale;
        //Crop
        //The top-left corner of the panel to display in the original image
        int x0=fmax(0,floor(x_center-(float)w()/2.));
        int y0=fmax(0,floor(y_center-(float)h()/2.));

        //The bottom-right corner
        int x1, y1;
        //check if ideal image not enough on the left or on top
        if(x0==0)
            x1=fmin(floor(x_center-(float)w()/2.)+(float)w()-floor(x_center-(float)w()/2.), image_roi.cols);
        else
            x1=fmin(floor(x_center-(float)w()/2.)+(float)w(), image_roi.cols);
        if(y0==0)
            y1=fmin(floor(y_center-(float)h()/2.)+(float)h()-floor(y_center-(float)h()/2.), image_roi.rows);
        else
            y1=fmin(floor(y_center-(float)h()/2.)+(float)h(), image_roi.rows);

        //same check if not enough on the bottom/right (except if already adjusted on the top/left)
        if(x1==image_roi.cols && x0!=0)
            x0=fmax(0, x0-(floor(x_center-(float)w()/2.)+(float)w()-image_roi.cols));
        if(y1==image_roi.rows && y0!=0)
            y0=fmax(0, y0-(floor(y_center-(float)h()/2.)+(float)h()-image_roi.rows));

        //Check if original image is smaller than the destination image
        int xd0=0, yd0=0;
        if((x1-x0)<w())
            xd0=(w()-(x1-x0))/2.;
        if((y1-y0)<h())
            yd0=(h()-(y1-y0))/2.;

        fimage=cv::Mat::zeros(h(), w(), image_roi.type());
        //fimage=fimage+Scalar(255,255,255); //For white background
        cv::Mat fimageROI=fimage(Rect(xd0, yd0, (x1-x0), (y1-y0)));
        image_roi(Rect(x0, y0, (x1-x0), (y1-y0))).copyTo(fimageROI);


    //Check the image type for: converting from Opencv BGR (or BGRA if 4 channels) to RGB AND to make sure the image is encoded in unsigned chars
    switch(image->channels())
    {
        //1 channel (GREY):
        case 1:
            cvtColor(fimage, fimage, CV_GRAY2RGB);
        break;

        //3 channels (BGR):
        case 3:
            cvtColor(fimage, fimage, CV_BGR2RGB);
        break;

        //4 channels (BGRA):
        case 4:
            cvtColor(fimage, fimage, CV_BGRA2RGB);
        break;

        //If invalid format
        default:
            //Blank image
            if(FlImg)
                delete FlImg;
            FlImg=new Fl_RGB_Image((uchar *)0, 0, 0, 3, 0);
            return;
        break;
    }

    //Create the image to be drawn
    if(FlImg)
        delete FlImg;
    FlImg=new Fl_RGB_Image((uchar *)fimage.datastart, w(), h(), fimage.channels(), 0);

    //Ask to draw the image
    redraw();
}


void Fl_ViewerCV::SetDisplayRect(Rect disprect)
{
    if(image)
    {
        DisplayRect=disprect;
        UpdateView();
    }
}

void Fl_ViewerCV::SetImage(Mat *nimage, Rect disprect)
{
    if(!nimage || nimage->cols<=0 || nimage->rows<=0)
    {
        image=NULL;
        if(FlImg)
            delete FlImg;
        FlImg=NULL;
    }
    else
    {
        image=nimage;
    }

    if(image)
    {
        SetDisplayRect(disprect);
    }
}


void ResizingTimer_cb(void *flopencv)
{
    Fl_ViewerCV *widget=(Fl_ViewerCV *)flopencv;
    if(widget->Resizing_cb!=NULL)
            widget->Resizing_cb(widget, widget->Resizing_cb_param);
        widget->SetImage(widget->image, widget->DisplayRect);
}

//When the widget is resized, re-compute the image to draw according to new dimensions
void Fl_ViewerCV::resize(int x, int y, int ww, int hh)
{
    Fl_Widget::resize(x, y, ww, hh);

    while(w()<=0 || h()<=0)
        Fl::check();

    //On windows the following is done later on: the system seems to wait for this function to return
    #ifndef __WINDOWS__
        if(Resizing_cb!=NULL)
            Resizing_cb(this, Resizing_cb_param);
        SetImage(image, DisplayRect);
    #else
        Fl::add_timeout(0.02, ResizingTimer_cb, (void*)this);
    #endif
}

//Convert the fltk mouse event into opencv ones (to be able to use open cv callback functions and events)
int fl_event2cv_event(int fl_event)
{
    switch(fl_event)
    {
        //Mouse buttons down events
        case FL_PUSH:
            //Left, middle or right button ?
            switch(Fl::event_button())
            {
                case FL_LEFT_MOUSE:
                    return CV_EVENT_LBUTTONDOWN;
                case FL_RIGHT_MOUSE:
                    return CV_EVENT_RBUTTONDOWN;
                case FL_MIDDLE_MOUSE:
                    return CV_EVENT_MBUTTONDOWN;
                default:
                    return -1;
            }
        //Mouse buttons up events
        case FL_RELEASE:
            //Left, middle or right button ?
            switch(Fl::event_button())
            {
                case FL_LEFT_MOUSE:
                    return CV_EVENT_LBUTTONUP;
                case FL_RIGHT_MOUSE:
                    return CV_EVENT_RBUTTONUP;
                case FL_MIDDLE_MOUSE:
                    return CV_EVENT_MBUTTONUP;
                default:
                    return -1;
            }

        //Two mouse move events:
        case FL_DRAG:
            return CV_EVENT_MOUSEMOVE;
        case FL_MOVE:
            return CV_EVENT_MOUSEMOVE;

        //Default: nothing
        default:
            return -1;
    }
}

int Fl_ViewerCV::handle(int event)
{
   //Mouse events are passed to opencv if an OpenCV callback function has been passed
   if(mouse_cb!=NULL)
   {
       if (event == FL_PUSH || event ==FL_DRAG || event == FL_MOVE || event == FL_RELEASE) {
         if(mouse_cb && image && (Fl::event_x()-x()) < image->cols && (Fl::event_y()-y()) <image->rows )
            mouse_cb(fl_event2cv_event(event), Fl::event_x()-x(), Fl::event_y()-y(), 0, CVcb_param);
       }
   }

   switch(event)
   {
        case FL_PUSH:
            if(OnClick_cb!=NULL)
            {
                OnClick_cb(this, OnClick_cb_param);
                //Event has been handled
                return 1;
            }
            break;

        case FL_RELEASE:
            if(OnRelease_cb!=NULL)
            {
                OnRelease_cb(this, OnRelease_cb_param);
                //Event has been handled
                return 1;
            }
            break;

        //Respond to Fl_FOCUS and UNFOCUS to get access to the FL_KEYs events
        case FL_FOCUS:
        case FL_UNFOCUS:
            if(HandleKeys)
                return 1;
            else
                return 0;

            case FL_KEYDOWN:
                if(HandleKeys)
                {
                    switch(Fl::event_key())
                    {
                        case '+':
                        case '+'+FL_KP:
                            CustomZoom+=0.1;
                            CustomZoom=fmin(CustomZoom, 1.2);
                            UpdateView();
                            //Event has been handled
                            return 1;

                        case '-':
                        case '-'+FL_KP:
                            CustomZoom-=0.1;
                            CustomZoom=fmax(CustomZoom, 0.5);
                            UpdateView();
                            //Event has been handled
                            return 1;
                    }
                }
                else
                    return 0;

        default:
            break;
   }

   return(Fl_Widget::handle(event));
}

Fl_ViewerCV::~Fl_ViewerCV()
{
}




