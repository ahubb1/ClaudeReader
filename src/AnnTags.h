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
 * \file AnnTags.h
 * \brief Genres and themes of a manga from the Anime News Network encyclopedia (https://www.animenewsnetwork.com),
 * to tag the comics of the library.
 *
 * ANN API terms: credit ANN as the source, link to the encyclopedia entry, at most 1 request per second,
 * cache the details (a week). The functions are thread safe (network calls: not from the user interface thread).
 */
#ifndef ANNTAGS_H
#define ANNTAGS_H

#include <string>
#include <vector>

//! A manga of the encyclopedia
struct AnnTitle
{
    int Id=0;
    std::string Name;
    std::vector<std::string> Genres, Themes;
    bool Detailed=false;    //!< Genres and themes loaded (the name search gives the ids and names only)
    std::string Url() const;
};

//! What to tag with
enum ANN_TAGS { ANN_NONE=0, ANN_GENRES=1, ANN_THEMES=2, ANN_BOTH=3 };

namespace Ann
{
    //! The series name of a comic title: without volume or chapter numbers, brackets, years, scan groups...
    //! ("Claymore v01 (2006) (Digital) (Group)" -> "Claymore")
    std::string SeriesName(const std::string &title);

    //! Manga matching a name: the titles whose main name starts with it (with their details), or else the
    //! titles containing it (names only: Details() for the one chosen)
    //! searched: the name finally searched (the longest word of the name when the name gives nothing)
    bool Find(const std::string &name, std::vector<AnnTitle> *titles, std::string *error, std::string *searched=nullptr);
    //! The same, in the order of ANN (Find sorts the closest names first)
    bool FindUnsorted(const std::string &name, std::vector<AnnTitle> *titles, std::string *error);
    //! Only a sure match (Tag all): a single title, or one whose main name is the name searched
    bool FindSure(const std::string &name, AnnTitle *title, std::string *error);
    //! Genres and themes of a title found by name
    bool Details(AnnTitle *title, std::string *error);

    //! Tag names: "female warriors" -> "Female warriors"
    std::vector<std::string> TagNames(const AnnTitle &title, int what);
}

#endif // ANNTAGS_H
