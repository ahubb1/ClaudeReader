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
#include "WebtoonView.h"
#include "Document.h"
#include "Theme.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <opencv2/imgproc.hpp>
#include <algorithm>

#define WEBTOON_MAX_COLUMN 900      //!< Column width at zoom 1 (webtoons are usually 690 to 800 pixels wide)
#define WEBTOON_KEEP 3              //!< Pages kept in memory around the visible ones
#define WEBTOON_LINE 80             //!< Arrow keys and mouse wheel step

Fl_Webtoon_View::Fl_Webtoon_View(int X, int Y, int W, int H):Fl_Widget(X, Y, W, H),
    Doc(NULL), Offset(0), Current(0), Aspect(1.5), Zoom(1.), DragY(0)
{
    box(FL_FLAT_BOX);
    color(THEME_EDITOR_BG);
}

Fl_Webtoon_View::~Fl_Webtoon_View()
{
    Fl::remove_timeout(Prefetch_cb, this);
}

int Fl_Webtoon_View::ColumnWidth() const
{
    return std::max(100, std::min(w()-16, (int)(WEBTOON_MAX_COLUMN*Zoom)));
}

int Fl_Webtoon_View::Top(int index) const
{
    int top=0;
    for(int i=0; i<index && i<(int)Items.size(); i++)
        top+=Items[i].Height;
    return top;
}

int Fl_Webtoon_View::TotalHeight() const
{
    return Top((int)Items.size());
}

void Fl_Webtoon_View::SetDocument(Document *doc, int page)
{
    Fl::remove_timeout(Prefetch_cb, this);
    Doc=doc;
    Items.clear();
    Offset=0;
    Current=0;
    if(Doc)
    {
        Items.resize(Doc->GetNbOfPages());
        for(size_t i=0; i<Items.size(); i++)
            Items[i].Height=(int)(ColumnWidth()*Aspect);
        ShowPage(page);
    }
    redraw();
}

void Fl_Webtoon_View::ShowPage(int page)
{
    if(Items.empty())
        return;
    int index=std::max(0, std::min((int)Items.size()-1, page-1));
    //Loaded first: its position does not depend on its own height
    Load(index);
    Offset=Top(index);
    Current=index;
    Scroll(0);
}

//! Load a page, at the column width
void Fl_Webtoon_View::Load(int index)
{
    if(!Doc || index<0 || index>=(int)Items.size() || !Items[index].Image.empty())
        return;
    //As Document::LoadPage: the document is opened for the page creation only (archives are read from their start)
    if(Doc->Open()<0)
        return;
    Page *page=Doc->CreatePage(index+1);
    Doc->Close();
    cv::Mat *src=page ? page->RenderPage() : NULL;
    int column=ColumnWidth();
    Item &item=Items[index];
    int old_height=item.Height;
    if(src && !src->empty())
    {
        int height=std::max(1, (int)((double)src->rows*column/src->cols));
        cv::Mat scaled, rgb;
        cv::resize(*src, scaled, cv::Size(column, height), 0, 0, column<src->cols ? cv::INTER_AREA : cv::INTER_LINEAR);
        if(scaled.channels()==1)
            cv::cvtColor(scaled, rgb, cv::COLOR_GRAY2RGB);
        else if(scaled.channels()==4)
            cv::cvtColor(scaled, rgb, cv::COLOR_BGRA2RGB);
        else
            cv::cvtColor(scaled, rgb, cv::COLOR_BGR2RGB);
        item.Image=rgb;
        item.Height=height;
        Aspect=(double)height/column;
    }
    delete page;

    //A page above the view changed of height: keep the view on the same content
    if(item.Height!=old_height && Top(index)+old_height<=Offset)
        Offset+=item.Height-old_height;
}

//! Load the visible pages and release the far ones
void Fl_Webtoon_View::LoadVisible()
{
    //Loading changes the heights: until the visible pages are all there
    for(int pass=0; pass<8; pass++)
    {
        int top=0, missing=-1;
        for(int i=0; i<(int)Items.size() && top<Offset+h(); i++)
        {
            if(top+Items[i].Height>Offset && Items[i].Image.empty())
            {
                missing=i;
                break;
            }
            top+=Items[i].Height;
        }
        if(missing<0)
            break;
        Load(missing);
    }
}

void Fl_Webtoon_View::Scroll(double delta)
{
    if(Items.empty())
        return;
    Offset+=delta;
    Offset=std::max(0., std::min(Offset, (double)std::max(0, TotalHeight()-h())));
    LoadVisible();
    Offset=std::max(0., std::min(Offset, (double)std::max(0, TotalHeight()-h())));

    //Current page: the one at the top quarter of the view
    int index=0, top=0;
    double probe=Offset+h()/4.;
    while(index<(int)Items.size()-1 && top+Items[index].Height<=probe)
        top+=Items[index++].Height;
    if(index!=Current)
    {
        Current=index;
        if(OnPageChanged)
            OnPageChanged(Current+1);
    }

    //Memory: only the pages around the view
    for(int i=0; i<(int)Items.size(); i++)
        if(!Items[i].Image.empty() && (i<Current-WEBTOON_KEEP || i>Current+WEBTOON_KEEP+2))
            Items[i].Image.release();
    redraw();

    //The next pages, while idle
    Fl::remove_timeout(Prefetch_cb, this);
    Fl::add_timeout(0.15, Prefetch_cb, this);
}

void Fl_Webtoon_View::Prefetch_cb(void *p)
{
    ((Fl_Webtoon_View*)p)->Prefetch();
}

//! One page ahead per call, so that scrolling down does not wait
void Fl_Webtoon_View::Prefetch()
{
    for(int i=Current; i<=Current+2 && i<(int)Items.size(); i++)
        if(Items[i].Image.empty())
        {
            Load(i);
            redraw();
            Fl::add_timeout(0.05, Prefetch_cb, this);
            return;
        }
}

void Fl_Webtoon_View::SetZoom(double zoom)
{
    zoom=std::max(0.3, std::min(3., zoom));
    if(zoom==Zoom)
        return;
    int old_column=ColumnWidth();
    Zoom=zoom;
    if(ColumnWidth()==old_column || Items.empty())
        return;
    //Same page, the images again at the new width
    int page=Current+1;
    for(size_t i=0; i<Items.size(); i++)
    {
        Items[i].Image.release();
        Items[i].Height=(int)(Items[i].Height*(double)ColumnWidth()/old_column);
    }
    ShowPage(page);
}

void Fl_Webtoon_View::draw()
{
    fl_color(color());
    fl_rectf(x(), y(), w(), h());
    if(Items.empty())
        return;

    fl_push_clip(x(), y(), w(), h());
    int column=ColumnWidth();
    int left=x()+(w()-column)/2;
    int top=0;
    for(int i=0; i<(int)Items.size() && top-Offset<h(); i++)
    {
        int py=y()+(int)(top-Offset);
        const Item &item=Items[i];
        if(py+item.Height>y())
        {
            if(!item.Image.empty() && item.Image.cols==column)
            {
                //Only the visible lines
                int first=std::max(0, y()-py), last=std::min(item.Image.rows, y()+h()-py);
                if(last>first)
                    fl_draw_image(item.Image.ptr(first), left, py+first, column, last-first, 3, (int)item.Image.step);
            }
            else
            {
                //Not loaded yet
                fl_color(THEME_BAR_BG);
                fl_rectf(left, py, column, item.Height);
                fl_color(THEME_FG_DIM);
                fl_font(FL_HELVETICA, 13);
                char text[32];
                snprintf(text, sizeof(text), "Page %d", i+1);
                int ty=std::max(py, y());
                fl_draw(text, left, ty, column, std::min(py+item.Height, y()+h())-ty, FL_ALIGN_CENTER);
            }
        }
        top+=item.Height;
    }
    fl_pop_clip();
}

void Fl_Webtoon_View::resize(int X, int Y, int W, int H)
{
    int old_column=ColumnWidth();
    Fl_Widget::resize(X, Y, W, H);
    if(ColumnWidth()!=old_column && !Items.empty())
    {
        //New column width: same page, the images again at the new width
        int page=Current+1;
        for(size_t i=0; i<Items.size(); i++)
        {
            Items[i].Image.release();
            Items[i].Height=(int)(Items[i].Height*(double)ColumnWidth()/old_column);
        }
        ShowPage(page);
    }
    else
        Scroll(0);
}

int Fl_Webtoon_View::handle(int event)
{
    bool at_end=!Items.empty() && Offset>=TotalHeight()-h()-1;
    switch(event)
    {
        case FL_FOCUS:
        case FL_UNFOCUS:
        case FL_ENTER:
            return 1;
        case FL_PUSH:
            take_focus();
            DragY=Fl::event_y();
            return 1;
        case FL_DRAG:
            Scroll(DragY-Fl::event_y());
            DragY=Fl::event_y();
            return 1;
        case FL_MOUSEWHEEL:
            if(Fl::event_state(FL_CTRL))
                SetZoom(Zoom*(Fl::event_dy()<0 ? 1.1 : 1/1.1));
            else if(Fl::event_dy()>0 && at_end && OnEnd)
                OnEnd();
            else
                Scroll(Fl::event_dy()*WEBTOON_LINE*1.5);
            return 1;
        case FL_KEYBOARD:
        {
            int key=Fl::event_key();
            double step=0;
            if(key==FL_Down) step=WEBTOON_LINE;
            else if(key==FL_Up) step=-WEBTOON_LINE;
            else if(key==FL_Page_Down || key==' ') step=h()*0.9;
            else if(key==FL_Page_Up) step=-h()*0.9;
            else if(key==FL_Home) {ShowPage(1); if(OnPageChanged) OnPageChanged(1); return 1;}
            else if(key==FL_End) {Offset=TotalHeight(); Scroll(0); return 1;}
            else if(key=='+' || key=='=' || key==FL_KP+'+') {SetZoom(Zoom*1.1); return 1;}
            else if(key=='-' || key==FL_KP+'-') {SetZoom(Zoom/1.1); return 1;}
            else
                return 0; //Left/Right (previous/next page) and the shortcuts: the menus
            if(step>0 && at_end && OnEnd)
                OnEnd();
            else
                Scroll(step);
            return 1;
        }
    }
    return Fl_Widget::handle(event);
}
