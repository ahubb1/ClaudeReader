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



class FL_EXPORT Fl_OpenCV : public Fl_Widget
{

protected:
  //Draw the image
  void draw(int,int,int,int);
  void draw(){
      draw(Fl_Widget::x(),Fl_Widget::y(),w(),h());
  }

  //Mat fimage;
  int iscolor;
  bool fit;
  float maxzoom;
  //Callbacks
  void (*Resizing_cb)(Fl_Widget *, void *);
  void *Resizing_cb_param;
  void (*OnClick_cb)(Fl_Widget *, void *);
  void *OnClick_cb_param;
  void (*OnRelease_cb)(Fl_Widget *, void *);
  void *OnRelease_cb_param;
  CvMouseCallback mouse_cb;
  void *CVcb_param;

public:
  //Assign a new image to draw (in Mat Opencv format)
  void SetImage(Mat *nimage);
  //void FitWindows();
  Fl_OpenCV(int X, int Y, float mzoom=0.5);
  Fl_OpenCV(int X,int Y,int W,int H, float mzoom=0.5);
  ~Fl_OpenCV();

  int handle(int event);
  void resize(int x, int y, int ww, int hh);
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

  Mat *image, fimage;
  Fl_RGB_Image *FlImg;
};

#endif


