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
#include <algorithm>




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
    //Nothing to display yet (events can arrive while a document is loading)
    if(!image || image->empty())
        return;

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


    //Real area to draw: centered on the DisplayRect but can be larger.
    //Only the visible part of the page is scaled: scaling the whole page first used up to gigabytes
    //with large pages or long strips (memory proportional to the page size and zoom).
        bool panel_view=(localDisplayRect.width!=image->cols);
        //Size of the whole page once scaled
        int scaled_w=std::max(1, cvFloor(image->cols/Scale)), scaled_h=std::max(1, cvFloor(image->rows/Scale));

        //Visible area, in scaled page coordinates
        Rect visible;
        if(!InContext && panel_view)
        {
            //The panel alone
            visible=Rect(cvFloor(localDisplayRect.x/Scale), cvFloor(localDisplayRect.y/Scale), cvFloor(localDisplayRect.width/Scale), cvFloor(localDisplayRect.height/Scale));
        }
        else
        {
            //A widget sized window centered on the panel, kept inside the page
            int visible_w=std::min(w(), scaled_w), visible_h=std::min(h(), scaled_h);
            int x_center=cvRound((localDisplayRect.x+localDisplayRect.width/2.)/Scale);
            int y_center=cvRound((localDisplayRect.y+localDisplayRect.height/2.)/Scale);
            visible=Rect(std::clamp(x_center-w()/2, 0, scaled_w-visible_w), std::clamp(y_center-h()/2, 0, scaled_h-visible_h), visible_w, visible_h);
        }
        visible&=Rect(0, 0, scaled_w, scaled_h);
        visible.width=std::min(visible.width, w());
        visible.height=std::min(visible.height, h());
        if(visible.width<1 || visible.height<1)
            visible=Rect(0, 0, std::min(w(), scaled_w), std::min(h(), scaled_h));

        //Corresponding area of the original page, scaled to the visible size
        Rect source(cvFloor(visible.x*Scale), cvFloor(visible.y*Scale), cvCeil(visible.width*Scale), cvCeil(visible.height*Scale));
        source&=Rect(0, 0, image->cols, image->rows);
        Mat image_roi;
        cv::resize((*image)(source), image_roi, visible.size(), 0, 0, Scale>1 ? INTER_AREA : INTER_CUBIC);

        //Add a visual border around the current panel
        if(panel_view && Border && InContext)
        {
            //Opaque dark frame (theme bars color) with a thin accent line against the panel (BGR)
            Rect panel(cvFloor(localDisplayRect.x/Scale)-visible.x, cvFloor(localDisplayRect.y/Scale)-visible.y, cvFloor(localDisplayRect.width/Scale), cvFloor(localDisplayRect.height/Scale));
            cv::rectangle(image_roi, Rect(panel.x-7, panel.y-7, panel.width+14, panel.height+14), Scalar(24, 24, 24, 255), 9);
            cv::rectangle(image_roi, Rect(panel.x-2, panel.y-2, panel.width+4, panel.height+4), Scalar(212, 120, 0, 255), 2);
        }

        //Centered if smaller than the widget
        int xd0=std::max(0, (w()-image_roi.cols)/2), yd0=std::max(0, (h()-image_roi.rows)/2);

        //Background around the image: the widget color (BGR)
        unsigned char bg_r, bg_g, bg_b;
        Fl::get_color(color(), bg_r, bg_g, bg_b);
        fimage=Mat(h(), w(), image_roi.type(), Scalar(bg_b, bg_g, bg_r, 255));
        image_roi.copyTo(fimage(Rect(xd0, yd0, image_roi.cols, image_roi.rows)));


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
        cv::resize(fimage(to_magnify), magnifier, Size(), zoom, zoom, INTER_CUBIC);

        //Draw a frame on it
        cv::rectangle(magnifier, Rect(1, 1, magnifier.cols-2, magnifier.rows-2), Scalar(212, 120, 0), 3); //Accent blue (BGR)

        //Place it on current image
        Rect dest_magnified=to_magnify;
        dest_magnified.x-=fimage.cols*rect_size_ratio/2.;dest_magnified.x=fmax(dest_magnified.x, 0);
        dest_magnified.y-=fimage.rows*rect_size_ratio/2.;dest_magnified.y=fmax(dest_magnified.y, 0);
        dest_magnified.width*=zoom;dest_magnified.width=fmin(dest_magnified.width, fimage.cols-dest_magnified.x);
        dest_magnified.height*=zoom;dest_magnified.height=fmin(dest_magnified.height, fimage.rows-dest_magnified.y);
        magnifier(Rect(0, 0, dest_magnified.width, dest_magnified.height)).copyTo(fimage(dest_magnified));
    }


    //Check the image type to: convert from Opencv BGR (or BGRA if 4 channels) to RGB AND to make sure the image is encoded in unsigned chars
    switch(image->channels())
    {
        //1 channel (GRAY):
        case 1:
            cvtColor(fimage, fimage, COLOR_GRAY2RGB);
        break;

        //3 channels (BGR):
        case 3:
            cvtColor(fimage, fimage, COLOR_BGR2RGB);
        break;

        //4 channels (BGRA):
        case 4:
            cvtColor(fimage, fimage, COLOR_BGRA2RGB);
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


int Fl_ViewerCV::handle(int event)
{
   if(Fl_Widget::active())
   {
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




