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
#include "FileUtf8.h"
#include <sys/stat.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <dirent.h>
#endif

#ifdef _WIN32
//! UTF-8 to UTF-16, in a local buffer (thread safe)
static std::wstring Wide(const std::string &s)
{
    if(s.empty())
        return std::wstring();
    int n=MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), NULL, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}

static std::string Narrow(const wchar_t *w)
{
    int n=WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
    std::string s(n>0 ? n-1 : 0, '\0');
    if(n>1)
        WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], n, NULL, NULL);
    return s;
}
#endif

#ifdef _WIN32
//! Paths longer than MAX_PATH (260): the extended form \\?\C:\... (absolute, backslashes only)
static std::wstring WidePath(const std::string &path)
{
    bool absolute=(path.size()>2 && path[1]==':' && (path[2]=='\\' || path[2]=='/')) ||
                  (path.size()>2 && (path[0]=='\\' || path[0]=='/') && (path[1]=='\\' || path[1]=='/'));
    if(path.size()<MAX_PATH-12 || !absolute || path.compare(0, 4, "\\\\?\\")==0)
        return Wide(path);
    std::string p=path;
    for(size_t i=0; i<p.size(); i++)
        if(p[i]=='/')
            p[i]='\\';
    //UNC: \\server\share -> \\?\UNC\server\share
    if(p.compare(0, 2, "\\\\")==0)
        return Wide("\\\\?\\UNC\\"+p.substr(2));
    return Wide("\\\\?\\"+p);
}
#endif

FILE *FileUtf8::Open(const std::string &path, const char *mode)
{
#ifdef _WIN32
    return _wfopen(WidePath(path).c_str(), Wide(mode).c_str());
#else
    return fopen(path.c_str(), mode);
#endif
}

bool FileUtf8::Stat(const std::string &path, long long *size, time_t *mtime, bool *is_dir)
{
#ifdef _WIN32
    //No trailing separator (except a drive root)
    std::string p=path;
    while(p.size()>3 && (p[p.size()-1]=='/' || p[p.size()-1]=='\\'))
        p.erase(p.size()-1);
    struct _stat64 info;
    if(_wstat64(Wide(p).c_str(), &info)!=0)
        return false;
#else
    struct stat info;
    if(stat(path.c_str(), &info)!=0)
        return false;
#endif
    if(size) *size=info.st_size;
    if(mtime) *mtime=info.st_mtime;
    if(is_dir) *is_dir=(info.st_mode & S_IFMT)==S_IFDIR;
    return true;
}

bool FileUtf8::Exists(const std::string &path)
{
    return Stat(path);
}

bool FileUtf8::IsDir(const std::string &path)
{
    bool dir=false;
    return Stat(path, NULL, NULL, &dir) && dir;
}

bool FileUtf8::MakeDirs(const std::string &path)
{
    if(path.empty() || IsDir(path))
        return true;
    //Parents first
    size_t sep=path.find_last_of("/\\", path.find_last_not_of("/\\"));
    if(sep!=std::string::npos && sep>0)
        MakeDirs(path.substr(0, sep));
#ifdef _WIN32
    return CreateDirectoryW(Wide(path).c_str(), NULL) || IsDir(path);
#else
    return mkdir(path.c_str(), 0755)==0 || IsDir(path);
#endif
}

bool FileUtf8::Remove(const std::string &path)
{
#ifdef _WIN32
    return DeleteFileW(Wide(path).c_str())!=0;
#else
    return unlink(path.c_str())==0;
#endif
}

bool FileUtf8::RemoveDir(const std::string &path)
{
#ifdef _WIN32
    return RemoveDirectoryW(Wide(path).c_str())!=0;
#else
    return rmdir(path.c_str())==0;
#endif
}

bool FileUtf8::Rename(const std::string &from, const std::string &to)
{
#ifdef _WIN32
    return MoveFileExW(Wide(from).c_str(), Wide(to).c_str(), MOVEFILE_REPLACE_EXISTING)!=0;
#else
    return rename(from.c_str(), to.c_str())==0;
#endif
}

std::vector<std::string> FileUtf8::List(const std::string &dir)
{
    std::vector<std::string> names;
#ifdef _WIN32
    WIN32_FIND_DATAW data;
    HANDLE h=FindFirstFileW(Wide(dir+"/*").c_str(), &data);
    if(h==INVALID_HANDLE_VALUE)
        return names;
    do
    {
        std::string name=Narrow(data.cFileName);
        if(name!="." && name!="..")
            names.push_back(name);
    } while(FindNextFileW(h, &data));
    FindClose(h);
#else
    DIR *d=opendir(dir.c_str());
    if(!d)
        return names;
    while(struct dirent *e=readdir(d))
    {
        std::string name=e->d_name;
        if(name!="." && name!="..")
            names.push_back(name);
    }
    closedir(d);
#endif
    return names;
}

bool FileUtf8::Read(const std::string &path, std::string *data)
{
    FILE *f=Open(path, "rb");
    if(!f)
        return false;
    data->clear();
    char chunk[65536];
    size_t n;
    while((n=fread(chunk, 1, sizeof(chunk), f))>0)
        data->append(chunk, n);
    fclose(f);
    return true;
}

bool FileUtf8::Write(const std::string &path, const std::string &data)
{
    FILE *f=Open(path, "wb");
    if(!f)
        return false;
    bool ok=fwrite(data.data(), 1, data.size(), f)==data.size();
    ok=(fclose(f)==0) && ok;
    return ok;
}
