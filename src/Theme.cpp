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
#include "Theme.h"
#include <FL/fl_draw.H>
#include <FL/Fl_Tooltip.H>
#include <string>
#ifdef _WIN32
    #include <FL/platform.H>
    #include <dwmapi.h>
#endif


//------------------------------------Box types------------------------

//! Flat fill with a 1px border all around
static void input_box(int x, int y, int w, int h, Fl_Color c)
{
    fl_color(c);
    fl_rectf(x, y, w, h);
    fl_color(THEME_INPUT_BORDER);
    fl_rect(x, y, w, h);
}

//! Flat fill with a 1px border on top
static void status_box(int x, int y, int w, int h, Fl_Color c)
{
    fl_color(c);
    fl_rectf(x, y, w, h);
    fl_color(THEME_BORDER);
    fl_xyline(x, y, x+w-1);
}

//! Flat fill with a 1px border on the right
static void sidebar_box(int x, int y, int w, int h, Fl_Color c)
{
    fl_color(c);
    fl_rectf(x, y, w, h);
    fl_color(THEME_BORDER);
    fl_yxline(x+w-1, y, y+h-1);
}

//---------------------------------------------------------------------------


void ApplyTheme()
{
    //Flat look, no gradients
    Fl::scheme("base");

    //Colors: also builds the gray ramp used by the remaining standard boxes
    Fl::background(24, 24, 24);
    Fl::background2(49, 49, 49);
    Fl::foreground(204, 204, 204);
    Fl::set_color(FL_SELECTION_COLOR, THEME_ACCENT);
    Fl::set_color(FL_INACTIVE_COLOR, 90, 90, 90);

    //Fonts (Segoe UI on Windows, the FLTK font name prefix gives the style)
    #ifdef _WIN32
        Fl::set_font(FL_HELVETICA, " Segoe UI");
        Fl::set_font(FL_HELVETICA_BOLD, "BSegoe UI Semibold");
    #endif
    FL_NORMAL_SIZE=13;

    //Tooltips
    Fl_Tooltip::color(fl_rgb_color(37, 37, 38));
    Fl_Tooltip::textcolor(THEME_FG);
    Fl_Tooltip::font(FL_HELVETICA);
    Fl_Tooltip::size(12);
    Fl_Tooltip::margin_width(8);
    Fl_Tooltip::margin_height(5);

    //Menus and scrollbars
    Fl::menu_linespacing(10);
    Fl::scrollbar_size(12);

    //No dotted focus rectangles
    Fl::visible_focus(0);

    //Flat boxes
    Fl::set_boxtype(THEME_INPUT_BOX, input_box, 1, 1, 2, 2);
    Fl::set_boxtype(THEME_STATUS_BOX, status_box, 0, 1, 0, 1);
    Fl::set_boxtype(THEME_SIDEBAR_BOX, sidebar_box, 0, 0, 1, 0);
    Fl::set_boxtype(FL_UP_BOX, FL_FLAT_BOX);
    Fl::set_boxtype(FL_DOWN_BOX, THEME_INPUT_BOX);
    Fl::set_boxtype(FL_THIN_UP_BOX, FL_FLAT_BOX);
    Fl::set_boxtype(FL_THIN_DOWN_BOX, THEME_INPUT_BOX);
    Fl::set_boxtype(FL_EMBOSSED_BOX, FL_FLAT_BOX);
}

void ApplyDarkTitleBar(Fl_Window *win)
{
    #ifdef _WIN32
        BOOL dark=TRUE;
        HWND hwnd=fl_win32_xid(win);
        if(hwnd)
        {
            //DWMWA_USE_IMMERSIVE_DARK_MODE: 20 on Windows 10 20H1 and later, 19 before
            if(DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark))!=S_OK)
                DwmSetWindowAttribute(hwnd, 19, &dark, sizeof(dark));
        }
    #endif
}


//------------------------------------Icons------------------------

//! SVG paths of the icons (24x24, stroke drawn with the color COLOR)
static const char *IconPaths(THEME_ICON icon)
{
    switch(icon)
    {
        case ICON_LIBRARY: //Books on a shelf
            return "<rect x='3.5' y='4' width='4' height='16' rx='.5'/><rect x='9.5' y='4' width='4' height='16' rx='.5'/>"
                   "<path d='M15.6 5.4l3.7-1 4 15.4-3.7 1z'/>";
        case ICON_READER: //Open book
            return "<path d='M3 5.5c3-1.2 6-1 9 1.2v13c-3-2.2-6-2.4-9-1.2z'/><path d='M21 5.5c-3-1.2-6-1-9 1.2v13c3-2.2 6-2.4 9-1.2z'/>";
        case ICON_OPEN: //Folder
            return "<path d='M3 6.5a1 1 0 0 1 1-1h5l2 2.2h9a1 1 0 0 1 1 1v9.8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1z'/>";
        case ICON_PAGES: //Side bar layout
            return "<rect x='3' y='4.5' width='18' height='15' rx='1'/><path d='M9 4.5v15'/>";
        case ICON_MANGA: //"JP" brush calligraphy: filled strokes (pressed entries, J flick, P needle tail, ink flecks)
            return "<g fill='COLOR' stroke='none' transform='translate(1.2 0) skewX(-6)'>"
                   "<path d='M4.6 4.7C5.3 3.1 8.7 2.0 11.1 3.0C11.8 3.3 11.7 4.3 11.1 4.9C10.6 5.4 10.3 6.2 10.3 7.2L10.2 13.8"
                   "C10.1 18.1 8.4 21.0 5.3 21.5C4.1 21.7 2.8 21.1 2.1 19.9C1.9 19.4 2.3 19.1 2.8 19.4C3.7 19.9 4.6 19.9 5.4 19.3"
                   "C7.0 18.2 7.8 16.3 7.9 13.5L8.0 6.4C8.0 5.6 7.4 5.1 6.5 4.9C5.8 4.8 5.0 5.0 4.6 4.7Z'/>"
                   "<path d='M12.6 3.8C13.4 3.0 15.8 2.8 16.1 4.1C16.2 4.7 15.8 5.2 15.6 5.9L15.2 16.5C15.1 19.0 14.6 21.1 13.9 22.3"
                   "C13.7 22.6 13.4 22.5 13.4 22.1L13.1 16.8L12.9 6.0C12.9 5.1 12.8 4.4 12.6 3.8Z'/>"
                   "<path d='M15.3 4.3C19.0 3.0 22.7 3.9 22.6 7.6C22.5 11.1 19.2 13.3 15.7 13.2C15.0 13.2 14.8 12.5 15.4 12.3"
                   "C18.0 11.5 19.9 10.0 20.0 7.9C20.1 5.9 18.5 5.0 15.7 5.5Z'/>"
                   "<circle cx='12.1' cy='2.2' r='0.45'/><circle cx='23.0' cy='11.9' r='0.35'/><circle cx='1.4' cy='18.6' r='0.3'/>"
                   "</g>";
        case ICON_FULLSCREEN: //Corners
            return "<path d='M4 9V4h5M20 9V4h-5M4 15v5h5M20 15v5h-5'/>";
        case ICON_HELP: //Question mark in a circle
            return "<circle cx='12' cy='12' r='9'/><path d='M9.6 9.5a2.5 2.5 0 1 1 3.6 2.2c-.7.4-1.2.9-1.2 1.7v.6'/><path d='M12 16.8v.4'/>";
        case ICON_CLOSE: //Cross
            return "<path d='M7 7l10 10M17 7L7 17'/>";
        case ICON_STORE: //Shopping bag
            return "<path d='M5 8h14l-1.2 12.2a1 1 0 0 1-1 .8H7.2a1 1 0 0 1-1-.8z'/><path d='M9 10.5V7a3 3 0 0 1 6 0v3.5'/>";
        case ICON_SEARCH: //Magnifier
            return "<circle cx='10.5' cy='10.5' r='6.5'/><path d='M15.5 15.5L20.5 20.5'/>";
        case ICON_REFRESH: //Circular arrow
            return "<path d='M19.5 12a7.5 7.5 0 1 1-2.2-5.3'/><path d='M19.5 4.5v4h-4'/>";
        case ICON_GEAR: //Settings
            return "<circle cx='12' cy='12' r='3'/><path d='M12 2.8l1.6 2.3 2.8-.6.6 2.8 2.3 1.6-1.4 2.5 1.4 2.5-2.3 1.6-.6 2.8-2.8-.6L12 21.2l-1.6-2.3-2.8.6-.6-2.8-2.3-1.6L6.1 12 4.7 9.5 7 7.9l.6-2.8 2.8.6z'/>";
        case ICON_TAG: //Label
            return "<path d='M3.5 12.2V4.5a1 1 0 0 1 1-1h7.7l8.3 8.3a1 1 0 0 1 0 1.4l-7.4 7.4a1 1 0 0 1-1.4 0z'/><circle cx='8' cy='8' r='1.3'/>";
        case ICON_BOOK: //Closed book (status bar badge)
            return "<path d='M5 4.5h12a1 1 0 0 1 1 1v14H6.5A1.5 1.5 0 0 1 5 18z'/><path d='M5 18a1.5 1.5 0 0 1 1.5-1.5H18'/>";
        default:
            return "";
    }
}

//! Build an icon image of the given color and size (see Theme.h)
Fl_SVG_Image *ThemeIcon(THEME_ICON icon, Fl_Color color, int size)
{
    unsigned char r, g, b;
    Fl::get_color(color, r, g, b);
    char rgb[16];
    snprintf(rgb, sizeof(rgb), "#%02x%02x%02x", r, g, b);

    std::string svg="<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='";
    svg+=rgb;
    svg+="' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'>";
    //Filled icons use the COLOR token
    std::string paths=IconPaths(icon);
    for(size_t pos=paths.find("COLOR"); pos!=std::string::npos; pos=paths.find("COLOR", pos))
        paths.replace(pos, 5, rgb);
    svg+=paths;
    svg+="</svg>";

    Fl_SVG_Image *img=new Fl_SVG_Image(NULL, svg.c_str());
    img->resize(size, size);
    return img;
}


//------------------------------------Activity button------------------------

Fl_Activity_Button::Fl_Activity_Button(int X, int Y, int W, int H, THEME_ICON icon, int icon_size):
    Fl_Button(X, Y, W, H, 0),
    MenuPath(NULL),
    Hover(false),
    CurrentView(false),
    Toggled(false)
{
    IconDim=ThemeIcon(icon, THEME_FG_DIM, icon_size);
    IconBright=ThemeIcon(icon, THEME_FG_BRIGHT, icon_size);
    IconAccent=ThemeIcon(icon, fl_rgb_color(55, 148, 255), icon_size);
    box(FL_FLAT_BOX);
    color(THEME_BAR_BG);
    clear_visible_focus(); //Keep the arrow keys for the navigation
}

Fl_Activity_Button::~Fl_Activity_Button()
{
    delete IconDim;
    delete IconBright;
    delete IconAccent;
}

int Fl_Activity_Button::handle(int event)
{
    switch(event)
    {
        case FL_ENTER:
            Hover=true;
            redraw();
            break;
        case FL_LEAVE:
            Hover=false;
            redraw();
            break;
        default:
            break;
    }
    return Fl_Button::handle(event);
}

void Fl_Activity_Button::draw()
{
    //Background
    fl_color(color());
    fl_rectf(x(), y(), w(), h());

    //Highlight tile: tinted when a toggle is on, subtle when hovered
    bool on=Toggled && active_r();
    if(on || (Hover && active_r()))
    {
        fl_color(on ? fl_rgb_color(4, 57, 94) : THEME_HOVER);
        fl_rectf(x()+5, y()+5, w()-10, h()-10);
    }

    //Current view or toggle on marker
    if(CurrentView || on)
    {
        fl_color(THEME_ACCENT);
        fl_rectf(x(), y(), 2, h());
    }

    //Icon, centered
    Fl_SVG_Image *icon=IconDim;
    if(active_r() && (on || Hover || CurrentView))
        icon=IconBright;
    icon->draw(x()+(w()-icon->w())/2, y()+(h()-icon->h())/2);
}


//------------------------------------Hint input------------------------

void Fl_Hint_Input::draw()
{
    Fl_Input::draw();
    if(size()==0 && Fl::focus()!=this && !Hint.empty())
    {
        fl_font(textfont(), textsize());
        fl_color(THEME_FG_DIM);
        fl_draw(Hint.c_str(), x()+6, y(), w()-12, h(), FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    }
}

int Fl_Hint_Input::handle(int event)
{
    if(event==FL_KEYBOARD && Fl::event_key()==FL_BackSpace && size()==0 && OnEmptyBackspace)
    {
        OnEmptyBackspace();
        return 1;
    }
    return Fl_Input::handle(event);
}
