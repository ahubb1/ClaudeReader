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
#include "MainWindow.h"

//To store the mouse wheel movements
int mouseWheel=0;


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
                sprintf(text, "Page %d/%d (Case %d/%d)", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNumberOfPages(), mw->Doc->currentPage->GetActiveCase()+1, mw->Doc->currentPage->GetNbCases());
          }
          else
          {
                sprintf(text, "Page %d/%d", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNumberOfPages());
          }
      }
      else
      {
          sprintf(text, "\0");
      }
      mw->StatusBar->copy_label(text);
      mw->StatusBar->redraw();
      Fl::wait(100);
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
void Timer_cb(void* param)
{
    MainWindow *mw=(MainWindow*)param;

    //If some movement has been detected
    if(mw->Doc && mouseWheel!=0)
    {
        //if positive, go to next case
        if(mouseWheel>0)
        {
            Fl_Menu_Item* nextcb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Next case");
            if(nextcb && nextcb->active())
                nextcb->do_callback((Fl_Widget*)mw->MenuBar);
        }
        //if negative, go to previous case
        else
        {
            Fl_Menu_Item* previouscb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Previous case");
            if(previouscb && previouscb->active())
                previouscb->do_callback((Fl_Widget*)mw->MenuBar);
        }
        //flush
        mouseWheel=0;
    }

    Fl::wait(50);
    //Repeat...
    Fl::repeat_timeout(0.1, Timer_cb, (void*)mw);
}



//---------------------------------File menu----------------------------------
//! Open a document and load it (through a call to Init())
void Open_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    char tmpFilename[1024];

    //Retrieve the menu item value to know if it's an open or a recent document item
    //if it's an open, do the stuff
    if(strcmp(((Fl_Menu_*)widget)->mvalue()->label(), "Open")==0)
    {
        //Window to define a directory and filename
        sprintf(tmpFilename, fl_file_chooser("Open a pdf file","Pdf (*.pdf)", ".", 0));
        //Check if filename not null or not a .tss file
        if(tmpFilename[0]=='\0' || strstr (tmpFilename, ".pdf" )==NULL)
        {
            //fl_alert("Invalid filename !");
            return;
        }
    }
    //if it's a recent document item
    else
    {
        //just use the name as filename
        sprintf(tmpFilename, "%s", ((Fl_Menu_*)widget)->mvalue()->label());
        if(tmpFilename[0]=='\0' || strstr (tmpFilename, ".pdf" )==NULL)
        {
            fl_alert("Invalid filename !");
            return;
        }
    }

    //Open file and do stuff
    mw->Init(tmpFilename);
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

    if( mw->window->fullscreen_active() == 0)
    {
        //Go in fullscreen mode
        mw->window->fullscreen();
        mw->window->resizable(NULL);
        Fl::wait(1);
        //Hide the bars (but not really, just make them 0px width=> trick to keep the shortcuts and remember their size/position (only w() is forgotten but it's the window one))
        mw->MenuBar->resize(mw->MenuBar->x(), mw->MenuBar->y(), 0, mw->MenuBar->h());
        mw->StatusBar->resize(mw->StatusBar->x(), mw->StatusBar->y(), 0, mw->StatusBar->h());
        //Adapt the display size
        mw->Display->resize(0, 0, mw->window->w(), mw->window->h());
        Fl::wait(1);

        //Add the ESC control as a fullscreen shortcut
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Fullscreen2"))->activate();
    }
    else
    {
        //Go back from fullscreen
        mw->window->fullscreen_off();
        mw->window->resizable(NULL);
        Fl::wait(1);
        //Show the bars again: redefine their width equal to the window one
        mw->MenuBar->resize(mw->MenuBar->x(), mw->MenuBar->y(), mw->window->w(), 25);
        mw->StatusBar->resize(mw->StatusBar->x(), mw->window->h()-20, mw->window->w(), 20);
        //Re-adapt the display size
        mw->Display->resize(0, 25, mw->window->w(), mw->window->h()-45);
        mw->window->resizable(mw->Display);
        //and let some time to update that
        mw->window->redraw();
        Fl::wait(1);

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
    StatusBarUpdate(mw);
}
//! Switch to strip reading mode
void StripMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(STRIP);
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
}
//! Switch to case reading mode
void CaseMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(CASE);
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
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
    StatusBarUpdate(mw);
}

//! Go to next page (through Doc)
void PreviousPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Update the current page number
    int p=mw->Doc->PreviousPage();
    StatusBarUpdate(mw);
    //wait some time
    Fl::wait(10);
    //If still the same page asked, load it, otherwise do nothing
    if(p==mw->Doc->GetCurrentPageNb())
    {
        if(mw->Doc->LoadPage()==-3)
        {
            fl_alert("Unable to access the document. Try to reopen it.");
        }
        //Update everything
        mw->Display->SetImage(mw->Doc->Render());
        StatusBarUpdate(mw);
        Fl::check();
        //and then preload the surrounding pages
        mw->Doc->PreloadPages();
    }
}
//! Go to previous page (through Doc)
void NextPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //Update the current page number
    int p=mw->Doc->NextPage();
    StatusBarUpdate(mw);
    //wait some time
    Fl::wait(10);
    //If still the same page asked, load it, otherwise do nothing
    if(p==mw->Doc->GetCurrentPageNb())
    {
        if(mw->Doc->LoadPage()==-3)
        {
            fl_alert("Unable to access the document. Try to reopen it.");
        }
        //Update everything
        mw->Display->SetImage(mw->Doc->Render());
        StatusBarUpdate(mw);
        Fl::check();
        //and then preload the surrounding pages
        mw->Doc->PreloadPages();
    }
}
//! Go to next case (through Doc)
void PreviousCase_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    int ret = mw->Doc->PreviousCase();
    StatusBarUpdate(mw);
    mw->Display->SetImage(mw->Doc->Render());

    //If a new page has been loaded, preload
    if(ret<0)
    {
        Fl::check();
        mw->Doc->PreloadPages();
    }
}
//! Go to previous case (through Doc)
void NextCase_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    int ret = mw->Doc->NextCase();
    StatusBarUpdate(mw);
    mw->Display->SetImage(mw->Doc->Render());

    //If a new page has been loaded, preload
    if(ret<0)
    {
        Fl::check();
        mw->Doc->PreloadPages();
    }
}


//! Actvate or deactivate (depending on bool val), the navigation menu items (Next, Previous...)
void ActivateNavigationMenus(void *param, bool val)
{
    MainWindow *mw=(MainWindow*)param;

    //Browse all the menu items
    Fl_Menu_Item *items=(Fl_Menu_Item *)mw->MenuBar->menu();
    for(int i=0; i<mw->MenuBar->size(); i++)
    {
        //Check if they are navigation ones
        if(items[i].label()!=NULL &&
          (strcmp(items[i].label(), "Next page")==0 ||
           strcmp(items[i].label(), "Previous page")==0 ||
           strcmp(items[i].label(), "Next case")==0 ||
           strcmp(items[i].label(), "Previous case")==0)
          )
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
}

//! Activate all the menu items
void ActivateMenus(void * param)
{
      MainWindow *mw=(MainWindow*)param;

      //Activate all the menu items
      Fl_Menu_Item *items=(Fl_Menu_Item *)mw->MenuBar->menu();
      for(int i=0; i<mw->MenuBar->size(); i++)
            items[i].activate();

}
//-----------------------------------------------------------------------------



//----------------------------------Help menu----------------------------------
//! Show a help information window
void Help_cb(Fl_Widget* widget, void * param)
{
    //MainWindow *mw=(MainWindow*)param;

    //Load the text from the "help" file
    char HelpText[2048];
    FILE* helpfile=fopen("help", "r");
    if(helpfile)
    {
        int i=0;
        while(!feof(helpfile))
        {
            HelpText[i++]=fgetc(helpfile);
        }
        HelpText[--i]='\0';
        fclose(helpfile);

        //Display it on a message window
        fl_message(HelpText);
    }
}
//! Show the about information window
void About_cb(Fl_Widget* widget, void * param)
{
    //MainWindow *mw=(MainWindow*)param;

    //Load the text from the "about" file
    char AboutText[2048];
    FILE* aboutfile=fopen("about", "r");
    if(aboutfile)
    {
        int i=0;
        while(!feof(aboutfile))
        {
            AboutText[i++]=fgetc(aboutfile);
        }
        AboutText[--i]='\0';
        fclose(aboutfile);

        //Display it on a message window
        fl_message(AboutText);
    }
    else
    {
        fl_message("BDReader\n A comics BDReader\n\n Vincent Crocher - 2014. (See License.txt).\n     ---http:\\\\bdreader.sourceforge.net---\n");
    }
}

//-----------------------------------------------------------------------------


//! Update widgets according to preferences values
void UpdateWidgets(void * param)
{
    MainWindow *mw=(MainWindow*)param;

    //The reading mode
    if(mw->Pref->GetReadingMode()==PAGE)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Page mode"))->setonly();
    }
    if(mw->Pref->GetReadingMode()==CASE)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Case mode"))->setonly();
    }

    //Preview page
    if(mw->Pref->GetPagePreview())
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Page preview"))->set();
    }
    else
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Page preview"))->clear();
    }

    //Recent docs
    //TODO or not...current behaviour is ok
}


//! On closing the window (and thus exiting the program), save the parameters in preferences file and close everything (delete is called in main())
void Window_cb(Fl_Widget* widget, void * param)
{
     MainWindow *mw=(MainWindow*)param;

     //Save the preferences in file
     mw->Pref->Save(mw->Doc);

     //Close the window
     mw->window->hide();
}





//! Constructor: create the widgets, build the window and call the Init() with NULL parameters (no document)
MainWindow::MainWindow()
{
    //Load user preferences
    Pref = new Preferences();
    Pref->Load("preferences");

    //Main window creation
    window=new Fl_Double_Window(600, 600, "BDReader");

    //Set an icon
    //Probleme with windows compile (not declared): version prolem of fltk
    #ifndef WINDOWS
        fl_open_display();
        Pixmap p = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display), (char *)icon32_bits, icon32_width, icon32_height);
        window->icon((char *)p);
    #endif

    window->begin();

        Fl_Menu_Item menuitems[] =
        {
            { "&File",              0, 0, 0, FL_SUBMENU },
                { "Open", FL_COMMAND + 'o', (Fl_Callback *)Open_cb, this },
                { "Recent files", 0, 0, 0, FL_SUBMENU },
                    {Pref->GetRecentDocs(0), 0, (Fl_Callback *)Open_cb, this },
                    {Pref->GetRecentDocs(1), 0, (Fl_Callback *)Open_cb, this },
                    {Pref->GetRecentDocs(2), 0, (Fl_Callback *)Open_cb, this },
                    {Pref->GetRecentDocs(3), 0, (Fl_Callback *)Open_cb, this },
                    {Pref->GetRecentDocs(4), 0, (Fl_Callback *)Open_cb, this },
                    { 0 },
                { "Quit", FL_COMMAND + 'q', (Fl_Callback *)Quit_cb, this },
                { 0 },
            { "&View",              0, 0, 0, FL_SUBMENU },
                {"Fullscreen2", FL_Escape, (Fl_Callback *)Fullscreen_cb, this, FL_MENU_INVISIBLE | FL_MENU_INACTIVE},
                {"Fullscreen", FL_COMMAND + 'f', (Fl_Callback *)Fullscreen_cb, this, FL_MENU_DIVIDER },
                { "Page mode", 'p', (Fl_Callback *)PageMode_cb, this, FL_MENU_RADIO|FL_MENU_INACTIVE },
                { "Strip mode", 's', (Fl_Callback *)StripMode_cb, this, FL_MENU_RADIO|FL_MENU_INACTIVE },
                { "Case mode", 'c', (Fl_Callback *)CaseMode_cb, this, FL_MENU_RADIO|FL_MENU_DIVIDER | FL_MENU_INACTIVE },
                { "Page preview", 0, (Fl_Callback *)PagePreview_cb, this, FL_MENU_TOGGLE|FL_MENU_DIVIDER | FL_MENU_INACTIVE },
                { "Next page", FL_Down, (Fl_Callback *)NextPage_cb, this, FL_MENU_INACTIVE },
                { "Previous page", FL_Up, (Fl_Callback *)PreviousPage_cb, this, FL_MENU_INACTIVE },
                { "Next case", FL_Right, (Fl_Callback *)NextCase_cb, this, FL_MENU_INACTIVE },
                { "Previous case", FL_Left, (Fl_Callback *)PreviousCase_cb, this, FL_MENU_INACTIVE },
                { 0 },
            { "&Help",              0, 0, 0, FL_SUBMENU },
                { "Help", FL_COMMAND + 'h', (Fl_Callback *)Help_cb, this },
                { "About", 0, (Fl_Callback *)About_cb, this },
                { 0 },
            { 0 }
         };

         MenuBar = new Fl_Menu_Bar(0, 0, window->w(), 25);
         MenuBar->copy(menuitems);

         //Main display
         Display=new Fl_OpenCV(0, 25, window->w(), window->h()-45);

         //Status bar
         StatusBar = new Fl_Box(0, window->h()-20,window->w(), 20);
         //StatusBar->box(FL_FLAT_BOX);
         StatusBar->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);

         //Closing callback
         window->callback(Window_cb, (void*)this);


         //Handle mouse wheel events (combine with the timer and a global variable mouseWheel...to be improved)
         Fl::add_handler(MouseWheelEvents);
         //Timer initializaton
         Fl::add_timeout(0.1, Timer_cb, (void*)this);


         //Make the window resizable around the Display
         window->resizable(Display);

         //Apply the platic theme/skin
         Fl::scheme("plastic");

         //TODO: Option background color (blanc ou noir)
         //TODO: Remember the page nb for each recent documents in preferences and the window size
         //TODO: Improve the change of view modes (to previous and next pages)
         //TODO: Gerer les bds en suites d'images (folders) (et cbr ?)
         //TODO: Version windows avec installateur
         //TODO: Deb package (donc Makefile) pour Linux
    window->end();

    //Init
    UpdateWidgets(this);
    Doc=NULL;
    sprintf(Filename, "%s", "\0");
    Init(Filename);
}

//! Destructor
MainWindow::~MainWindow()
{
   if(Doc)
        delete Doc;

   if(window)
        delete window;

    delete Pref;
}

//! New document openning.
//!Create and load the document and update the window and widgets accordingly.
void MainWindow::Init(char *filename)
{
    //If filename is correct
    if(filename[0]!='\0')
    {
        //Save the filename
        sprintf(Filename, "%s", filename);

        //If an active document was open, save the preferences
        if(Doc)
        {
            //Save preferences (especially add the doc to the recent docs)
            Pref->Save(Doc);
        }

        //Create a document with the new filename
        Document *new_doc=new Document(Filename);
        //Check if the document has been opened
        if(new_doc->GetNumberOfPages()>0)
        {
            //Delete old doc if one
            if(Doc)
                delete Doc;
            //use the new one
            Doc=new_doc;

            //Load the first page
            Doc->LoadPage(1);

            //Apply the preferences to the document and the window widgets
            Pref->Load("preferences");
            UpdateWidgets((void*)this);

            //Get the image to draw
            Display->SetImage(Doc->Render());

            //Change the window title
            char title[1060];
            sprintf(title, "BDReader - %s", Filename);
            window->label(title);

            //Update status bar
            StatusBarUpdate(this);

            //Activate menus
            ActivateMenus(this);

            //Preload the surrounding pages
            Fl::check();
            Doc->PreloadPages();
        }
        //if the document couldn't be openned
        else
        {
            //Say it to the user
            fl_alert("Unable to open the document. Is it a valid pdf file ?");

            //Check if we have an active document
            if(!Doc)
            {
                //Update the window accordingly
                //menus
                ActivateNavigationMenus(this, false);
                //display
                Display->SetImage(NULL);
                //window title
                char title[1060];
                sprintf(title, "BDReader");
                window->label(title);
                //and status bar
                StatusBarUpdate(this);
            }
        }
    }

    //Show the main window
    Fl::visual(FL_RGB);
    window->show();
}
