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
 * \file Theme.h
 * \brief Dark theme (VSCodium "Dark Modern" like): colors, fonts, flat boxes and the activity bar buttons
 *
 */
#ifndef THEME_H
#define THEME_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_Input.H>
#include <string>
#include <functional>

//Palette
#define THEME_EDITOR_BG     fl_rgb_color(31, 31, 31)    //!< Reading area, library
#define THEME_BAR_BG        fl_rgb_color(24, 24, 24)    //!< Menu, activity, side and status bars
#define THEME_BORDER        fl_rgb_color(43, 43, 43)    //!< Separation lines
#define THEME_FG            fl_rgb_color(204, 204, 204) //!< Text
#define THEME_FG_DIM        fl_rgb_color(134, 134, 134) //!< Secondary text, inactive icons
#define THEME_FG_BRIGHT     fl_rgb_color(231, 231, 231) //!< Active icons
#define THEME_INPUT_BG      fl_rgb_color(49, 49, 49)    //!< Text inputs, choices
#define THEME_INPUT_BORDER  fl_rgb_color(60, 60, 60)
#define THEME_HOVER         fl_rgb_color(43, 45, 46)    //!< Hovered items
#define THEME_ACCENT        fl_rgb_color(0, 120, 212)   //!< Selection, focus, active view

//Sizes
#define THEME_MENU_H        30  //!< Menu bar height
#define THEME_STATUS_H      24  //!< Status bar height
#define THEME_ACTIVITY_W    48  //!< Activity bar width
#define THEME_SIDEBAR_HEADER_H 32 //!< Side bar title height

//Box types (flat, with a thin border line)
#define THEME_INPUT_BOX     FL_FREE_BOXTYPE          //!< Filled, 1px border all around
#define THEME_STATUS_BOX    (Fl_Boxtype)(FL_FREE_BOXTYPE+1) //!< Filled, 1px border on top
#define THEME_SIDEBAR_BOX   (Fl_Boxtype)(FL_FREE_BOXTYPE+2) //!< Filled, 1px border on the right

//! Apply the theme to FLTK (colors, fonts, schemes, box types). Call before creating the widgets
void ApplyTheme();

//! Dark native title bar (Windows 10/11), the window must be shown
void ApplyDarkTitleBar(Fl_Window *win);

//! The next message box (fl_alert, fl_choice, fl_input...) centered on the ClaudeReader window
//! instead of under the mouse, where it could be cut by the screen edge (issue #7)
void CenterNextMessage();

//! Icons (24x24 SVG, stroke drawn) of the user interface
enum THEME_ICON { ICON_LIBRARY, ICON_READER, ICON_OPEN, ICON_PAGES, ICON_MANGA, ICON_FULLSCREEN, ICON_HELP, ICON_CLOSE, ICON_BOOK,
                  ICON_STORE, ICON_SEARCH, ICON_REFRESH, ICON_GEAR, ICON_TAG };

//! Icon image of the given color and size (pixels), to be deleted by the caller
Fl_SVG_Image *ThemeIcon(THEME_ICON icon, Fl_Color color, int size);

//! Square icon button of the activity bar (and side bar): dim icon, bright when hovered,
//!accent bar on the left when it is the current view, accent icon when a toggle is on
class Fl_Activity_Button : public Fl_Button
{
    public:
        Fl_Activity_Button(int X, int Y, int W, int H, THEME_ICON icon, int icon_size=24);
        ~Fl_Activity_Button();

        int handle(int event);
        void draw();

        void SetCurrentView(bool val) {CurrentView=val; redraw();}  //!< Accent bar on the left side
        void SetToggled(bool val) {Toggled=val; redraw();}          //!< Accent colored icon

    private:
        Fl_SVG_Image *IconDim, *IconBright, *IconAccent;
        bool Hover, CurrentView, Toggled;
};

//! Text input showing a hint when empty
class Fl_Hint_Input : public Fl_Input
{
    public:
        Fl_Hint_Input(int X, int Y, int W, int H):Fl_Input(X, Y, W, H) {}
        void draw();
        int handle(int event);
        std::string Hint;
        std::function<void()> OnEmptyBackspace;    //!< Backspace with nothing to delete
};

#endif // THEME_H
