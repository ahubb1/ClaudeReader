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
  Resizing=false;
  Magnifier=false;
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
    Rect localDisplayRect;
    //If there is NO display rect
    if( !(DisplayRect.width>0 && DisplayRect.height>0 && DisplayRect.x>=0 && DisplayRect.y>=0) )
    {
        //Then set it to the whole page
        localDisplayRect.width=image->cols;
        localDisplayRect.height=image->rows;
        localDisplayRect.x=0;
        localDisplayRect.y=0;
    }
    else
    {
        localDisplayRect=DisplayRect;
    }

    //Compute the scaling factor such the image will best fit in the widget area, keeping the original aspect ratio
    float fx, fy;
    fx=(float)localDisplayRect.width/w();
    fy=(float)localDisplayRect.height/h();
    Scale=max(fx,fy);

    //Apply custom zoom if in Panel only (not when strip: page width or whole page: page width) and In context
    if(localDisplayRect.width!=image->cols && InContext)
        Scale/=CustomZoom;

    //Impose a maximum zoom value (=>a minimum ratio value), defined by maxzoom
    Scale=fmax(Scale, 1./MaxZoom);


    //Real area to draw: centered on the DisplayRect but can be larger
        //Scale the original image to it's new size
        Mat image_roi;
        cv::resize(*image, image_roi, Size(cvFloor(image->cols/Scale), cvFloor(image->rows/Scale)), 0, 0, INTER_CUBIC);

        //Add a visual border around the current panel
        if(localDisplayRect.width!=image->cols && Border && InContext)
        {
            Mat image_roi_rect;
            image_roi.copyTo(image_roi_rect);
            for (int i=1; i<12; i++)
                cv::rectangle(image_roi_rect, Rect(localDisplayRect.x/Scale-i, localDisplayRect.y/Scale-i, localDisplayRect.width/Scale+2*i, localDisplayRect.height/Scale+2*i), Scalar(0,0,22*i), 1);
            addWeighted(image_roi_rect, 0.5, image_roi, 0.5, 0.0, image_roi);
        }


        //Center of the display in resized image
        int x_center=(localDisplayRect.x+localDisplayRect.width/2.)/Scale;
        int y_center=(localDisplayRect.y+localDisplayRect.height/2.)/Scale;

        //Display in context or not
        if(!InContext && localDisplayRect.width!=image->cols)
        {
            /*if(ShowSuroundings)
            {
                int x0=fmax(0, x_center-w()/2.);
                int y0=fmax(0, y_center-h()/2.);
                int x1=fmin(100000, x_center+w()/2.);
                int y1=fmin(100000, y_center+h()/2.);
                image_roi(Rect(x0, y0, x1-x0, y1-y0)).copyTo(image_roi);
                imwrite("truc.tiff", image_roi);
            }
            else*/
                image_roi(Rect(localDisplayRect.x/Scale, localDisplayRect.y/Scale, localDisplayRect.width/Scale, localDisplayRect.height/Scale)).copyTo(image_roi);
        }


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

        fimage=Mat::zeros(h(), w(), image_roi.type());
        //fimage=fimage+Scalar(255,255,255); //For white background
        Mat fimageROI=fimage(Rect(xd0, yd0, (x1-x0), (y1-y0)));
        image_roi(Rect(x0, y0, (x1-x0), (y1-y0))).copyTo(fimageROI);


    //Apply and draw a magnifier rect
    if(Magnifier)
    {
        //Get mouse coord and transform in image space
        int x=fmin(Fl::event_x()-Fl_Widget::x(), Fl_Widget::w()), y=fmin(Fl::event_y()-Fl_Widget::y(), Fl_Widget::h());

        //Extract ROI around coord of 1/6th of image
        float rect_size_ratio=0.20;
        float zoom=2.0;
        Mat magnifier;
        Rect to_magnify;
        to_magnify.x=fmax(x-fimage.cols*rect_size_ratio/2., 0);
        to_magnify.y=fmax(y-fimage.rows*rect_size_ratio/2., 0);
        to_magnify.width=fmin(fimage.cols*rect_size_ratio, fimage.cols-to_magnify.x);
        to_magnify.height=fmin(fimage.rows*rect_size_ratio, fimage.rows-to_magnify.y) ;

        //Zoom on it: multiply by 2
        cv::resize(fimage(to_magnify), magnifier, Size(), zoom, zoom, CV_INTER_CUBIC);

        //Draw a frame on it
        cv::rectangle(magnifier, Rect(1, 1, magnifier.cols-2, magnifier.rows-2), Scalar(0, 0, 150), 3);

        //Place it on current image
        Rect dest_magnified=to_magnify;
        dest_magnified.x-=fimage.cols*rect_size_ratio/2.;dest_magnified.x=fmax(dest_magnified.x, 0);
        dest_magnified.y-=fimage.rows*rect_size_ratio/2.;dest_magnified.y=fmax(dest_magnified.y, 0);
        dest_magnified.width*=zoom;dest_magnified.width=fmin(dest_magnified.width, fimage.cols-dest_magnified.x);
        dest_magnified.height*=zoom;dest_magnified.height=fmin(dest_magnified.height, fimage.rows-dest_magnified.y);
        magnifier(Rect(0, 0, dest_magnified.width, dest_magnified.height)).copyTo(fimage(dest_magnified));
    }


    //Check the image type for: converting from Opencv BGR (or BGRA if 4 channels) to RGB AND to make sure the image is encoded in unsigned chars
    switch(image->channels())
    {
        //1 channel (GRAY):
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
        //Bound display rect to image size
        disprect.x=fmax(0, disprect.x);
        disprect.y=fmax(0, disprect.y);
        int max_rect_w=image->cols-disprect.x;
        int max_rect_h=image->rows-disprect.y;
        disprect.width=fmin(max_rect_w, disprect.width);
        disprect.height=fmin(max_rect_h, disprect.height);

        //Turn off magnifier
        Magnifier=false;

        //Apply and update the view
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

    //Mechanism to truly resize (and thus panel detection...) only
    //when resizing is "over"
    if(widget->Resizing)
    {
        widget->Resizing=false;
        Fl::repeat_timeout(0.05, ResizingTimer_cb, (void*) widget);
    }
    else
    {
        if(widget->Resizing_cb!=NULL)
            widget->Resizing_cb(widget, widget->Resizing_cb_param);
        widget->SetImage(widget->image, widget->DisplayRect);
    }
}

//When the widget is resized, re-compute the image to draw according to new dimensions
void Fl_ViewerCV::resize(int xx, int yy, int ww, int hh)
{
    //Only if needed
    if(xx!=x() || yy!=y() || ww!=w() || hh!=h())
    {
        //Apply new size
        x(xx);y(yy);w(ww);h(hh);

        //On windows the following is done later on: the system seems to wait for this function to return
        /*#ifndef __WINDOWS__
            if(Resizing_cb!=NULL)
                Resizing_cb(this, Resizing_cb_param);
            SetImage(image, DisplayRect);
        #else*/
            //Fl::add_timeout(0.02, ResizingTimer_cb, (void*)this);
            Resizing=true;
            Fl::add_timeout(0.05, ResizingTimer_cb, (void*)this);
        //#endif
    }
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
   if(Fl_Widget::active())
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
                else
                {
                    //Same behavior as space bar: panel temporary zoom out
                    if(HandleKeys && Fl::event_button()==FL_RIGHT_MOUSE)
                    {
                        if(!ShowPreview && !Magnifier)
                        {
                            ShowPreview=true;
                            //Remember original state
                            RmbInContext=InContext;
                            RmbCustomZoom=CustomZoom;
                            //Force large context
                            InContext=true;
                            CustomZoom=0.5;
                            UpdateView();
                        }
                        //Event has been handled
                        return 1;
                    }

                    //Magnifier
                    if(Fl::event_button()==FL_LEFT_MOUSE)
                    {
                        if(!ShowPreview)
                        {
                            Magnifier=true;
                            UpdateView();
                        }
                        //Event has been handled
                        return 1;
                    }
                }
                break;

            case FL_DRAG:
                 //Magnifier
                if(Fl::event_button()==FL_LEFT_MOUSE)
                {
                    if(Magnifier && !ShowPreview)
                    {
                        UpdateView();
                    }
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
                else
                {
                    //Same behavior as space bar: panel temporary zoom out
                    if(HandleKeys && Fl::event_button()==FL_RIGHT_MOUSE)
                    {
                        if(ShowPreview)
                        {
                            ShowPreview=false;
                            //Reestablish original values
                            InContext=RmbInContext;
                            CustomZoom=RmbCustomZoom;
                            UpdateView();
                        }
                        //Event has been handled
                        return 1;
                    }

                    //Magnifier
                    if(Fl::event_button()==FL_LEFT_MOUSE)
                    {
                        if(Magnifier)
                        {
                            Magnifier=false;
                            UpdateView();
                        }
                        //Event has been handled
                        return 1;
                    }
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
                            case 32:
                                //Panel temporary zoom out
                                if(!ShowPreview)
                                {
                                    ShowPreview=true;
                                    //Remember original state
                                    RmbInContext=InContext;
                                    RmbCustomZoom=CustomZoom;
                                    //Force large context
                                    InContext=true;
                                    CustomZoom=0.5;
                                    UpdateView();
                                }
                                //Event has been handled
                                return 1;

                            default:
                                break;
                        }
                    }
                    else
                        return 0;
                    break;

                case FL_KEYUP:
                    if(HandleKeys)
                    {
                        switch(Fl::event_key())
                        {
                            case 32:
                                //Panel temporary zoom out
                                Fl::wait(0.1);
                                if(ShowPreview && !Fl::event_key(32)) //wait() and test of actual state of key are needed in X which is sending periodic KEY_UP events when held down...
                                {
                                    ShowPreview=false;
                                    //Reestablish original values
                                    InContext=RmbInContext;
                                    CustomZoom=RmbCustomZoom;
                                    UpdateView();
                                }
                                //Event has been handled
                                return 1;

                            default:
                                break;
                        }
                    }
                    else
                        return 0;
                    break;

            default:
                break;
       }
   }

   return(Fl_Widget::handle(event));
}

Fl_ViewerCV::~Fl_ViewerCV()
{
}




