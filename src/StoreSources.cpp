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
 * \file StoreSources.cpp
 * \brief Built-in store sources (Lua scripts, see LuaSource.h for the scripts interface)
 *
 * To fix or add a source without rebuilding, put a .lua file in the user sources folder
 * (%APPDATA%\ClaudeReader\sources): a script with the same SOURCE.id replaces the built-in one.
 */

//! Built-in sources, NULL terminated
extern const char *BuiltinSources[];

//------------------------------------MangaDex------------------------
//Official JSON API: https://api.mangadex.org/docs/
static const char *MangaDex=R"lua(
SOURCE = {
    id = "mangadex", name = "MangaDex", version = "1.1", kind = "manga",
    site = "https://mangadex.org",
    headers = { Referer = "https://mangadex.org/" },
}

local API = "https://api.mangadex.org"
local LANGUAGE = "en"
-- All the content is listed (no rating filter)
local ALL_RATINGS = "&contentRating[]=safe&contentRating[]=suggestive&contentRating[]=erotica&contentRating[]=pornographic"
local SEARCH_RATINGS = ALL_RATINGS

local function value(v) -- json.null and missing values as nil
    if v == nil or v == json.null then return nil end
    return v
end

local function get_json(url, cache)
    local body, status = http.get(url, nil, cache)
    if not body then error("network error: " .. tostring(status), 0) end
    if status ~= 200 then error("HTTP " .. status .. " for " .. url, 0) end
    local data, err = json.decode(body)
    if not data then error(err, 0) end
    return data
end

local function pick_title(attributes)
    local titles = value(attributes.title) or {}
    if titles.en then return titles.en end
    for _, alt in ipairs(value(attributes.altTitles) or {}) do
        if alt.en then return alt.en end
    end
    for _, t in pairs(titles) do return t end
    return "?"
end

-- Genres and themes: the MangaDex tags (cached a week)
function filters()
    local list = {}
    for _, tag in ipairs(get_json(API .. "/manga/tag", 604800).data) do
        local group = tag.attributes.group
        if group == "genre" or group == "theme" then
            list[#list + 1] = { id = tag.id, name = value(tag.attributes.name.en) or tag.id,
                                group = group == "genre" and "Genres" or "Themes" }
        end
    end
    table.sort(list, function(a, b) return a.group == b.group and a.name < b.name or a.group < b.group end)
    return list
end

function search(query, page, tags)
    local url = API .. "/manga?limit=24&offset=" .. ((page - 1) * 24) .. "&includes[]=cover_art" .. SEARCH_RATINGS
    for _, id in ipairs(tags or {}) do
        url = url .. "&includedTags[]=" .. util.urlencode(id)
    end
    if query ~= "" then
        url = url .. "&title=" .. util.urlencode(query) .. "&order[relevance]=desc"
    else
        url = url .. "&order[followedCount]=desc"
    end
    local results = {}
    for _, manga in ipairs(get_json(url, 600).data) do
        local a = manga.attributes
        local cover = ""
        for _, r in ipairs(manga.relationships) do
            if r.type == "cover_art" and r.attributes and value(r.attributes.fileName) then
                cover = "https://uploads.mangadex.org/covers/" .. manga.id .. "/" .. r.attributes.fileName .. ".256.jpg"
            end
        end
        local description = value(a.description) and value(a.description.en) or ""
        results[#results + 1] = {
            id = manga.id, title = pick_title(a), cover = cover, description = description,
            url = "https://mangadex.org/title/" .. manga.id,
            status = value(a.status), year = value(a.year),
        }
    end
    return results
end

function chapters(manga)
    local list, offset, total = {}, 0, 1
    while offset < total do
        local data = get_json(API .. "/manga/" .. manga.id .. "/feed?limit=500&offset=" .. offset
            .. "&translatedLanguage[]=" .. LANGUAGE .. "&order[volume]=asc&order[chapter]=asc"
            .. "&includes[]=scanlation_group" .. ALL_RATINGS, 600)
        total = data.total
        for _, c in ipairs(data.data) do
            local a = c.attributes
            -- Chapters hosted on another site have no pages here
            if not value(a.externalUrl) and (value(a.pages) or 0) > 0 then
                local group = ""
                for _, r in ipairs(c.relationships) do
                    if r.type == "scanlation_group" and r.attributes then group = value(r.attributes.name) or "" end
                end
                local number = value(a.chapter) or ""
                local name = value(a.title) or ""
                local title = (number ~= "" and ("Chapter " .. number) or "Oneshot") .. (name ~= "" and (": " .. name) or "")
                list[#list + 1] = {
                    id = c.id, title = title, number = number, volume = value(a.volume) or "",
                    group = group, language = a.translatedLanguage,
                    date = (value(a.publishAt) or ""):sub(1, 10), pages = a.pages,
                }
            end
        end
        offset = offset + 500
    end
    return list
end

function pages(chapter)
    -- The image servers change: the cache key is the image itself
    local data = get_json(API .. "/at-home/server/" .. chapter.id, 0)
    local list = {}
    for _, file in ipairs(data.chapter.data) do
        list[#list + 1] = {
            url = data.baseUrl .. "/data/" .. data.chapter.hash .. "/" .. file,
            key = "mangadex/" .. data.chapter.hash .. "/" .. file,
        }
    end
    return list
end
)lua";


//------------------------------------Mangapill------------------------
//Web pages (no API); the images server checks the Referer
static const char *Mangapill=R"lua(
SOURCE = {
    id = "mangapill", name = "Mangapill", version = "1.2", kind = "manga",
    site = "https://mangapill.com",
    headers = { Referer = "https://mangapill.com/" },
}

local BASE = "https://mangapill.com"

local function get_page(url)
    local body, status = http.get(url, { Referer = BASE .. "/" }, 600)
    if not body then error("network error: " .. tostring(status), 0) end
    if status ~= 200 then error("HTTP " .. status .. " for " .. url, 0) end
    return html.parse(body)
end

-- Genres: the checkboxes of the search page (cached a week)
function filters()
    local doc = html.parse((http.get(BASE .. "/search", { Referer = BASE .. "/" }, 604800)))
    local list = {}
    for _, input in ipairs(doc:select("input[name=genre]")) do
        local name = input:attr("value")
        if name and name ~= "" then list[#list + 1] = { id = name, name = name, group = "Genres" } end
    end
    return list
end

function search(query, page, tags)
    local doc
    tags = tags or {}
    if #tags > 0 then
        local url = BASE .. "/search?q=" .. util.urlencode(query) .. "&page=" .. page
        for _, genre in ipairs(tags) do url = url .. "&genre=" .. util.urlencode(genre) end
        doc = get_page(url)
    elseif query == "" then
        -- No query: the trending titles of the home page (a single page)
        if page > 1 then return {} end
        doc = get_page(BASE .. "/")
    else
        doc = get_page(BASE .. "/search?q=" .. util.urlencode(query) .. "&page=" .. page)
    end
    local results, seen = {}, {}
    for _, card in ipairs(doc:select("div.grid > div")) do
        local link = card:first("a[href^='/manga/']")
        -- The series name is the first line of a series link (on the home page the bold "#48" is the chapter)
        local title
        for _, a in ipairs(card:select("a[href^='/manga/']")) do
            local line = a:first("div")
            if line and line:text() ~= "" then title = line break end
        end
        if link and title and not seen[link:attr("href")] then
            seen[link:attr("href")] = true
            local img = card:first("img")
            local result = {
                id = link:attr("href"), url = BASE .. link:attr("href"), title = title:text(),
                cover = img and (img:attr("data-src") or img:attr("src")) or "",
            }
            -- Badges: type, year, status
            for _, badge in ipairs(card:select("div.text-xs")) do
                local text = badge:text()
                if text:match("^%d%d%d%d$") then result.year = text
                elseif text == "publishing" or text == "finished" or text == "on hiatus" or text == "discontinued" then result.status = text end
            end
            results[#results + 1] = result
        end
    end
    return results
end

function chapters(manga)
    local doc = get_page(BASE .. manga.id)
    local list = {}
    local links = doc:select("#chapters a[href^='/chapters/']")
    -- Newest first on the site
    for i = #links, 1, -1 do
        local a = links[i]
        local text = a:text()
        list[#list + 1] = {
            id = a:attr("href"), url = BASE .. a:attr("href"), title = text,
            number = text:match("Chapter%s+([%d%.]+)") or "",
        }
    end
    return list
end

function pages(chapter)
    local doc = get_page(BASE .. chapter.id)
    local list = {}
    for _, img in ipairs(doc:select("img.js-page")) do
        local url = img:attr("data-src") or img:attr("src")
        if url then list[#list + 1] = url end
    end
    return list
end
)lua";

//------------------------------------Nyaa------------------------
//Torrents index: the downloads are done by a torrent client (magnet links)
static const char *Nyaa=R"lua(
SOURCE = {
    id = "nyaa", name = "Nyaa", version = "1.0", kind = "torrent",
    site = "https://nyaa.si",
}

local BASE = "https://nyaa.si"
-- Literature, English-translated (use "3_0" for all the literature)
local CATEGORY = "3_1"

function search(query, page)
    local url = BASE .. "/?f=0&c=" .. CATEGORY .. "&q=" .. util.urlencode(query) .. "&p=" .. page
    if query == "" then url = url .. "&s=seeders&o=desc" end
    local body, status = http.get(url, nil, 300)
    if not body then error("network error: " .. tostring(status), 0) end
    if status ~= 200 then error("HTTP " .. status, 0) end
    local results = {}
    for _, row in ipairs(html.parse(body):select("table.torrent-list tbody tr")) do
        local cells = row:select("td")
        local title
        for _, a in ipairs(cells[2] and cells[2]:select("a") or {}) do
            if not (a:attr("class") or ""):find("comments") then title = a end
        end
        local magnet = row:first("a[href^='magnet:']")
        if title and #cells >= 7 then
            results[#results + 1] = {
                id = title:attr("href"), url = BASE .. title:attr("href"),
                title = title:attr("title") or title:text(),
                magnet = magnet and magnet:attr("href") or "",
                size = cells[4]:text(), date = cells[5]:text(),
                seeders = cells[6]:text(), leechers = cells[7]:text(),
            }
        end
    end
    return results
end
)lua";


const char *BuiltinSources[]=
{
    MangaDex,
    Mangapill,
    Nyaa,
    0
};
