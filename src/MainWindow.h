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
/**
 * \file MainWindow.h
 * \brief Declaration file of the MainWindow class (managing the windows and widgets).
 * \author Vincent Crocher
 * \version 0.8
 * \date June 2014
 *
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_message.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Counter.H>


#include "Fl_Help_Window.h"
#include "Fl_OpenCV.h"
#include "Fl_Overlay_Arrow_Button.h"
#include "Preferences.h"
#include "Document.h"
#include "Document_pdf.h"
#include "Document_archive.h"
#include "Document_folder.h"
#include "Export_Document.h"

//Icon
#include "icon32.xbm"



//! The main window and all it widgets (buttons, sliders...). Also the main "document" of the program.
class MainWindow
{
    public:
        MainWindow(const char *exepath, const char *f=NULL);
        ~MainWindow();

        void Init(char *filename);


        friend void Window_cb(Fl_Widget* widget, void * param);                       //!< Window callback function (when closing)

        friend int DragNDrop(int e);                                                  //!< Handler of drag n drop events (when user dropped a file)
        friend void TimerDND_cb(void*);                                               //!< The timer callback function used to check the DND events
        friend int MouseWheelEvents(int e);                                           //!< Handler of the mouse wheel events
        friend void TimerMouseWheel_cb(void*);                                        //!< The timer callback function used to check the mouse wheel movements
        friend void ResizingEvents(Fl_Widget* widget, void*);

        //File menu
        friend void Open_cb(Fl_Widget* widget, void * param);
        friend void Quit_cb(Fl_Widget* widget, void * param);

        //View menu
        friend void Fullscreen_cb(Fl_Widget* widget, void * param);
        friend void PageMode_cb(Fl_Widget* widget, void * param);
        friend void StripMode_cb(Fl_Widget* widget, void * param);
        friend void CaseMode_cb(Fl_Widget* widget, void * param);
        friend void PagePreview_cb(Fl_Widget* widget, void * param);
        friend void Thumbnails_cb(Fl_Widget* widget, void * param);
        friend void UpdatePage(MainWindow* mw);
        friend void PreviousPage_cb(Fl_Widget* widget, void * param);
        friend void NextPage_cb(Fl_Widget* widget, void * param);
        friend void PreviousCase_cb(Fl_Widget* widget, void * param);
        friend void NextCase_cb(Fl_Widget* widget, void * param);
        friend void FirstPage_cb(Fl_Widget* widget, void * param);
        friend void LastPage_cb(Fl_Widget* widget, void * param);
        friend void GoToPage_cb(Fl_Widget* widget, void * param);
        friend void ExportDoc_cb(Fl_Widget* widget, void * param);

        //Help menu
        friend void Help_cb(Fl_Widget* widget, void * param);
        friend void About_cb(Fl_Widget* widget, void * param);

        //Info/status/update
        friend void UpdateWidgets(void * param);
        friend void ActivateNavigationWidgets(void *param, bool val);
        friend void AddRecentDocsItems(void *param);
        friend void StatusUpdate(void * param);
        friend void StatusBarUpdate(void * param);
        friend void StatusFullScreenUpdate(void * param);


    private:
        //FLTK param
            Fl_Double_Window *window;
            Fl_Help_Window *HelpDlg;
            Fl_Menu_Bar *MenuBar;
            Fl_Box *StatusBar;
            Fl_Group *DisplayGrp, *MainDisplayGrp, *ThumbDisplayGrp;
            Fl_OpenCV *Display, *ThumbDisplayCurrent, *ThumbDisplayPrev, *ThumbDisplayNext;
            Fl_Button *PrevCloseButton;
            Fl_Overlay_Arrow_Button *NextButton, *PreviousButton, *NextPageButton, *PreviousPageButton;
            Fl_Group *PageNbFullGrp;
            Fl_Progress * PageNbFullProgress;               //!< The fullscreen page number indicator

        //Other param
            char ExePath[FL_PATH_MAX];     //!< Executable path, used to store preferences file on Windows
            char Filename[FL_PATH_MAX];    //!< Local copy of the filename of the currently open document
            Preferences *Pref;              //!< User global preferences (last one used and saved)
            Preferences *PrefDoc;           //!< User preferences for this document
            Document *Doc;                  //!< Pdf document
};


#endif // MAINWINDOW_H
