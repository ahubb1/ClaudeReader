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
 * \file AnnDialog.h
 * \brief Dialogs of the online tags (Anime News Network): find the manga of a comic, choose genres, themes or both
 *
 */
#ifndef ANNDIALOG_H
#define ANNDIALOG_H

#include <string>
#include "AnnTags.h"

//! Search a manga (the name can be corrected), show its genres and themes: the title chosen and what to tag
//! with (ANN_TAGS), ANN_NONE if canceled. scope: what will be tagged ("Claymore Vol 01 and 21 other comics")
int FindTagsDialog(const std::string &name, const std::string &scope, AnnTitle *chosen);

//! Genres, themes or both (ANN_TAGS), ANN_NONE if canceled
int AskTagKind(const std::string &message);

#endif // ANNDIALOG_H
