//--------------------------------------------------------------------------
//
//    This file is part of ClaudeReader (based on BDReader, Copyright (C) 2014-2016  Vincent Crocher).
//
//    ClaudeReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ClaudeReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ClaudeReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file WebtoonView.h
 * \brief Scroll reading mode: the pages one under the other at the same width, scrolled continuously (webtoons, long strips)
 *
 */
#ifndef WEBTOONVIEW_H
#define WEBTOONVIEW_H

#include <vector>
#include <functional>
#include <FL/Fl_Widget.H>
#include <opencv2/core.hpp>

class Document;

class Fl_Webtoon_View : public Fl_Widget
{
    public:
        Fl_Webtoon_View(int X, int Y, int W, int H);
        ~Fl_Webtoon_View();

        //! Show the pages of a document, from the top of a page (NULL: nothing)
        void SetDocument(Document *doc, int page);
        //! Release the pages (the document is closed or another mode is used)
        void Clear() {SetDocument(NULL, 1);}
        //! Scroll to the top of a page
        void ShowPage(int page);
        //! Page at the top of the view (1 to the number of pages)
        int CurrentPage() const {return Current+1;}

        std::function<void(int)> OnPageChanged;   //!< The page at the top of the view changed
        std::function<void()> OnEnd;              //!< Asked to go further than the end of the last page

        int handle(int event);
        void draw();
        void resize(int X, int Y, int W, int H);

    private:
        struct Item
        {
            int Height;         //!< In the column (estimated until the page is loaded)
            cv::Mat Image;      //!< RGB, at the column width (empty when not in memory)
        };

        int ColumnWidth() const;
        int Top(int index) const;           //!< Position of a page in the strip
        int TotalHeight() const;
        void Scroll(double delta);          //!< Clamps, loads what becomes visible, updates the current page
        void SetZoom(double zoom);
        void LoadVisible();
        void Load(int index);
        void Prefetch();
        static void Prefetch_cb(void *p);

        Document *Doc;
        std::vector<Item> Items;
        double Offset;              //!< Position of the top of the view in the strip
        int Current;                //!< Index of the page at the top of the view
        double Aspect;              //!< Height/width of the last loaded page: estimate for the others
        double Zoom;                //!< Column width factor (+/- keys, Ctrl+wheel)
        int DragY;
};

#endif // WEBTOONVIEW_H
