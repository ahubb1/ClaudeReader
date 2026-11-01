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
#include "StoreView.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <memory>
#include <algorithm>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#define STORE_TOPBAR_H  36
#define STORE_DETAIL_W  380
#define STORE_PAD       16
#define CARD_W          160
#define CARD_H          272
#define CARD_GAP        14
#define CARD_COVER_W    150
#define CARD_COVER_H    214
#define ROW_H           52      //Torrent results
#define DETAIL_COVER_W  120
#define DETAIL_COVER_H  170


//------------------------------------Widgets------------------------

//! Store tab (VSCodium editor tab look)
class Fl_Store_Tab : public Fl_Button
{
    public:
        Fl_Store_Tab(int X, int Y, int W, int H, const char *l):Fl_Button(X, Y, W, H), Active(false), Hover(false)
        {
            copy_label(l);
            box(FL_FLAT_BOX);
            clear_visible_focus();
        }
        int handle(int e)
        {
            if(e==FL_ENTER || e==FL_LEAVE)
            {
                Hover=(e==FL_ENTER);
                redraw();
            }
            return Fl_Button::handle(e);
        }
        void draw()
        {
            fl_color(Active ? THEME_EDITOR_BG : THEME_BAR_BG);
            fl_rectf(x(), y(), w(), h());
            if(Active)
            {
                fl_color(THEME_ACCENT);
                fl_rectf(x(), y(), w(), 2);
            }
            fl_color(THEME_BORDER);
            fl_yxline(x()+w()-1, y(), y()+h()-1);
            fl_font(FL_HELVETICA, 13);
            fl_color((Active || Hover) ? THEME_FG_BRIGHT : THEME_FG_DIM);
            fl_draw(label(), x(), y(), w(), h(), FL_ALIGN_CENTER);
        }
        bool Active, Hover;
};

//! A search result: cover and title (manga) or a row (torrent)
class Fl_Store_Card : public Fl_Button
{
    public:
        Fl_Store_Card(int X, int Y, int W, int H, bool torrent):Fl_Button(X, Y, W, H),
            Cover(NULL), Hover(false), Selected(false), Torrent(torrent), Index(-1)
        {
            box(FL_FLAT_BOX);
            clear_visible_focus();
        }
        ~Fl_Store_Card() { delete Cover; }
        void SetCover(Fl_RGB_Image *img) { delete Cover; Cover=img; redraw(); }
        int handle(int e)
        {
            if(e==FL_ENTER || e==FL_LEAVE)
            {
                Hover=(e==FL_ENTER);
                redraw();
            }
            return Fl_Button::handle(e);
        }
        void draw()
        {
            fl_color(Selected ? fl_rgb_color(4, 57, 94) : (Hover ? THEME_HOVER : THEME_EDITOR_BG));
            fl_rectf(x(), y(), w(), h());
            if(Torrent)
            {
                fl_font(FL_HELVETICA, 13);
                fl_color(THEME_FG_BRIGHT);
                fl_push_clip(x()+10, y(), w()-20, h());
                fl_draw(Title.c_str(), x()+10, y()+6, w()-20, 20, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
                fl_font(FL_HELVETICA, 12);
                fl_color(THEME_FG_DIM);
                fl_draw(Subtitle.c_str(), x()+10, y()+26, w()-20, 20, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
                fl_pop_clip();
                fl_color(THEME_BORDER);
                fl_xyline(x(), y()+h()-1, x()+w()-1);
                return;
            }
            //Cover
            int cx=x()+(w()-CARD_COVER_W)/2, cy=y()+5;
            if(Cover)
                Cover->draw(cx+(CARD_COVER_W-Cover->w())/2, cy+(CARD_COVER_H-Cover->h())/2);
            else
            {
                fl_color(THEME_INPUT_BG);
                fl_rectf(cx, cy, CARD_COVER_W, CARD_COVER_H);
            }
            //Title (two lines)
            fl_font(FL_HELVETICA, 12);
            fl_color(Hover || Selected ? THEME_FG_BRIGHT : THEME_FG);
            fl_push_clip(cx, cy+CARD_COVER_H+6, CARD_COVER_W, h()-CARD_COVER_H-12);
            fl_draw(Title.c_str(), cx, cy+CARD_COVER_H+6, CARD_COVER_W, 40, FL_ALIGN_TOP_LEFT|FL_ALIGN_WRAP);
            fl_pop_clip();
        }

        std::string Title, Subtitle;
        Fl_RGB_Image *Cover;
        bool Hover, Selected, Torrent;
        int Index;
};

//! Small flat button
static Fl_Button *FlatButton(int X, int Y, int W, int H, const char *label, bool accent)
{
    Fl_Button *b=new Fl_Button(X, Y, W, H, label);
    b->box(FL_FLAT_BOX);
    b->color(accent ? THEME_ACCENT : THEME_INPUT_BG, accent ? fl_rgb_color(2, 106, 192) : THEME_HOVER);
    b->labelcolor(accent ? FL_WHITE : THEME_FG);
    b->labelsize(12);
    b->clear_visible_focus();
    return b;
}

//! Label box: text wrapped in the box, top left
static Fl_Box *TextBox(int X, int Y, int W, int H, int size, Fl_Color color, bool bold=false)
{
    Fl_Box *b=new Fl_Box(X, Y, W, H);
    b->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP|FL_ALIGN_CLIP);
    b->labelsize(size);
    b->labelcolor(color);
    if(bold)
        b->labelfont(FL_HELVETICA_BOLD);
    return b;
}

//! Image downloaded and fitted in a box, decoded in a worker thread
struct DecodedImage
{
    std::vector<unsigned char> Rgb;
    int W, H;
    DecodedImage():W(0), H(0) {}
    Fl_RGB_Image *Make() const
    {
        if(Rgb.empty())
            return NULL;
        unsigned char *data=new unsigned char[Rgb.size()];
        memcpy(data, Rgb.data(), Rgb.size());
        Fl_RGB_Image *img=new Fl_RGB_Image(data, W, H, 3);
        img->alloc_array=1;
        return img;
    }
};

static void FetchImage(const std::string &url, const NetHeaders &headers, int box_w, int box_h, DecodedImage *out)
{
    NetResponse r=Net::Get(url, headers, NET_CACHE_FOREVER);
    if(!r.Ok())
        return;
    std::vector<unsigned char> raw(r.Body.begin(), r.Body.end());
    cv::Mat img=cv::imdecode(raw, cv::IMREAD_COLOR);
    if(img.empty())
        return;
    double scale=std::min(box_w/(double)img.cols, box_h/(double)img.rows);
    cv::Mat fitted, rgb;
    cv::resize(img, fitted, cv::Size(std::max(1, (int)(img.cols*scale)), std::max(1, (int)(img.rows*scale))), 0, 0, cv::INTER_AREA);
    cv::cvtColor(fitted, rgb, cv::COLOR_BGR2RGB);
    out->W=rgb.cols;
    out->H=rgb.rows;
    out->Rgb.assign(rgb.data, rgb.data+rgb.total()*3);
}


//------------------------------------Store view------------------------

Fl_Store_View::Fl_Store_View(int X, int Y, int W, int H):
    Fl_Group(X, Y, W, H),
    DetailCoverImage(NULL),
    Current(-1),
    Page(1),
    LastPage(false),
    Generation(0),
    DetailGeneration(0),
    Selected(-1),
    Activated(false),
    Downloading(false),
    Cancel(false),
    Tasks(2),
    Images(4)
{
    box(FL_FLAT_BOX);
    color(THEME_EDITOR_BG);

    //Top bar: stores tabs, search, sources folder
    TopBar=new Fl_Group(X, Y, W, STORE_TOPBAR_H);
        TabsGroup=new Fl_Group(X, Y, W-530, STORE_TOPBAR_H);
        TabsGroup->end();
        FilterButton=FlatButton(X+W-522, Y+6, 102, STORE_TOPBAR_H-12, "Genres", false);
        FilterButton->tooltip("Search by genres and themes");
        FilterButton->callback(Filter_cb, this);
        FilterButton->hide();
        SearchInput=new Fl_Hint_Input(X+W-412, Y+6, 300, STORE_TOPBAR_H-12);
        SearchInput->box(THEME_INPUT_BOX);
        SearchInput->color(THEME_INPUT_BG);
        SearchInput->textcolor(THEME_FG);
        SearchInput->textsize(13);
        SearchInput->cursor_color(THEME_FG);
        SearchInput->when(FL_WHEN_ENTER_KEY_ALWAYS);
        SearchInput->callback(Search_cb, this);
        Fl_Activity_Button *search_button=new Fl_Activity_Button(X+W-108, Y+2, 32, STORE_TOPBAR_H-4, ICON_SEARCH, 18);
        search_button->tooltip("Search (Enter); an empty search shows the popular titles");
        search_button->callback(Search_cb, this);
        Fl_Activity_Button *reload_button=new Fl_Activity_Button(X+W-72, Y+2, 32, STORE_TOPBAR_H-4, ICON_REFRESH, 18);
        reload_button->tooltip("Reload the stores (after adding or editing a source script)");
        reload_button->callback(ReloadSources_cb, this);
        Fl_Activity_Button *folder_button=new Fl_Activity_Button(X+W-36, Y+2, 32, STORE_TOPBAR_H-4, ICON_OPEN, 18);
        folder_button->tooltip("Open the folder of your own store scripts (.lua)");
        folder_button->callback(SourcesFolder_cb, this);
    TopBar->end();
    TopBar->box(THEME_STATUS_BOX); //Bar color, separation line (drawn on top: hidden by the tabs, visible at the right)
    TopBar->color(THEME_BAR_BG);
    TopBar->resizable(TabsGroup);

    //Results
    Results=new Fl_Infinite_Scroll(X, Y+STORE_TOPBAR_H, W, H-STORE_TOPBAR_H);
    Results->OnNearEnd=[this]() { LoadMore(); };
        //Content of the scroll: the cards are placed relative to it (see LayoutCards)
        Canvas=new Fl_Group(X, Y+STORE_TOPBAR_H, W, H-STORE_TOPBAR_H);
            Message=new Fl_Box(X, Y+STORE_TOPBAR_H, W, 160);
            Message->labelcolor(THEME_FG_DIM);
            Message->labelsize(14);
            Message->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
            MoreBox=new Fl_Box(X, Y, 180, 32, "Loading more...");
            MoreBox->labelcolor(THEME_FG_DIM);
            MoreBox->labelsize(12);
            MoreBox->hide();
            LoadingMore=false;
        Canvas->end();
        Canvas->resizable(NULL);
    Results->end();
    Results->type(Fl_Scroll::VERTICAL);
    Results->box(FL_FLAT_BOX);
    Results->color(THEME_EDITOR_BG);
    Results->scrollbar.color(THEME_EDITOR_BG, fl_rgb_color(79, 79, 79));
    Results->scrollbar.box(FL_FLAT_BOX);

    //Details of a result
    int dx=X+W-STORE_DETAIL_W, dy=Y+STORE_TOPBAR_H, dw=STORE_DETAIL_W, dh=H-STORE_TOPBAR_H;
    Detail=new Fl_Group(dx, dy, dw, dh);
        Fl_Box *separator=new Fl_Box(FL_FLAT_BOX, dx, dy, 1, dh, "");
        separator->color(THEME_BORDER);
        Fl_Activity_Button *close_button=new Fl_Activity_Button(dx+dw-32, dy+4, 28, 28, ICON_CLOSE, 16);
        close_button->tooltip("Close");
        close_button->callback(CloseDetail_cb, this);
        DetailCover=new Fl_Box(dx+STORE_PAD, dy+STORE_PAD, DETAIL_COVER_W, DETAIL_COVER_H);
        DetailTitle=TextBox(dx+STORE_PAD+DETAIL_COVER_W+12, dy+STORE_PAD, dw-2*STORE_PAD-DETAIL_COVER_W-12-24, 70, 15, THEME_FG_BRIGHT, true);
        DetailInfo=TextBox(dx+STORE_PAD+DETAIL_COVER_W+12, dy+STORE_PAD+74, dw-2*STORE_PAD-DETAIL_COVER_W-12, DETAIL_COVER_H-74, 12, THEME_FG_DIM);
        int my=dy+STORE_PAD+DETAIL_COVER_H+10;

        //Manga: description, chapters, download
        MangaGroup=new Fl_Group(dx+1, my, dw-1, dy+dh-my);
            DetailDescription=TextBox(dx+STORE_PAD, my, dw-2*STORE_PAD, 92, 12, THEME_FG);
            ChaptersLabel=TextBox(dx+STORE_PAD, my+100, dw-2*STORE_PAD-100, 22, 11, THEME_FG, true);
            ChaptersLabel->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
            Fl_Button *all=FlatButton(dx+dw-STORE_PAD-96, my+100, 46, 22, "All", false);
            all->callback(SelectAll_cb, this);
            Fl_Button *none=FlatButton(dx+dw-STORE_PAD-46, my+100, 46, 22, "None", false);
            none->callback(SelectNone_cb, this);
            int bottom=dy+dh;
            ChapterList=new Fl_Check_Browser(dx+STORE_PAD, my+128, dw-2*STORE_PAD, bottom-(my+128)-96);
            ChapterList->box(THEME_INPUT_BOX);
            ChapterList->color(THEME_EDITOR_BG);
            ChapterList->textcolor(THEME_FG);
            ChapterList->textsize(12);
            ChapterList->selection_color(fl_rgb_color(4, 57, 94));
            ChapterList->scrollbar.color(THEME_EDITOR_BG, fl_rgb_color(79, 79, 79));
            ChapterList->scrollbar.box(FL_FLAT_BOX);
            ChapterList->clear_visible_focus();
            FormatChoice=new Fl_Choice(dx+STORE_PAD, bottom-86, 92, 30);
            FormatChoice->add("CBZ");
            FormatChoice->add("PDF");
            FormatChoice->add("ZIP");
            FormatChoice->add("Images");
            FormatChoice->value(0);
            FormatChoice->box(THEME_INPUT_BOX);
            FormatChoice->color(THEME_INPUT_BG);
            FormatChoice->textcolor(THEME_FG);
            FormatChoice->textsize(12);
            FormatChoice->down_box(FL_FLAT_BOX);
            FormatChoice->tooltip("Download format");
            DownloadButton=FlatButton(dx+STORE_PAD+100, bottom-86, 130, 30, "Download", true);
            DownloadButton->tooltip("Download the checked chapters to your library");
            DownloadButton->callback(Download_cb, this);
            ReadButton=FlatButton(dx+STORE_PAD+238, bottom-86, dw-2*STORE_PAD-238, 30, "Read", false);
            ReadButton->tooltip("Read the first checked (or highlighted) chapter: downloaded first if needed");
            ReadButton->callback(Read_cb, this);
            Progress=new Fl_Progress(dx+STORE_PAD, bottom-48, dw-2*STORE_PAD, 4);
            Progress->box(FL_FLAT_BOX);
            Progress->color(THEME_BORDER, THEME_ACCENT);
            Progress->minimum(0);
            Progress->maximum(1);
            Progress->value(0);
            DetailStatus=TextBox(dx+STORE_PAD, bottom-40, dw-2*STORE_PAD-80, 36, 12, THEME_FG_DIM);
            CancelButton=FlatButton(dx+dw-STORE_PAD-72, bottom-38, 72, 24, "Cancel", false);
            CancelButton->tooltip("Stop the download and empty the waiting list");
            CancelButton->callback(Cancel_cb, this);
            CancelButton->hide();
        MangaGroup->end();
        MangaGroup->resizable(ChapterList);

        //Torrent: links
        TorrentGroup=new Fl_Group(dx+1, my, dw-1, dy+dh-my);
            MagnetButton=FlatButton(dx+STORE_PAD, my+8, dw-2*STORE_PAD, 32, "Open magnet link", true);
            MagnetButton->tooltip("Open the magnet link in your torrent client");
            MagnetButton->callback(Magnet_cb, this);
            CopyMagnetButton=FlatButton(dx+STORE_PAD, my+48, dw-2*STORE_PAD, 30, "Copy magnet link", false);
            CopyMagnetButton->callback(CopyMagnet_cb, this);
            Fl_Box *torrent_space=new Fl_Box(dx+1, my+90, dw-1, dy+dh-my-90);
        TorrentGroup->end();
        TorrentGroup->resizable(torrent_space);
        TorrentGroup->hide();
    Detail->end();
    Detail->box(FL_FLAT_BOX);
    Detail->color(THEME_BAR_BG);
    Detail->resizable(MangaGroup);
    Detail->hide();

    end();
    resizable(Results);
}

void Fl_Store_View::resize(int X, int Y, int W, int H)
{
    Fl_Group::resize(X, Y, W, H);
    LayoutCards();
}

void Fl_Store_View::Activate()
{
    if(Activated)
        return;
    Activated=true;
    LoadSources();
}

void Fl_Store_View::LoadSources()
{
    std::vector<std::string> errors;
    //The former sources can still be used by a task: they are kept (a few kB)
    Sources=LoadStoreSources(&errors);
    BuildTabs();
    if(Sources.empty())
    {
        ShowMessage("No store available.\n\nAdd store scripts (.lua) in the folder opened by the folder button, top right.");
        return;
    }
    SelectSource(std::max(0, std::min(Current, (int)Sources.size()-1)));
    if(!errors.empty())
    {
        std::string text="Some store scripts have errors:";
        for(size_t i=0; i<errors.size(); i++)
            text+="\n"+errors[i];
        (CenterNextMessage(), fl_alert)("%s", text.c_str());
    }
}

void Fl_Store_View::BuildTabs()
{
    TabsGroup->clear();
    Tabs.clear();
    TabsGroup->begin();
    int tx=TabsGroup->x();
    fl_font(FL_HELVETICA, 13);
    for(size_t i=0; i<Sources.size(); i++)
    {
        int tw=(int)fl_width(Sources[i]->Name.c_str())+32;
        Fl_Store_Tab *tab=new Fl_Store_Tab(tx, TabsGroup->y(), tw, STORE_TOPBAR_H, Sources[i]->Name.c_str());
        std::string tip=Sources[i]->Site+(Sources[i]->Origin=="built-in" ? "" : "\n"+Sources[i]->Origin);
        tab->copy_tooltip(tip.c_str());
        tab->callback(Tab_cb, this);
        Tabs.push_back(tab);
        tx+=tw;
    }
    TabsGroup->end();
    TabsGroup->redraw();
}

void Fl_Store_View::SelectSource(int index)
{
    Current=index;
    for(size_t i=0; i<Tabs.size(); i++)
    {
        ((Fl_Store_Tab*)Tabs[i])->Active=((int)i==index);
        Tabs[i]->redraw();
    }
    SearchInput->Hint="Search "+Sources[index]->Name;
    SearchInput->value("");
    SearchInput->redraw();
    UpdateFilterButton();
    LoadFilters();
    Search("", 1);
}

void Fl_Store_View::SetStatus(const std::string &text)
{
    Status=text;
    if(OnStatus)
        OnStatus();
}

void Fl_Store_View::ShowMessage(const std::string &text)
{
    Message->copy_label(text.c_str());
    Message->show();
    LayoutCards();
}

void Fl_Store_View::ClearResults()
{
    Images.ClearPending();
    for(size_t i=0; i<Cards.size(); i++)
    {
        Canvas->remove(Cards[i]);
        Fl::delete_widget(Cards[i]);
    }
    Cards.clear();
    Entries.clear();
    Selected=-1;
    Results->scroll_to(0, 0);
}

void Fl_Store_View::Search(const std::string &query, int page)
{
    if(Current<0)
        return;
    LuaSource *source=Sources[Current];
    if(page==1)
    {
        Generation++;
        ClearResults();
        ShowDetail(false);
        ShowMessage(query.empty() ? "Loading the popular titles..." : "Searching...");
    }
    Query=query;
    Page=page;
    LoadingMore=page>1;
    SetStatus(source->Name+": searching...");

    unsigned generation=Generation;
    std::shared_ptr< std::vector<StoreEntry> > results(new std::vector<StoreEntry>());
    std::shared_ptr<std::string> error(new std::string());
    std::shared_ptr<bool> ok(new bool(false));
    std::vector<std::string> tags=Chosen[source];
    Tasks.Run([=]() { *ok=source->Search(query, page, results.get(), error.get(), tags); },
              [=]()
    {
        if(generation!=Generation)
            return; //Another search was started
        LoadingMore=false;
        if(!*ok)
        {
            //No retry loop when scrolling: the end of the results
            LastPage=true;
            LayoutCards();
            if(page==1)
                ShowMessage("The search failed:\n"+UserError(*error));
            SetStatus(source->Name+": search failed");
            return;
        }
        //No more results: none, or fewer than a page (the stores return 20 to 100 results a page)
        LastPage=results->empty() || results->size()<10;
        Message->hide();
        AddCards(*results);
        if(Entries.empty())
            ShowMessage("Nothing found");
        SetStatus(source->Name+": "+std::to_string(Entries.size())+(Entries.size()==1 ? " result" : " results"));
    });
}

void Fl_Store_View::AddCards(const std::vector<StoreEntry> &results)
{
    bool torrent=Sources[Current]->IsTorrent();
    for(size_t i=0; i<results.size(); i++)
    {
        Fl_Store_Card *card=new Fl_Store_Card(0, 0, CARD_W, torrent ? ROW_H : CARD_H, torrent);
        card->Index=(int)Entries.size();
        card->Title=results[i].Get("title", "?");
        if(torrent)
        {
            std::string sub;
            const char *fields[]={"size", "seeders", "leechers", "date"};
            const char *names[]={"", " seeders", " leechers", ""};
            for(int f=0; f<4; f++)
                if(!results[i].Get(fields[f]).empty())
                    sub+=(sub.empty() ? "" : "   \xc2\xb7   ")+results[i].Get(fields[f])+names[f];
            card->Subtitle=sub;
        }
        card->copy_tooltip(card->Title.c_str());
        card->callback(Card_cb, this);
        Canvas->add(card);
        Cards.push_back(card);
        Entries.push_back(results[i]);
        if(!torrent && !results[i].Get("cover").empty())
            LoadCover(card, results[i].Get("cover"), Generation);
    }
    LayoutCards();
}

void Fl_Store_View::LoadCover(Fl_Store_Card *card, const std::string &url, unsigned generation)
{
    NetHeaders headers=Sources[Current]->Headers;
    std::shared_ptr<DecodedImage> image(new DecodedImage());
    Images.Run([=]() { FetchImage(url, headers, CARD_COVER_W, CARD_COVER_H, image.get()); },
               [=]()
    {
        if(generation==Generation) //The card still exists
            card->SetCover(image->Make());
    });
}

void Fl_Store_View::LayoutCards()
{
    if(!Results)
        return;
    //Content placed from the top of the canvas, which is moved by the scroll position
    int cw=Results->w()-Fl::scrollbar_size();
    int cx=Results->x(), cy=Results->y()-Results->yposition();
    //Canvas placed first: moving a group moves its children, they are placed after
    Canvas->resize(cx, cy, cw, Canvas->h());
    int sx=cx+STORE_PAD, sy=cy+STORE_PAD;
    int avail=cw-2*STORE_PAD;
    bool torrent=Current>=0 && Current<(int)Sources.size() && Sources[Current]->IsTorrent();
    int bottom=sy;
    if(torrent)
    {
        for(size_t i=0; i<Cards.size(); i++)
        {
            Cards[i]->resize(sx, sy+(int)i*ROW_H, avail, ROW_H);
            bottom=sy+(int)(i+1)*ROW_H;
        }
    }
    else
    {
        int cols=std::max(1, (avail+CARD_GAP)/(CARD_W+CARD_GAP));
        //Center the grid
        int x0=sx+(avail-(cols*(CARD_W+CARD_GAP)-CARD_GAP))/2;
        for(size_t i=0; i<Cards.size(); i++)
        {
            int col=(int)i%cols, row=(int)i/cols;
            Cards[i]->resize(x0+col*(CARD_W+CARD_GAP), sy+row*(CARD_H+CARD_GAP), CARD_W, CARD_H);
            bottom=sy+(row+1)*(CARD_H+CARD_GAP);
        }
    }
    //While more results can come: the loading line, and room to scroll to it
    if(!Cards.empty() && !LastPage)
    {
        MoreBox->resize(cx+(cw-180)/2, bottom+8, 180, 32);
        MoreBox->show();
        bottom+=48;
    }
    else
        MoreBox->hide();
    Message->resize(cx, cy+40, cw, 160);
    //The canvas covers the content (the scroll range)
    Canvas->size(cw, std::max(bottom+STORE_PAD-cy, Results->h())); //Same position: the children do not move
    Results->redraw();
}

void Fl_Store_View::ShowDetail(bool show)
{
    int dy=y()+STORE_TOPBAR_H, dh=h()-STORE_TOPBAR_H;
    if(show)
    {
        Detail->resize(x()+w()-STORE_DETAIL_W, dy, STORE_DETAIL_W, dh);
        Results->resize(x(), dy, w()-STORE_DETAIL_W, dh);
        Detail->show();
    }
    else
    {
        Detail->hide();
        Results->resize(x(), dy, w(), dh);
        //Out of the results area, not to be resized with it
        Detail->resize(x()+w(), dy, STORE_DETAIL_W, dh);
    }
    init_sizes();
    LayoutCards();
    redraw();
}

void Fl_Store_View::OpenDetail(int index)
{
    if(index<0 || index>=(int)Entries.size())
        return;
    LuaSource *source=Sources[Current];
    const StoreEntry &entry=Entries[index];
    for(size_t i=0; i<Cards.size(); i++)
    {
        Cards[i]->Selected=((int)i==index);
        Cards[i]->redraw();
    }
    Selected=index;
    DetailGeneration++;
    unsigned generation=DetailGeneration;

    DetailTitle->copy_label(entry.Get("title", "?").c_str());
    std::string info;
    const char *fields[]={"status", "year", "author", "size", "seeders", "leechers", "date"};
    const char *names[]={"", "", "", "Size: ", "Seeders: ", "Leechers: ", ""};
    for(int f=0; f<7; f++)
        if(!entry.Get(fields[f]).empty())
            info+=std::string(names[f])+entry.Get(fields[f])+"\n";
    info+=source->Name;
    DetailInfo->copy_label(info.c_str());

    //Cover (from the cache: already downloaded for the card)
    delete DetailCoverImage;
    DetailCoverImage=NULL;
    DetailCover->image(NULL);
    if(!entry.Get("cover").empty())
    {
        std::string url=entry.Get("cover");
        NetHeaders headers=source->Headers;
        std::shared_ptr<DecodedImage> image(new DecodedImage());
        Images.Run([=]() { FetchImage(url, headers, DETAIL_COVER_W, DETAIL_COVER_H, image.get()); },
                   [=]()
        {
            if(generation!=DetailGeneration)
                return;
            delete DetailCoverImage;
            DetailCoverImage=image->Make();
            DetailCover->image(DetailCoverImage);
            DetailCover->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
            DetailCover->redraw();
        });
    }

    if(source->IsTorrent())
    {
        MangaGroup->hide();
        TorrentGroup->show();
        if(entry.Get("magnet").empty())
            MagnetButton->deactivate();
        else
            MagnetButton->activate();
    }
    else
    {
        TorrentGroup->hide();
        MangaGroup->show();
        std::string description=entry.Get("description");
        if(description.size()>700)
            description=description.substr(0, 700)+"...";
        DetailDescription->copy_label(description.c_str());
        ChapterList->clear();
        Chapters.clear();
        ChaptersLabel->copy_label("CHAPTERS   loading...");

        StoreEntry manga=entry;
        std::shared_ptr< std::vector<StoreEntry> > list(new std::vector<StoreEntry>());
        std::shared_ptr<std::string> error(new std::string());
        std::shared_ptr<bool> ok(new bool(false));
        Tasks.Run([=]() { *ok=source->Chapters(manga, list.get(), error.get()); },
                  [=]()
        {
            if(generation!=DetailGeneration)
                return;
            if(!*ok)
            {
                ChaptersLabel->copy_label("CHAPTERS   unavailable");
                DetailStatus->copy_label(UserError(*error).c_str());
                return;
            }
            Chapters=*list;
            FillChapters();
            DetailStatus->copy_label(Chapters.empty() ?
                "No chapter available on this store (often because the series is licensed): try another store." : "");
        });
    }
    ShowDetail(true);
}

std::string Fl_Store_View::ChapterBase(const StoreEntry &chapter) const
{
    return ChapterOutputBase(LibraryFolder, Sources[Current], Entries[Selected], chapter);
}

void Fl_Store_View::FillChapters()
{
    //Keep the checked chapters (refresh after a download)
    std::vector<bool> checked(Chapters.size(), false);
    for(int i=1; i<=ChapterList->nitems() && i<=(int)Chapters.size(); i++)
        checked[i-1]=ChapterList->checked(i)!=0;
    int top=ChapterList->position();

    ChapterList->clear();
    int downloaded=0;
    for(size_t i=0; i<Chapters.size(); i++)
    {
        const StoreEntry &c=Chapters[i];
        bool done=!FindDownloadedChapter(ChapterBase(c)).empty();
        downloaded+=done;
        std::string label=std::string(done ? "\xe2\x9c\x93  " : "     ")+c.Get("title", c.Get("id"));
        if(!c.Get("group").empty())
            label+="   ["+c.Get("group")+"]";
        ChapterList->add(label.c_str(), checked[i] ? 1 : 0);
    }
    ChapterList->position(top);
    std::string text="CHAPTERS   "+std::to_string(Chapters.size());
    if(downloaded)
        text+="   \xc2\xb7   "+std::to_string(downloaded)+" downloaded";
    ChaptersLabel->copy_label(text.c_str());
    ChapterList->redraw();
}

void Fl_Store_View::QueueDownloads(bool read_first)
{
    if(Selected<0 || Chapters.empty())
        return;
    STORE_FORMAT format=(STORE_FORMAT)FormatChoice->value();

    std::vector<int> picked;
    for(int i=1; i<=ChapterList->nitems(); i++)
        if(ChapterList->checked(i))
            picked.push_back(i-1);

    if(read_first)
    {
        //First checked chapter, or the highlighted one, or the first one
        int index=!picked.empty() ? picked[0] : (ChapterList->value()>0 ? ChapterList->value()-1 : 0);
        std::string existing=FindDownloadedChapter(ChapterBase(Chapters[index]));
        if(!existing.empty())
        {
            if(OnOpen)
                OnOpen(existing);
            return;
        }
        Queue.push_front(DownloadJob{Sources[Current], Entries[Selected], Chapters[index], format, true});
    }
    else
    {
        if(picked.empty())
        {
            DetailStatus->copy_label("Check the chapters to download first.");
            return;
        }
        for(size_t i=0; i<picked.size(); i++)
            Queue.push_back(DownloadJob{Sources[Current], Entries[Selected], Chapters[picked[i]], format, false});
        ChapterList->check_none();
    }
    StartNextDownload();
    if(Downloading && !Queue.empty())
        DetailStatus->copy_label((std::to_string(Queue.size())+" chapter(s) waiting").c_str());
}

void Fl_Store_View::StartNextDownload()
{
    if(Downloading || Queue.empty())
        return;
    DownloadJob job=Queue.front();
    Queue.pop_front();
    Downloading=true;
    Cancel=false;
    CancelButton->show();
    std::string base=ChapterOutputBase(LibraryFolder, job.Source, job.Manga, job.Chapter);
    std::string title=job.Chapter.Get("title", "chapter");
    Progress->value(0);
    SetStatus("Downloading "+title+"...");
    DetailStatus->copy_label(Status.c_str());

    std::thread([=]()
    {
        std::string output, error;
        //Its own instance of the store: the store tab stays usable during the download (issue #8)
        std::unique_ptr<LuaSource> worker(job.Source->Clone());
        LuaSource *source=worker->IsValid() ? worker.get() : job.Source;
        bool ok=DownloadChapter(source, job.Chapter, base, job.Format,
            [=](int done, int total)
            {
                RunInUiThread([=]()
                {
                    if(Cancel)
                        return;
                    Progress->value(total>0 ? done/(float)total : 0);
                    SetStatus("Downloading "+title+"   "+std::to_string(done)+"/"+std::to_string(total)+" pages"
                              +(Queue.empty() ? "" : "   ("+std::to_string(Queue.size())+" waiting)"));
                    DetailStatus->copy_label(Status.c_str());
                    Progress->redraw();
                });
            }, &Cancel, &output, &error);
        RunInUiThread([=]()
        {
            Downloading=false;
            if(Queue.empty())
                CancelButton->hide();
            Progress->value(ok ? 1 : 0);
            if(ok)
            {
                SetStatus("Downloaded "+title);
                if(Selected>=0 && job.Manga.Get("id")==Entries[Selected].Get("id"))
                    FillChapters();
                if(OnDownloaded)
                    OnDownloaded();
                if(job.OpenAfter && OnOpen)
                    OnOpen(output);
            }
            else if(Cancel)
                SetStatus("Download canceled ("+title+")");
            else
                SetStatus("Download failed ("+title+"): "+UserError(error));
            DetailStatus->copy_label(Status.c_str());
            StartNextDownload();
        });
    }).detach();
}


//------------------------------------Callbacks------------------------

void Fl_Store_View::Tab_cb(Fl_Widget *w, void *p)
{
    Fl_Store_View *v=(Fl_Store_View*)p;
    for(size_t i=0; i<v->Tabs.size(); i++)
        if(v->Tabs[i]==w && (int)i!=v->Current)
            v->SelectSource((int)i);
}

void Fl_Store_View::Search_cb(Fl_Widget *w, void *p)
{
    Fl_Store_View *v=(Fl_Store_View*)p;
    v->Search(v->SearchInput->value(), 1);
}

void Fl_Store_View::Card_cb(Fl_Widget *w, void *p)
{
    ((Fl_Store_View*)p)->OpenDetail(((Fl_Store_Card*)w)->Index);
}

//! The filters of the store shown (once, in a task)
void Fl_Store_View::LoadFilters()
{
    if(Current<0 || !Sources[Current]->HasFilters() || Filters.count(Sources[Current]))
        return;
    LuaSource *source=Sources[Current];
    Filters[source];    //Asked: not again
    std::shared_ptr< std::vector<StoreEntry> > list(new std::vector<StoreEntry>());
    std::shared_ptr<std::string> error(new std::string());
    std::shared_ptr<bool> ok(new bool(false));
    Tasks.Run([=]() { *ok=source->Filters(list.get(), error.get()); },
              [=]()
    {
        if(*ok)
            Filters[source]=*list;
        else
            Filters.erase(source);  //Tried again next time
        UpdateFilterButton();
    });
}

//! "Genres" (and the number chosen), for the stores with filters only
void Fl_Store_View::UpdateFilterButton()
{
    if(Current<0 || !Sources[Current]->HasFilters())
    {
        FilterButton->hide();
        TopBar->redraw();
        return;
    }
    size_t chosen=Chosen[Sources[Current]].size();
    FilterLabel=chosen ? "Genres ("+std::to_string(chosen)+")" : "Genres";
    FilterButton->copy_label(FilterLabel.c_str());
    FilterButton->color(chosen ? fl_rgb_color(4, 57, 94) : THEME_INPUT_BG);
    FilterButton->show();
    TopBar->redraw();
}

//! The genres and themes as check items (submenus per group): a choice searches again
void Fl_Store_View::Filter_cb(Fl_Widget *w, void *p)
{
    Fl_Store_View *v=(Fl_Store_View*)p;
    if(v->Current<0)
        return;
    LuaSource *source=v->Sources[v->Current];
    std::vector<StoreEntry> &filters=v->Filters[source];
    std::vector<std::string> &chosen=v->Chosen[source];
    if(filters.empty())
    {
        v->Filters.erase(source);
        v->SetStatus(source->Name+": loading the genres...");
        v->LoadFilters();
        return;
    }
    //Menu: one submenu per group, then "Clear"
    std::vector<std::string> labels;
    labels.reserve(filters.size()+4);
    std::vector<Fl_Menu_Item> menu;
    std::vector<int> index_of;  //Menu item -> filter index (-1: none, -2: clear)
    std::string group;
    for(size_t i=0; i<filters.size(); i++)
    {
        std::string g=filters[i].Get("group", "Genres");
        if(g!=group)
        {
            if(!group.empty())
            {
                menu.push_back(Fl_Menu_Item());
                index_of.push_back(-1);
            }
            group=g;
            labels.push_back(g);
            Fl_Menu_Item sub={};
            sub.flags=FL_SUBMENU;
            menu.push_back(sub);
            index_of.push_back(-1);
            menu.back().text=NULL;      //Set below (the labels vector must not grow after)
        }
        Fl_Menu_Item item={};
        bool on=std::find(chosen.begin(), chosen.end(), filters[i].Get("id"))!=chosen.end();
        item.flags=FL_MENU_TOGGLE|(on ? FL_MENU_VALUE : 0);
        menu.push_back(item);
        index_of.push_back((int)i);
        labels.push_back(filters[i].Get("name", filters[i].Get("id")));
    }
    menu.push_back(Fl_Menu_Item());     //End of the last submenu
    index_of.push_back(-1);
    Fl_Menu_Item clear={};
    menu.push_back(clear);
    index_of.push_back(-2);
    labels.push_back(chosen.empty() ? "No genre chosen" : "Clear all ("+std::to_string(chosen.size())+")");
    menu.push_back(Fl_Menu_Item());
    index_of.push_back(-1);
    //The labels (stable now)
    size_t label=0;
    for(size_t m=0; m<menu.size(); m++)
    {
        bool is_end=index_of[m]==-1 && !(menu[m].flags&FL_SUBMENU);
        if(!is_end)
            menu[m].text=labels[label++].c_str();
    }
    if(chosen.empty())
        menu[menu.size()-2].flags|=FL_MENU_INACTIVE;
    const Fl_Menu_Item *picked=menu[0].popup(w->x(), w->y()+w->h());
    if(!picked)
        return;
    int index=index_of[picked-&menu[0]];
    if(index==-2)
        chosen.clear();
    else if(index>=0)
    {
        std::string id=filters[index].Get("id");
        std::vector<std::string>::iterator it=std::find(chosen.begin(), chosen.end(), id);
        if(it==chosen.end())
            chosen.push_back(id);
        else
            chosen.erase(it);
    }
    else
        return;
    v->UpdateFilterButton();
    v->Search(v->SearchInput->value(), 1);
}

//! Near the end of the results (or they do not fill the view): the next page
void Fl_Store_View::LoadMore()
{
    if(Current<0 || Cards.empty() || LastPage || LoadingMore || !visible_r())
        return;
    Search(Query, Page+1);
}

void Fl_Infinite_Scroll::draw()
{
    Fl_Scroll::draw();
    //Content end: the bottom of the child (the canvas of the results)
    if(children()<1 || !OnNearEnd)
        return;
    Fl_Widget *content=child(0);
    int content_bottom=content->y()+content->h();
    if(content_bottom-(y()+h())<std::max(400, h()))
    {
        //Not from draw(): the search adds widgets
        Fl::remove_timeout(NearEnd_cb, this);
        Fl::add_timeout(0.05, NearEnd_cb, this);
    }
}

void Fl_Infinite_Scroll::NearEnd_cb(void *p)
{
    Fl_Infinite_Scroll *s=(Fl_Infinite_Scroll*)p;
    if(s->OnNearEnd)
        s->OnNearEnd();
}

void Fl_Store_View::CloseDetail_cb(Fl_Widget *w, void *p)
{
    Fl_Store_View *v=(Fl_Store_View*)p;
    for(size_t i=0; i<v->Cards.size(); i++)
    {
        v->Cards[i]->Selected=false;
        v->Cards[i]->redraw();
    }
    v->DetailGeneration++;
    v->ShowDetail(false);
}

void Fl_Store_View::SelectAll_cb(Fl_Widget *w, void *p)
{
    ((Fl_Store_View*)p)->ChapterList->check_all();
}

void Fl_Store_View::SelectNone_cb(Fl_Widget *w, void *p)
{
    ((Fl_Store_View*)p)->ChapterList->check_none();
}

void Fl_Store_View::Download_cb(Fl_Widget *w, void *p)
{
    ((Fl_Store_View*)p)->QueueDownloads(false);
}

void Fl_Store_View::Read_cb(Fl_Widget *w, void *p)
{
    ((Fl_Store_View*)p)->QueueDownloads(true);
}

void Fl_Store_View::Cancel_cb(Fl_Widget *w, void *p)
{
    //The current chapter stops at its next page (nothing is written), the waiting ones are dropped
    Fl_Store_View *v=(Fl_Store_View*)p;
    v->Queue.clear();
    if(v->Downloading)
    {
        v->Cancel=true;
        v->SetStatus("Canceling...");
        v->DetailStatus->copy_label(v->Status.c_str());
    }
    else
        v->CancelButton->hide();
}

void Fl_Store_View::Magnet_cb(Fl_Widget *w, void *p)
{
    Fl_Store_View *v=(Fl_Store_View*)p;
    if(v->Selected<0)
        return;
    std::string magnet=v->Entries[v->Selected].Get("magnet");
    char error[256]="";
    if(!fl_open_uri(magnet.c_str(), error, sizeof(error)))
        (CenterNextMessage(), fl_alert)("No application opens magnet links (install a torrent client):\n%s", error);
}

void Fl_Store_View::CopyMagnet_cb(Fl_Widget *w, void *p)
{
    Fl_Store_View *v=(Fl_Store_View*)p;
    if(v->Selected<0)
        return;
    std::string magnet=v->Entries[v->Selected].Get("magnet");
    Fl::copy(magnet.c_str(), (int)magnet.size(), 1);
    v->SetStatus("Magnet link copied");
}

void Fl_Store_View::SourcesFolder_cb(Fl_Widget *w, void *p)
{
    std::string uri="file:///"+UserSourcesDir();
    for(size_t i=0; i<uri.size(); i++)
        if(uri[i]=='\\')
            uri[i]='/';
    fl_open_uri(uri.c_str());
}

void Fl_Store_View::ReloadSources_cb(Fl_Widget *w, void *p)
{
    Fl_Store_View *v=(Fl_Store_View*)p;
    v->Generation++;
    v->ClearResults();
    v->ShowDetail(false);
    v->LoadSources();
}
