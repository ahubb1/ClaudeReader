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
 * \file SettingsView.h
 * \brief Settings view: tags, panel highlight, library folder, store cache
 *
 */
#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <string>
#include <vector>
#include <functional>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include "Theme.h"
#include "PanelHighlight.h"

//! The settings (shown instead of the library, the store or the reader)
class Fl_Settings_View : public Fl_Group
{
    public:
        Fl_Settings_View(int X, int Y, int W, int H);

        //! Update everything from the current values (when shown)
        void Refresh();
        void resize(int X, int Y, int W, int H);

        std::function<std::string()> GetLibraryFolder;
        std::function<bool()> GetUseLibrary;
        std::function<void(const std::string &)> OnLibraryFolder;  //!< A new library folder was chosen
        std::function<void(bool)> OnUseLibrary;
        std::function<void()> OnTagsChanged;                        //!< Tags added, renamed, recolored, removed
        std::function<PanelHighlight()> GetHighlight;
        std::function<void(const PanelHighlight &)> OnHighlight;    //!< Style, thickness or color of the panel highlight changed

        //Callbacks
        static void TagColor_cb(Fl_Widget *w, void *p);
        static void TagName_cb(Fl_Widget *w, void *p);
        static void TagDelete_cb(Fl_Widget *w, void *p);
        static void TagAdd_cb(Fl_Widget *w, void *p);
        static void LibraryFolder_cb(Fl_Widget *w, void *p);
        static void UseLibrary_cb(Fl_Widget *w, void *p);
        static void ClearCache_cb(Fl_Widget *w, void *p);
        static void SourcesFolder_cb(Fl_Widget *w, void *p);
        static void Highlight_cb(Fl_Widget *w, void *p);
        static void HighlightColor_cb(Fl_Widget *w, void *p);
        static void HighlightCustom_cb(Fl_Widget *w, void *p);

    private:
        void Layout();
        void BuildTagRows();
        Fl_Box *Section(const char *title, const char *description);

        Fl_Scroll *Scroll;
        Fl_Group *Content;
        Fl_Box *Title;
        Fl_Box *TagsTitle, *NoTags;
        std::vector<Fl_Widget*> TagRows;    //!< Widgets of the tags list (rebuilt), 4 per tag
        Fl_Hint_Input *NewTagInput;
        Fl_Button *NewTagButton;
        void HighlightChanged();

        Fl_Box *HighlightTitle, *StyleLabel, *ThicknessLabel, *ColorLabel;
        Fl_Choice *StyleChoice, *ThicknessChoice;
        std::vector<Fl_Button*> ColorSwatches;
        Fl_Button *CustomColorButton;
        Fl_Widget *HighlightPreview;
        PanelHighlight Highlight;
        Fl_Box *LibraryTitle, *LibraryPath;
        Fl_Button *LibraryButton;
        Fl_Check_Button *UseLibraryCheck;
        Fl_Box *StoreTitle, *CacheInfo, *SourcesInfo;
        Fl_Button *ClearCacheButton, *SourcesButton;
        std::string LibraryText, CacheText;
};

#endif // SETTINGSVIEW_H
