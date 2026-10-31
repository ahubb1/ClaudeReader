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
 * \file Tags.h
 * \brief Tags of the comics (user interface thread only), saved in tags.prefs of the settings folder.
 *
 * The tags have a stable id (renaming a tag does not change the comics); the comics are identified
 * by their path in the library folder (moving the whole library keeps the tags).
 */
#ifndef TAGS_H
#define TAGS_H

#include <string>
#include <vector>
#include <FL/Enumerations.H>

struct TagDef
{
    int Id;
    std::string Name;
    int Color;      //!< Index in the palette (see Tags::Color)
};

namespace Tags
{
    //! All the tags, sorted by name
    const std::vector<TagDef> &All();
    const TagDef *Get(int id);
    //! Tag of this name (case insensitive), -1 if none
    int Find(const std::string &name);

    //! New tag (or the existing one with this name): its id, -1 for an empty name
    int Create(const std::string &name);
    void Rename(int id, const std::string &name);
    void SetColor(int id, int color);
    //! Remove a tag (and its use by the comics)
    void Remove(int id);

    //! Palette of the tags colors
    int PaletteSize();
    Fl_Color Color(int index);

    //! Tags of a comic, identified by its path in the library ("folder/name.cbz")
    std::vector<int> OfComic(const std::string &library_path);
    void SetComic(const std::string &library_path, const std::vector<int> &ids);
    //! Add or remove a tag of a comic
    void Toggle(const std::string &library_path, int id);
}

#endif // TAGS_H
