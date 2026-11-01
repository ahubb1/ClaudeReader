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
#include <ctype.h>

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

//! Resources a store script may use: memory, and Lua instructions for each load or call
//! (a script stuck in a loop stops with an error instead of freezing ClaudeReader)
struct LuaLimits
{
    size_t Memory=0;        //!< Allocated by the Lua state
    long long Budget=0;     //!< Instructions left for the current load or call
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

        //! tags: ids of the filters chosen (genres, themes), passed to search() as its third argument
        bool Search(const std::string &query, int page, std::vector<StoreEntry> *results, std::string *error,
                    const std::vector<std::string> &tags=std::vector<std::string>());
        //! The script can filter its searches (genres, themes...): it has a filters() function
        bool HasFilters() const {return FiltersFunction;}
        //! The filters of the store: {id=, name=, group=} ("Genres", "Themes"...)
        bool Filters(std::vector<StoreEntry> *filters, std::string *error);
        bool Chapters(const StoreEntry &manga, std::vector<StoreEntry> *chapters, std::string *error);
        bool Pages(const StoreEntry &chapter, std::vector<StorePage> *pages, std::string *error);

        bool IsTorrent() const {return Kind=="torrent";}
        //! A new instance of the same script (its own Lua state): the downloads use one, so that the searches
        //! and chapter lists of the store do not wait for them (issue #8)
        LuaSource *Clone() const {return new LuaSource(Script, Origin);}

        std::string Id, Name, Kind, Site, Version, Origin;
        NetHeaders Headers;     //!< Default headers of the source (images download)

    private:
        bool CallEntries(const char *function, int nargs, std::vector<StoreEntry> *entries, std::string *error);
        void PushEntry(const StoreEntry &entry);

        lua_State *L;
        std::string Script;
        bool FiltersFunction=false;
        LuaLimits Limits;
        std::mutex Lock;
        bool Valid;
        std::string Error;
};

//! Built-in sources and the ones of the user sources folder (a user source with the same id replaces the built-in one)
std::vector<LuaSource*> LoadStoreSources(std::vector<std::string> *errors);

//! A store error for the user: without the Lua stack traceback (kept for the --store command line)
inline std::string UserError(const std::string &error)
{
    size_t cut=error.find("stack traceback:");
    std::string text=cut==std::string::npos ? error : error.substr(0, cut);
    while(!text.empty() && isspace((unsigned char)text.back()))
        text.pop_back();
    return text;
}

//! Folder of the user sources (*.lua), created if needed
std::string UserSourcesDir();

//! A store of the catalog (stores/index.json of the ClaudeReader repository)
struct StoreCatalogEntry
{
    std::string Id, Name, Version, Description, Url;
};
#define STORE_CATALOG_URL "https://raw.githubusercontent.com/ahubb1/ClaudeReader/modernize/stores/index.json"
//! The catalog address: STORE_CATALOG_URL, or the CLAUDEREADER_STORE_CATALOG environment variable (to test a catalog)
std::string StoreCatalogUrl();
//! Read a catalog: {"stores": [{"id", "name", "version", "description", "url"}, ...]}, the urls relative to index_url
bool ParseStoreCatalog(const std::string &json, const std::string &index_url, std::vector<StoreCatalogEntry> *entries, std::string *error);

//! A store as installed: built-in, or a script of the user sources folder
struct InstalledStore
{
    std::string Id, Name, Version, Origin;  //!< Origin: "built-in" or the script path
    bool BuiltIn=false;
    bool ReplacesBuiltIn=false;             //!< A user script with the id of a built-in store
};
std::vector<InstalledStore> ListStores(std::vector<std::string> *errors);
//! Check a store script and save it in the user sources folder (<id>.lua): false and the error if it is not a valid store
bool InstallStoreScript(const std::string &script, InstalledStore *installed, bool *replaced, std::string *error);
//! Compare two versions ("1.10" > "1.9"): <0, 0 or >0
int CompareVersions(const std::string &a, const std::string &b);

#endif // LUASOURCE_H
