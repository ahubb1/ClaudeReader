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
#include "Net.h"
#include <curl/curl.h>
#include "FileUtf8.h"
#include <time.h>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>
#include "md5.h"
#include "Preferences.h" //GetUserDataDir

//What ClaudeReader is: MangaDex (and other sites) send a bot challenge to a program presenting itself as a browser
#ifndef CLAUDEREADER_VERSION
    #define CLAUDEREADER_VERSION "1.1"
#endif
#define NET_USER_AGENT "ClaudeReader/" CLAUDEREADER_VERSION " (+https://github.com/ahubb1/ClaudeReader)"
//2: drops the bot challenge pages cached by ClaudeReader 1.1 (it presented itself as a browser) in place of
//MangaDex answers
#define NET_CACHE_VERSION "2"
#define NET_HOST_DELAY_MS 100   //Minimum delay between two requests to the same host (sites rate limits; HTTP 429 answers are retried)


static std::once_flag CurlInit;
static std::mutex HostsMutex;
static std::map<std::string, std::chrono::steady_clock::time_point> HostsLastRequest;


std::string Net::CacheDir()
{
    //Resolved once (GetUserDataDir uses FLTK functions: call CacheDir from the main thread first)
    static std::string dir;
    static std::once_flag once;
    std::call_once(once, []()
    {
        dir=std::string(GetUserDataDir())+"/cache";
        FileUtf8::MakeDirs(dir);
        //A cache of an older format: started again
        std::string version;
        if(!FileUtf8::Read(dir+"/version", &version) || version!=NET_CACHE_VERSION)
        {
            std::vector<std::string> names=FileUtf8::List(dir);
            for(size_t i=0; i<names.size(); i++)
                FileUtf8::Remove(dir+"/"+names[i]);
            FileUtf8::Write(dir+"/version", NET_CACHE_VERSION);
        }
    });
    return dir;
}

//! Host of a URL ("https://api.example.org/x" -> "api.example.org")
static std::string UrlHost(const std::string &url)
{
    size_t start=url.find("://");
    start=start==std::string::npos ? 0 : start+3;
    size_t end=url.find_first_of("/?#", start);
    return url.substr(start, end==std::string::npos ? std::string::npos : end-start);
}

static std::string CacheFile(const std::string &url)
{
    return Net::CacheDir()+"/"+MD5(url).hexdigest();
}

//! Cached content if it exists and is recent enough
static bool ReadCache(const std::string &url, int ttl, std::string *body)
{
    if(ttl==NET_NO_CACHE)
        return false;
    std::string file=CacheFile(url);
    time_t mtime;
    if(!FileUtf8::Stat(file, NULL, &mtime))
        return false;
    if(ttl>0 && difftime(time(NULL), mtime)>ttl)
        return false;
    return FileUtf8::Read(file, body);
}

static void WriteCache(const std::string &url, const std::string &body)
{
    //Write a temporary file first: another thread can read the cache at the same time
    std::string file=CacheFile(url);
    std::string tmp=file+".tmp"+std::to_string(std::hash<std::thread::id>()(std::this_thread::get_id()));
    if(FileUtf8::Write(tmp, body))
        FileUtf8::Rename(tmp, file);
    else
        FileUtf8::Remove(tmp);
}

//! Wait so that two requests to the same host are not too close
static void PoliteDelay(const std::string &url)
{
    size_t start=url.find("://");
    start=(start==std::string::npos) ? 0 : start+3;
    std::string host=url.substr(start, url.find('/', start)-start);

    std::chrono::steady_clock::time_point when;
    {
        std::lock_guard<std::mutex> lock(HostsMutex);
        std::chrono::steady_clock::time_point now=std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point next=HostsLastRequest[host]+std::chrono::milliseconds(NET_HOST_DELAY_MS);
        when=(next>now) ? next : now;
        HostsLastRequest[host]=when;
    }
    std::this_thread::sleep_until(when);
}

static size_t WriteBody(char *data, size_t size, size_t nmemb, void *body)
{
    ((std::string*)body)->append(data, size*nmemb);
    return size*nmemb;
}

static NetResponse Request(const std::string &url, const std::string *post_body, const NetHeaders &headers)
{
    std::call_once(CurlInit, [](){ curl_global_init(CURL_GLOBAL_DEFAULT); });

    NetResponse response;
    response.Status=0;
    response.FromCache=false;

    CURL *curl=curl_easy_init();
    if(!curl)
    {
        response.Error="Unable to initialize the network";
        return response;
    }

    struct curl_slist *list=NULL;
    for(size_t i=0; i<headers.size(); i++)
        list=curl_slist_append(list, (headers[i].first+": "+headers[i].second).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, NET_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); //All the supported compressions
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_NATIVE_CA); //Windows certificates store
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.Body);
    if(post_body)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body->c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)post_body->size());
    }

    PoliteDelay(url);
    CURLcode res=curl_easy_perform(curl);
    if(res==CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.Status);
        if(!response.Ok())
            response.Error="HTTP error "+std::to_string(response.Status);
        //Redirected to another host: a bot challenge or a login page, not the content asked for
        char *final_url=NULL;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &final_url);
        response.Redirected=final_url && UrlHost(final_url)!=UrlHost(url);
    }
    else
    {
        response.Error=curl_easy_strerror(res);
    }

    curl_slist_free_all(list);
    curl_easy_cleanup(curl);
    return response;
}

NetResponse Net::Get(const std::string &url, const NetHeaders &headers, int cache_ttl, const std::string &cache_key)
{
    NetResponse response;
    const std::string &key=cache_key.empty() ? url : cache_key;
    if(ReadCache(key, cache_ttl, &response.Body))
    {
        response.Status=200;
        response.FromCache=true;
        return response;
    }

    //Retry once on network errors and on "too many requests"
    response=Request(url, NULL, headers);
    if(response.Status==0 || response.Status==429 || response.Status>=500)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        response=Request(url, NULL, headers);
    }

    if(response.Ok() && !response.Redirected && cache_ttl!=NET_NO_CACHE)
        WriteCache(key, response.Body);
    return response;
}

NetResponse Net::Post(const std::string &url, const std::string &body, const NetHeaders &headers)
{
    return Request(url, &body, headers);
}

long long Net::CacheSize()
{
    long long total=0, size;
    std::string dir=CacheDir();
    std::vector<std::string> names=FileUtf8::List(dir);
    for(size_t i=0; i<names.size(); i++)
        if(FileUtf8::Stat(dir+"/"+names[i], &size))
            total+=size;
    return total;
}

void Net::ClearCache()
{
    std::string dir=CacheDir();
    std::vector<std::string> names=FileUtf8::List(dir);
    for(size_t i=0; i<names.size(); i++)
        FileUtf8::Remove(dir+"/"+names[i]);
    FileUtf8::Write(dir+"/version", NET_CACHE_VERSION);
}

std::string Net::UrlEncode(const std::string &text)
{
    static const char *hex="0123456789ABCDEF";
    std::string out;
    for(size_t i=0; i<text.size(); i++)
    {
        unsigned char c=(unsigned char)text[i];
        if(isalnum(c) || c=='-' || c=='_' || c=='.' || c=='~')
            out+=(char)c;
        else
        {
            out+='%';
            out+=hex[c>>4];
            out+=hex[c&15];
        }
    }
    return out;
}
