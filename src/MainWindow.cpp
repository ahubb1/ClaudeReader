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
#include "MainWindow.h"



//Help and about window strings
#include "help.h"
#include "about.h"


//To store the mouse wheel movements
int mouseWheel=0;
//To store the DragNDrop state
bool isDNDavailableTmp=false;
bool isDNDavailable=false;
char DNDFilename[FL_PATH_MAX];


//-------------------------------- Events ---------------------------------
//! Catch the Drag N Drop events (if the user drops a file on the window)
//!This function only raise a flag that will be catched later on by the TimerDND_cb() function.
int DragNDrop(int e)
{
     //Select the DND related events
     switch(e)
     {
         case FL_DND_ENTER:
         case FL_DND_DRAG:
            //Tell the system we are interested by DND events
            return 1;
            break;

         case FL_DND_RELEASE:
         case FL_PASTE: //FL_PASTE never occurs since DND events cannot be used in widgets derived from Fl_Group or Fl_Window=> so need a trick
            //Once something has been "received" through dnd, raise a flag
            isDNDavailableTmp=true;
            return 1;
            break;

         //After a DND_RELEASE occured, we need to catch any event untill the event_text() is updated
         default:
            if(isDNDavailableTmp && strlen(Fl::event_text())==Fl::event_length())
            {
                //Get the content of the dropped object (=>the filename)
                strncpy(DNDFilename, Fl::event_text(), Fl::event_length());
                //Add a terminating character (needed on Windows)
                DNDFilename[Fl::event_length()]='\0';
                //Reset the tmp flag
                isDNDavailableTmp=false;
                //Raise the final flag (i.e. txt is arrived)
                isDNDavailable=true;
            }
            return 0;
     }
 }
//! The timer callback function used to check the DND events.
//!If a DragNdrop event has been detected this function proceeds the text associated to it and call
//!the MainWindow::Init() method to open the dropped file (if it is a file). Note that if several
//!objects are dropped together, only the first one will be open.
void TimerDND_cb(void* param)
{
    MainWindow *mw=(MainWindow*)param;

    //If something has been received
    if(isDNDavailable)
    {
        //No more DND available
        isDNDavailable=false;
        //Decode the filename
        fl_decode_uri(DNDFilename);
        //Replace the \n by a \0 (by the way, select only the first file (corresponding to the first line) if several are dropped at the same time)
        char *c = strchr(DNDFilename,'\n');
        if(c)
            *c='\0';
        //Check if it's a locale file
        #ifndef __WINDOWS__ //On linux a locale file uri begin with "file://"
            if(strncmp(DNDFilename, "file://", 7) != 0)
            {
                fl_alert("Invalid filename !");
            }
            else
            {
                //Try to open it (without the 7 first characters: "file://")
                mw->Init(DNDFilename+7);
            }
        #else   //On Windows, check if it's begin with a disk letter (A->Z, other ones (AA...) are not managed)
            if( (DNDFilename[0]>=65 && DNDFilename[0]<=90) &&  DNDFilename[1]==':' )
            {
                //Try to open it
                mw->Init(DNDFilename);
            }
            else
            {
                fl_alert("Invalid filename !");
            }
        #endif
    }
    //Repeat...
    Fl::repeat_timeout(0.01, TimerDND_cb, (void*)mw);
}

//! Catch the mouse wheel events
int MouseWheelEvents(int p)
{
    if(Fl::event()==FL_MOUSEWHEEL)
    {
        //Just add the movement to the global memory of mouse wheel movements
        mouseWheel+=Fl::event_dy();
        //it should be catched by the timer later on
        //Say fltk the event as been managed
        return 1;
    }
    else
    {
        //say fltk that the event has not been managed
        return 0;
    }
}
//! The timer callback function used to check the mouse wheel movements
void TimerMouseWheel_cb(void* param)
{
    MainWindow *mw=(MainWindow*)param;

    //If some movement has been detected
    if(mw->Doc && mouseWheel!=0)
    {
        //If the mouse wheel has been done on the thumbnail view
        bool page=false;
        if(mw->ThumbDisplayGrp->visible() && Fl::event_inside(mw->ThumbDisplayGrp)!=0)
        {
            //then switch page-to-page
            page=true;
        }

        //if positive, go to next case
        if(mouseWheel>0)
        {
            Fl_Menu_Item* nextcb;
            if(page)
                nextcb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Next page");
            else
               nextcb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Next panel");

            if(nextcb && nextcb->active())
                nextcb->do_callback((Fl_Widget*)mw->MenuBar);
        }
        //if negative, go to previous case
        else
        {
            Fl_Menu_Item* previouscb;
            if(page)
                previouscb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Previous page");
            else
                previouscb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Previous panel");

            if(previouscb && previouscb->active())
                previouscb->do_callback((Fl_Widget*)mw->MenuBar);
        }
        //flush
        mouseWheel=0;
    }

    Fl::wait(50);
    //Repeat...
    Fl::repeat_timeout(0.1, TimerMouseWheel_cb, (void*)mw);
}

//! Catch the resizing events and update the Display aspect ratio accordingly to new size
void ResizingEvents(Fl_Widget* widget, void* param)
{
    MainWindow *mw=(MainWindow*)param;


    //If in STRIP MODE: recall the Detection (through the SetDisplayRatio) to adjust the size of the strip sto the window width
    if(mw->Doc)
    {
        //Adjust the strip size according to the new display ratio
        mw->Doc->SetDisplayRatio(mw->Display->w()/(float)mw->Display->h());

        //If needed
        if( mw->Doc->GetReadingMode()==STRIP || (mw->Doc->GetReadingMode()==CASE && mw->Doc->currentPage->HasDetectionFailed()) )
        {
            //And re-render
            mw->Doc->Render();
            StatusUpdate(mw);

            //Give some time to fltk
            Fl::check();
        }
        if(mw->Doc->GetReadingMode()==STRIP || mw->Doc->GetReadingMode()==CASE)
        {
            //Update the preloaded pages
            mw->Doc->UpdatePreloadedPages();
        }
    }
}
//-----------------------------------------------------------------------------


//--------------------------------- Status ----------------------------------
//! Update the status bar text according to current state (document current page...)
void StatusBarUpdate(void * param)
{
      MainWindow *mw=(MainWindow*)param;

      //Get number of pages... blabla
      char text[1024];
      if(mw->Doc)
      {
          if(mw->Doc->GetReadingMode()==CASE)
          {
                sprintf(text, "PANEL\t\tPage %d/%d (Panel %d/%d)", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNbOfPages(), mw->Doc->currentPage->GetActiveCase()+1, mw->Doc->currentPage->GetNbCases());
          }
          else
          {
                if(mw->Doc->GetReadingMode()==STRIP)
                {
                     sprintf(text, "STRIP\t\t\t");
                }
                else
                {
                     sprintf(text, "PAGE\t\t");
                }

                sprintf(text, "%sPage %d/%d", text , mw->Doc->GetCurrentPageNb(), mw->Doc->GetNbOfPages());
          }
      }
      else
      {
          sprintf(text, "%s", "\0");
      }
      mw->StatusBar->copy_label(text);
      mw->StatusBar->redraw();
}
//! Update the fullscreen status
void StatusFullScreenUpdate(void * param)
{
     MainWindow *mw=(MainWindow*)param;

     if(mw->Doc)
     {
        //Update the dial
        mw->PageNbDial->range(1, mw->Doc->GetNbOfPages());
        mw->PageNbDial->value(mw->Doc->GetCurrentPageNb());
        //and the tooltip text
        char txt[16];
        sprintf(txt, "Page %d/%d", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNbOfPages());
        mw->PageNbDial->copy_tooltip(txt);
        mw->PageNbDial->redraw();
     }
}
//! Call both the status bar update and the fullscreen status update
void StatusUpdate(void * param)
{
    StatusBarUpdate(param);
    StatusFullScreenUpdate(param);
    Fl::wait(50);
}
//-----------------------------------------------------------------------------


//---------------------------------File menu----------------------------------
//! Open a document and load it (through a call to Init())
void Open_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    char tmpFilename[FL_PATH_MAX];

    //Retrieve the menu item value to know if it's an open or a recent document item
    //if it's an open, do the stuff
    if(strstr(((Fl_Menu_*)widget)->mvalue()->label(), "Open")!=NULL)
    {
        //Window to define a directory and filename
        Fl_Native_File_Chooser chooser=Fl_Native_File_Chooser();
        chooser.title("Open a file");
        chooser.filter("Supported formats\t*.{pdf,cbz,cbr,cbt,cb7,rar,zip,tar,7z}\nPdf\t*.pdf\nComics archive\t*.{cbz,cbr,cbt,cb7}\nArchive\t*.{rar,zip,tar,7z}");
        chooser.options(Fl_Native_File_Chooser::PREVIEW);

        //Select either open a file or a folder depending on the calling widget label
        if(strstr(((Fl_Menu_*)widget)->mvalue()->label(), "folder")!=NULL)
            chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
        else
            chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);

        //Select the working directory
        char folder[FL_PATH_MAX];
        //if a current doc is open
        if(mw->Doc)
        {
            //use this directory
            int last_separator=max((strrchr(mw->Doc->GetFilename(), '\/')-mw->Doc->GetFilename()), (strrchr(mw->Doc->GetFilename(), '\\')-mw->Doc->GetFilename()));
            strncpy(folder, mw->Doc->GetFilename(), last_separator);
            chooser.directory(folder);
        }
        else
        {
            //if lastdoc information is available
            if(mw->Pref && mw->Pref->GetLastDoc()[0]!='\0')
            {
                //use the last doc directory
                int last_separator=max((strrchr(mw->Pref->GetLastDoc(), '\/')-mw->Pref->GetLastDoc()), (strrchr(mw->Pref->GetLastDoc(), '\\')-mw->Pref->GetLastDoc()));
                strncpy(folder, mw->Pref->GetLastDoc(), last_separator);
                chooser.directory(folder);
            }
        }

        //Show the chooser window
        int ret=chooser.show();
        if(ret !=1 && ret!=-1)
        {
            sprintf(tmpFilename, "%s", chooser.filename());
        }
        else
        {
            return;
        }
    }
    //if it's a recent document item
    else
    {
        //just use the name as filename
        sprintf(tmpFilename, "%s", ((Fl_Menu_*)widget)->mvalue()->label());
        if( tmpFilename[0]=='\0' ||
           ( strstr(tmpFilename, ".pdf")==NULL &&
             strstr(tmpFilename, ".cbz")==NULL && strstr(tmpFilename, ".cbr")==NULL && strstr(tmpFilename, ".cbt")==NULL && strstr(tmpFilename, ".cb7")==NULL &&
             strstr(tmpFilename, ".rar")==NULL && strstr(tmpFilename, ".zip")==NULL && strstr(tmpFilename, ".tar")==NULL && strstr(tmpFilename, ".7z")==NULL  &&
             fl_filename_isdir(tmpFilename)==0
           ))
        {
            fl_alert("Invalid filename !");
            return;
        }
    }

    //Open file and do stuff
    mw->Init(tmpFilename);
}

//! Export cancel button callback
void ExportCancel_cb(Fl_Widget* widget, void* param)
{
    Export_Document* exportdoc=(Export_Document*)param;

    //Give cancel signal to export doc
    exportdoc->Cancel();
}
//! Export the current document (as an images folder)
void ExportDoc_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //If a document is open
    if(mw->Doc!=NULL)
    {
        //Remember the document current page

        //Open a file chooser
        Fl_Native_File_Chooser chooser=Fl_Native_File_Chooser();
        chooser.title("Select a folder to export to");
        //only directory selection
        chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
        //can create a new one
        chooser.options(Fl_Native_File_Chooser:: NEW_FOLDER);
        //in the current doc directory
        char folder[FL_PATH_MAX];
        int last_separator=max((strrchr(mw->Doc->GetFilename(), '\/')-mw->Doc->GetFilename()), (strrchr(mw->Doc->GetFilename(), '\\')-mw->Doc->GetFilename()));
        strncpy(folder, mw->Doc->GetFilename(), last_separator);
        chooser.directory(folder);
        int ret=chooser.show();
        if(ret !=1 && ret!=-1)
        {
            //Ask if export panels or pages
            bool panel, preview;
            fl_message_title("Export");
            int panelchoice=fl_choice("Export as ?\n -One image for each Page\n -One image for each Panel\n -One image for each Panel+Page preview", "Panel+Preview", "Panel", "Page");
            switch(panelchoice)
            {
                case 0:
                    panel=true;
                    preview=true;
                    break;
                case 1:
                    panel=true;
                    preview=false;
                    break;
                case 2:
                    panel=false;
                    preview=false;
                    break;
                default:
                    panel=false;
                    preview=false;
            }

            //Select the image format
            char fileext[5];
            fl_message_title("Export");
            int formatchoice=fl_choice("Select an image format to export to:", "Jpeg", "Png", "Bmp");
            switch(formatchoice)
            {
                case 0:
                    sprintf(fileext, ".jpg");
                    break;
                case 1:
                    sprintf(fileext, ".png");
                    break;
                case 2:
                    sprintf(fileext, ".bmp");
                    break;
                default:
                    sprintf(fileext, ".jpg\0");
            }

            //Ask for confirmation first
            fl_message_title("Export");
            if(fl_ask("Export %s to %s as images (%s) ?", fl_filename_name(mw->Doc->GetFilename()), chooser.filename(), fileext)==0)
                return;

            //Go to document first page
            FirstPage_cb(NULL, mw);

            //Create an export object
            Export_Document exportdoc(mw->Doc, chooser.filename(), fileext, panel, preview);

            //Disable the window (to avoid any interaction)
            mw->window->hide();

            //Create a progress window
            int ww=fmax(500., mw->window->w()/1.5);
            Fl_Window ProgressWindow(mw->window->x()+mw->window->w()/2.-ww/2., mw->window->y()+mw->window->h()/2.-100, ww, 200, "Export in progress...");
            ProgressWindow.set_modal();
            ProgressWindow.begin();
                Fl_Button fileLabel(20, 10, (ww-40), 80, "Exporting ");
                char label[255];
                sprintf(label, "Exporting: %s\nTo: %s\nFormat:\t%s\n\n", fl_filename_name(mw->Doc->GetFilename()), chooser.filename(), fileext);
                fileLabel.label(label);
                fileLabel.box(FL_FLAT_BOX);
                fileLabel.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                Fl_Progress *ProgressBar;
                ProgressBar=new Fl_Progress(10, fileLabel.y()+fileLabel.h()+10, ProgressWindow.w()-20, 20, "Page ");
                ProgressBar->minimum(0);
                ProgressBar->maximum(mw->Doc->GetNbOfPages());
                ProgressBar->value(0);
                Fl_Button cancelButton((ww-70)/2., 150, 70, 30, "Cancel");
                cancelButton.take_focus();
                cancelButton.callback(ExportCancel_cb, (void*)&exportdoc);
            ProgressWindow.end();
            ProgressWindow.show();

            //Export
            int retexp=exportdoc.Export(ProgressBar);
            if(retexp>=0)
            {
                fl_message_title("Export");
                fl_alert("Export succesfull (%d images created) !", retexp);
            }
            else
            {
                switch(retexp)
                {
                    case -1:
                        fl_message_title("Export");
                        fl_alert("Error: Invalid export path (%s) !", chooser.filename());
                        break;
                    case -2:
                        fl_message_title("Export");
                        fl_alert("Error: Image format not supported (%s) !", fileext);
                        break;
                    default:
                        fl_message_title("Export");
                        fl_alert("Error while exporting !");
                        break;
                }
            }
            ProgressWindow.hide();

            //Go to document first page
            FirstPage_cb(NULL, mw);
            Fl::check();

            //Re-set window title
            char title[1060];
            sprintf(title, "BDReader - %s", mw->Filename);
            mw->window->label(title);

            //Re-enable the window
            mw->window->show();
        }
    }
    else
    {
        fl_alert("Open a comics first !");
    }
}

//! Exit the program calling the window callback
void Quit_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->window->do_callback();
}
//-----------------------------------------------------------------------------



//----------------------------------View menu----------------------------------
//! Fullscreen menu callback: activate or deactivate the fullscreen mode
void Fullscreen_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    if( mw->window->fullscreen_active() == 0 )
    {
        //Go in fullscreen mode
        mw->window->resizable(NULL);
        Fl::wait(10);
        mw->window->fullscreen();
        Fl::wait(10);
        //Hide the bars (but not really, just make them 0px width=> trick to keep the shortcuts and remember their size/position (only w() is forgotten but it's the window one))
        mw->MenuBar->resize(mw->MenuBar->x(), mw->MenuBar->y(), 0, mw->MenuBar->h());
        mw->StatusBar->resize(mw->StatusBar->x(), mw->StatusBar->y(), 0, mw->StatusBar->h());
        //Adapt the displayGrp size (no more menu and status bar shown=>screen size)
        while(mw->window->w()<Fl::w() && mw->window->h()<Fl::h())
            Fl::wait(10);
        mw->DisplayGrp->resize(0, 0, mw->window->w(), mw->window->h());
        mw->PageNbDialGrp->resize((int)(mw->window->w()-(mw->window->w()/20.)), (int)(mw->window->h()-(mw->window->w()/20.)), mw->window->w()/20., mw->window->w()/20.);
        mw->PageNbDialGrp->show();
        Fl::wait(10);
        mw->DisplayGrp->redraw();
        Fl::wait(10);
        Fl::check();

        //Add the ESC control as a fullscreen shortcut
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Fullscreen2"))->activate();
    }
    else
    {
        //Go back from fullscreen
        mw->window->fullscreen_off();
        Fl::wait(100);
        mw->window->resizable(NULL);
        Fl::wait(100);
        //Show the bars again: redefine their width equal to the window one
        mw->MenuBar->resize(mw->MenuBar->x(), mw->MenuBar->y(), mw->window->w(), 25);
        mw->StatusBar->resize(mw->StatusBar->x(), mw->window->h()-20, mw->window->w(), 20);
        Fl::wait(100);
        //HIde the page number indicator
        mw->PageNbDialGrp->hide();
        //Resize the display group
        mw->DisplayGrp->resize(0, 25, mw->window->w(), mw->window->h()-45);
        Fl::wait(100);
        mw->window->resizable(mw->DisplayGrp);
        Fl::wait(100);

        //and let some time to update that
        mw->window->redraw();
        Fl::wait(100);
        Fl::check();

        //Remove the ESC control as a fullscreen shortcut
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Fullscreen2"))->deactivate();
    }
}

//! Switch to page reading mode
void PageMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(PAGE);
    mw->Display->SetImage(mw->Doc->Render());
    StatusUpdate(mw);
    Fl::check();
    mw->Doc->UpdatePreloadedPages();
}
//! Switch to strip reading mode
void StripMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(STRIP);
    //Adjust the strip size according to the new display ratio
    mw->Doc->SetDisplayRatio(mw->Display->w()/(float)mw->Display->h());
    mw->Display->SetImage(mw->Doc->Render());
    StatusUpdate(mw);
    Fl::check();
    mw->Doc->UpdatePreloadedPages();
}
//! Switch to case reading mode
void CaseMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(CASE);
    mw->Display->SetImage(mw->Doc->Render());
    StatusUpdate(mw);
    Fl::check();
    mw->Doc->UpdatePreloadedPages();
}

//! Actvate/deactivate the page preview
void PagePreview_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //If it was on
    if(mw->Doc->IsPagePreview())
    {
        mw->Doc->SetPagePreview(false);
    }
    //if it was off
    else
    {
        mw->Doc->SetPagePreview(true);
    }
    StatusUpdate(mw);
}

//! Hide or show the thumbnails pages preview
void Thumbnails_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Show or hide according to the current state
    if(mw->ThumbDisplayGrp->visible())
    {
        //hide it
        mw->ThumbDisplayGrp->hide();
        mw->DisplayGrp->remove(mw->ThumbDisplayGrp);
        //and resize the main display to take the full space
        mw->MainDisplayGrp->resize(mw->DisplayGrp->x(), mw->DisplayGrp->y(), mw->DisplayGrp->w(), mw->DisplayGrp->h());
        Fl::wait(10);
        //Update the menu item state if needed
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view"))->clear();
    }
    else
    {
        //show it (resize before: window size can have changed)
        mw->ThumbDisplayGrp->resize(mw->DisplayGrp->x(), mw->DisplayGrp->y(), mw->DisplayGrp->w()/5., mw->DisplayGrp->h());
        mw->ThumbDisplayGrp->show();
        mw->DisplayGrp->add(mw->ThumbDisplayGrp);
        //and reduce the size of the main display
        mw->MainDisplayGrp->resize(mw->DisplayGrp->x()+mw->ThumbDisplayGrp->w(), mw->DisplayGrp->y(), mw->DisplayGrp->w()*4/5., mw->DisplayGrp->h());
        //Update the image
        if(mw->Doc)
        {
             mw->ThumbDisplayCurrent->SetImage(mw->Doc->ThumbnailsRenderCurrent());
             mw->ThumbDisplayPrev->SetImage(mw->Doc->ThumbnailsRenderPrevious());
             mw->ThumbDisplayNext->SetImage(mw->Doc->ThumbnailsRenderNext());
        }

        mw->ThumbDisplayGrp->redraw();
        //Update the menu item state if needed
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view"))->set();
        Fl::wait(100);
    }
}

//! Update the current view (Display, status bar, thumbnails...) and preload
//!Called when a new page should be displayed.
void UpdatePage(MainWindow *mw)
{
    //Update the main display
    mw->Display->SetImage(mw->Doc->Render());

    //Update the status bar
    StatusUpdate(mw);
    Fl::check();

    //Preload the surrounding pages
    mw->Doc->PreloadPages();

    //Update the thumbnails (if needed)
    if(mw->ThumbDisplayGrp->visible())
    {
        mw->ThumbDisplayCurrent->SetImage(mw->Doc->ThumbnailsRenderCurrent());
        mw->ThumbDisplayPrev->SetImage(mw->Doc->ThumbnailsRenderPrevious());
        mw->ThumbDisplayNext->SetImage(mw->Doc->ThumbnailsRenderNext());
    }
    Fl::check();
}

//! Go to previous page (through Doc)
void PreviousPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Update the current page number
    int p=mw->Doc->PreviousPage();
    StatusUpdate(mw);
    //wait some time
    Fl::check();
    Fl::wait(10);
    //If still the same page asked, load it, otherwise do nothing
    if(p==mw->Doc->GetCurrentPageNb())
    {
        if(mw->Doc->LoadPage()==-3)
        {
            fl_alert("Unable to access the document. Try to reopen it.");
        }

        //Update everything
        UpdatePage(mw);
    }
}
//! Go to next page (through Doc)
void NextPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Update the current page number
    int p=mw->Doc->NextPage();
    StatusUpdate(mw);
    //wait some time
    Fl::check();
    Fl::wait(10);
    //If still the same page asked, load it, otherwise do nothing
    if(p==mw->Doc->GetCurrentPageNb())
    {
        if(mw->Doc->LoadPage()==-3)
        {
            fl_alert("Unable to access the document. Try to reopen it.");
        }

        //Update everything
        UpdatePage(mw);
    }
}
//! Go to previous case (through Doc)
void PreviousCase_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    int ret = mw->Doc->PreviousCase();
    StatusUpdate(mw);
    mw->Display->SetImage(mw->Doc->Render());
    StatusUpdate(mw);

    //If a new page has been loaded, preload
    if(ret<0)
    {
        //Update everything
        UpdatePage(mw);
    }
}
//! Go to next case (through Doc)
void NextCase_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    int ret = mw->Doc->NextCase();
    StatusUpdate(mw);
    mw->Display->SetImage(mw->Doc->Render());
    StatusUpdate(mw);

    //If a new page has been loaded, preload
    if(ret<0)
    {
        //Update everything
        UpdatePage(mw);
    }
}
//! Go to first page (through Doc)
void FirstPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    if(mw->Doc->LoadPage(1)==-3)
    {
        fl_alert("Unable to access the document. Try to reopen it.");
    }

    //Update everything
    UpdatePage(mw);
}
//! Go to last page (through Doc)
void LastPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    if(mw->Doc->LoadPage(mw->Doc->GetNbOfPages())==-3)
    {
        fl_alert("Unable to access the document. Try to reopen it.");
    }

    //Update everything
    UpdatePage(mw);
}
//! Go to a given page, choosen by the user through an Fl_input window
void GoToPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Ask the user which page he wants to go to:
    char disp[5];
    sprintf(disp, "%d", mw->Doc->GetCurrentPageNb());
    const char *ret=fl_input("Go to page: ", disp);
    //If not Cancel
    if(ret!=NULL)
    {
        int pagenb=atoi(ret);

        //Check if the page nb entered is valid
        if(pagenb<1 || pagenb>mw->Doc->GetNbOfPages())
        {
            fl_alert("Not a valid page number (or out of range) !");
        }
        else
        {
            if(mw->Doc->LoadPage(pagenb)==-3)
            {
                fl_alert("Unable to access the document. Try to reopen it.");
            }

            //Update everything
            UpdatePage(mw);
        }
    }
}

//! Actvate or deactivate (depending on bool val), the navigation menu items (Next, Previous...) and buttons
void ActivateNavigationWidgets(void *param, bool val)
{
    MainWindow *mw=(MainWindow*)param;

    //Browse all the menu items
    Fl_Menu_Item *items=(Fl_Menu_Item *)mw->MenuBar->menu();
    for(int i=0; i<mw->MenuBar->size(); i++)
    {
        //Check if they are navigation ones
        if(items[i].label()!=NULL &&
          (/*strcmp(items[i].label(), "Next page")==0 ||
           strcmp(items[i].label(), "Previous page")==0 ||
           strcmp(items[i].label(), "First page")==0 ||
           strcmp(items[i].label(), "Last page")==0 ||
           strcmp(items[i].label(), "Go to page")==0 ||
           strcmp(items[i].label(), "Next panel")==0 ||
           strcmp(items[i].label(), "Previous panel")==0 ||
           strcmp(items[i].label(), "Panel mode")==0 ||
           strcmp(items[i].label(), "Strip mode")==0 ||
           strcmp(items[i].label(), "Page mode")==0 ||
           strcmp(items[i].label(), "Page preview")==0 ||
           strcmp(items[i].label(), "Thumbnails view")==0 ||*/
           strcmp(items[i].label(), "&View")==0 ||
           strcmp(items[i].label(), "Export (as images)")==0
          ))
        {
            //Then activate or deactivate it
            if(val)
            {
                items[i].activate();
            }
            else
            {
                items[i].deactivate();
            }
        }
    }
    mw->MenuBar->redraw();

    //The navigation buttons
    if(val)
    {
        mw->PreviousButton->show();
        mw->NextButton->show();
        mw->PreviousPageButton->show();
        mw->NextPageButton->show();
    }
    else
    {
        mw->PreviousButton->hide();
        mw->NextButton->hide();
        mw->PreviousPageButton->hide();
        mw->NextPageButton->hide();
    }
}
//! Add recent documents items according to global Preferences
void AddRecentDocsItems(void *param)
{
    MainWindow *mw=(MainWindow*)param;

    //WARNING: MenuItem is doing its own filtering to "delete" the potential duplicates
    //Use only the gloabl preferences (doesn't make sense with doc prefrences)
    if(mw->Pref->GetNbRecentDocs()>0)
    {
        //First remove the recent docs menu item (to delete the previous submenus)
        int recent_item_index=mw->MenuBar->find_index("&File/Recent files");
        if(recent_item_index!=-1)
            mw->MenuBar->remove(recent_item_index);
        //and re-create it (at its previous position)
        mw->MenuBar->insert(recent_item_index, "Recent files", 0, 0, 0, FL_SUBMENU);

        //Then re-create the recent docs items
        for(int i=0; i<mw->Pref->GetNbRecentDocs(); i++)
        {
            char tmp[FL_PATH_MAX];
            //init the menu item
            sprintf(tmp, "&File/Recent files/");
            //Copy the string and replace '\' by '\\' for windows style files paths
            int n_d=strlen(tmp);
            for(unsigned int n_s=0; n_s<strlen(mw->Pref->GetRecentDocs(i)); n_s++)
            {
                if(mw->Pref->GetRecentDocs(i)[n_s]=='\\')
                {
                    tmp[n_d++]='\\';
                }
                tmp[n_d++]=mw->Pref->GetRecentDocs(i)[n_s];
            }
            //Terminating char
            tmp[n_d++]='\0';
            //Add it to the menu
            mw->MenuBar->add(tmp, 0, (Fl_Callback *)Open_cb, mw);
        }
    }
    //If no recent documents
    else
    {
        //Deactivate the corresponding menu item
        Fl_Menu_Item* recentitem=(Fl_Menu_Item*)mw->MenuBar->find_item("&File/Recent files");
        recentitem->deactivate();
    }
}
//-----------------------------------------------------------------------------



//----------------------------------Help menu----------------------------------
//! Show a help information window
void Help_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    /*fl_message_title("Help");
    fl_message(HELP_STRING);*/
    mw->HelpDlg->show();
}
//! Show the about information window
void About_cb(Fl_Widget* widget, void * param)
{
    //MainWindow *mw=(MainWindow*)param;

    fl_message_title("About");
    fl_message(ABOUT_STRING);
}
//-----------------------------------------------------------------------------


//! Update widgets according to preferences values
void UpdateWidgets(void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Which prefrences to use ?
    Preferences *localPref=NULL;
    if(mw->PrefDoc)
        localPref=mw->PrefDoc; //Doc ones if exist
    else
        localPref=mw->Pref; //Gloabl ones otherwise

    //The reading mode
    if(localPref->GetReadingMode()==PAGE)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Page mode"))->setonly();
    }
    if(localPref->GetReadingMode()==CASE)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode"))->setonly();
    }
    if(localPref->GetReadingMode()==STRIP)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Strip mode"))->setonly();
    }

    //Preview page
    if(localPref->GetPagePreview())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Page preview"))->set();
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Page preview"))->clear();
    }

    //Thumbnails view
    if(mw->Doc)
    {
        if(localPref->GetThumbnails())
        {
            ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view"))->set();
        }
        else
        {
            ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view"))->clear();
        }
        if(localPref->GetThumbnails()!=mw->ThumbDisplayGrp->visible())
        {
            Thumbnails_cb(mw->DisplayGrp, mw);
        }
    }

    //The preferences preferences (!): use the global ones for these
    if(mw->Pref->GetSavePref())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Preferences/Save preferences"))->set();
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Preferences/Save preferences"))->clear();
    }
    if(mw->Pref->GetLocalPref())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Preferences/Use documents preferences"))->setonly();
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Preferences/Use global preferences"))->setonly();
    }

    //Recent docs: will only add real new ones (no duplicates) if some
    AddRecentDocsItems(mw);
}

//! On closing the window (and thus exiting the program), save the parameters in preferences
//!file and close everything (delete is called in main())
void Window_cb(Fl_Widget* widget, void * param)
{
     MainWindow *mw=(MainWindow*)param;

     //Save the preferences in files
        //Global preferences
         //The window size, except if in Fullscreen
         if(mw->window->fullscreen_active()==0)
         {
            mw->Pref->SetWinW(mw->window->w());
            mw->Pref->SetWinH(mw->window->h());
            mw->Pref->SetWinX(mw->window->x());
            mw->Pref->SetWinY(mw->window->y());
         }
         //The thumbnails view
         mw->Pref->SetThumbnails(mw->ThumbDisplayGrp->visible());
         //The preferences preferences
         mw->Pref->SetSavePref(((Fl_Menu_Item*)mw->MenuBar->find_item("&Preferences/Save preferences"))->value());
         mw->Pref->SetLocalPref(((Fl_Menu_Item*)mw->MenuBar->find_item("&Preferences/Use documents preferences"))->value());
         //Write to pref file
         mw->Pref->Save(mw->Doc);
       //Current doc preferences
         if(mw->PrefDoc)
         {
             //The thumbnails view
             mw->PrefDoc->SetThumbnails(mw->ThumbDisplayGrp->visible());
             //Write to pref file
             mw->PrefDoc->Save(mw->Doc);
         }

     //Close the main window
     mw->window->hide();
     //Close other windows if some (Help)
     Fl_Widget * toclose;
     while(toclose=Fl::first_window())
     {
        toclose->hide();
     }
}





//! Constructor.
//!Create the widgets, build the window and call the Init() with NULL parameters (no document).
//!\param f the filename of a file top open on startup, can be '\0' if nothing
MainWindow::MainWindow(const char *exepath, const char *f)
{
    //No doc or doc pref yet
    Doc=NULL;
    PrefDoc=NULL;

    //Load user preferences
        Pref = new Preferences();
        //Use the program name (first argument) to get its full path (path used to store preferences on Windows)
            char tmp[FL_PATH_MAX];
            fl_filename_absolute(tmp, sizeof(tmp), exepath);
            strncpy(ExePath, tmp, strlen(tmp)-strlen(fl_filename_name(tmp)));
        Pref->Load("preferences", ExePath);


    //Main window creation
        //Use saved window size and position (will be 0, 0, 600, 600 if nothing saved, and always lower than the available space on the screen)
        window=new Fl_Double_Window(Pref->GetWinX(), Pref->GetWinY(), min(Fl::w()-2,Pref->GetWinW()) , min(Fl::h()-2,Pref->GetWinH()), "BDReader");

    //Set an icon
    //Problem with windows compile (Pixmap not declared)
    #ifndef __WINDOWS__
        fl_open_display();
        Pixmap p = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display), (char *)icon32_bits, icon32_width, icon32_height);
        window->icon((char *)p);
    #endif

    //Widgets creation
    window->begin();

        //Main menu
            Fl_Menu_Item menuitems[] =
            {
                { "&File",              0, 0, 0, FL_SUBMENU },
                    { "Open file", FL_COMMAND + 'o', (Fl_Callback *)Open_cb, this },
                    { "Open folder", FL_SHIFT + FL_COMMAND + 'o', (Fl_Callback *)Open_cb, this },
                    { "Recent files", 0, 0, 0, FL_SUBMENU },
                        { 0 },
                    { "Export (as images)", FL_COMMAND + 'e', (Fl_Callback *)ExportDoc_cb, this, FL_MENU_INACTIVE },
                    { "Quit", FL_COMMAND + 'q', (Fl_Callback *)Quit_cb, this },
                    { 0 },
                { "&View",              0, 0, 0, FL_SUBMENU | FL_MENU_INACTIVE},
                    {"Fullscreen2", FL_Escape, (Fl_Callback *)Fullscreen_cb, this, FL_MENU_INVISIBLE | FL_MENU_INACTIVE},
                    {"Fullscreen", FL_COMMAND + 'f', (Fl_Callback *)Fullscreen_cb, this, FL_MENU_DIVIDER },
                    { "Panel mode", 'c', (Fl_Callback *)CaseMode_cb, this, FL_MENU_RADIO },
                    { "Strip mode", 's', (Fl_Callback *)StripMode_cb, this, FL_MENU_RADIO },
                    { "Page mode", 'p', (Fl_Callback *)PageMode_cb, this, FL_MENU_RADIO | FL_MENU_DIVIDER },
                    { "Page preview", FL_COMMAND+'p', (Fl_Callback *)PagePreview_cb, this, FL_MENU_TOGGLE | FL_MENU_DIVIDER },
                    { "Thumbnails view", FL_COMMAND + 't', (Fl_Callback *)Thumbnails_cb, this, FL_MENU_TOGGLE | FL_MENU_DIVIDER },
                    { "Next panel", FL_Right, (Fl_Callback *)NextCase_cb, this },
                    { "Previous panel", FL_Left, (Fl_Callback *)PreviousCase_cb, this },
                    { "Next page", FL_Down , (Fl_Callback *)NextPage_cb, this },
                    { "Previous page", FL_Up, (Fl_Callback *)PreviousPage_cb, this },
                    { "First page", FL_Home , (Fl_Callback *)FirstPage_cb, this },
                    { "Last page", FL_End , (Fl_Callback *)LastPage_cb, this },
                    { "Go to page", FL_COMMAND + 'g' , (Fl_Callback *)GoToPage_cb, this },
                    { 0 },
                { "&Preferences",              0, 0, 0, FL_SUBMENU },
                    { "Save preferences", 0, (Fl_Callback *)0, this, FL_MENU_TOGGLE |  FL_MENU_DIVIDER | FL_MENU_INVISIBLE },
                    { "Use global preferences", 0, (Fl_Callback *)0, this, FL_MENU_RADIO },
                    { "Use documents preferences", 0, (Fl_Callback *)0, this, FL_MENU_RADIO },
                    { 0 },
                { "&Help",              0, 0, 0, FL_SUBMENU },
                    { "Help", FL_COMMAND + 'h', (Fl_Callback *)Help_cb, this },
                    { "About", 0, (Fl_Callback *)About_cb, this },
                    { 0 },
                { 0 }
             };

             MenuBar = new Fl_Menu_Bar(0, 0, window->w(), 25);
             MenuBar->copy(menuitems);

             //Create the list of recent docs
             AddRecentDocsItems(this);


         //Main group containing the main display, navigation buttons and page preview display
            DisplayGrp = new Fl_Group(0, 25, window->w(), window->h()-45);
            DisplayGrp->begin();

                //Sub group containing the preview thumbnails and the separator
                ThumbDisplayGrp = new Fl_Group(DisplayGrp->x(), DisplayGrp->y(), DisplayGrp->w()/6., DisplayGrp->h());
                ThumbDisplayGrp->begin();
                 //Thumbnails display
                     ThumbDisplayPrev = new Fl_OpenCV(ThumbDisplayGrp->x(), ThumbDisplayGrp->y(), ThumbDisplayGrp->w()-5, ThumbDisplayGrp->h()/3.-20-4);
                     ThumbDisplayPrev->SetOnRelease_cb(PreviousPage_cb, (void*)this);
                     ThumbDisplayCurrent = new Fl_OpenCV(ThumbDisplayGrp->x(), ThumbDisplayPrev->y()+ThumbDisplayPrev->h()+6, ThumbDisplayGrp->w()-5, ThumbDisplayGrp->h()/3.+40-4);
                     ThumbDisplayNext = new Fl_OpenCV(ThumbDisplayGrp->x(), ThumbDisplayCurrent->y()+ThumbDisplayCurrent->h()+6, ThumbDisplayGrp->w()-5, ThumbDisplayGrp->h()/3.-20-4);
                     ThumbDisplayNext->SetOnRelease_cb(NextPage_cb, (void*)this);
                     ThumbDisplayGrp->selection_color(FL_BLACK);ThumbDisplayPrev->color(FL_BLACK);ThumbDisplayCurrent->color(FL_BLACK);ThumbDisplayNext->color(FL_BLACK);
                 //Horizontal separators
                     Fl_Box *separator1 = new Fl_Box(FL_FLAT_BOX, ThumbDisplayGrp->x(), ThumbDisplayPrev->y()+ThumbDisplayPrev->h(), ThumbDisplayGrp->w()-5, 6, "");
                     separator1->color(FL_BLACK);
                     Fl_Box *separator2 = new Fl_Box(FL_FLAT_BOX, ThumbDisplayGrp->x(), ThumbDisplayCurrent->y()+ThumbDisplayCurrent->h(), ThumbDisplayGrp->w()-5, 6, "");
                     separator2->color(FL_BLACK);
                 //Close button
                     PrevCloseButton = new Fl_Button(ThumbDisplayGrp->x()+ThumbDisplayGrp->w()-25, ThumbDisplayGrp->y(), 20, 20, "X");
                     PrevCloseButton->clear_visible_focus();
                     PrevCloseButton->color(FL_BLACK);PrevCloseButton->labelcolor(FL_WHITE);PrevCloseButton->selection_color(FL_WHITE);PrevCloseButton->box(FL_FLAT_BOX);
                     PrevCloseButton->callback(Thumbnails_cb, (void*)this);
                 //Vertical Separator
                     Fl_Box *separator = new Fl_Box(FL_PLASTIC_UP_BOX, ThumbDisplayGrp->x()+ThumbDisplayGrp->w()-5, ThumbDisplayGrp->y(), 5, ThumbDisplayGrp->h(), "");
                 ThumbDisplayGrp->end();
                 ThumbDisplayGrp->color(FL_BLACK);

                 //Sub group containing the main display and the navigation buttons
                 MainDisplayGrp = new Fl_Group(DisplayGrp->x(), DisplayGrp->y(), DisplayGrp->w(), DisplayGrp->h());
                 MainDisplayGrp->begin();
                 //Main display
                     Display = new Fl_OpenCV(MainDisplayGrp->x(), MainDisplayGrp->y(), MainDisplayGrp->w(), MainDisplayGrp->h());

                 //Navigation buttons
                     NextButton = new Fl_Overlay_Arrow_Button(Display->x()+Display->w()-0.045*window->w(), (int)(Display->y()), 0.045*window->w(), Display->h(), "", 0, 0.8, 0.15, 0);
                     NextButton->color(fl_rgb_color(100, 100, 100));NextButton->selection_color(fl_rgb_color(245,245,200));
                     NextButton->callback(NextCase_cb, (void*)this);
                     //Prevent the widget to get the focus and thus prevent to use the arrow keys to
                     //navigate through the widgets: keep the Arrows shortcut behavior (to menu_items)
                     //needed to apply to any other widget than the display (which can interecpt the arrow keys)
                     NextButton->clear_visible_focus();
                     NextButton->hide();
                     NextButton->tooltip("Next panel");
                     PreviousButton = new Fl_Overlay_Arrow_Button(Display->x(), (int)(Display->y()), 0.045*window->w(), Display->h(), "", 180, 0.8, 0.15, 0);
                     PreviousButton->color(fl_rgb_color(100, 100, 100));PreviousButton->selection_color(fl_rgb_color(245,245,200));
                     PreviousButton->callback(PreviousCase_cb, (void*)this);
                     PreviousButton->clear_visible_focus();
                     PreviousButton->hide();
                     PreviousButton->tooltip("Previous panel");
                     NextPageButton = new Fl_Overlay_Arrow_Button(Display->x(), (int)(Display->y()+Display->h()-0.045*window->h()), Display->w(), 0.045*window->h(), "", -90, 0.10, 0.8, 0);
                     NextPageButton->color(fl_rgb_color(100, 100, 100));NextPageButton->selection_color(fl_rgb_color(245,245,200));
                     NextPageButton->callback(NextPage_cb, (void*)this);
                     NextPageButton->clear_visible_focus();
                     NextPageButton->hide();
                     NextPageButton->tooltip("Next page");
                     PreviousPageButton = new Fl_Overlay_Arrow_Button(Display->x(), (int)(Display->y()), Display->w(), 0.045*window->h(), "", 90, 0.10, 0.8, 0);
                     PreviousPageButton->color(fl_rgb_color(100, 100, 100));PreviousPageButton->selection_color(fl_rgb_color(245,245,200));
                     PreviousPageButton->callback(PreviousPage_cb, (void*)this);
                     PreviousPageButton->clear_visible_focus();
                     PreviousPageButton->hide();
                     PreviousPageButton->tooltip("Previous page");

                     //The fullscreen page number indicator
                     PageNbDialGrp=new Fl_Group(MainDisplayGrp->x()+MainDisplayGrp->w()-70, MainDisplayGrp->y()+MainDisplayGrp->h()-70, 50, 50);
                     PageNbDialGrp->begin();
                     PageNbDial = new Fl_Dial(PageNbDialGrp->x(), PageNbDialGrp->y(), PageNbDialGrp->w(), PageNbDialGrp->h(), "");
                         PageNbDial->angles(180, 180+360);
                         PageNbDial->type(FL_FILL_DIAL);
                         PageNbDial->precision(1);
                         PageNbDial->box(FL_ROUND_DOWN_BOX);
                         PageNbDial->selection_color(FL_BLACK);
                         PageNbDial->tooltip("Page ");
                     PageNbDialGrp->end();
                     PageNbDialGrp->resizable(NULL);
                     PageNbDialGrp->hide();
                MainDisplayGrp->end();

            DisplayGrp->end();
            //Hide the thumbnails (will be displayed at document opening if preferences say so)
            ThumbDisplayGrp->hide();
            DisplayGrp->remove(ThumbDisplayGrp);


         //Status bar
             StatusBar = new Fl_Box(0, window->h()-20,window->w(), 20);
             StatusBar->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
             StatusBar->box(FL_PLASTIC_UP_BOX);


             //TODO: A terminer et ameliorer
             /*Fl_Counter* StatusCounter = new Fl_Counter(StatusBar->x()+StatusBar->w()/2.0-100, StatusBar->y(), 200, 20);
             StatusCounter->box(FL_PLASTIC_ROUND_UP_BOX);
             StatusCounter->labeltype(FL_NO_LABEL);
             StatusCounter->labelcolor(FL_DARK_RED);
             StatusCounter->minimum(1);
             StatusCounter->maximum(10);
             StatusCounter->step(1);*/
             //StatusBar->

         //Events callbacks
             //Closing callback
             window->callback(Window_cb, (void*)this);

             //Handle mouse wheel events (combine with the timer and a global variable mouseWheel...to be improved)
             Fl::add_handler(MouseWheelEvents);
             //Timer initializaton
             Fl::add_timeout(0.1, TimerMouseWheel_cb, (void*)this);
             //Handle drag'n'drop event (actually only dropping of a file on the window)
             Fl::add_handler(DragNDrop);
             //Timer initializaton
             Fl::add_timeout(0.1, TimerDND_cb, (void*)this);

             //Handle the display rezising events
             Display->SetResizing_cb(ResizingEvents, (void*)this);


        //Graphics
             //Make the window resizable around the Display (and define minimum dimensions)
             window->resizable(DisplayGrp);
             window->size_range(400, 400, 0, 0, 0, 0, 0);

             //Apply the plastic theme/skin
             Fl::scheme("plastic");

         //Default title for message windows
         fl_message_title_default("BDReader");

         //TODO: Gerer les bds en suites d'images (folders)
    window->end();

    //Initialize help window
    HelpDlg= new Fl_Help_Window(600, 400, "Help");
    HelpDlg->value(HELP_STRING_HTML);
    HelpDlg->hide();


    //Initialize the window and Document
    UpdateWidgets(this);
    if(f)
    {
        //Convert the filename from relative to absolute path
        fl_filename_absolute(Filename, FL_PATH_MAX, f);
    }
    else
    {
        sprintf(Filename, "%s", "\0");
    }
    Init(Filename);
}

//! Destructor.
MainWindow::~MainWindow()
{
    //TODO delete the wigets

   if(Doc)
        delete Doc;

   if(window)
        delete window;

    delete Pref;
}

//! New document openning.
//!Create and load the document and update the window and widgets accordingly.
//!\param filename the full filename (path+name) of the document to open, can be "\0" if nothing
void MainWindow::Init(char *filename)
{
    //Show the main window
    Fl::visual(FL_RGB);
    window->show();

    //If filename is correct
    if(filename[0]!='\0')
    {
        //Save the new filename
        sprintf(Filename, "%s", filename);

        //If an active document was open, save the preferences
        if(Doc)
        {
            //Global pref
            //Save preferences (especially add the doc to the recent docs)
            if(window->fullscreen_active()==0)
            {
                Pref->SetWinW(window->w());
                Pref->SetWinH(window->h());
                Pref->SetWinX(window->x());
                Pref->SetWinY(window->y());
            }
            Pref->SetThumbnails(ThumbDisplayGrp->visible());
            Pref->SetSavePref(((Fl_Menu_Item*)MenuBar->find_item("&Preferences/Save preferences"))->value());
            Pref->SetLocalPref(((Fl_Menu_Item*)MenuBar->find_item("&Preferences/Use documents preferences"))->value());
            Pref->Save(Doc);
            //Doc relative pref
            if(PrefDoc)
            {
                PrefDoc->SetThumbnails(ThumbDisplayGrp->visible());
                PrefDoc->Save(Doc);
            }
        }

        //Create a document using the new filename
        Document *new_doc;
        bool extOk=false;
        //select type according to extension
        if(strstr(Filename, ".pdf" )!=NULL)
        {
            new_doc=new Document_pdf(Filename);
            extOk=true;
        }
        else
        {
            if( (strstr(Filename, ".cbr" )!=NULL) || (strstr(Filename, ".cbz" )!=NULL) || (strstr(Filename, ".cbt" )!=NULL) || (strstr(Filename, ".cb7" )!=NULL) ||
                (strstr(Filename, ".zip" )!=NULL) || (strstr(Filename, ".rar" )!=NULL) || (strstr(Filename, ".tar" )!=NULL) || (strstr(Filename, ".7z" )!=NULL))
            {
                new_doc=new Document_archive(Filename);
                extOk=true;
            }
            else
            {
                if(fl_filename_isdir(Filename)!=0)
                {
                    new_doc=new Document_folder(Filename);
                    extOk=true;
                }
            }
        }

        //Check if the document has been opened
        if(extOk && new_doc->GetNbOfPages()>0)
        {
            //Delete old doc if one (using an intermediate pointer to avoid access)
            if(Doc)
            {
                Document *olddoc=Doc;
                Doc=NULL;
                delete olddoc;
            }

            //use the new one
            Doc=new_doc;

            //Load the first page
            Doc->LoadPage(1);

            //Apply the preferences to the document and the window widgets
                //Apply global preferences first
                Pref->Load("preferences", ExePath);
                Pref->Apply(Doc);

                //Then deal with possible document preferences
                //Delete old PrefDoc if one
                if(PrefDoc)
                    delete PrefDoc;
                //Create a document preference
                PrefDoc = new Preferences();
                //Then load these preferences if the document exists, or keep using the global ones if not
                char buff[2048];
                sprintf(buff, "%s", fl_filename_name(Doc->GetFilename()));
                const char *filenameshort=fl_filename_setext(buff, sizeof(buff), ".pref"); //(preferences file as same name as the document file)
                if(PrefDoc->Load(filenameshort, ExePath)>=0 && Pref->GetLocalPref())
                {
                    PrefDoc->Apply(Doc);
                }
                //Otherwise apply the gloabl pref to the newly created doc pref
                else
                {
                    PrefDoc->SetThumbnails(Pref->GetThumbnails());
                    PrefDoc->Save(Doc);
                    //and reload
                    PrefDoc->Load(filenameshort, ExePath);
                }

            //Set the display ratio
            Doc->SetDisplayRatio(Display->w()/(float)Display->h());

            //Update the widgets according to these preferences (should be done after the preview displayed has been updated)
            UpdateWidgets((void*)this);

            //Change the window title
            char title[1060];
            sprintf(title, "BDReader - %s", Filename);
            window->label(title);

            //Activate menus
            ActivateNavigationWidgets(this, true);

            //Update everything
            UpdatePage(this);
        }
        //if the document couldn't be openned
        else
        {
            //Say it to the user
            fl_alert("Unable to open the %s.\nIs it a valid comics file (pdf or archive) or folder (two images or more) ?", Filename);

            //Check if we have an active document
            if(!Doc)
            {
                //Update the window accordingly
                //menus
                ActivateNavigationWidgets(this, false);
                //display
                Display->SetImage(NULL);
                //window title
                char title[1060];
                sprintf(title, "BDReader");
                window->label(title);
                //and status bar
                StatusUpdate(this);
            }
        }
    }
}
