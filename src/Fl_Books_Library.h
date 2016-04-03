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
/**
 * \file Fl_Books_Library.h
 * \brief Declaration file of the Fl_Books_Library and Fl_Library_Element classes.
 * \author Vincent Crocher
 * \version 0.1
 * \date February 2016
 *
 *
 */
#ifndef FL_BOOKS_LIBRARY_H
#define FL_BOOKS_LIBRARY_H

#include <string>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>

#ifdef __WINDOWS__
    #include <windows.h>
    #include <ole2.h>
    #include <shlobj.h>
    #include <shlguid.h>
    #include <shellapi.h>
    #include <commctrl.h>
    #include <commoncontrols.h>
    #include <time.h>
#else
    #include <glib.h>
#endif


class Fl_Library_Element: public Fl_Button
{
	#define LENGTH_SHORT 24

	public:
		Fl_Library_Element(int xx, int yy, int ww, int hh, const char *l, const char *abs_lib_dir, const char * relative_path, const char *filename, const char *windir);

		bool HasThumbnail(){if(Img->w()>0)return true;else return false;}
		bool HasPrefFile(){if(LastTime>0)return true;else return false;}
		long int GetLastTime(){return LastTime;}
		long int GetCreationTime(){return CreationTime;}
		int GetAlphaNumPosition(){return AlphaNumPosition;}
		void SetAlphaNumPosition(int val){AlphaNumPosition=val;}
		const char * GetFilename(){return Filename;}
		const char * GetShortFilenameLC(){return ShortFilenameLC;}
		char * GetAbsoluteFilename(){return AbsoluteFilename;}

	private:
	    Fl_Image * Img;

		char Filename[FL_PATH_MAX];
		char ShortFilenameLC[FL_PATH_MAX];
		char AbsoluteFilename[FL_PATH_MAX];
		char ShortFilename[LENGTH_SHORT];
		unsigned char FileURI[FL_PATH_MAX+7];
		char ThumbnailName[FL_PATH_MAX];
		char PrefFilename[FL_PATH_MAX];
		unsigned long int LastTime, CreationTime;
		int AlphaNumPosition; //Keep track of the position of elements in the original alphanum sorting (made by fltk function)
};




class Fl_Books_Library: public Fl_Double_Window
{
	public:
		Fl_Books_Library(int ww, int hh, const char *l, const char * lib_dir, const char *wind, unsigned int child_ww, unsigned int child_hh, unsigned int sp);
		~Fl_Books_Library();



        //Internal callbacks
        static void LibFolder_cb(Fl_Widget* v, void *p)
        {
            Fl_Books_Library* bl=((Fl_Books_Library*)p);
            bl->apply_dir(bl->LibFolder_Input->value(), bl->windir);
        }
        static void Filter_cb(Fl_Widget* w, void * p)
        {
            ((Fl_Books_Library*)p)->refresh();
        }
        static void Sort_cb(Fl_Widget* w, void * p)
        {
            ((Fl_Books_Library*)p)->refresh();
        }
        static void Search_cb(Fl_Widget* w, void * p)
        {
            ((Fl_Books_Library*)p)->refresh();
        }


		void spacing(int val)
		{
			_spacing=val;
			//Apply new spacing to main (Vertical) Pack
			Main_Table->spacing(_spacing);
			//Apply spacing to each subpack (Horizontals)
			for(int i=0; i<children(); i++)
			{
				//Check if child is an Fl_Pack and not the top level one (type(Fl_Pack::VERTICAL))
				if(child(i)->as_group() && child(i)->type()==Fl_Pack::HORIZONTAL)
				{
					((Fl_Pack*)child(i))->spacing(_spacing);
				}
			}
			_cols=(w()-spacing())/((child_w+spacing()));
			refresh();
		}

		int spacing()
		{
			return _spacing;
		}


		void callback(void (*cb)(Fl_Widget*, void *), void * param=NULL)
		{
		    //Remember callback
		    elem_callback=cb;
            elem_callback_param=param;

            //Assign callback to children
            for(int i=0; i<elements.size(); i++)
                elements[i]->callback(cb, param);
		}


        void add_subpack();
		bool add_element(const char *relative_path, const char *filename, const char *windir);
		void refresh();
		void reload();

        bool set_dir(const char * lib_dir, const char *wind);
        void apply_dir(const char * lib_dir=0, const char *wind=0);

        char *get_absolute_dir() {return abs_lib_dir;}

        //TO IMPROVE
        void resize(int xx, int yy, int ww, int hh)
        {
            int old_w=w(), old_h=h();
            Fl_Double_Window::resize(xx, yy, ww, hh);

            if(ww!=old_w || hh!=old_h)
            {
                MenuBar->resize(0, 0, w(), 25);
                if((MenuBar->child(MenuBar->children()))->w()<100)//the space children
                {
                    MenuBar->resizable(MenuBar);
                }
                else
                {
                    printf("ok\n");
                    MenuBar->resizable((MenuBar->child(MenuBar->children()-1)));
                }
                emptyboxtop->resize(Main_Scroll->x(), Main_Scroll->y(), Main_Scroll->w()-20, 10);
                Main_Table->resize(Main_Scroll->x(), Main_Scroll->y()+emptyboxtop->h(), Main_Scroll->w()-20, Main_Scroll->h()-emptyboxtop->h());
                _cols=(Main_Table->w()-spacing())/((child_w+spacing()));
                refresh();
                Main_Scroll->init_sizes();
                Main_Scroll->scroll_to(0, 0);
            }
        }


		int cols(){return _cols;}
		int rows(){return sub_packs.size();}

	private:
		int nb_visible_elem;
		unsigned int child_w, child_h, _spacing, _cols;

		Fl_Group *MenuBar;
		Fl_Input *LibFolder_Input;
		Fl_Choice *Filter_Choice;
		Fl_Choice *Sort_Choice;
		Fl_Input *Search_Input;
		Fl_Box *Search_Label;
		Fl_Scroll *Main_Scroll;
		Fl_Box *emptyboxtop, *emptyboxbottom;
		Fl_Pack *Main_Table;
		std::vector<Fl_Pack*> sub_packs;
		std::vector<Fl_Library_Element*> elements;
		void (*elem_callback)(Fl_Widget*, void *);
		void * elem_callback_param;

		char abs_lib_dir[FL_PATH_MAX];
		char windir[FL_PATH_MAX];
};


#endif // FL_BOOKS_LIBRARY_H
