#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "Fl_OpenCV.h"



//   Fl_OpenCv


void Fl_OpenCV::draw(int xx, int yy, int ww, int hh)
{
    if(!FlImg)
        return;
    if(FlImg->w()<=0 || FlImg->h()<=0)
        return;

    FlImg->draw(xx, yy);
}


Fl_OpenCV::Fl_OpenCV(int X, int Y, float mzoom):Fl_Widget(X,Y,1,1,0)
{
  image=NULL;
  FlImg=NULL;
  fit=true;
  mouse_cb=NULL;
  x(X);y(Y);
  maxzoom=mzoom;
}

Fl_OpenCV::Fl_OpenCV(int X,int Y,int W,int H, float mzoom):Fl_Widget(X,Y,W,H,0)
{
  x(X);y(Y);w(W);h(H);
  image=NULL;
  FlImg=NULL;
  fit=true;
  mouse_cb=NULL;
  maxzoom=mzoom;
}


void Fl_OpenCV::SetImage(Mat *nimage)
{
    if(!nimage)
        return;

    image=nimage;

    if(image->cols<=0 || image->rows<=0) return;

    if(image && fit==false){
      w(image->cols);
      h(image->rows);
      resize(x(),y(),w(),h());
    }

    //Compute the scaling factor such the image will best fit in the widget area, keeping the original aspect ratio
    float fx, fy, scale;
    fx=(float)image->cols/w();
    fy=(float)image->rows/h();
    scale=max(fx,fy);
    //Impose a maximum zoom value (=>a minimum ratio value), defined by maxzoom
    scale=max(scale, maxzoom);


    //Prepare the image to draw
    fimage.release();
    fimage = cv::Mat::zeros(h(), w(), image->type()); //The final image (the image to draw) size is the widget size
    cv::resize((*image), fimage(Rect((int)((w()-cvFloor(image->cols/scale))/2.), (int)((h()-cvFloor(image->rows/scale))/2.), cvFloor(image->cols/scale), cvFloor(image->rows/scale))), Size(cvFloor(image->cols/scale), cvFloor(image->rows/scale)), 0, 0, INTER_CUBIC);


    //Check the image type for: converting from Opencv BGR (or BGRA if 4 channels) to RGB AND to make sure the image is encoded in unsigned chars
    //TODO: manage binary and gray images
    switch(image->type())
    {
        //3 channels (BGR):
        case 16:
        case 17:
            cvtColor(fimage, fimage, CV_BGR2RGB);
        break;

        //4 channels (BGRA):
        case 24:
        case 25:
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


/*void Fl_OpenCV::FitWindows()
{
   fit=true;
}*/

void Fl_OpenCV::SetMouseCallback(CvMouseCallback on_mouse, void * param)
{
  mouse_cb=on_mouse;

  cb_param=param;
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


int Fl_OpenCV::handle(int event)
{
//  Mouse Events
   if (event == FL_PUSH || event ==FL_DRAG || event == FL_MOVE || event == FL_RELEASE) {
     if(mouse_cb && image && (Fl::event_x()-x()) < image->cols && (Fl::event_y()-y()) <image->rows )
//     if(mouse_cb)
        mouse_cb(fl_event2cv_event(event), Fl::event_x()-x(), Fl::event_y()-y(), 0, cb_param);
   }
   return(Fl_Widget::handle(event));
}

Fl_OpenCV::~Fl_OpenCV()
{
}




