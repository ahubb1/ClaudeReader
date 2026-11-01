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
 * \file StoreCli.cpp
 * \brief Command line access to the store sources (--store)
 *
 */
#include "StoreCli.h"
#include "LuaSource.h"
#include "StoreDownload.h"
#include <stdio.h>
#include <stdlib.h>

static void PrintEntry(const StoreEntry &e)
{
    printf("%s\t%s", e.Get("id").c_str(), e.Get("title").c_str());
    for(std::map<std::string, std::string>::const_iterator it=e.Fields.begin(); it!=e.Fields.end(); ++it)
        if(it->first!="id" && it->first!="title" && it->first!="description")
            printf("\t%s=%s", it->first.c_str(), it->second.c_str());
    printf("\n");
}

static int Usage()
{
    printf("Usage:\n"
           "  ClaudeReader --store list\n"
           "  ClaudeReader --store <store> search \"<query>\" [page] [filter id,filter id...]\n"
           "  ClaudeReader --store <store> filters\n"
           "  ClaudeReader --store <store> chapters <manga id>\n"
           "  ClaudeReader --store <store> pages <chapter id>\n"
           "  ClaudeReader --store <store> download <manga id> <chapter id> [cbz|zip|pdf|images] [library folder]\n");
    return 1;
}

int StoreCommandLine(int argc, char **argv)
{
    if(argc<1)
        return Usage();

    Net::CacheDir(); //From the main thread first (see Net::CacheDir)
    std::vector<std::string> errors;
    std::vector<LuaSource*> sources=LoadStoreSources(&errors);
    for(size_t i=0; i<errors.size(); i++)
        fprintf(stderr, "Source error: %s\n", errors[i].c_str());

    std::string id=argv[0];
    if(id=="list")
    {
        for(size_t i=0; i<sources.size(); i++)
            printf("%s\t%s\t%s\t%s\t%s\n", sources[i]->Id.c_str(), sources[i]->Name.c_str(), sources[i]->Kind.c_str(), sources[i]->Site.c_str(), sources[i]->Origin.c_str());
        return 0;
    }

    LuaSource *source=NULL;
    for(size_t i=0; i<sources.size(); i++)
        if(sources[i]->Id==id)
            source=sources[i];
    if(!source)
    {
        fprintf(stderr, "Unknown store %s (see --store list)\n", id.c_str());
        return 1;
    }
    std::string command=argc>1 ? argv[1] : "";
    std::string error;
    std::vector<StoreEntry> entries;
    if(command=="filters")
    {
        if(!source->Filters(&entries, &error))
        {
            fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        for(size_t i=0; i<entries.size(); i++)
            PrintEntry(entries[i]);
        return 0;
    }
    if(argc<3)
        return Usage();

    if(command=="search")
    {
        std::vector<std::string> tags;
        std::string list=argc>4 ? argv[4] : "";
        for(size_t start=0; start<list.size();)
        {
            size_t comma=list.find(',', start);
            if(comma==std::string::npos)
                comma=list.size();
            if(comma>start)
                tags.push_back(list.substr(start, comma-start));
            start=comma+1;
        }
        if(!source->Search(argv[2], argc>3 ? atoi(argv[3]) : 1, &entries, &error, tags))
        {
            fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        for(size_t i=0; i<entries.size(); i++)
            PrintEntry(entries[i]);
        return 0;
    }
    if(command=="chapters")
    {
        StoreEntry manga;
        manga.Fields["id"]=argv[2];
        manga.Fields["url"]=argv[2];
        if(!source->Chapters(manga, &entries, &error))
        {
            fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        for(size_t i=0; i<entries.size(); i++)
            PrintEntry(entries[i]);
        return 0;
    }
    if(command=="pages")
    {
        StoreEntry chapter;
        chapter.Fields["id"]=argv[2];
        chapter.Fields["url"]=argv[2];
        std::vector<StorePage> pages;
        if(!source->Pages(chapter, &pages, &error))
        {
            fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        for(size_t i=0; i<pages.size(); i++)
            printf("%s\n", pages[i].Url.c_str());
        return 0;
    }
    if(command=="download" && argc>=4)
    {
        //The manga and chapter descriptions (titles for the file names) come from the source
        StoreEntry manga, chapter;
        manga.Fields["id"]=argv[2];
        manga.Fields["url"]=argv[2];
        if(!source->Chapters(manga, &entries, &error))
        {
            fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        bool found=false;
        for(size_t i=0; i<entries.size() && !found; i++)
            if(entries[i].Get("id")==argv[3])
            {
                chapter=entries[i];
                found=true;
            }
        if(!found)
        {
            fprintf(stderr, "Chapter %s not found\n", argv[3]);
            return 1;
        }
        manga.Fields["title"]=argv[2];
        STORE_FORMAT format=StoreFormatFromName(argc>4 ? argv[4] : "cbz");
        std::string library=argc>5 ? argv[5] : ".";
        std::string output;
        bool ok=DownloadChapter(source, chapter, ChapterOutputBase(library, source, manga, chapter), format,
                                [](int done, int total){ fprintf(stderr, "\r%d/%d pages", done, total); }, NULL, &output, &error);
        fprintf(stderr, "\n");
        if(!ok)
        {
            fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        printf("%s\n", output.c_str());
        return 0;
    }
    return Usage();
}
