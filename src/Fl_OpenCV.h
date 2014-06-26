/*
Fl_OpenCv 0.9
License : LGPL
Fl_OpenCv is a widget which is render OpenCv image within the FLTK window
Programmed by BlueKid
http://derindelimavi.blogspot.com/
Sent me any suggestion, modification or bugs. Don't hesitate to contact
me for any question, I will be very grateful with your feedbacks.
bluekid70@gmail.com
Modified by Vincent Crocher - 2014
*/

#ifndef Fl_OpenCV_H
#define Fl_OpenCV_H

#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <opencv2/opencv.hpp>

using namespace cv;


IplImage *fl_LoadImage(char*filename,int iscolor);
bool fl_SaveImage(char*filename,IplImage *image);

//Convert the fltk mouse event into opencv ones (to be able to use open cv callback functions and events)
int fl_event2cv_event(int fl_event);



class FL_EXPORT Fl_OpenCV : public Fl_Widget
{
  int       _x,_y,_w,_h;

protected:
  //Draw the image
  void draw(int,int,int,int);
  void draw(){
      draw(Fl_Widget::x(),Fl_Widget::y(),w(),h());
  }

  void x(int X){ _x=X;};
  void y(int Y){ _y=Y;};
  void w(int W){ _w=W;};
  void h(int H){ _h=H;};

  //Mat fimage;
  int iscolor;
  CvMouseCallback mouse_cb;
  void *cb_param; //Vincent: a parameter for the callback function
  bool fit;
  float maxzoom;
  void (*Resizing_cb)(void *);
  void *Resizing_cb_param;

public:
  //Assign a new image to draw (in Mat Opencv format)
  void SetImage(Mat *nimage);
  //void FitWindows();
  Fl_OpenCV(int X, int Y, float mzoom=0.5);
  Fl_OpenCV(int X,int Y,int W,int H, float mzoom=0.5);
  ~Fl_OpenCV();

  void SetMouseCallback(CvMouseCallback on_mouse, void * param=0);
  int handle(int event);

  //When the widget is resized, re-compute the image to draw according to new dimensions
  void resize(int x, int y, int ww, int hh){
      w(ww);
      h(hh);
      Fl_Widget::resize(x, y, ww, hh);
      if(Resizing_cb!=NULL)
        Resizing_cb(Resizing_cb_param);
      SetImage(image);
  }

  void SetResizing_cb(void (*resizing_cb)(void *), void * param=NULL)
  {
      Resizing_cb=resizing_cb;
      Resizing_cb_param=param;
  }

  int x(){return _x;};
  int y(){return _y;};
  int w(){return _w;};
  int h(){return _h;};

  Mat *image, fimage;
  Fl_RGB_Image *FlImg;
};

#endif


