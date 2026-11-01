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
#include "AnnTags.h"
#include "Net.h"
#include <regex>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <ctype.h>
#include <string.h>

#define ANN_API "https://cdn.animenewsnetwork.com/encyclopedia/api.xml"
#define ANN_REPORTS "https://www.animenewsnetwork.com/encyclopedia/reports.xml"
#define ANN_CACHE (7*24*3600)       //!< A week, as ANN asks
#define ANN_DELAY_MS 1100           //!< ANN: 1 request per second

std::string AnnTitle::Url() const
{
    return "https://www.animenewsnetwork.com/encyclopedia/manga.php?id="+std::to_string(Id);
}

namespace
{
    //! A request to ANN: from the cache (a week), or at most one per second
    NetResponse AnnFetch(const std::string &url)
    {
        static std::mutex lock;
        static std::chrono::steady_clock::time_point last;
        std::lock_guard<std::mutex> guard(lock);
        NetResponse r;
        if(Net::FromCache(url, ANN_CACHE, &r.Body))
        {
            r.Status=200;
            r.FromCache=true;
            return r;
        }
        std::this_thread::sleep_until(last+std::chrono::milliseconds(ANN_DELAY_MS));
        r=Net::Get(url, NetHeaders(), ANN_CACHE);
        last=std::chrono::steady_clock::now();
        return r;
    }

    std::string Lower(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return (char)tolower(c); });
        return text;
    }

    //! XML text: the entities decoded
    std::string XmlText(const std::string &text)
    {
        std::string out;
        for(size_t i=0; i<text.size(); i++)
        {
            if(text[i]!='&')
            {
                out+=text[i];
                continue;
            }
            size_t end=text.find(';', i);
            if(end==std::string::npos || end-i>10)
            {
                out+=text[i];
                continue;
            }
            std::string entity=text.substr(i+1, end-i-1);
            if(entity=="amp") out+='&';
            else if(entity=="lt") out+='<';
            else if(entity=="gt") out+='>';
            else if(entity=="quot") out+='"';
            else if(entity=="apos") out+='\'';
            else if(entity.size()>1 && entity[0]=='#')
            {
                long code=entity[1]=='x' ? strtol(entity.c_str()+2, NULL, 16) : strtol(entity.c_str()+1, NULL, 10);
                //UTF-8
                if(code<0x80) out+=(char)code;
                else if(code<0x800) { out+=(char)(0xC0|(code>>6)); out+=(char)(0x80|(code&0x3F)); }
                else if(code<0x10000) { out+=(char)(0xE0|(code>>12)); out+=(char)(0x80|((code>>6)&0x3F)); out+=(char)(0x80|(code&0x3F)); }
                else { out+=(char)(0xF0|(code>>18)); out+=(char)(0x80|((code>>12)&0x3F)); out+=(char)(0x80|((code>>6)&0x3F)); out+=(char)(0x80|(code&0x3F)); }
            }
            else
            {
                out+=text.substr(i, end-i+1);
            }
            i=end;
        }
        return out;
    }

    //! Value of an attribute in a start tag
    std::string Attribute(const std::string &tag, const char *name)
    {
        std::string key=std::string(" ")+name+"=\"";
        size_t p=tag.find(key);
        if(p==std::string::npos)
            return "";
        p+=key.size();
        size_t end=tag.find('"', p);
        return end==std::string::npos ? "" : XmlText(tag.substr(p, end-p));
    }

    //! The <manga> elements of an api.xml answer, with their genres and themes
    std::vector<AnnTitle> ParseDetails(const std::string &xml)
    {
        std::vector<AnnTitle> titles;
        size_t p=0;
        while((p=xml.find("<manga ", p))!=std::string::npos)
        {
            size_t tag_end=xml.find('>', p);
            size_t end=xml.find("</manga>", p);
            if(tag_end==std::string::npos || end==std::string::npos)
                break;
            std::string tag=xml.substr(p, tag_end-p);
            AnnTitle title;
            title.Id=atoi(Attribute(tag, "id").c_str());
            title.Name=Attribute(tag, "name");
            title.Detailed=true;
            std::string body=xml.substr(tag_end+1, end-tag_end-1);
            size_t q=0;
            while((q=body.find("<info ", q))!=std::string::npos)
            {
                size_t info_end=body.find('>', q);
                size_t close=body.find("</info>", q);
                if(info_end==std::string::npos || close==std::string::npos || close<info_end)
                    break;
                std::string info_tag=body.substr(q, info_end-q);
                std::string type=Attribute(info_tag, "type"), value=XmlText(body.substr(info_end+1, close-info_end-1));
                if(type=="Genres" && !value.empty())
                    title.Genres.push_back(value);
                else if(type=="Themes" && !value.empty())
                    title.Themes.push_back(value);
                q=close+7;
            }
            if(title.Id>0)
                titles.push_back(title);
            p=end+8;
        }
        return titles;
    }

    //! The <item> of a reports.xml answer: id and name
    std::vector<AnnTitle> ParseReport(const std::string &xml)
    {
        std::vector<AnnTitle> titles;
        size_t p=0;
        while((p=xml.find("<item>", p))!=std::string::npos)
        {
            size_t end=xml.find("</item>", p);
            if(end==std::string::npos)
                break;
            std::string item=xml.substr(p, end-p);
            auto field=[&item](const char *name)
            {
                std::string open=std::string("<")+name+">", close=std::string("</")+name+">";
                size_t a=item.find(open), b=item.find(close);
                return (a==std::string::npos || b==std::string::npos) ? std::string() : XmlText(item.substr(a+open.size(), b-a-open.size()));
            };
            AnnTitle title;
            title.Id=atoi(field("id").c_str());
            title.Name=field("name");
            if(title.Id>0 && !title.Name.empty())
                titles.push_back(title);
            p=end+7;
        }
        return titles;
    }
}

std::string Ann::SeriesName(const std::string &title)
{
    std::string s=title;
    //"<series> - <chapter>" (the store downloads)
    size_t dash=s.find(" - ");
    if(dash!=std::string::npos && dash>0)
        s.erase(dash);
    //Brackets: groups, years, sources
    s=std::regex_replace(s, std::regex("\\[[^\\]]*\\]|\\([^)]*\\)|\\{[^}]*\\}"), " ");
    std::replace(s.begin(), s.end(), '_', ' ');
    //Volume or chapter numbers, and what follows
    s=std::regex_replace(s, std::regex("\\s(v|vol|volume|ch|chap|chapter|c|tome|t|book|part|#)\\s*\\.?\\s*\\d.*$", std::regex::icase), "");
    //Trailing numbers ("Claymore 01")
    s=std::regex_replace(s, std::regex("(\\s+\\d+(\\.\\d+)?)+\\s*$"), "");
    //Spaces
    s=std::regex_replace(s, std::regex("\\s+"), " ");
    size_t a=s.find_first_not_of(" -.,"), b=s.find_last_not_of(" -.,");
    return a==std::string::npos ? title : s.substr(a, b-a+1);
}

//! The closest names first: the name itself, then the names starting with it, then the shortest
//! ("One Piece" before "ONE PIECE CHOPPER's")
static void SortMatches(std::vector<AnnTitle> *titles, const std::string &name)
{
    std::string key=Lower(name);
    auto rank=[&key](const AnnTitle &t)
    {
        std::string n=Lower(t.Name);
        return n==key ? 0 : n.compare(0, key.size(), key)==0 ? 1 : 2;
    };
    std::stable_sort(titles->begin(), titles->end(), [&](const AnnTitle &a, const AnnTitle &b)
    {
        int ra=rank(a), rb=rank(b);
        return ra!=rb ? ra<rb : a.Name.size()<b.Name.size();
    });
}

bool Ann::Find(const std::string &name, std::vector<AnnTitle> *titles, std::string *error, std::string *searched)
{
    if(searched)
        *searched=name;
    bool found=FindUnsorted(name, titles, error);
    //Nothing: the longest word ("Sousou no Frieren", a romanized title, is "Frieren: Beyond Journey's End")
    if(!found)
    {
        std::string longest, word;
        for(size_t i=0; i<=name.size(); i++)
        {
            if(i<name.size() && !isspace((unsigned char)name[i]))
                word+=name[i];
            else
            {
                if(word.size()>longest.size())
                    longest=word;
                word.clear();
            }
        }
        std::string retry_error;
        if(longest.size()>=4 && longest!=name && FindUnsorted(longest, titles, &retry_error))
        {
            found=true;
            if(searched)
                *searched=longest;
        }
    }
    if(found)
        SortMatches(titles, searched ? *searched : name);
    return found;
}

bool Ann::FindUnsorted(const std::string &name, std::vector<AnnTitle> *titles, std::string *error)
{
    titles->clear();
    if(name.empty())
    {
        *error="Nothing to search";
        return false;
    }
    //Main names starting with the name: the details directly
    NetResponse r=AnnFetch(std::string(ANN_API)+"?manga=~"+Net::UrlEncode(name));
    if(!r.Ok())
    {
        *error="Anime News Network: "+r.Error;
        return false;
    }
    *titles=ParseDetails(r.Body);
    if(!titles->empty())
        return true;
    //Any name containing it (alternative titles too)
    r=AnnFetch(std::string(ANN_REPORTS)+"?id=155&type=manga&nlist=20&name="+Net::UrlEncode(name));
    if(!r.Ok())
    {
        *error="Anime News Network: "+r.Error;
        return false;
    }
    *titles=ParseReport(r.Body);
    if(titles->empty())
        *error="No manga named \""+name+"\" on Anime News Network";
    return !titles->empty();
}

bool Ann::FindSure(const std::string &name, AnnTitle *title, std::string *error)
{
    NetResponse r=AnnFetch(std::string(ANN_API)+"?manga=~"+Net::UrlEncode(name));
    if(!r.Ok())
    {
        *error="Anime News Network: "+r.Error;
        return false;
    }
    std::vector<AnnTitle> titles=ParseDetails(r.Body);
    for(size_t i=0; i<titles.size(); i++)
        if(Lower(titles[i].Name)==Lower(name))
        {
            *title=titles[i];
            return true;
        }
    if(titles.size()==1)
    {
        *title=titles[0];
        return true;
    }
    *error=titles.empty() ? "not found" : std::to_string(titles.size())+" possible titles";
    return false;
}

bool Ann::Details(AnnTitle *title, std::string *error)
{
    if(title->Detailed)
        return true;
    NetResponse r=AnnFetch(std::string(ANN_API)+"?manga="+std::to_string(title->Id));
    if(!r.Ok())
    {
        *error="Anime News Network: "+r.Error;
        return false;
    }
    std::vector<AnnTitle> titles=ParseDetails(r.Body);
    if(titles.empty())
    {
        *error="No details for "+title->Name;
        return false;
    }
    *title=titles[0];
    return true;
}

std::vector<std::string> Ann::TagNames(const AnnTitle &title, int what)
{
    std::vector<std::string> names;
    auto add=[&names](const std::vector<std::string> &list)
    {
        for(size_t i=0; i<list.size(); i++)
        {
            std::string name=list[i];
            if(!name.empty())
                name[0]=(char)toupper((unsigned char)name[0]);
            names.push_back(name);
        }
    };
    if(what&ANN_GENRES)
        add(title.Genres);
    if(what&ANN_THEMES)
        add(title.Themes);
    return names;
}
