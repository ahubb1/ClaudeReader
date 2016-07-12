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


//--------------------------- Update functions ---------------------------
//! Update the current view (Display, status bar, thumbnails...) and preload
//!Called when a new page should be displayed.
void UpdatePage(MainWindow *mw)
{
    //Turn cursor to wait
    fl_cursor(FL_CURSOR_WAIT);
    Fl::check();

    //Wait for the page to be ready to be displayed
    while(!mw->Doc->currentPage->IsLoaded())
        Fl::wait(0.01);

    //Update the main display
    mw->Display->SetImage(mw->Doc->RenderPage(), *mw->Doc->Render());

    //Update the status bar
    StatusUpdate(mw);
    Fl::check();

    //Turn cursor back to normal
    fl_cursor(FL_CURSOR_DEFAULT);
    Fl::check();

    //Preload the surrounding pages
    mw->Doc->PreloadPages();

    //Update the thumbnails (if needed)
    if(mw->ThumbDisplayGrp->visible())
    {
        mw->ThumbDisplayCurrent->SetImage(mw->Doc->ThumbnailsRenderCurrent());
        mw->ThumbDisplayPrev->SetImage(mw->Doc->ThumbnailsRenderPrevious());
        mw->ThumbDisplayNext->SetImage(mw->Doc->ThumbnailsRenderNext());
        mw->PrevCloseButton->redraw();
    }
    Fl::check();
}

//! Update the surrounding pages
void UpdatePreloadPages(void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->UpdatePreloadedPages();
}
//-----------------------------------------------------------------------------



//--------------------------------- Status ----------------------------------
//! Update the status bar text according to current state (document current page...)
void StatusBarUpdate(void * param)
{
      MainWindow *mw=(MainWindow*)param;

      char text[1024];
      char textZoom[1024];
      char textMaxZoom[10];

      //If document
      if(mw->DisplayGrp->visible())
      {
          //Get number of pages... blabla
          if(mw->Display->GetMaxZoom()>=100)
          {
              sprintf(textMaxZoom, " - ");
          }
          else
          {
              sprintf(textMaxZoom, "%d%%", (int)(mw->Display->GetMaxZoom()*100.));
          }

          if(mw->Doc)
          {
              if(mw->Doc->GetReadingMode()==CASE)
              {
                    sprintf(text, "PANEL\t\tPage %d/%d (Panel %d/%d)", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNbOfPages(), mw->Doc->currentPage->GetActiveCase()+1, mw->Doc->currentPage->GetNbCases());

                    sprintf(textZoom, "Zoom: %d%% (Max:%s / Additionnal:x%0.1f)", (int)(mw->Display->GetCurrentZoom()*100.), textMaxZoom, mw->Display->GetCustomZoom());
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
                    sprintf(textZoom, "Zoom: %d%% (Max:%s)", (int)(mw->Display->GetCurrentZoom()*100.), textMaxZoom);
              }
          }
          else
          {
              sprintf(text, "%s", "\0");
          }
          mw->StatusBar->copy_label(text);
          mw->StatusBar->redraw();
          mw->StatusBarZoom->copy_label(textZoom);
      }
      else
      {
          //No document open but library
          sprintf(text, "Library folder: ", mw->Library->get_absolute_dir());
          mw->StatusBar->copy_label("");
          mw->StatusBar->redraw();
          mw->StatusBarZoom->copy_label(text);
      }

      mw->StatusBarZoom->redraw();
}
//! Update the fullscreen status
void StatusFullScreenUpdate(void * param)
{
     MainWindow *mw=(MainWindow*)param;

     if(mw->Doc)
     {
        //Update the dial
        mw->PageNbFullProgress->minimum(1);
        mw->PageNbFullProgress->maximum(mw->Doc->GetNbOfPages());
        mw->PageNbFullProgress->value(mw->Doc->GetCurrentPageNb());
        //and the tooltip text
        char txt[16];
        sprintf(txt, "%d/%d", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNbOfPages());
        mw->PageNbFullProgress->copy_label(txt);
        mw->PageNbFullProgress->redraw();
     }
}
//! Call both the status bar update and the fullscreen status update
void StatusUpdate(void * param)
{
    StatusBarUpdate(param);
    StatusFullScreenUpdate(param);
    Fl::wait(.50);
}
//-----------------------------------------------------------------------------



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
            if(isDNDavailableTmp && (int)strlen(Fl::event_text())==Fl::event_length())
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
                nextcb=(Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Next page\t");
            else
               nextcb=(Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Next panel\t");

            if(nextcb && nextcb->active())
                nextcb->do_callback((Fl_Widget*)mw->MenuBar);
        }
        //if negative, go to previous case
        else
        {
            Fl_Menu_Item* previouscb;
            if(page)
                previouscb=(Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Previous page\t");
            else
                previouscb=(Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Previous panel\t");

            if(previouscb && previouscb->active())
                previouscb->do_callback((Fl_Widget*)mw->MenuBar);
        }
        //flush
        mouseWheel=0;
    }

    Fl::wait(.50);
    //Repeat...
    Fl::repeat_timeout(0.01, TimerMouseWheel_cb, (void*)mw);
}

//! Catch the resizing events and update the Display aspect ratio accordingly to new size
void ResizingEvents(Fl_Widget* widget, void* param)
{
    MainWindow *mw=(MainWindow*)param;

    //printf("Ratio:%f (=1.33)\n", (float)mw->window->w()/mw->window->h());

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
        }
        if(mw->Doc->GetReadingMode()==STRIP || mw->Doc->GetReadingMode()==CASE)
        {
            //Update the preloaded pages (let time to fltk to redraw first)
            Fl::add_timeout(0.2, UpdatePreloadPages, (void*)mw);
        }

        //Update status bar (for zoom values)
        StatusBarUpdate((void*)mw);

        //Give some time to fltk
        Fl::check();
    }
}
//-----------------------------------------------------------------------------



//---------------------------------File menu----------------------------------
//! Open document from library
void Library_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Make library unavailabe
    mw->Library->deactivate();
    Fl::check();

    //Turn cursor to wait
    mw->window->cursor(FL_CURSOR_WAIT);
    Fl::check();

    //Open doc
    mw->Init(((Fl_Library_Element*)widget)->GetAbsoluteFilename());

    //Turn cursor back to normal
    mw->window->cursor(FL_CURSOR_DEFAULT);
    Fl::check();
}

//! Open a document and load it (through a call to Init())
void Open_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Turn cursor to wait
    mw->window->cursor(FL_CURSOR_WAIT);
    Fl::check();

    char tmpFilename[FL_PATH_MAX];

    //Retrieve the menu item value to know if it's an open or a recent document item
    //if it's an open, do the stuff
    if(strstr(((Fl_Menu_*)widget)->mvalue()->label(), "Open")!=NULL)
    {
        //Window to define a directory and filename
        Fl_Native_File_Chooser chooser=Fl_Native_File_Chooser();
        chooser.title("Open a file");
        chooser.filter("Supported formats\t*.{pdf,cbz,cbr,cbt,cb7,epub,rar,zip,tar,7z}\nPdf\t*.pdf\nComics archive\t*.{cbz,cbr,cbt,cb7}\nEbook\t*.epub\nArchive\t*.{rar,zip,tar,7z}");
        //chooser.options(Fl_Native_File_Chooser::PREVIEW);

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
            //Turn cursor back to normal
            mw->window->cursor(FL_CURSOR_DEFAULT);
            Fl::check();
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
             strstr(tmpFilename, ".epub")==NULL &&
             strstr(tmpFilename, ".rar")==NULL && strstr(tmpFilename, ".zip")==NULL && strstr(tmpFilename, ".tar")==NULL && strstr(tmpFilename, ".7z")==NULL  &&
             fl_filename_isdir(tmpFilename)==0
           ))
        {
            fl_alert("Invalid filename !");
            //Turn cursor back to normal
            mw->window->cursor(FL_CURSOR_DEFAULT);
            Fl::check();
            return;
        }
    }

    //Open file and do stuff
    mw->Init(tmpFilename);

    //Turn cursor back to normal
    mw->window->cursor(FL_CURSOR_DEFAULT);
    Fl::check();
}

//! Close openned document and load library
void Close_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Come back from fullscreen if needed
    if(mw->window->fullscreen_active()!= 0)
    {
        Fullscreen_cb(widget, param);
    }

    //Save the preferences in files
    mw->SavePref();

    //Close document
    mw->Init("");

    //Make library availabe
    mw->Library->activate();
    Fl::check();
    StatusBarUpdate(mw);
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
                    sprintf(fileext, ".jpg");
            }

            //Ask for confirmation first
            fl_message_title("Export");
            if(fl_choice("Export %s to %s as images (%s) ?", "Cancel", "Export", NULL, fl_filename_name(mw->Doc->GetFilename()), chooser.filename(), fileext)!=1)
                return;

            //Go to document first page
            FirstPage_cb(NULL, mw);

            //Create an export object
            Export_Document exportdoc(mw->Doc, chooser.filename(), fileext, panel, preview);

            //Disable the window (to avoid any interaction)
            mw->window->hide();

            //Create a progress window
            char label[255]; //Window label
            sprintf(label, "Exporting: %s\nTo: %s\nFormat:\t%s\n\n", fl_filename_name(mw->Doc->GetFilename()), chooser.filename(), fileext);
            int label_w=0, label_h=0;
            fl_measure(label, label_w, label_h, 1); //Get label length to adjust window size accordingly

            int ww=fmax(500., label_w+40);
            Fl_Window ProgressWindow(mw->window->x()+mw->window->w()/2.-ww/2., mw->window->y()+mw->window->h()/2.-100, ww, 200, "Export in progress...");
            ProgressWindow.set_modal();
            ProgressWindow.begin();
                Fl_Button fileLabel(20, 10, (ww-40), 80, "Exporting ");
                fileLabel.label(label);
                fileLabel.box(FL_FLAT_BOX);
                fileLabel.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                Fl_Progress *ProgressBar;
                ProgressBar=new Fl_Progress(10, fileLabel.y()+fileLabel.h()+10, ProgressWindow.w()-20, 20, "Page ");
                ProgressBar->minimum(0);
                ProgressBar->maximum(mw->Doc->GetNbOfPages());
                ProgressBar->value(0);
                ProgressBar->selection_color(FL_DARK_RED);
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
                fl_alert("Export succesfull ! (%d images created)", retexp);
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

//! Suggest next file to open (based on filename and library)
void SuggestNextFile(void *param)
{
    MainWindow *mw=(MainWindow*)param;

    //Get suggested next comics
    char suggestion_filename[2048], suggestion_short_filename[2048];
    int ret = mw->Library->GetSuggestionNext(mw->Filename, suggestion_filename, suggestion_short_filename);


    //Is there a suggestion?
    fl_message_title("BDReader - What do we do ?");
    if(ret<0)
        ret=fl_choice("Open the comics library ?", "Cancel", "Library", NULL);
    else
        ret=fl_choice("Open %s ?", "Cancel", "Library", "Open", suggestion_short_filename);

    switch(ret)
    {
        //Cancel: do nothing
        case 0:
            break;

        //Open library: close current one
        case 1:
            Close_cb(NULL, param);
            break;

        //Open suggested file:
        case 2:
            mw->Init(suggestion_filename);
            break;
    }
}
//-----------------------------------------------------------------------------



//----------------------------------View menu----------------------------------
//! Fullscreen menu callback: activate or deactivate the fullscreen mode
void Fullscreen_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //If not in fullscreem
    if( mw->window->fullscreen_active() == 0 )
    {
        //Go in fullscreen mode
        mw->window->resizable(NULL);
        Fl::wait(.10);
        mw->window->fullscreen();
        Fl::wait(.10);
        //Hide the bars (but not really, just make them 0px width=> trick to keep the shortcuts and remember their size/position (only w() is forgotten but it's the window one))
        mw->MenuBar->resize(mw->MenuBar->x(), mw->MenuBar->y(), 0, mw->MenuBar->h());
        mw->StatusBarGrp->resize(mw->StatusBar->x(), mw->StatusBar->y(), 0, mw->StatusBar->h());

        //Adapt the displayGrp size (no more menu and status bar shown=>screen size)
        while(mw->window->w()<Fl::w() || mw->window->h()<Fl::h())
            Fl::wait(.10);
        mw->DisplayGrp->resize(0, 0, mw->window->w(), mw->window->h());
        mw->PageNbFullGrp->resize((int)(mw->window->w()-(mw->window->w()/15.)), (int)(mw->window->h()-(mw->window->w()/40.)), mw->window->w()/20., mw->window->w()/80.);
        mw->PageNbFullGrp->show();
        Fl::wait(.10);
        mw->DisplayGrp->redraw();
        Fl::wait(.10);
        Fl::check();

        //Add the ESC control as a fullscreen shortcut
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Fullscreen2\t"))->activate();
    }
    //If was in fullscreem
    else
    {
        //Go back from fullscreen
        mw->window->fullscreen_off();
        Fl::wait(.20);
        mw->window->resizable(NULL);
        Fl::wait(.20);
        //Show the bars again: redefine their width equal to the window one
        mw->MenuBar->resize(mw->MenuBar->x(), mw->MenuBar->y(), mw->window->w(), mw->MenuBar->h());
        mw->StatusBarGrp->resize(mw->StatusBar->x(), mw->window->h()-mw->StatusBar->h(), mw->window->w(), mw->StatusBar->h());
        Fl::wait(.20);
        //HIde the page number indicator
        mw->PageNbFullGrp->hide();
        //Resize the display group
        mw->DisplayGrp->resize(0, 25, mw->window->w(), mw->window->h()-(mw->MenuBar->h()+mw->StatusBar->h()));
        Fl::wait(.20);
        mw->window->resizable(mw->DisplayGrp);
        Fl::wait(.20);

        //and let some time to update that
        mw->window->redraw();
        Fl::wait(.20);
        Fl::check();

        //Remove the ESC control as a fullscreen shortcut
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Fullscreen2\t"))->deactivate();
    }
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
        Fl::wait(0.1);
        //Update the menu item state if needed
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view\t"))->clear();
    }
    else
    {
        //show it (resize before: window size can have changed)
        mw->ThumbDisplayGrp->resize(mw->DisplayGrp->x(), mw->DisplayGrp->y(), mw->DisplayGrp->w()/7., mw->DisplayGrp->h());
        mw->ThumbDisplayGrp->show();
        mw->DisplayGrp->add(mw->ThumbDisplayGrp);
        //and reduce the size of the main display
        mw->MainDisplayGrp->resize(mw->DisplayGrp->x()+mw->ThumbDisplayGrp->w(), mw->DisplayGrp->y(), mw->DisplayGrp->w()*6/7., mw->DisplayGrp->h());
        //Update the image
        if(mw->Doc)
        {
             mw->ThumbDisplayCurrent->SetImage(mw->Doc->ThumbnailsRenderCurrent());
             mw->ThumbDisplayPrev->SetImage(mw->Doc->ThumbnailsRenderPrevious());
             mw->ThumbDisplayNext->SetImage(mw->Doc->ThumbnailsRenderNext());
        }

        mw->ThumbDisplayGrp->redraw();
        //Update the menu item state if needed
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view\t"))->set();
        Fl::wait(0.2);
    }

    //Update fullscreen state
    StatusFullScreenUpdate((void*)mw);
}

void MaxZoom_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Get the option index based on the submenu index
    int option_index=((Fl_Menu_*)widget)->value()-((Fl_Menu_*)widget)->find_index("&View/Maximal zoom\t")-1;
    //Apply new value
    mw->Display->SetMaxZoom((mw->Pref->GetMaxZoomOption(option_index)).value);
    mw->Display->UpdateView();

    //Update status bar
    StatusBarUpdate((void*)mw);

    //Update the preferences accordingly
    mw->Pref->SetMaxZoomIdx(option_index);
    mw->PrefDoc->SetMaxZoomIdx(option_index);
}

void AdditionnalZoom_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Get the option index relative to the submenu index
    int base_index=((Fl_Menu_*)widget)->find_index("&View/Panel mode view\t/Additional zoom\t\t+\\-");
    int option_index=((Fl_Menu_*)widget)->value()-base_index-1;


    //Get the current selected value (if it's a +/- item)
    int current_index=0;
    if(option_index==8 || option_index==9)
    {
        const Fl_Menu_Item* items=((Fl_Menu_*)widget)->menu();
        for(int i=1; i<9; i++)
        {
            const Fl_Menu_Item item=items[base_index+i];
            if(item.checked())
                current_index=i;
        }
    }

    //Value item or +/- item ?
    char item_name[255];
    switch(option_index)
    {
        //"+" shortcut
        case 8:
            //New value: -1 on the item list or minimum
            option_index=fmin(7, current_index);
            //Apply on to the display
            mw->Display->SetCustomZoom(mw->Pref->GetAdditionnalZoomOption(option_index).value);
            //Update the menu item accordingly
            sprintf(item_name, "&View/Panel mode view\t/Additional zoom\t\t+\\-/%s", (mw->Pref->GetAdditionnalZoomOption(option_index).caption));
            ((Fl_Menu_Item*)mw->MenuBar->find_item(item_name))->setonly();
            break;

        //"-" shortcut
        case 9:
           //New value: -1 on the item list or minimum
            option_index=fmax(0, current_index-2);
            //Apply on to the display
            mw->Display->SetCustomZoom(mw->Pref->GetAdditionnalZoomOption(option_index).value);
            //Update the menu item accordingly
            sprintf(item_name, "&View/Panel mode view\t/Additional zoom\t\t+\\-/%s", (mw->Pref->GetAdditionnalZoomOption(option_index).caption));
            ((Fl_Menu_Item*)mw->MenuBar->find_item(item_name))->setonly();
            break;

        //Values
        default:
            //Apply new value
            mw->Display->SetCustomZoom((mw->Pref->GetAdditionnalZoomOption(option_index)).value);
    }
    //Update display accordingly
    mw->Display->UpdateView();

    //Update status bar
    StatusBarUpdate((void*)mw);

    //Update the preferences accordingly
    mw->Pref->SetAdditionnalZoomIdx(option_index);
    mw->PrefDoc->SetAdditionnalZoomIdx(option_index);
}

//! Show the panel in context or not
void InContext_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //In context or not depending on the current state
    mw->Display->SetInContext(!mw->Display->IsInContext());
    mw->Display->UpdateView();
    //activate/deactivate the depending options (Panel border and additionnal zoom)
    if(mw->Display->IsInContext())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Show panel border\t"))->activate();
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Additional zoom\t\t+\\-"))->activate();
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Show panel border\t"))->deactivate();
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Additional zoom\t\t+\\-"))->deactivate();
    }

    //Update the preferences accordingly
    mw->Pref->SetInContext(mw->Display->IsInContext());
    mw->PrefDoc->SetInContext(mw->Display->IsInContext());
}
//! Show or hide the panel border when in panel mode
void PanelBorder_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Invert border/no border
    mw->Display->SetBorder(!mw->Display->IsBorder());
    mw->Doc->ReloadPages();
    mw->Display->UpdateView();

    //Update the preferences accordingly
    mw->Pref->SetBorder(mw->Display->IsBorder());
    mw->PrefDoc->SetBorder(mw->Display->IsBorder());
}

//! Page rotation callback
void Rotate_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Turn cursor to wait
    mw->window->cursor(FL_CURSOR_WAIT);
    Fl::check();

    //If a specific rotation is picked, set it
    if(strstr(((Fl_Menu_*)widget)->mvalue()->label(), mw->Pref->GetRotationOption(0).caption)!=NULL)
        mw->Doc->SetRotation(0);
    else
        if(strstr(((Fl_Menu_*)widget)->mvalue()->label(), mw->Pref->GetRotationOption(1).caption)!=NULL)
            mw->Doc->SetRotation(1);
        else
            if(strstr(((Fl_Menu_*)widget)->mvalue()->label(), mw->Pref->GetRotationOption(2).caption)!=NULL)
                mw->Doc->SetRotation(2);
            else
                if(strstr(((Fl_Menu_*)widget)->mvalue()->label(), mw->Pref->GetRotationOption(3).caption)!=NULL)
                    mw->Doc->SetRotation(3);
                else
                {
                    //otherwise rotate +90 (next rotation value)
                    mw->Doc->SetRotation(mw->Doc->GetRotation()+1);

                    //and update the menu item accordingly
                    char item_name[255];
                    sprintf(item_name, "&View/Rotation\t\tCtrl+R/%s", (mw->Pref->GetRotationOption(mw->Doc->GetRotation()).caption));
                    ((Fl_Menu_Item*)mw->MenuBar->find_item(item_name))->setonly();
                }

    //And update everything (long...)
    mw->Doc->ReloadPages();
    UpdatePage(mw);

    //Turn cursor back to normal
    mw->window->cursor(FL_CURSOR_DEFAULT);
    Fl::check();

    //Update the preferences accordingly
    mw->Pref->SetRotationIdx(mw->Doc->GetRotation());
    mw->PrefDoc->SetRotationIdx(mw->Doc->GetRotation());
}
//! Contrast enhancement callback
void Contrast_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Turn cursor to wait
    mw->window->cursor(FL_CURSOR_WAIT);
    Fl::check();

    //Toggle contrast enhancement
    mw->Doc->SetContrast(!mw->Doc->GetContrast());
    //And update everything (long...)
    mw->Doc->ReloadPages();
    UpdatePage(mw);

    //Turn cursor back to normal
    mw->window->cursor(FL_CURSOR_DEFAULT);
    Fl::check();
}
//-----------------------------------------------------------------------------



//-------------------------------Navigation menu-------------------------------
//! Switch to page reading mode
void PageMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(PAGE);
    //mw->Display->SetImage(mw->Doc->Render());
    mw->Display->SetImage(mw->Doc->RenderPage(), *mw->Doc->Render());
    StatusUpdate(mw);
    ActivatePanelViewWidgets(mw, false);
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
    //mw->Display->SetImage(mw->Doc->Render());
    mw->Display->SetImage(mw->Doc->RenderPage(), *mw->Doc->Render());
    StatusUpdate(mw);
    ActivatePanelViewWidgets(mw, false);
    Fl::check();
    mw->Doc->UpdatePreloadedPages();
}
//! Switch to case reading mode
void CaseMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(CASE);
    //mw->Display->SetImage(mw->Doc->Render());
    mw->Display->SetImage(mw->Doc->RenderPage(), *mw->Doc->Render());
    StatusUpdate(mw);
    ActivatePanelViewWidgets(mw, true);
    Fl::check();
    mw->Doc->UpdatePreloadedPages();
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
    Fl::wait(.10);
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
    Fl::wait(.10);

    //If last page, suggest to open next file
    if(p==-2)
    {
        SuggestNextFile(param);
    }

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
    mw->Display->SetDisplayRect(*mw->Doc->Render());
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
    mw->Display->SetDisplayRect(*mw->Doc->Render());
    StatusUpdate(mw);

    //If new page requested (was on last panel) but on last page
    if(ret==-2)
    {
        SuggestNextFile(param);
    }
    else
    {
        //If a new page has been loaded, preload
        if(ret<0)
        {
            //Update everything
            UpdatePage(mw);
        }
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
          (
           strcmp(items[i].label(), "Close\t")==0 ||
           strcmp(items[i].label(), "&View")==0 ||
           strcmp(items[i].label(), "&Navigation")==0 ||
           strcmp(items[i].label(), "Export (as images)\t")==0
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

    //The main display
    if(val)
    {
        mw->Display->activate();
    }
    else
    {
        mw->Display->deactivate();
    }
}
//! Actvate or deactivate (depending on bool val), the panel view menu items (border, in context...)
void ActivatePanelViewWidgets(void *param, bool val)
{
    MainWindow *mw=(MainWindow*)param;

    if(val)
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t"))->activate();
    else
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t"))->deactivate();
}
//-----------------------------------------------------------------------------



//----------------------------------Help menu----------------------------------
//! Show a help information window
void Help_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Initialize help window if not already
    if(!mw->HelpDlg)
    {
        mw->HelpDlg=new Fl_Help_Window(max(mw->window->w()/2, 600), 400, "Help");
        mw->HelpDlg->value(HELP_STRING_HTML);
    }
    //Show it
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
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Page mode\t"))->setonly();
        ActivatePanelViewWidgets(mw, false);
    }
    if(localPref->GetReadingMode()==CASE)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Panel mode\t"))->setonly();
        ActivatePanelViewWidgets(mw, true);
    }
    if(localPref->GetReadingMode()==STRIP)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Strip mode\t"))->setonly();
        ActivatePanelViewWidgets(mw, false);
    }

    //Preview page
    if(localPref->GetPagePreview())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Page preview\t"))->set();
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&Navigation/Page preview\t"))->clear();
    }

    //Thumbnails view
    if(mw->Doc)
    {
        if(localPref->GetThumbnails())
        {
            ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view\t"))->set();
        }
        else
        {
            ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Thumbnails view\t"))->clear();
        }
        if(localPref->GetThumbnails()!=mw->ThumbDisplayGrp->visible())
        {
            Thumbnails_cb(mw->DisplayGrp, mw);
        }
    }

    //Max zoom
    char item_name[255];
    sprintf(item_name, "&View/Maximal zoom\t/%s", (mw->Pref->GetMaxZoomOption(localPref->GetMaxZoomIdx())).caption);
    ((Fl_Menu_Item*)mw->MenuBar->find_item(item_name))->setonly();
    mw->Display->SetMaxZoom((mw->Pref->GetMaxZoomOption(localPref->GetMaxZoomIdx())).value);

    //Additionnal zoom
    sprintf(item_name, "&View/Panel mode view\t/Additional zoom\t\t+\\-/%s", (mw->Pref->GetAdditionnalZoomOption(localPref->GetAdditionnalZoomIdx())).caption);
    ((Fl_Menu_Item*)mw->MenuBar->find_item(item_name))->setonly();
    mw->Display->SetCustomZoom((mw->Pref->GetAdditionnalZoomOption(localPref->GetAdditionnalZoomIdx())).value);


    //Panel in context or not
    if(localPref->GetInContext())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Show panel in context\t"))->set();
        mw->Display->SetInContext(true);
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Show panel in context\t"))->clear();
        mw->Display->SetInContext(false);
    }

    //Panel border
    if(localPref->GetBorder())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Show panel border\t"))->set();
        mw->Display->SetBorder(true);
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode view\t/Show panel border\t"))->clear();
        mw->Display->SetBorder(false);
    }

    //Rotation
    if(mw->Doc)
        sprintf(item_name, "&View/Rotation\t\tCtrl+R/%s", (mw->Pref->GetRotationOption(mw->Doc->GetRotation()).caption));
    else
        sprintf(item_name, "&View/Rotation\t\tCtrl+R/%s", (mw->Pref->GetRotationOption(0).caption));
    ((Fl_Menu_Item*)mw->MenuBar->find_item(item_name))->setonly();

    /*Contrast
    if(mw->Doc && mw->Doc->GetContrast())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Enhance contrast\t"))->set();
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Enhance contrast\t"))->clear();
    }*/

    //Recent docs: will only add real new ones (no duplicates) if some
    AddRecentDocsItems(mw);
}

//! On closing the window (and thus exiting the program), save the parameters in preferences
//!file and close everything (delete is called in main())
void Window_cb(Fl_Widget* widget, void * param)
{
     MainWindow *mw=(MainWindow*)param;


     //Ignore Escape key
     if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape)
        return;

     //Save the preferences in files
     mw->SavePref();

     //Close the main window
     mw->window->hide();
     //Close other windows if some (Help)
     Fl_Widget * toclose;
     while((toclose=Fl::first_window()))
     {
        toclose->hide();
     }
}





/// Constructor.
/**Create the widgets, build the window and call the Init() with the document filename to open (NULL if no document specified).
\param exepath the fullpath of execution
\param f the filename of a file to open on startup, can be '\0' if nothing*/
MainWindow::MainWindow(const char *exepath, const char *f)
{
    //No doc or doc pref yet
    Doc=NULL;
    PrefDoc=NULL;

    //Use the program name (first argument) to get its full path (path used to store preferences on Windows)
    char tmp[FL_PATH_MAX];
    fl_filename_absolute(tmp, sizeof(tmp), exepath);
    strncpy(ExePath, tmp, strlen(tmp)-strlen(fl_filename_name(tmp)));

    //Load user preferences
        Pref = new Preferences(exepath);
        Pref->Load("");

    //Main window creation
        //Use saved window size and position (will be 0, 0, 600, 600 if nothing saved, and always lower than the available space on the screen)
        window=new Fl_Double_Window(Pref->GetWinX(), Pref->GetWinY(), min(Fl::w()-2,Pref->GetWinW()), min(Fl::h()-2,Pref->GetWinH()), "BDReader");
        window->color(fl_rgb_color(242,241,240));

        //Set an icon
        //Problem with windows compile (Pixmap not declared)
        #ifndef __WINDOWS__
            Pixmap mask;
            XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display), (char **)Icon2_48_xpm, &icon_pix, &mask, 0);
            window->icon((char *)icon_pix);
        #else
            window->icon((char *)LoadIcon(fl_display, MAKEINTRESOURCE(MAIN_ICON)));
        #endif

        //Change predefined colors
        Fl::set_color(FL_BACKGROUND_COLOR, 242,241,240);
        Fl::set_color(FL_BACKGROUND2_COLOR, 229,229,228);
        Fl::set_color(FL_SELECTION_COLOR, 120, 0, 0);


        //Widgets creation:

        //Main menu
            Fl_Menu_Item menuitems[] =
            {
                { "&File",              0, 0, 0, FL_SUBMENU },
                    { "Open file\t", FL_COMMAND + 'o', (Fl_Callback *)Open_cb, this },
                    { "Open folder\t", FL_COMMAND + FL_SHIFT + 'o', (Fl_Callback *)Open_cb, this },
                    { "Recent files", 0, 0, 0, FL_SUBMENU },
                        { 0 },
                    { "Close\t", FL_COMMAND + 'w', (Fl_Callback *)Close_cb, this, FL_MENU_DIVIDER },
                    { "Export (as images)\t", FL_COMMAND + 'e', (Fl_Callback *)ExportDoc_cb, this, FL_MENU_INACTIVE | FL_MENU_DIVIDER },
                    { "Quit\t", FL_COMMAND + 'q', (Fl_Callback *)Quit_cb, this },
                    { 0 },
                { "&View",              0, 0, 0, FL_SUBMENU | FL_MENU_INACTIVE},
                    { "Thumbnails view\t", FL_COMMAND + 't', (Fl_Callback *)Thumbnails_cb, this, FL_MENU_TOGGLE },
                    {"Fullscreen2\t", FL_Escape, (Fl_Callback *)Fullscreen_cb, this, FL_MENU_INVISIBLE | FL_MENU_INACTIVE },
                    {"Fullscreen\t", FL_COMMAND + 'f', (Fl_Callback *)Fullscreen_cb, this, FL_MENU_DIVIDER },
                    { "Maximal zoom\t", 0, 0, 0, FL_SUBMENU },
                        { Pref->GetMaxZoomOption(0).caption, 0, (Fl_Callback *)MaxZoom_cb, this, FL_MENU_RADIO | FL_MENU_CHECK },
                        { Pref->GetMaxZoomOption(1).caption, 0, (Fl_Callback *)MaxZoom_cb, this, FL_MENU_RADIO },
                        { Pref->GetMaxZoomOption(2).caption, 0, (Fl_Callback *)MaxZoom_cb, this, FL_MENU_RADIO },
                        { Pref->GetMaxZoomOption(3).caption, 0, (Fl_Callback *)MaxZoom_cb, this, FL_MENU_RADIO },
                        { Pref->GetMaxZoomOption(4).caption, 0, (Fl_Callback *)MaxZoom_cb, this, FL_MENU_RADIO },
                        { 0 },
                    { "Panel mode view\t", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER },
                        { "Show panel in context\t", FL_COMMAND + 'i', (Fl_Callback *)InContext_cb, this, FL_MENU_TOGGLE },
                        { "Show panel border\t", FL_COMMAND + 'b', (Fl_Callback *)PanelBorder_cb, this, FL_MENU_TOGGLE },
                        { "Additional zoom\t\t+\\-", 0, 0, 0, FL_SUBMENU },
                            { Pref->GetAdditionnalZoomOption(0).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO },
                            { Pref->GetAdditionnalZoomOption(1).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO },
                            { Pref->GetAdditionnalZoomOption(2).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO },
                            { Pref->GetAdditionnalZoomOption(3).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO },
                            { Pref->GetAdditionnalZoomOption(4).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO | FL_MENU_CHECK },
                            { Pref->GetAdditionnalZoomOption(5).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO },
                            { Pref->GetAdditionnalZoomOption(6).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO },
                            { Pref->GetAdditionnalZoomOption(7).caption, 0, (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_RADIO },
                            { "+", '+', (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_INVISIBLE },
                            { "-", '-', (Fl_Callback *)AdditionnalZoom_cb, this, FL_MENU_INVISIBLE },
                            { 0 },
                        { 0 },
                    { "Rotation\t\tCtrl+R", FL_COMMAND + 'r', (Fl_Callback *)Rotate_cb, this, FL_SUBMENU },
                        { Pref->GetRotationOption(0).caption, 0, (Fl_Callback *)Rotate_cb, this, FL_MENU_RADIO | FL_MENU_CHECK },
                        { Pref->GetRotationOption(1).caption, 0, (Fl_Callback *)Rotate_cb, this, FL_MENU_RADIO },
                        { Pref->GetRotationOption(2).caption, 0, (Fl_Callback *)Rotate_cb, this, FL_MENU_RADIO },
                        { Pref->GetRotationOption(3).caption, 0, (Fl_Callback *)Rotate_cb, this, FL_MENU_RADIO },
                        { 0 },
                    //{ "Enhance contrast\t", FL_COMMAND + 'c', (Fl_Callback *)Contrast_cb, this, FL_MENU_TOGGLE},
                    { 0 },
                { "&Navigation",              0, 0, 0, FL_SUBMENU | FL_MENU_INACTIVE},
                    { "Panel mode\t", 'c', (Fl_Callback *)CaseMode_cb, this, FL_MENU_RADIO },
                    { "Strip mode\t", 's', (Fl_Callback *)StripMode_cb, this, FL_MENU_RADIO },
                    { "Page mode\t", 'p', (Fl_Callback *)PageMode_cb, this, FL_MENU_RADIO | FL_MENU_DIVIDER },
                    { "Page preview\t", FL_COMMAND+'p', (Fl_Callback *)PagePreview_cb, this, FL_MENU_TOGGLE | FL_MENU_DIVIDER },
                    { "Next panel\t", FL_Right, (Fl_Callback *)NextCase_cb, this },
                    { "Previous panel\t", FL_Left, (Fl_Callback *)PreviousCase_cb, this },
                    { "Next page\t", FL_Down , (Fl_Callback *)NextPage_cb, this },
                    { "Previous page\t", FL_Up, (Fl_Callback *)PreviousPage_cb, this },
                    { "First page\t", FL_Home , (Fl_Callback *)FirstPage_cb, this },
                    { "Last page\t", FL_End , (Fl_Callback *)LastPage_cb, this },
                    { "Go to page\t", FL_COMMAND + 'g' , (Fl_Callback *)GoToPage_cb, this },
                    { 0 },
                { "&Help",              0, 0, 0, FL_SUBMENU },
                    { "Help\t", FL_COMMAND + 'h', (Fl_Callback *)Help_cb, this },
                    { "About\t", 0, (Fl_Callback *)About_cb, this },
                    { 0 },
                { 0 }
             };

             MenuBar = new Fl_Menu_Bar(0, 0, window->w(), 25);
             MenuBar->copy(menuitems);
             MenuBar->box(FL_FLAT_BOX);


             //Create the list of recent docs
             AddRecentDocsItems(this);


         //Main group containing the main display, navigation buttons and page preview display
            DisplayGrp = new Fl_Group(0, MenuBar->h(), window->w(), window->h()-2*MenuBar->h());
            //DisplayGrp = new Fl_Tile(0, 25, window->w(), window->h()-45);
            DisplayGrp->begin();

                //Sub group containing the preview thumbnails and the separator
                ThumbDisplayGrp = new Fl_Group(DisplayGrp->x(), DisplayGrp->y(), DisplayGrp->w()/7., DisplayGrp->h());
                ThumbDisplayGrp->begin();
                 //Thumbnails display
                     int rounding = (int)(ThumbDisplayGrp->h()-3.*round(ThumbDisplayGrp->h()/3.));
                     ThumbDisplayPrev = new Fl_ViewerCV(ThumbDisplayGrp->x(), ThumbDisplayGrp->y(), ThumbDisplayGrp->w(), round(ThumbDisplayGrp->h()/3.)-20-4);
                     ThumbDisplayPrev->SetOnRelease_cb(PreviousPage_cb, (void*)this);
                     ThumbDisplayCurrent = new Fl_ViewerCV(ThumbDisplayGrp->x(), ThumbDisplayPrev->y()+ThumbDisplayPrev->h()+6+rounding, ThumbDisplayGrp->w(), round(ThumbDisplayGrp->h()/3.)+40-4);
                     ThumbDisplayNext = new Fl_ViewerCV(ThumbDisplayGrp->x(), ThumbDisplayCurrent->y()+ThumbDisplayCurrent->h()+6, ThumbDisplayGrp->w(), round(ThumbDisplayGrp->h()/3.)-20-4);
                     ThumbDisplayNext->SetOnRelease_cb(NextPage_cb, (void*)this);
                     ThumbDisplayGrp->selection_color(FL_BLACK);ThumbDisplayPrev->color(FL_BLACK);ThumbDisplayCurrent->color(FL_BLACK);ThumbDisplayNext->color(FL_BLACK);
                 //Horizontal separators
                     Fl_Box *separator1 = new Fl_Box(FL_FLAT_BOX, ThumbDisplayGrp->x(), ThumbDisplayPrev->y()+ThumbDisplayPrev->h(), ThumbDisplayGrp->w(), 6+rounding, "");
                     separator1->color(FL_BLACK);
                     Fl_Box *separator2 = new Fl_Box(FL_FLAT_BOX, ThumbDisplayGrp->x(), ThumbDisplayCurrent->y()+ThumbDisplayCurrent->h(), ThumbDisplayGrp->w(), 6, "");
                     separator2->color(FL_BLACK);
                 //Close button
                     Fl_Group *Grp=new Fl_Group(ThumbDisplayGrp->x(), ThumbDisplayGrp->y(), ThumbDisplayGrp->w(), ThumbDisplayGrp->h());
                        PrevCloseButton = new Fl_Button(ThumbDisplayGrp->x()+ThumbDisplayGrp->w()-20, ThumbDisplayGrp->y(), 20, 20, "X");
                        PrevCloseButton->clear_visible_focus();
                        PrevCloseButton->color(FL_DARK_RED);PrevCloseButton->labelcolor(FL_WHITE);PrevCloseButton->selection_color(FL_WHITE);PrevCloseButton->box(FL_UP_BOX);
                        PrevCloseButton->tooltip("Close thumbnail");
                        PrevCloseButton->callback(Thumbnails_cb, (void*)this);
                        Fl_Box* central = new Fl_Box(FL_DOWN_BOX, ThumbDisplayGrp->x(), ThumbDisplayGrp->y()+20, ThumbDisplayGrp->w()-20, ThumbDisplayGrp->h()-20, "");
                        Fl_Box* box1 = new Fl_Box(FL_DOWN_BOX, ThumbDisplayGrp->x(), ThumbDisplayGrp->y(), ThumbDisplayGrp->w()-20, 20, "");
                        Fl_Box* box2 = new Fl_Box(FL_DOWN_BOX, ThumbDisplayGrp->x()+ThumbDisplayGrp->w()-20, ThumbDisplayGrp->y()+20, 20, ThumbDisplayGrp->h()-20, "");
                        central->hide();
                        box1->hide();
                        box2->hide();
                     Grp->end();
                     Grp->resizable(central);
                 //Vertical Separator
                     //Fl_Box *separator = new Fl_Box(FL_PLASTIC_UP_BOX, ThumbDisplayGrp->x()+ThumbDisplayGrp->w()-5, ThumbDisplayGrp->y(), 5, ThumbDisplayGrp->h(), "");
                 ThumbDisplayGrp->end();
                 ThumbDisplayGrp->color(FL_BLACK);
                 ThumbDisplayGrp->box(FL_DOWN_BOX);

                 //Sub group containing the main display and the navigation buttons
                 MainDisplayGrp = new Fl_Group(DisplayGrp->x()+ThumbDisplayGrp->w(), DisplayGrp->y(), DisplayGrp->w()-ThumbDisplayGrp->w(), DisplayGrp->h());
                 MainDisplayGrp->begin();
                 //Main display
                     Display = new Fl_ViewerCV(MainDisplayGrp->x(), MainDisplayGrp->y(), MainDisplayGrp->w(), MainDisplayGrp->h());
                     Display->SetHandleKeys(true);
                     Display->color(FL_BACKGROUND2_COLOR);

                 //Navigation buttons
                     NextButton = new Fl_Overlay_Arrow_Button(Display->x()+Display->w()-0.045*window->w(), (int)(Display->y()), 0.045*window->w(), Display->h(), "", 0, 0.8, 0.15, 0);
                     NextButton->color(fl_rgb_color(100, 100, 100));NextButton->selection_color(fl_rgb_color(245,245,200));NextButton->labelcolor(FL_SELECTION_COLOR);
                     NextButton->callback(NextCase_cb, (void*)this);
                     //Prevent the widget to get the focus and thus prevent to use the arrow keys to
                     //navigate through the widgets: keep the Arrows shortcut behavior (to menu_items)
                     //needed to apply to any other widget than the display (which can interecpt the arrow keys)
                     NextButton->clear_visible_focus();
                     NextButton->hide();
                     NextButton->tooltip("Next panel");
                     PreviousButton = new Fl_Overlay_Arrow_Button(Display->x(), (int)(Display->y()), 0.045*window->w(), Display->h(), "", 180, 0.8, 0.15, 0);
                     PreviousButton->color(fl_rgb_color(100, 100, 100));PreviousButton->selection_color(fl_rgb_color(245,245,200));PreviousButton->labelcolor(FL_SELECTION_COLOR);
                     PreviousButton->callback(PreviousCase_cb, (void*)this);
                     PreviousButton->clear_visible_focus();
                     PreviousButton->hide();
                     PreviousButton->tooltip("Previous panel");
                     NextPageButton = new Fl_Overlay_Arrow_Button(Display->x(), (int)(Display->y()+Display->h()-0.045*window->h()), Display->w(), 0.045*window->h(), "", -90, 0.10, 0.8, 0);
                     NextPageButton->color(fl_rgb_color(100, 100, 100));NextPageButton->selection_color(fl_rgb_color(245,245,200));NextPageButton->labelcolor(FL_SELECTION_COLOR);
                     NextPageButton->callback(NextPage_cb, (void*)this);
                     NextPageButton->clear_visible_focus();
                     NextPageButton->hide();
                     NextPageButton->tooltip("Next page");
                     PreviousPageButton = new Fl_Overlay_Arrow_Button(Display->x(), (int)(Display->y()), Display->w(), 0.045*window->h(), "", 90, 0.10, 0.8, 0);
                     PreviousPageButton->color(fl_rgb_color(100, 100, 100));PreviousPageButton->selection_color(fl_rgb_color(245,245,200));PreviousPageButton->labelcolor(FL_SELECTION_COLOR);
                     PreviousPageButton->callback(PreviousPage_cb, (void*)this);
                     PreviousPageButton->clear_visible_focus();
                     PreviousPageButton->hide();
                     PreviousPageButton->tooltip("Previous page");

                     //The fullscreen page number indicator
                     PageNbFullGrp=new Fl_Group(MainDisplayGrp->x()+MainDisplayGrp->w()-110, MainDisplayGrp->y()+MainDisplayGrp->h()-30, 100, 20);
                     PageNbFullGrp->begin();
                     PageNbFullProgress = new Fl_Progress(PageNbFullGrp->x(), PageNbFullGrp->y(), PageNbFullGrp->w(), PageNbFullGrp->h(), "1/1");
                         PageNbFullProgress->color(FL_BACKGROUND_COLOR, FL_SELECTION_COLOR);
                         PageNbFullProgress->tooltip("Page");
                     PageNbFullGrp->end();
                     PageNbFullGrp->resizable(NULL);
                     PageNbFullGrp->hide();
                MainDisplayGrp->end();
                MainDisplayGrp->box(FL_DOWN_BOX);
                MainDisplayGrp->resizable(Display);

            DisplayGrp->end();
            DisplayGrp->resizable(MainDisplayGrp);
            //Hide the thumbnails (will be displayed at document opening if preferences say so)
            Thumbnails_cb(ThumbDisplayGrp, (void*)this);


         //Status bar
             int status_bar_height=MenuBar->h();
             StatusBarGrp = new Fl_Group(0, window->h()-status_bar_height, window->w(), status_bar_height);
                StatusBar = new Fl_Box(0, window->h()-status_bar_height, window->w(), status_bar_height);
                StatusBar->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
                StatusBar->box(FL_UP_BOX);
                StatusBarZoom = new Fl_Box(0, window->h()-status_bar_height, window->w()/2., status_bar_height);
                StatusBarZoom->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
            StatusBarGrp->end();


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

             //Apply the gtk+ theme/skin
             Fl::scheme("gtk+");

         //Default title for message windows
         fl_message_title_default("BDReader");

    window->end();

    //Initialize help window
    HelpDlg=NULL;

    //A library view
    Library = new Fl_Books_Library(0, MenuBar->y()+MenuBar->h(), DisplayGrp->w(), DisplayGrp->h(), "", Pref->GetLibraryFolder(), ExePath, 180, 180*1.53, 10);
    //Library->resize(0, MenuBar->h(), DisplayGrp->w(), DisplayGrp->h());
    window->add(Library);
    Library->callback(Library_cb, (void*)this);

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
   if(Doc)
        delete Doc;

   if(window)
        delete window;

    delete Pref;
}

/// New document opening.
/**Create and load the document and update the window and widgets accordingly.
\param filename the full filename (path+name) of the document to open, can be "\0" if nothing*/
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
           //Save the preferences in files
           SavePref();
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
                (strstr(Filename, ".epub" )!=NULL) ||
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
            //Close library
            Library->hide();
            DisplayGrp->show();

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
                Pref->Load("");
                Pref->Apply(Doc);

                //Then deal with possible document preferences
                //Delete old PrefDoc if one
                if(PrefDoc)
                    delete PrefDoc;
                //Create a document preference
                PrefDoc = new Preferences(ExePath);
                //Then load these preferences if the document exists, or keep using the global ones if not
                char buff[2048];
                sprintf(buff, "%s", fl_filename_name(Doc->GetFilename()));
                const char *filenameshort=fl_filename_setext(buff, sizeof(buff), ".pref"); //(preferences file as same name as the document file)
                if(PrefDoc->Load(Doc->GetFilename())>=0 && Pref->GetLocalPref())
                {
                    PrefDoc->Apply(Doc);
                }
                //Otherwise apply the gloabl pref to the newly created doc pref
                else
                {
                    Pref->CopyLocalTo(PrefDoc);
                    PrefDoc->Save(Doc);
                    //and reload
                    Pref->Load(Doc->GetFilename());
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

                //Reactivate library
                Library->activate();
            }
        }
    }
    else
    {
        window->label("BDReader");
        //Library->clear_modal_states(); //was originally modal since not attached to a parent
        ActivateNavigationWidgets(this, false);
        DisplayGrp->hide();
        Library->show();
        Library->redraw();
    }
}


//! Save the preferences in files
void MainWindow::SavePref()
{
    //Current doc preferences (if there are some)
     if(PrefDoc)
     {
         //The thumbnails view
         PrefDoc->SetThumbnails(ThumbDisplayGrp->visible());
         //Write to pref file
         PrefDoc->Save(Doc);

         //Copy current doc pref to global ones
         PrefDoc->CopyLocalTo(Pref);
     }

    //Then set the global only preferences
     //The window size, except if in Fullscreen
     if(window->fullscreen_active()==0)
     {
        Pref->SetWinW(window->w());
        Pref->SetWinH(window->h());
        Pref->SetWinX(window->x());
        Pref->SetWinY(window->y());
     }
     //The Library path
     Pref->SetLibraryFolder(Library->get_absolute_dir());

     /*The preferences preferences
     Pref->SetSavePref(((Fl_Menu_Item*)MenuBar->find_item("&Preferences/Save preferences\t"))->value());
     Pref->SetLocalPref(((Fl_Menu_Item*)MenuBar->find_item("&Preferences/Use documents preferences\t"))->value());*/

     //Write to global pref file
     Pref->Save(Doc);
}
