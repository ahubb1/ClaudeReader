/**
 * \file MainWindow.h
 * \brief Declaration file of the MainWindow class (managing the windows and widgets).
 * \author Vincent Crocher
 * \version 0.1
 * \date April 2014
 *
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_message.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Help_Dialog.H>
/*#include <FL/Fl_Pack.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>*/

#include "Fl_OpenCV.h"
#include "Preferences.h"
#include "Document.h"

//Icon
#include "icon32.xbm"



//! The main window and all it widgets (buttons, sliders...). Also the main "document" of the program.
class MainWindow
{
    public:
        MainWindow();                                                                   //!< Constructor
        ~MainWindow();                                                                  //!< Destructor

        void Init(char *filename);


        friend void Window_cb(Fl_Widget* widget, void * param);                       //!< Window callback function (when closing)

        friend int MouseWheelEvents(int p);                                           //!< Handler of the mouse wheel events
        friend void Timer_cb(void*);                                                  //!< The timer callback function used to check the mouse wheel movements

        //File menu
        friend void Open_cb(Fl_Widget* widget, void * param);
        friend void Quit_cb(Fl_Widget* widget, void * param);

        //View menu
        friend void Fullscreen_cb(Fl_Widget* widget, void * param);
        friend void PageMode_cb(Fl_Widget* widget, void * param);
        friend void StripMode_cb(Fl_Widget* widget, void * param);
        friend void CaseMode_cb(Fl_Widget* widget, void * param);
        friend void PagePreview_cb(Fl_Widget* widget, void * param);
        friend void PreviousPage_cb(Fl_Widget* widget, void * param);
        friend void NextPage_cb(Fl_Widget* widget, void * param);
        friend void PreviousCase_cb(Fl_Widget* widget, void * param);
        friend void NextCase_cb(Fl_Widget* widget, void * param);

        //Help menu
        friend void Help_cb(Fl_Widget* widget, void * param);
        friend void About_cb(Fl_Widget* widget, void * param);


        //Info/status/update
        friend void UpdateWidgets(void * param);
        friend void ActivateMenus(void * param);
        friend void ActivateNavigationMenus(void *param, bool val);

        friend void StatusBarUpdate(void * param);


    private:
        //FLTK param
            Fl_Double_Window *window;

            Fl_Menu_Bar *MenuBar;
            Fl_Box *StatusBar;
            Fl_OpenCV *Display;


        //Other param
            char Filename[1024];   //!< Local copy of the filename of the currently open pdf
            Preferences *Pref;      //!< User preferences
            Document *Doc;          //!< Pdf document
};


#endif // MAINWINDOW_H
