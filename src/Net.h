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
 * \file Net.h
 * \brief HTTP(S) requests with a disk cache (store sources, covers, pages)
 *
 */
#ifndef NET_H
#define NET_H

#include <string>
#include <vector>
#include <utility>

//! Cache durations (seconds) for Net::Get
#define NET_NO_CACHE        0
#define NET_CACHE_HOUR      3600
#define NET_CACHE_FOREVER   -1  //!< Images: covers and pages never change

typedef std::vector< std::pair<std::string, std::string> > NetHeaders;

//! Result of a request
struct NetResponse
{
    long Status;            //!< HTTP status, 0 if the request failed
    std::string Body;
    std::string Error;      //!< Empty when Status is 2xx
    bool FromCache;
    bool Ok() const { return Status>=200 && Status<300; }
    bool Redirected=false;  //!< Answered by another host (a bot challenge, a login page...): not cached
};

//! HTTP(S) requests (thread safe), with a disk cache and a polite delay between requests to the same host
namespace Net
{
    //! GET a URL. The successful responses are cached for cache_ttl seconds (NET_CACHE_FOREVER: always reused)
    //! cache_key: identifies the content in the cache (default: the URL), for contents served by changing URLs
    NetResponse Get(const std::string &url, const NetHeaders &headers=NetHeaders(), int cache_ttl=NET_NO_CACHE, const std::string &cache_key="");
    //! POST a body (never cached)
    NetResponse Post(const std::string &url, const std::string &body, const NetHeaders &headers=NetHeaders());

    //! Folder of the cache (created if needed)
    std::string CacheDir();
    //! Size (bytes) of the cache and removal of everything in it
    long long CacheSize();
    void ClearCache();

    //! Percent-encoding of a string for an URL query
    std::string UrlEncode(const std::string &text);
}

#endif // NET_H
