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
 * \file HtmlQuery.h
 * \brief CSS selectors on an HTML5 document parsed by gumbo (used by the store sources)
 *
 */
#ifndef HTMLQUERY_H
#define HTMLQUERY_H

#include <string>
#include <vector>
#include <gumbo.h>

//! Elements below root (root excluded), in document order, matching a CSS selector.
/**Supported: tag, *, #id, .class, [attr], [attr=v], [attr^=v], [attr$=v], [attr*=v], [attr~=v],
:first-child, :last-child, descendant ( ) and child (>) combinators, selector lists (a, b).
\return false (and an error message) if the selector is invalid*/
bool HtmlSelect(const GumboNode *root, const std::string &selector, std::vector<const GumboNode*> *found, std::string *error);

//! Text of a node and its descendants, white spaces collapsed and trimmed
std::string HtmlText(const GumboNode *node);

//! Value of an attribute of an element, empty if absent
std::string HtmlAttr(const GumboNode *node, const char *name);

//! Tag name of an element (lower case), empty for other nodes
std::string HtmlTag(const GumboNode *node);

#endif // HTMLQUERY_H
