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
#include "StoreDownload.h"
#include "FileUtf8.h"
#include <FL/fl_utf8.h> //fl_utf8towc (no shared buffer)
#include <thread>
#include <mutex>
#include <archive.h>
#include <archive_entry.h>
#include <mupdf/fitz.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#define DOWNLOAD_THREADS 4


const char *StoreFormatExtension(STORE_FORMAT format)
{
    switch(format)
    {
        case FORMAT_ZIP: return ".zip";
        case FORMAT_PDF: return ".pdf";
        case FORMAT_IMAGES: return "";
        default: return ".cbz";
    }
}

STORE_FORMAT StoreFormatFromName(const std::string &name)
{
    if(name=="zip") return FORMAT_ZIP;
    if(name=="pdf") return FORMAT_PDF;
    if(name=="images") return FORMAT_IMAGES;
    return FORMAT_CBZ;
}

std::string SafeFileName(const std::string &name)
{
    std::string out;
    for(size_t i=0; i<name.size(); i++)
    {
        unsigned char c=(unsigned char)name[i];
        if(c<0x20 || strchr("<>:\"/\\|?*", c))
            out+=(c==':') ? " -" : "_";
        else
            out+=(char)c;
    }
    //No leading/trailing spaces or dots (Windows)
    size_t a=out.find_first_not_of(" .");
    size_t b=out.find_last_not_of(" .");
    out=(a==std::string::npos) ? "_" : out.substr(a, b-a+1);
    //Length limit, without cutting an UTF-8 character
    if(out.size()>120)
    {
        size_t cut=120;
        while(cut>0 && ((unsigned char)out[cut]&0xC0)==0x80)
            cut--;
        out=out.substr(0, cut);
    }
    return out;
}

std::string ChapterOutputBase(const std::string &library_dir, const LuaSource *source, const StoreEntry &manga, const StoreEntry &chapter)
{
    std::string dir=library_dir;
    if(!dir.empty() && dir[dir.size()-1]!='/' && dir[dir.size()-1]!='\\')
        dir+="/";
    std::string name;
    if(!chapter.Get("volume").empty())
        name="Vol. "+chapter.Get("volume")+" ";
    name+=chapter.Get("title", chapter.Get("id"));
    //Different groups can translate the same chapter
    if(!chapter.Get("group").empty())
        name+=" ["+chapter.Get("group")+"]";
    //The series in the file name too: "Chapter 1" exists in every series (the reading history is per file name)
    std::string series=SafeFileName(manga.Get("title", manga.Get("id")));
    return dir+"Store/"+SafeFileName(source->Name)+"/"+series+"/"+SafeFileName(series+" - "+name);
}

static bool Exists(const std::string &path)
{
    return FileUtf8::Exists(path);
}

//! Remove a file, or a folder and its files (the images export has no sub-folders)
static void RemovePath(const std::string &path)
{
    if(FileUtf8::IsDir(path))
    {
        std::vector<std::string> names=FileUtf8::List(path);
        for(size_t i=0; i<names.size(); i++)
            FileUtf8::Remove(path+"/"+names[i]);
        FileUtf8::RemoveDir(path);
    }
    else
        FileUtf8::Remove(path);
}

std::string FindDownloadedChapter(const std::string &output_base)
{
    const char *extensions[]={".cbz", ".zip", ".pdf"};
    for(int i=0; i<3; i++)
        if(Exists(output_base+extensions[i]))
            return output_base+extensions[i];
    if(FileUtf8::IsDir(output_base))
        return output_base;
    return "";
}

//! Image file extension from its first bytes
static const char *ImageExtension(const std::string &data)
{
    const unsigned char *d=(const unsigned char*)data.data();
    if(data.size()>=3 && d[0]==0xFF && d[1]==0xD8) return ".jpg";
    if(data.size()>=4 && d[0]==0x89 && d[1]=='P' && d[2]=='N' && d[3]=='G') return ".png";
    if(data.size()>=12 && !memcmp(d, "RIFF", 4) && !memcmp(d+8, "WEBP", 4)) return ".webp";
    if(data.size()>=4 && !memcmp(d, "GIF8", 4)) return ".gif";
    if(data.size()>=2 && d[0]=='B' && d[1]=='M') return ".bmp";
    return "";
}

static std::string PageName(size_t index, size_t total, const std::string &data)
{
    char name[32];
    snprintf(name, sizeof(name), total>=1000 ? "%04d" : "%03d", (int)index+1);
    return std::string(name)+ImageExtension(data);
}

static bool WriteFile(const std::string &path, const std::string &data)
{
    return FileUtf8::Write(path, data);
}

//! Zip archive (CBZ or ZIP), images stored (already compressed)
static bool WriteArchive(const std::string &path, const std::vector<std::string> &images, std::string *error)
{
    struct archive *a=archive_write_new();
    archive_write_set_format_zip(a);
    archive_write_zip_set_compression_store(a);
    #ifdef _WIN32
        std::vector<wchar_t> wpath(path.size()+1);
        fl_utf8towc(path.c_str(), (unsigned)path.size(), wpath.data(), (unsigned)wpath.size());
        int ret=archive_write_open_filename_w(a, wpath.data());
    #else
        int ret=archive_write_open_filename(a, path.c_str());
    #endif
    if(ret!=ARCHIVE_OK)
    {
        *error=std::string("Unable to write ")+path+": "+archive_error_string(a);
        archive_write_free(a);
        return false;
    }
    bool ok=true;
    for(size_t i=0; i<images.size() && ok; i++)
    {
        struct archive_entry *entry=archive_entry_new();
        archive_entry_set_pathname(entry, PageName(i, images.size(), images[i]).c_str());
        archive_entry_set_size(entry, (la_int64_t)images[i].size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        ok=archive_write_header(a, entry)==ARCHIVE_OK &&
           archive_write_data(a, images[i].data(), images[i].size())==(la_ssize_t)images[i].size();
        archive_entry_free(entry);
    }
    if(!ok)
        *error=std::string("Unable to write ")+path+": "+archive_error_string(a);
    if(archive_write_close(a)!=ARCHIVE_OK && ok)
    {
        *error=std::string("Unable to write ")+path+": "+archive_error_string(a);
        ok=false;
    }
    archive_write_free(a);
    return ok;
}

//! PDF with one page per image, at the image size (96 dpi)
static bool WritePdf(const std::string &path, const std::vector<std::string> &images, std::string *error)
{
    fz_context *ctx=fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if(!ctx)
    {
        *error="Not enough memory";
        return false;
    }
    bool ok=true;
    fz_document_writer *writer=NULL;
    fz_var(writer);
    fz_try(ctx)
    {
        writer=fz_new_document_writer(ctx, path.c_str(), "pdf", "compress");
        for(size_t i=0; i<images.size(); i++)
        {
            //Formats MuPDF may not read (WebP, GIF...) are converted to PNG first
            std::string data=images[i];
            std::string ext=ImageExtension(data);
            if(ext!=".jpg" && ext!=".png")
            {
                std::vector<unsigned char> raw(data.begin(), data.end()), png;
                cv::Mat img=cv::imdecode(raw, cv::IMREAD_COLOR);
                if(!img.empty() && cv::imencode(".png", img, png))
                    data.assign(png.begin(), png.end());
            }
            fz_buffer *buffer=fz_new_buffer_from_copied_data(ctx, (const unsigned char*)data.data(), data.size());
            fz_image *image=NULL;
            fz_try(ctx)
                image=fz_new_image_from_buffer(ctx, buffer);
            fz_always(ctx)
                fz_drop_buffer(ctx, buffer);
            fz_catch(ctx)
                fz_rethrow(ctx);
            float w=image->w*72.f/96.f, h=image->h*72.f/96.f;
            fz_device *dev=fz_begin_page(ctx, writer, fz_make_rect(0, 0, w, h));
            fz_fill_image(ctx, dev, image, fz_scale(w, h), 1, fz_default_color_params);
            fz_end_page(ctx, writer);
            fz_drop_image(ctx, image);
        }
        fz_close_document_writer(ctx, writer);
    }
    fz_always(ctx)
        fz_drop_document_writer(ctx, writer);
    fz_catch(ctx)
    {
        *error=std::string("Unable to write the PDF: ")+fz_caught_message(ctx);
        ok=false;
    }
    fz_drop_context(ctx);
    return ok;
}

bool DownloadChapter(LuaSource *source, const StoreEntry &chapter, const std::string &output_base, STORE_FORMAT format,
                     std::function<void(int, int)> progress, std::atomic<bool> *cancel, std::string *output_path, std::string *error)
{
    std::vector<StorePage> pages;
    if(!source->Pages(chapter, &pages, error))
        return false;
    if(cancel && *cancel)
    {
        *error="Canceled";
        return false;
    }
    if(pages.empty())
    {
        *error="No pages in this chapter";
        return false;
    }

    //Parallel download (the images are cached forever)
    std::vector<std::string> images(pages.size());
    std::atomic<size_t> next(0), done(0);
    std::mutex error_lock;
    std::string first_error;
    if(progress)
        progress(0, (int)pages.size());
    std::vector<std::thread> workers;
    for(int t=0; t<DOWNLOAD_THREADS; t++)
    {
        workers.push_back(std::thread([&]()
        {
            for(size_t i=next++; i<pages.size(); i=next++)
            {
                if(cancel && *cancel)
                    return;
                NetResponse r=Net::Get(pages[i].Url, pages[i].Headers, NET_CACHE_FOREVER, pages[i].CacheKey);
                if(!r.Ok() || ImageExtension(r.Body)[0]==0)
                {
                    std::lock_guard<std::mutex> lock(error_lock);
                    if(first_error.empty())
                        first_error="Page "+std::to_string(i+1)+": "+(r.Ok() ? std::string("not an image") : r.Error);
                    continue;
                }
                images[i].swap(r.Body);
                if(progress)
                    progress((int)++done, (int)pages.size());
            }
        }));
    }
    for(size_t t=0; t<workers.size(); t++)
        workers[t].join();
    if(cancel && *cancel)
    {
        *error="Canceled";
        return false;
    }
    if(!first_error.empty())
    {
        *error=first_error;
        return false;
    }

    //Export: written under a temporary name, renamed when complete
    FileUtf8::MakeDirs(output_base.substr(0, output_base.find_last_of("/\\")));
    *output_path=output_base+StoreFormatExtension(format);
    std::string tmp=*output_path+".part";
    bool ok;
    if(format==FORMAT_IMAGES)
    {
        FileUtf8::MakeDirs(tmp);
        ok=true;
        for(size_t i=0; i<images.size() && ok; i++)
            ok=WriteFile(tmp+"/"+PageName(i, images.size(), images[i]), images[i]);
        if(!ok)
            *error="Unable to write the images in "+tmp;
    }
    else if(format==FORMAT_PDF)
        ok=WritePdf(tmp, images, error);
    else
        ok=WriteArchive(tmp, images, error);

    //Download again: replace the former file or folder (rename does not overwrite on Windows)
    if(ok && Exists(*output_path))
        RemovePath(*output_path);
    if(ok && !FileUtf8::Rename(tmp, *output_path))
    {
        *error="Unable to rename "+tmp;
        ok=false;
    }
    if(!ok)
        RemovePath(tmp);
    return ok;
}
