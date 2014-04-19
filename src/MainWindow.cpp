#include "MainWindow.h"



/*Timer retrieving information from the input program (vision)
void Timer_cb(void* param)
{
      MainWindow *mw=(MainWindow*)param;

      //Repeat...
      Fl::repeat_timeout(0.01, Timer_cb, (void*)mw);
}*/


void StatusBarUpdate(void * param)
{
      MainWindow *mw=(MainWindow*)param;

      //Get number of pages... lbala
      char text[1024];
      if(mw->Doc->GetReadingMode()==CASE)
      {
            sprintf(text, "Page %d/%d (Case %d/%d)", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNumberOfPages(), mw->Doc->CurrentPage->GetActiveCase()+1, mw->Doc->CurrentPage->GetNbCases());
      }
      else
      {
            sprintf(text, "Page %d/%d", mw->Doc->GetCurrentPageNb(), mw->Doc->GetNumberOfPages());
      }

      mw->StatusBar->copy_label(text);
}


//Catch the mouse wheel events
//TODO: pas de parametres.... donc pas de mw....
int MouseWheelEvents(int p)
{
    if(Fl::event()==FL_MOUSEWHEEL)
    {
        //printf("Hop %d\n", Fl::event_dy());
        if(abs(Fl::event_dy()>0))
        {
            return 0;
        }
        return 1;
    }
    else
    {
        //say fltk that the event has not been managed
        return 0;
    }

}



//---------------------------------File menu----------------------------------
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


void Quit_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->window->do_callback();
}
//-----------------------------------------------------------------------------



//----------------------------------View menu----------------------------------
void Fullscreen_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;
    //printf("%s\n", widget->label());

    if( mw->window->fullscreen_active() == 0)
    {
        mw->window->fullscreen();
    }
    else
    {
        mw->window->fullscreen_off();
    }

}

void PageMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(PAGE);
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
}
void StripMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(STRIP);
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
}
void CaseMode_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->SetReadingMode(CASE);
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
}

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

void PreviousPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    ActivateNavigationMenus(mw, false);
    //Et vider la pile de touches ???
    mw->Doc->PreviousPage();
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
    ActivateNavigationMenus(mw, true);
}
void NextPage_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    ActivateNavigationMenus(mw, false);
    //Et vider la pile de touches ???
    mw->Doc->NextPage();
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
    ActivateNavigationMenus(mw, true);
}
void PreviousCase_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->PreviousCase();
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
}
void NextCase_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    mw->Doc->NextCase();
    mw->Display->SetImage(mw->Doc->Render());
    StatusBarUpdate(mw);
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
void Help_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    char HelpText[2048];
    FILE* helpfile=fopen("help", "r");
    int i=0;
    while(!feof(helpfile))
    {
        HelpText[i++]=fgetc(helpfile);
    }
    HelpText[--i]='\0';
    fclose(helpfile);

    fl_message(HelpText);
}

void About_cb(Fl_Widget* widget, void * param)
{
    MainWindow *mw=(MainWindow*)param;

    char AboutText[2048];
    FILE* aboutfile=fopen("about", "r");
    int i=0;
    while(!feof(aboutfile))
    {
        AboutText[i++]=fgetc(aboutfile);
    }
    AboutText[--i]='\0';
    fclose(aboutfile);

    fl_message(AboutText);
}

//-----------------------------------------------------------------------------




//! On closing the window, save the parameters close everything (delete is called in main)
void Window_cb(Fl_Widget* widget, void * param)
{
     MainWindow *mw=(MainWindow*)param;

     //Save the preferences in file
     mw->Pref->Save(mw->Doc);

     //Close the window
     mw->window->hide();
}

//! Constructor: create the widgets and build the window and call the Init() with NULL parameters (no document)
MainWindow::MainWindow()
{
    //Load user preferences
    Pref = new Preferences();
    Pref->Load("preferences");

    //Main window creation
    window=new Fl_Double_Window(1024, 640, "BDReader");

    //Set an icon
    fl_open_display();
    Pixmap p = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display), (char *)icon32_bits, icon32_width, icon32_height);
    window->icon((char *)p);

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
                {"Fullscreen2", FL_Escape, (Fl_Callback *)Fullscreen_cb, this, FL_MENU_INVISIBLE },
                {"Fullscreen", FL_COMMAND + 'f', (Fl_Callback *)Fullscreen_cb, this, FL_MENU_DIVIDER },
                { "Page mode", 'p', (Fl_Callback *)PageMode_cb, this, FL_MENU_RADIO|FL_MENU_INACTIVE },
                //{ "Strip mode", 's', (Fl_Callback *)StripMode_cb, this, FL_MENU_RADIO|FL_MENU_INACTIVE },
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

         //Make it resizable
         window->resizable(Display);

         //Handle mouse wheel events
         Fl::add_handler(MouseWheelEvents);

         //TODO: gerer molette: chgt case ou page
         //TODO: tempo pour verifier qu'une nouvelle page/case a ete chargee
         //(TODO: meilleur decoupage des pages par defaut: recherche lignes horizontales (si 4 trouvees, les utiliser, sinon decoupage a 10%))
         //TODO: Option background color (blanc ou noir)
         //TODO: Remember the page nb for each recent documents in preferences
         //TODO: FUITE MEMOIRE
         //TODO: Meilleur gestion chgt des pages...
         //TODO: Gerer les bds en suites d'images (folders) (et cbr ?)
         //TODO: Version windows avec installateur
         //TODO: Deb package (donc Makefile) pour Linux
    window->end();

    //Init
    Doc=NULL;
    sprintf(Filename, "\0");
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


void MainWindow::Init(char *filename)
{
    //If filename is correct
    if(filename[0]!='\0')
    {
        //Save the filename
        sprintf(Filename, "%s", filename);

        //Save pref and delete old doc if one
        if(Doc)
        {
            //Save preferences (especially add the doc to the recent docs)
            Pref->Save(Doc);
            //Delete it
            delete Doc;
        }

        //Create a document with the new filename
        Doc=new Document(Filename);

        //Set to first page
        Doc->LoadPage(1);

        //Apply the preferences to the document
        Pref->Load("preferences");
        Pref->Apply(Doc);

        //Update the widget values according to these preferences
        //TODO

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
    }

    //Show the main window
    Fl::visual(FL_RGB);
    window->show();

    //Timer initializaton
    //Fl::add_timeout(0.1, Timer_cb, (void*)this);
}
