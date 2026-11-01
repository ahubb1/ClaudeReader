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
 * \file StoreView.h
 * \brief Store view: one tab per source, search results, details and chapters download
 *
 */
#ifndef STOREVIEW_H
#define STOREVIEW_H

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <atomic>
#include <functional>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_RGB_Image.H>
#include "Theme.h"
#include "LuaSource.h"
#include "StoreDownload.h"
#include "Async.h"

class Fl_Store_Card;

//! Scroll telling when its end is near (shown or scrolled to): the next results are loaded then
class Fl_Infinite_Scroll : public Fl_Scroll
{
    public:
        Fl_Infinite_Scroll(int X, int Y, int W, int H):Fl_Scroll(X, Y, W, H) {}
        ~Fl_Infinite_Scroll() { Fl::remove_timeout(NearEnd_cb, this); }
        void draw();
        std::function<void()> OnNearEnd;
    private:
        static void NearEnd_cb(void *p);
};

//! The store view (shown instead of the library or the reader)
class Fl_Store_View : public Fl_Group
{
    public:
        Fl_Store_View(int X, int Y, int W, int H);

        //! First display: load the sources and show the popular titles of the first store
        void Activate();
        void SetLibraryFolder(const std::string &folder) {LibraryFolder=folder;}
        void resize(int X, int Y, int W, int H);

        //! Text for the status bar (current store, results, download progress)
        std::string StatusText() const {return Status;}

        std::function<void(const std::string &)> OnOpen;    //!< Read a downloaded chapter
        std::function<void()> OnDownloaded;                 //!< A chapter was added to the library
        std::function<void()> OnStatus;                     //!< StatusText() changed

        //Widgets callbacks
        static void Tab_cb(Fl_Widget *w, void *p);
        static void Search_cb(Fl_Widget *w, void *p);
        static void Card_cb(Fl_Widget *w, void *p);
        static void CloseDetail_cb(Fl_Widget *w, void *p);
        static void SelectAll_cb(Fl_Widget *w, void *p);
        static void SelectNone_cb(Fl_Widget *w, void *p);
        static void Download_cb(Fl_Widget *w, void *p);
        static void Read_cb(Fl_Widget *w, void *p);
        static void Cancel_cb(Fl_Widget *w, void *p);
        static void Magnet_cb(Fl_Widget *w, void *p);
        static void CopyMagnet_cb(Fl_Widget *w, void *p);
        static void SourcesFolder_cb(Fl_Widget *w, void *p);
        static void ReloadSources_cb(Fl_Widget *w, void *p);

    private:
        struct DownloadJob
        {
            LuaSource *Source;
            StoreEntry Manga, Chapter;
            STORE_FORMAT Format;
            bool OpenAfter;
        };

        void LoadSources();
        void BuildTabs();
        void SelectSource(int index);
        void Search(const std::string &query, int page);
        void ClearResults();
        void AddCards(const std::vector<StoreEntry> &results);
        void LayoutCards();
        void LoadCover(Fl_Store_Card *card, const std::string &url, unsigned generation);
        void ShowDetail(bool show);
        void OpenDetail(int index);
        void FillChapters();
        void QueueDownloads(bool read_first);
        void StartNextDownload();
        void SetStatus(const std::string &text);
        void ShowMessage(const std::string &text);
        std::string ChapterBase(const StoreEntry &chapter) const;

        //Widgets
        Fl_Group *TopBar, *TabsGroup;
        std::vector<Fl_Button*> Tabs;
        Fl_Hint_Input *SearchInput;
        Fl_Button *FilterButton;                            //!< Genres and themes, for the stores with filters
        std::map<LuaSource*, std::vector<StoreEntry> > Filters;
        std::map<LuaSource*, std::vector<std::string> > Chosen;    //!< Ids of the filters chosen, per store
        std::string FilterLabel;
        void LoadFilters();
        void UpdateFilterButton();
        static void Filter_cb(Fl_Widget *w, void *p);
        Fl_Infinite_Scroll *Results;
        Fl_Group *Canvas;               //!< Content of Results
        Fl_Box *Message;
        Fl_Box *MoreBox;                    //!< "Loading more..." under the results
        bool LoadingMore;                   //!< The next page is being loaded
        void LoadMore();
        std::vector<Fl_Store_Card*> Cards;
        Fl_Group *Detail, *MangaGroup, *TorrentGroup;
        Fl_Box *DetailCover, *DetailTitle, *DetailInfo, *DetailDescription, *ChaptersLabel, *DetailStatus;
        Fl_Check_Browser *ChapterList;
        Fl_Choice *FormatChoice;
        Fl_Button *DownloadButton, *ReadButton, *MagnetButton, *CopyMagnetButton, *CancelButton;
        Fl_Progress *Progress;
        Fl_RGB_Image *DetailCoverImage;

        //State
        std::vector<LuaSource*> Sources;
        int Current;                        //!< Current source
        std::string Query;
        int Page;
        bool LastPage;
        unsigned Generation;                //!< Incremented by each new search: late results are ignored
        unsigned DetailGeneration;
        std::vector<StoreEntry> Entries;    //!< Search results (as many as cards)
        int Selected;                       //!< Result in the detail panel
        std::vector<StoreEntry> Chapters;
        std::string LibraryFolder;
        std::string Status;
        bool Activated;

        //Downloads (one chapter at a time, in its own thread)
        std::deque<DownloadJob> Queue;
        bool Downloading;
        std::atomic<bool> Cancel;

        TaskPool Tasks, Images;
};

#endif // STOREVIEW_H
