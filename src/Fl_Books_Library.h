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
 * \version 0.2
 * \date June 2016
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
#include <FL/fl_draw.H>
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
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Native_File_Chooser.H>

#ifdef __WINDOWS__
    #include <windows.h>
    #include <ole2.h>
    #include <shlobj.h>
    #include <shlguid.h>
    #include <shellapi.h>
    #include <commctrl.h>
    #include <commoncontrols.h>
    #include <time.h>
#endif

#include "md5.h"
#include "Theme.h"
#include "Fl_Path_Bar.h"
#include "Tags.h"
#include <functional>

#include "Preferences.h"
#include "Document.h"
#include "Document_pdf.h"
#include "Document_archive.h"
#include "Document_folder.h"

#define MAX_LIBRARY_DEPTH 8 //Sub-folders levels browsed in the library folder

class Fl_Books_Library;

class Fl_Library_Element: public Fl_Button
{
	#define LENGTH_SHORT 24

	public:
		Fl_Library_Element(int xx, int yy, int ww, int hh, const char *l, const char *abs_lib_dir, const char * relative_path, const char *filename, const char *windir);

		bool HasThumbnail(){return Img && Img->w()>0;}
		int handle(int event);
		void draw();
		bool HasPrefFile(){if(LastTime>0)return true;else return false;}
		int GetLastPage(){return LastPage;}
		long int GetLastTime(){return LastTime;}
		unsigned long int GetCreationTime(){return CreationTime;}
		unsigned long int GetAccessTime(){return AccessTime;}
		int GetAlphaNumPosition(){return AlphaNumPosition;}
		void SetAlphaNumPosition(int val){AlphaNumPosition=val;}
		const char * GetFilename(){return Filename;}
		const char * GetShortFilenameLC(){return ShortFilenameLC;}
		char * GetAbsoluteFilename(){return AbsoluteFilename;}
		//! Folder of the comic, relative to the library folder ("" or ending with '/')
		const char * GetRelativeDir(){return RelativeDir;}
		const std::vector<int> &GetTags(){return TagIds;}
		//! Identity of the comic for the tags: its path in the library
		std::string TagKey(){return std::string(RelativeDir)+Filename;}
		bool HasTag(int id);
		void ReloadTags();

		Fl_Books_Library *Library;  //!< Library owning the element (context menu)

	private:
	    void ShowTagsMenu();

	    Fl_Image * Img;
	    bool Hover;
	    char RelativeDir[FL_PATH_MAX];
	    std::vector<int> TagIds;

		char Filename[FL_PATH_MAX];
		char ShortFilenameLC[FL_PATH_MAX];
		char AbsoluteFilename[FL_PATH_MAX];
		char ShortFilename[LENGTH_SHORT];
		unsigned char FileURI[FL_PATH_MAX+7];
		char ThumbnailName[FL_PATH_MAX];
		char PrefFilename[FL_PATH_MAX];
		long int LastTime;
		unsigned long int CreationTime, AccessTime;
		int LastPage, NbPages;
		int AlphaNumPosition; //Keep track of the position of elements in the original alphanum sorting (made by fltk function)
};




//! A sub-folder of the current library folder (folders view)
class Fl_Library_Folder: public Fl_Button
{
	public:
		Fl_Library_Folder(int ww, int hh, const std::string &name, const std::string &relative_dir, int count);
		~Fl_Library_Folder();
		int handle(int event);
		void draw();

		std::string Name, RelativeDir;
		int Count;  //!< Comics inside (sub-folders included)

	private:
		bool Hover;
		Fl_SVG_Image *Icon, *IconHover;
};


class Fl_Books_Library: public Fl_Group//public Fl_Double_Window
{
	public:
		Fl_Books_Library(int xx, int yy, int ww, int hh, const char *l, const char * lib_dir, const char *wind, unsigned int child_ww, unsigned int child_hh, unsigned int sp);
		~Fl_Books_Library();



        //Internal callbacks
        static void LibFolder_cb(Fl_Widget* v, void *p)
        {
            ((Fl_Books_Library*)p)->path_chosen(((Fl_Books_Library*)p)->LibFolder_Input->value());
        }
        static void Folder_cb(Fl_Widget* w, void * p)
        {
            ((Fl_Books_Library*)p)->open_folder(((Fl_Library_Folder*)w)->RelativeDir);
        }
        static void Tag_cb(Fl_Widget* w, void * p)
        {
            ((Fl_Books_Library*)p)->refresh();
        }
        static void View_cb(Fl_Widget* w, void * p)
        {
            Fl_Books_Library *bl=(Fl_Books_Library*)p;
            bl->FoldersView=!bl->FoldersView;
            bl->View_Button->SetToggled(bl->FoldersView);
            bl->refresh();
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

        //External callbacks
        friend void SelectLibFolder_cb(Fl_Widget* v, void *p);


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
		void scan_entry(const char *relative_dir, const char *name, int depth);
		void refresh();
		void reload();

		void set_filter(int idx)
		{
		    Filter_Choice->value(idx);
		    refresh();
		}

        bool set_dir(const char * lib_dir, const char *wind);
        void apply_dir(const char * lib_dir=0, const char *wind=0);

        char *get_absolute_dir() {return abs_lib_dir;}

        //! Show a folder of the library ("" for the library folder, else ending with '/')
        void open_folder(const std::string &relative_dir);
        //! The tags changed (settings, context menu): chips, tag filter
        void refresh_tags();
        int handle(int event);

        std::function<void()> OnManageTags;    //!< "Manage tags..." of the comics context menu

        int GetSuggestionNext(char current_filename[2048], char suggestion_filename[2048], char suggestion_short_filename[2048]);

        void resize(int xx, int yy, int ww, int hh);

		int cols(){return _cols;}
		int rows(){return sub_packs.size();}

	private:
		int nb_visible_elem;
		unsigned int child_w, child_h, _spacing, _cols;

		Fl_Group *MenuBar;
		Fl_Group * Main_Group;
		Fl_Input *LibFolder_Input;
		Fl_Choice *Filter_Choice;
		Fl_Choice *Sort_Choice;
		Fl_Choice *Tag_Choice;
		std::vector<int> TagChoiceIds;  //!< Tag of each Tag_Choice item (after "Any")
		Fl_Activity_Button *View_Button;
		bool FoldersView;               //!< Folders view (else all the comics in one list)
		std::string CurrentDir;         //!< Folder shown, relative to the library folder
		std::vector<Fl_Library_Folder*> folders;
		Fl_Group * Search_Group;
		Fl_Hint_Input *Search_Input;
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

		void layout_toolbar();
		void update_path_bar();
		void path_chosen(const char *path);
		void fill_tag_choice();
};


#endif // FL_BOOKS_LIBRARY_H
