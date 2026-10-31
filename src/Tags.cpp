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
#include "Tags.h"
#include "Preferences.h" //GetUserDataDir
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include <algorithm>
#include <string.h>
#include <strings.h>

namespace
{
    std::vector<TagDef> List;
    bool Loaded=false;

    //! tags.prefs: group "tags" (one sub group per tag), group "comics" (name: "id,id,...")
    Fl_Preferences *Open()
    {
        return new Fl_Preferences(GetUserDataDir(), "claudereader", "tags", Fl_Preferences::C_LOCALE);
    }

    //! Key of a comic: its path in the library, '|' as separator (no '/' in the preferences names)
    std::string ComicKey(const std::string &library_path)
    {
        std::string key=library_path;
        for(size_t i=0; i<key.size(); i++)
            if(key[i]=='/' || key[i]=='\\')
                key[i]='|';
        return key;
    }

    void Sort()
    {
        std::sort(List.begin(), List.end(), [](const TagDef &a, const TagDef &b)
        {
            return strcasecmp(a.Name.c_str(), b.Name.c_str())<0;
        });
    }

    void Load()
    {
        if(Loaded)
            return;
        Loaded=true;
        List.clear();
        Fl_Preferences *prefs=Open();
        {
            //The groups must be released before the root
            Fl_Preferences tags(prefs, "tags");
            for(int i=0; i<tags.groups(); i++)
            {
                Fl_Preferences tag(tags, i);
                TagDef def;
                char name[256];
                tag.get("name", name, "", sizeof(name));
                tag.get("color", def.Color, 0);
                def.Id=atoi(tags.group(i));
                def.Name=name;
                if(def.Id>0 && !def.Name.empty())
                    List.push_back(def);
            }
        }
        delete prefs;
        Sort();
    }

    void SaveTag(const TagDef &def)
    {
        Fl_Preferences *prefs=Open();
        {
            Fl_Preferences tags(prefs, "tags");
            Fl_Preferences tag(tags, std::to_string(def.Id).c_str());
            tag.set("name", def.Name.c_str());
            tag.set("color", def.Color);
        }
        delete prefs;
    }

    std::vector<int> Parse(const std::string &text)
    {
        std::vector<int> ids;
        size_t i=0;
        while(i<text.size())
        {
            size_t comma=text.find(',', i);
            if(comma==std::string::npos)
                comma=text.size();
            int id=atoi(text.substr(i, comma-i).c_str());
            if(id>0)
                ids.push_back(id);
            i=comma+1;
        }
        return ids;
    }
}

const std::vector<TagDef> &Tags::All()
{
    Load();
    return List;
}

const TagDef *Tags::Get(int id)
{
    Load();
    for(size_t i=0; i<List.size(); i++)
        if(List[i].Id==id)
            return &List[i];
    return NULL;
}

int Tags::Find(const std::string &name)
{
    Load();
    for(size_t i=0; i<List.size(); i++)
        if(strcasecmp(List[i].Name.c_str(), name.c_str())==0)
            return List[i].Id;
    return -1;
}

int Tags::Create(const std::string &name)
{
    //Trimmed, no comma (the comics tags are a comma separated list of ids)
    std::string clean=name;
    std::replace(clean.begin(), clean.end(), ',', ' ');
    size_t a=clean.find_first_not_of(" \t"), b=clean.find_last_not_of(" \t");
    if(a==std::string::npos)
        return -1;
    clean=clean.substr(a, b-a+1);
    int existing=Find(clean);
    if(existing>0)
        return existing;

    int id=1;
    for(size_t i=0; i<List.size(); i++)
        id=std::max(id, List[i].Id+1);
    TagDef def{id, clean, (int)(List.size()%PaletteSize())};
    List.push_back(def);
    Sort();
    SaveTag(def);
    return id;
}

void Tags::Rename(int id, const std::string &name)
{
    Load();
    std::string clean=name;
    std::replace(clean.begin(), clean.end(), ',', ' ');
    if(clean.find_first_not_of(" \t")==std::string::npos)
        return;
    for(size_t i=0; i<List.size(); i++)
        if(List[i].Id==id)
        {
            List[i].Name=clean;
            SaveTag(List[i]);
        }
    Sort();
}

void Tags::SetColor(int id, int color)
{
    Load();
    for(size_t i=0; i<List.size(); i++)
        if(List[i].Id==id)
        {
            List[i].Color=color%PaletteSize();
            SaveTag(List[i]);
        }
}

void Tags::Remove(int id)
{
    Load();
    List.erase(std::remove_if(List.begin(), List.end(), [id](const TagDef &t){ return t.Id==id; }), List.end());
    Fl_Preferences *prefs=Open();
    {
        Fl_Preferences tags(prefs, "tags");
        tags.deleteGroup(std::to_string(id).c_str());
        //Comics using it
        Fl_Preferences comics(prefs, "comics");
        for(int i=comics.entries()-1; i>=0; i--)
        {
            std::string key=comics.entry(i);
            char value[1024];
            comics.get(key.c_str(), value, "", sizeof(value));
            std::vector<int> ids=Parse(value);
            ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
            std::string text;
            for(size_t k=0; k<ids.size(); k++)
                text+=(k ? "," : "")+std::to_string(ids[k]);
            if(text.empty())
                comics.deleteEntry(key.c_str());
            else
                comics.set(key.c_str(), text.c_str());
        }
    }
    delete prefs;
}

int Tags::PaletteSize()
{
    return 8;
}

Fl_Color Tags::Color(int index)
{
    //VSCodium like label colors
    static const unsigned char palette[8][3]={{55, 148, 255}, {137, 209, 133}, {204, 167, 0}, {209, 134, 22},
                                              {241, 76, 76}, {177, 128, 215}, {78, 201, 176}, {139, 148, 158}};
    index=((index%8)+8)%8;
    return fl_rgb_color(palette[index][0], palette[index][1], palette[index][2]);
}

std::vector<int> Tags::OfComic(const std::string &filename)
{
    Load();
    Fl_Preferences *prefs=Open();
    char value[1024];
    {
        Fl_Preferences comics(prefs, "comics");
        comics.get(ComicKey(filename).c_str(), value, "", sizeof(value));
    }
    delete prefs;
    //Only the existing tags
    std::vector<int> ids=Parse(value), existing;
    for(size_t i=0; i<ids.size(); i++)
        if(Get(ids[i]))
            existing.push_back(ids[i]);
    return existing;
}

void Tags::SetComic(const std::string &filename, const std::vector<int> &ids)
{
    std::string text;
    for(size_t k=0; k<ids.size(); k++)
        text+=(k ? "," : "")+std::to_string(ids[k]);
    Fl_Preferences *prefs=Open();
    {
        Fl_Preferences comics(prefs, "comics");
        if(text.empty())
            comics.deleteEntry(ComicKey(filename).c_str());
        else
            comics.set(ComicKey(filename).c_str(), text.c_str());
    }
    delete prefs;
}

void Tags::Toggle(const std::string &filename, int id)
{
    std::vector<int> ids=OfComic(filename);
    std::vector<int>::iterator it=std::find(ids.begin(), ids.end(), id);
    if(it==ids.end())
        ids.push_back(id);
    else
        ids.erase(it);
    SetComic(filename, ids);
}
