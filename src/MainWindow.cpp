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
                sprintf(text, "Page %d/%d (Panel %d/%d)", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNumberOfPages(), mw->Doc->currentPage->GetActiveCase()+1, mw->Doc->currentPage->GetNbCases());
          }
          else
          {
                sprintf(text, "Page %d/%d", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNumberOfPages());
          }
      }
      else
      {
          sprintf(text, "%s", "\0");
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
            Fl_Menu_Item* nextcb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Next panel");
            if(nextcb && nextcb->active())
                nextcb->do_callback((Fl_Widget*)mw->MenuBar);
        }
        //if negative, go to previous case
        else
        {
            Fl_Menu_Item* previouscb=(Fl_Menu_Item*)mw->MenuBar->find_item("&View/Previous panel");
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
        Fl_Native_File_Chooser chooser=Fl_Native_File_Chooser();
        chooser.title("Open a pdf file");
        chooser.filter("Pdf\t*.pdf");
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
          (strcmp(items[i].label(), "Next page")==0 ||
           strcmp(items[i].label(), "Previous page")==0 ||
           strcmp(items[i].label(), "Next panel")==0 ||
           strcmp(items[i].label(), "Previous panel")==0)
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

//! Activate all the menu items and buttons
void ActivateWidgets(void * param)
{
      MainWindow *mw=(MainWindow*)param;

      //Activate all the menu items
      Fl_Menu_Item *items=(Fl_Menu_Item *)mw->MenuBar->menu();
      for(int i=0; i<mw->MenuBar->size(); i++)
            items[i].activate();

      //The navigation buttons
      mw->PreviousButton->show();
      mw->NextButton->show();
      mw->PreviousPageButton->show();
      mw->NextPageButton->show();
}
//-----------------------------------------------------------------------------



//----------------------------------Help menu----------------------------------
//! Show a help information window
void Help_cb(Fl_Widget* widget, void * param)
{
    //MainWindow *mw=(MainWindow*)param;

    //Load the text from the "help" file
    char HelpText[2048];
    #ifdef __WINDOWS__
        FILE* helpfile=fopen("help", "r");
    #else
        char filename[1024];
        sprintf(filename, "%s/.bdreader/help", getenv("HOME"));
        FILE* helpfile=fopen(filename, "r");
    #endif
    //If success
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
    #ifdef __WINDOWS__
        FILE* aboutfile=fopen("about", "r");
    #else
        char filename[1024];
        sprintf(filename, "%s/.bdreader/about", getenv("HOME"));
        FILE* aboutfile=fopen(filename, "r");
    #endif
    //If success
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
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Panel mode"))->setonly();
    }
    if(mw->Pref->GetReadingMode()==STRIP)
    {
        ((Fl_Menu_Item*)mw->MenuBar->find_item("&View/Strip mode"))->setonly();
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





//! Constructor
//!Create the widgets, build the window and call the Init() with NULL parameters (no document).
MainWindow::MainWindow()
{
    //Load user preferences
    Pref = new Preferences();
    Pref->Load("preferences");

    //Main window creation
    window=new Fl_Double_Window(600, 600, "BDReader");

    //Set an icon
    //Problem with windows compile (not declared): version prolem of fltk
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
                    { "Open", FL_COMMAND + 'o', (Fl_Callback *)Open_cb, this },
                    { "Recent files", 0, 0, 0, FL_SUBMENU },
                        { 0 },
                    { "Quit", FL_COMMAND + 'q', (Fl_Callback *)Quit_cb, this },
                    { 0 },
                { "&View",              0, 0, 0, FL_SUBMENU },
                    {"Fullscreen2", FL_Escape, (Fl_Callback *)Fullscreen_cb, this, FL_MENU_INVISIBLE | FL_MENU_INACTIVE},
                    {"Fullscreen", FL_COMMAND + 'f', (Fl_Callback *)Fullscreen_cb, this, FL_MENU_DIVIDER },
                    { "Panel mode", 'c', (Fl_Callback *)CaseMode_cb, this, FL_MENU_RADIO| FL_MENU_INACTIVE },
                    { "Strip mode", 's', (Fl_Callback *)StripMode_cb, this, FL_MENU_RADIO|FL_MENU_INACTIVE },
                    { "Page mode", 'p', (Fl_Callback *)PageMode_cb, this, FL_MENU_RADIO|FL_MENU_DIVIDER |FL_MENU_INACTIVE },
                    { "Page preview", 0, (Fl_Callback *)PagePreview_cb, this, FL_MENU_TOGGLE|FL_MENU_DIVIDER | FL_MENU_INACTIVE },
                    { "Next panel", FL_Right, (Fl_Callback *)NextCase_cb, this, FL_MENU_INACTIVE },
                    { "Previous panel", FL_Left, (Fl_Callback *)PreviousCase_cb, this, FL_MENU_INACTIVE },
                    { "Next page", FL_Down, (Fl_Callback *)NextPage_cb, this, FL_MENU_INACTIVE },
                    { "Previous page", FL_Up, (Fl_Callback *)PreviousPage_cb, this, FL_MENU_INACTIVE },
                    { 0 },
                { "&Help",              0, 0, 0, FL_SUBMENU },
                    { "Help", FL_COMMAND + 'h', (Fl_Callback *)Help_cb, this },
                    { "About", 0, (Fl_Callback *)About_cb, this },
                    { 0 },
                { 0 }
             };

             MenuBar = new Fl_Menu_Bar(0, 0, window->w(), 25);
             MenuBar->copy(menuitems);
             //Add recent documents items according to Preferences
             //WARNING: MenuItem is doing its own filtering to "delete" the potential doubles
             if(Pref->GetNbRecentDocs()>0)
             {
                 for(int i=0; i<Pref->GetNbRecentDocs(); i++)
                 {
                     char tmp[1024];
                     sprintf(tmp, "&File/Recent files/%s", Pref->GetRecentDocs(i));
                     MenuBar->add(tmp, 0, (Fl_Callback *)Open_cb, this);
                 }
             }
             //If no recent documents
             else
             {
                //Deactivate the corresponding menu item
                Fl_Menu_Item* recentitem=(Fl_Menu_Item*)MenuBar->find_item("&File/Recent files");
                recentitem->deactivate();
             }


         //Main display
             Display = new Fl_OpenCV(0, 25, window->w(), window->h()-45);


         //Navigation buttons
             NextButton = new Fl_Overlay_Arrow_Button(window->w()-25, (int)(Display->y()), 25, Display->h(), "", 0, 0.8, 0.15, 2);
             NextButton->color(fl_rgb_color(100, 100, 100));
             NextButton->selection_color(fl_rgb_color(245,245,200));
             NextButton->callback(NextCase_cb, (void*)this);
             NextButton->hide();
             PreviousButton = new Fl_Overlay_Arrow_Button(0, (int)(Display->y()), 25, Display->h(), "", 180, 0.8, 0.15, 2);
             PreviousButton->color(fl_rgb_color(100, 100, 100));
             PreviousButton->selection_color(fl_rgb_color(245,245,200));
             PreviousButton->callback(PreviousCase_cb, (void*)this);
             PreviousButton->hide();
             NextPageButton = new Fl_Overlay_Arrow_Button(0, (int)(Display->y()+Display->h()-25), Display->w(), 25, "", -90, 0.15, 0.8, 2);
             NextPageButton->color(fl_rgb_color(100, 100, 100));
             NextPageButton->selection_color(fl_rgb_color(245,245,200));
             NextPageButton->callback(NextPage_cb, (void*)this);
             NextPageButton->hide();
             PreviousPageButton = new Fl_Overlay_Arrow_Button(0, (int)(Display->y()), Display->w(), 25, "", 90, 0.15, 0.8, 2);
             PreviousPageButton->color(fl_rgb_color(100, 100, 100));
             PreviousPageButton->selection_color(fl_rgb_color(245,245,200));
             PreviousPageButton->callback(PreviousPage_cb, (void*)this);
             PreviousPageButton->hide();


         //Status bar
             StatusBar = new Fl_Box(0, window->h()-20,window->w(), 20);
             //StatusBar->box(FL_FLAT_BOX);
             StatusBar->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);


         //Events and callbacks
             //Closing callback
             window->callback(Window_cb, (void*)this);

             //Handle mouse wheel events (combine with the timer and a global variable mouseWheel...to be improved)
             Fl::add_handler(MouseWheelEvents);
             //Timer initializaton
             Fl::add_timeout(0.1, Timer_cb, (void*)this);

             //Make the window resizable around the Display (and define minimum dimensions)
             window->resizable(Display);
             window->size_range(200, 200, 0, 0, 0, 0, 0);

             //Apply the platic theme/skin
             Fl::scheme("plastic");

         //TODO: Option background color (blanc ou noir)
         //TODO: Remember the page nb for each recent documents in preferences and the window size
         //TODO: Improve the change of view modes (to previous and next pages)
         //TODO: Gerer les bds en suites d'images (folders) (et cbr ?)
         //TODO: Version windows avec installateur
         //TODO: Deb package (donc Makefile) pour Linux
    window->end();


    //Initialize the window and Document
    UpdateWidgets(this);
    Doc=NULL;
    sprintf(Filename, "%s", "\0");
    Init(Filename);
}

//! Destructor
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
//!\param filename: the full filename (path+name) of the document to open, can be "\0" if nothing
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
            Pref->Apply(Doc);
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
            ActivateWidgets(this);

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
                ActivateNavigationWidgets(this, false);
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
