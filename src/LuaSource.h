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
 * \file LuaSource.h
 * \brief Store sources: Lua scripts searching and downloading comics from a web site
 *
 * A source script defines:
 *  SOURCE = { id="...", name="...", kind="manga" or "torrent", site="https://...", headers={...} }
 *  function search(query, page)  -> { {id=, title=, cover=, description=, url=, ...}, ... }
 *                                   (torrent sources: magnet=, size=, seeders=, leechers=, date=)
 *  function chapters(manga)      -> { {id=, title=, number=, volume=, group=, date=, ...}, ... } (manga: a search result)
 *  function pages(chapter)       -> { "url", ... } or { {url=, headers={...}, key=}, ... }
 * Available to the scripts: http.get(url[, headers[, cache_seconds]]), http.post(url, body[, headers]),
 * json.decode(text), json.encode(value), json.null, html.parse(text) (nodes: select(css), first(css),
 * text(), attr(name), tag()), util.urlencode(text), util.trim(text), util.log(...)
 */
#ifndef LUASOURCE_H
#define LUASOURCE_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "Net.h"

struct lua_State;

//! A search result or a chapter: the fields returned by the script (as text)
struct StoreEntry
{
    std::map<std::string, std::string> Fields;
    std::string Get(const std::string &key, const std::string &def="") const
    {
        std::map<std::string, std::string>::const_iterator it=Fields.find(key);
        return it==Fields.end() ? def : it->second;
    }
};

//! A page image to download
struct StorePage
{
    std::string Url;
    NetHeaders Headers;
    std::string CacheKey;   //!< Empty: the URL
};

//! A store: one Lua script (thread safe, the calls to the script are serialized)
class LuaSource
{
    public:
        //! \param origin: where the script comes from ("built-in" or a file path)
        LuaSource(const std::string &script, const std::string &origin);
        ~LuaSource();

        bool IsValid() const {return Valid;}
        const std::string &LoadError() const {return Error;}

        bool Search(const std::string &query, int page, std::vector<StoreEntry> *results, std::string *error);
        bool Chapters(const StoreEntry &manga, std::vector<StoreEntry> *chapters, std::string *error);
        bool Pages(const StoreEntry &chapter, std::vector<StorePage> *pages, std::string *error);

        bool IsTorrent() const {return Kind=="torrent";}

        std::string Id, Name, Kind, Site, Version, Origin;
        NetHeaders Headers;     //!< Default headers of the source (images download)

    private:
        bool CallEntries(const char *function, int nargs, std::vector<StoreEntry> *entries, std::string *error);
        void PushEntry(const StoreEntry &entry);

        lua_State *L;
        std::mutex Lock;
        bool Valid;
        std::string Error;
};

//! Built-in sources and the ones of the user sources folder (a user source with the same id replaces the built-in one)
std::vector<LuaSource*> LoadStoreSources(std::vector<std::string> *errors);

//! Folder of the user sources (*.lua), created if needed
std::string UserSourcesDir();

#endif // LUASOURCE_H
