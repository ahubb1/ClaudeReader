//--------------------------------------------------------------------------
//
//    Copyright (C) 2014-2016  Vincent Crocher
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
 * \file Fl_Path_Bar.h
 * \brief File explorer like path bar (breadcrumb)
 *
 */
#ifndef FL_PATH_BAR_H
#define FL_PATH_BAR_H

#include <string>
#include <vector>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include "Theme.h"

class Fl_Path_Bar;

//! Text field of the path bar (edit mode): leaves the edit mode when losing the focus or with Esc
class Fl_Path_Input : public Fl_Input
{
    public:
        Fl_Path_Input(int X, int Y, int W, int H, Fl_Path_Bar *bar):Fl_Input(X, Y, W, H), Bar(bar) {}
        int handle(int event);
    private:
        Fl_Path_Bar *Bar;
};

//! File explorer like path bar: each folder of the path is a clickable segment, the chevron after a folder
//!lists its sub-folders, a click in the empty part edits the path as text and the folder button browses.
/**The current path is the value of input(): its callback is called when a new folder is chosen (the value is
then the new folder). The path is a folder, with '/' or '\' separators.*/
class Fl_Path_Bar : public Fl_Group
{
    public:
        Fl_Path_Bar(int X, int Y, int W, int H);

        Fl_Input *input() {return Input;}                       //!< Path value and "folder chosen" callback
        Fl_Activity_Button *browse_button() {return BrowseButton;} //!< Folder chooser button

        void draw();
        int handle(int event);
        void EditMode(bool on);

    private:
        enum ITEM_KIND { ITEM_FOLDER, ITEM_CHEVRON, ITEM_OVERFLOW };
        struct Item
        {
            int x, w;
            std::string label;
            std::string path;                   //!< Folder of the item (for a chevron: the folder whose sub-folders are listed)
            std::vector<std::string> hidden;    //!< Overflow item: the hidden folders
            ITEM_KIND kind;
        };

        void LayoutItems();
        int ItemAt(int X);
        void Navigate(const std::string &path);
        void PopupSubFolders(const Item &item);
        void PopupHidden(const Item &item);

        std::vector<Item> Items;
        int HoverItem;
        bool Editing;
        Fl_Path_Input *Input;
        Fl_Activity_Button *BrowseButton;
};

#endif // FL_PATH_BAR_H
