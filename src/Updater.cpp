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
#include "Updater.h"
#include "Net.h"
#include "FileUtf8.h"
#include "Async.h"
#include "Theme.h"
#include "Preferences.h" //GetUserDataDir
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <map>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    #include <bcrypt.h>
#endif

#ifndef CLAUDEREADER_VERSION
    #define CLAUDEREADER_VERSION "0"
#endif
#define UPDATE_REPOSITORY "ahubb1/ClaudeReader"
#define UPDATE_LATEST_URL "https://api.github.com/repos/" UPDATE_REPOSITORY "/releases/latest"
#define UPDATE_PORTABLE_ASSET "ClaudeReader-windows-x64.exe"
#define UPDATE_INSTALLER_SUFFIX "-setup.exe"
#define UNINSTALLER_NAME "Uninstall ClaudeReader.exe"   //!< Written by the installer next to the exe
#define UPDATE_START_DELAY 3    //!< Seconds after the start: the window and the library first

namespace
{
    std::function<void()> Quit;
    std::atomic<bool> Checking(false);
    std::atomic<bool> AskedWhileChecking(false);   //!< Check now during the startup check: its result is shown

    //------------------------------------JSON (GitHub answers)------------------------------------
    struct Json
    {
        enum TYPE { NONE, STRING, NUMBER, BOOL, ARRAY, OBJECT } Type=NONE;
        std::string String;
        double Number=0;
        std::vector<Json> Items;
        std::map<std::string, Json> Fields;

        const Json &operator[](const std::string &key) const
        {
            static const Json none;
            std::map<std::string, Json>::const_iterator it=Fields.find(key);
            return it==Fields.end() ? none : it->second;
        }
    };

    struct JsonParser
    {
        const char *p, *end;
        int Depth=0;

        void Spaces() { while(p<end && isspace((unsigned char)*p)) p++; }

        bool ParseString(std::string *out)
        {
            if(p>=end || *p!='"')
                return false;
            p++;
            while(p<end && *p!='"')
            {
                if(*p!='\\')
                {
                    *out+=*p++;
                    continue;
                }
                if(++p>=end)
                    return false;
                char c=*p++;
                switch(c)
                {
                    case 'n': *out+='\n'; break;
                    case 't': *out+='\t'; break;
                    case 'r': *out+='\r'; break;
                    case 'b': *out+='\b'; break;
                    case 'f': *out+='\f'; break;
                    case 'u':
                    {
                        if(end-p<4)
                            return false;
                        unsigned code=(unsigned)strtoul(std::string(p, 4).c_str(), NULL, 16);
                        p+=4;
                        //Surrogate pair
                        if(code>=0xD800 && code<0xDC00 && end-p>=6 && p[0]=='\\' && p[1]=='u')
                        {
                            unsigned low=(unsigned)strtoul(std::string(p+2, 4).c_str(), NULL, 16);
                            if(low>=0xDC00 && low<0xE000)
                            {
                                code=0x10000+((code-0xD800)<<10)+(low-0xDC00);
                                p+=6;
                            }
                        }
                        if(code<0x80) *out+=(char)code;
                        else if(code<0x800) { *out+=(char)(0xC0|(code>>6)); *out+=(char)(0x80|(code&0x3F)); }
                        else if(code<0x10000) { *out+=(char)(0xE0|(code>>12)); *out+=(char)(0x80|((code>>6)&0x3F)); *out+=(char)(0x80|(code&0x3F)); }
                        else { *out+=(char)(0xF0|(code>>18)); *out+=(char)(0x80|((code>>12)&0x3F)); *out+=(char)(0x80|((code>>6)&0x3F)); *out+=(char)(0x80|(code&0x3F)); }
                        break;
                    }
                    default: *out+=c;   // " \ /
                }
            }
            if(p>=end)
                return false;
            p++;
            return true;
        }

        bool Parse(Json *value)
        {
            Spaces();
            if(p>=end || ++Depth>64)
                return false;
            bool ok=true;
            if(*p=='{')
            {
                value->Type=Json::OBJECT;
                p++;
                Spaces();
                if(p<end && *p=='}')
                    p++;
                else
                    while(ok)
                    {
                        std::string key;
                        Spaces();
                        if(!ParseString(&key))
                            return false;
                        Spaces();
                        if(p>=end || *p++!=':')
                            return false;
                        ok=Parse(&value->Fields[key]);
                        Spaces();
                        if(!ok || p>=end)
                            return false;
                        if(*p=='}') { p++; break; }
                        if(*p++!=',')
                            return false;
                    }
            }
            else if(*p=='[')
            {
                value->Type=Json::ARRAY;
                p++;
                Spaces();
                if(p<end && *p==']')
                    p++;
                else
                    while(ok)
                    {
                        value->Items.push_back(Json());
                        ok=Parse(&value->Items.back());
                        Spaces();
                        if(!ok || p>=end)
                            return false;
                        if(*p==']') { p++; break; }
                        if(*p++!=',')
                            return false;
                    }
            }
            else if(*p=='"')
            {
                value->Type=Json::STRING;
                ok=ParseString(&value->String);
            }
            else if(end-p>=4 && std::string(p, 4)=="true") { value->Type=Json::BOOL; value->Number=1; p+=4; }
            else if(end-p>=5 && std::string(p, 5)=="false") { value->Type=Json::BOOL; p+=5; }
            else if(end-p>=4 && std::string(p, 4)=="null") { p+=4; }
            else
            {
                char *after=NULL;
                std::string number(p, std::min<size_t>(end-p, 40));
                value->Number=strtod(number.c_str(), &after);
                if(after==number.c_str())
                    return false;
                value->Type=Json::NUMBER;
                p+=after-number.c_str();
            }
            Depth--;
            return ok;
        }
    };

    bool ParseJson(const std::string &text, Json *value)
    {
        JsonParser parser;
        parser.p=text.c_str();
        parser.end=text.c_str()+text.size();
        return parser.Parse(value);
    }

    //------------------------------------Preferences------------------------------------
    //! The global preferences file (the keys of the updates only: the other ones are kept)
    Fl_Preferences *OpenPrefs()
    {
        return new Fl_Preferences(GetUserDataDir(), "claudereader", "claudereader");
    }

    std::string SkippedVersion()
    {
        std::unique_ptr<Fl_Preferences> prefs(OpenPrefs());
        char version[64];
        prefs->get("SkipVersion", version, "", sizeof(version));
        return version;
    }

    void SkipVersion(const std::string &version)
    {
        std::unique_ptr<Fl_Preferences> prefs(OpenPrefs());
        prefs->set("SkipVersion", version.c_str());
    }

    //------------------------------------Files------------------------------------
#ifdef _WIN32
    std::wstring Wide(const std::string &text)
    {
        int n=MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
        std::wstring wide(n>0 ? n-1 : 0, L'\0');
        if(n>1)
            MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wide[0], n);
        return wide;
    }

    std::string Utf8(const std::wstring &wide)
    {
        int n=WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
        std::string text(n>0 ? n-1 : 0, '\0');
        if(n>1)
            WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &text[0], n, NULL, NULL);
        return text;
    }

    std::string ExePath()
    {
        wchar_t path[32768];
        DWORD n=GetModuleFileNameW(NULL, path, 32768);
        return n ? Utf8(std::wstring(path, n)) : std::string();
    }

    //! Folder of the downloaded updates (in the temporary folder)
    std::string DownloadDir()
    {
        wchar_t path[MAX_PATH+1];
        DWORD n=GetTempPathW(MAX_PATH+1, path);
        std::string dir=(n ? Utf8(std::wstring(path, n)) : std::string(GetUserDataDir())+"/")+"ClaudeReader-update";
        FileUtf8::MakeDirs(dir);
        return dir;
    }

    //! SHA-256 of a file, in hexadecimal (empty on error)
    std::string FileSha256(const std::string &path)
    {
        std::string hex;
        BCRYPT_ALG_HANDLE algorithm=NULL;
        BCRYPT_HASH_HANDLE hash=NULL;
        FILE *f=FileUtf8::Open(path, "rb");
        if(f && BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0)==0 &&
           BCryptCreateHash(algorithm, &hash, NULL, 0, NULL, 0, 0)==0)
        {
            std::vector<unsigned char> buffer(1<<20);
            size_t n;
            bool ok=true;
            while(ok && (n=fread(buffer.data(), 1, buffer.size(), f))>0)
                ok=BCryptHashData(hash, buffer.data(), (ULONG)n, 0)==0;
            unsigned char digest[32];
            if(ok && !ferror(f) && BCryptFinishHash(hash, digest, sizeof(digest), 0)==0)
            {
                static const char *digits="0123456789abcdef";
                for(int i=0; i<32; i++)
                {
                    hex+=digits[digest[i]>>4];
                    hex+=digits[digest[i]&15];
                }
            }
        }
        if(hash)
            BCryptDestroyHash(hash);
        if(algorithm)
            BCryptCloseAlgorithmProvider(algorithm, 0);
        if(f)
            fclose(f);
        return hex;
    }
#endif

    //! The CHANGES entries newer than the running version, up to the release ("ClaudeReader 1.2 (date)" titles)
    std::string NotesFromChanges(const std::string &changes, const std::string &latest)
    {
        std::string notes, line;
        bool keep=false;
        size_t start=0;
        while(start<changes.size())
        {
            size_t eol=changes.find('\n', start);
            if(eol==std::string::npos)
                eol=changes.size();
            line=changes.substr(start, eol-start);
            start=eol+1;
            if(!line.empty() && line.back()=='\r')
                line.pop_back();
            if(line.compare(0, 13, "ClaudeReader ")==0)
            {
                std::string version=line.substr(13, line.find(' ', 13)-13);
                keep=Updater::CompareVersions(version, Updater::CurrentVersion())>0 &&
                     Updater::CompareVersions(version, latest)<=0 && line.find("unreleased")==std::string::npos;
                if(keep)
                    notes+=(notes.empty() ? "" : "\n")+std::string("Version ")+version+"\n";
                continue;
            }
            if(!keep || line.empty())
                continue;
            //"\t-change" and "\t continued"
            size_t text=line.find_first_not_of('\t');
            if(text==std::string::npos)
                continue;
            if(line[text]=='-')
                notes+="  \xE2\x80\xA2 "+line.substr(text+1)+"\n";
            else
                notes+="     "+line.substr(line.find_first_not_of(' ', text))+"\n";
        }
        return notes;
    }

    //------------------------------------Dialogs------------------------------------
    Fl_Button *DialogButton(int x, int y, int w, const char *label, bool accent)
    {
        Fl_Button *b=new Fl_Button(x, y, w, 28, label);
        b->box(FL_FLAT_BOX);
        b->color(accent ? THEME_ACCENT : THEME_INPUT_BG, accent ? fl_rgb_color(2, 106, 192) : THEME_HOVER);
        b->labelcolor(accent ? FL_WHITE : THEME_FG);
        b->labelsize(13);
        b->clear_visible_focus();
        return b;
    }

    Fl_Box *DialogText(int x, int y, int w, int h, int size, Fl_Color color)
    {
        Fl_Box *b=new Fl_Box(x, y, w, h);
        b->align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_INSIDE|FL_ALIGN_WRAP|FL_ALIGN_CLIP);
        b->labelsize(size);
        b->labelcolor(color);
        return b;
    }

    void CenterOnMain(Fl_Window *dialog)
    {
        Fl_Window *main=Fl::first_window();
        while(main && main->window())
            main=main->window();
        if(main && main!=dialog)
            dialog->position(main->x()+(main->w()-dialog->w())/2, main->y()+(main->h()-dialog->h())/2);
    }

    std::string Megabytes(long long bytes)
    {
        char text[32];
        snprintf(text, sizeof(text), "%.1f MB", bytes/1048576.0);
        return text;
    }

    //! The update dialog and its download (the download thread keeps the state while it runs)
    struct UpdateState
    {
        Updater::Release Release;
        Fl_Double_Window *Window=NULL;
        Fl_Box *Status=NULL;
        Fl_Progress *Bar=NULL;
        Fl_Button *UpdateButton=NULL, *PageButton=NULL, *SkipButton=NULL, *LaterButton=NULL;
        bool Alive=true;
        bool Busy=false;
        std::atomic<bool> Cancel{false};
        std::string StatusText;
    };

    void SetStatus(UpdateState *s, const std::string &text, bool error)
    {
        s->StatusText=text;
        s->Status->label(s->StatusText.c_str());
        s->Status->labelcolor(error ? fl_rgb_color(241, 112, 112) : THEME_FG);
        s->Window->redraw();
    }

    void SetBusy(UpdateState *s, bool busy)
    {
        s->Busy=busy;
        if(busy)
        {
            s->UpdateButton->deactivate();
            s->SkipButton->deactivate();
            s->LaterButton->label("Cancel");
            s->Bar->show();
        }
        else
        {
            s->UpdateButton->activate();
            s->SkipButton->activate();
            s->LaterButton->label("Later");
            s->Bar->hide();
        }
        s->Window->redraw();
    }

    //! Start the downloaded update and close ClaudeReader. False (and an error) if it could not be started
    bool Apply(const std::string &file, std::string *error)
    {
#ifdef _WIN32
        if(Updater::IsInstalled())
        {
            //The installer: silent, waits for ClaudeReader to close, then starts it again
            HINSTANCE r=ShellExecuteW(NULL, L"open", Wide(file).c_str(), L"/S /RELAUNCH", NULL, SW_SHOWNORMAL);
            if((INT_PTR)r<=32)
            {
                *error="The installer could not be started (error "+std::to_string((long long)(INT_PTR)r)+")";
                return false;
            }
        }
        else
        {
            //Portable: Windows lets a running exe be renamed, the new one takes its name
            std::wstring exe=Wide(ExePath()), old=exe+L".old";
            DeleteFileW(old.c_str());
            if(!MoveFileExW(exe.c_str(), old.c_str(), MOVEFILE_REPLACE_EXISTING))
            {
                *error="ClaudeReader cannot replace itself in its folder (no permission?): download it from the release page";
                return false;
            }
            if(!MoveFileExW(Wide(file).c_str(), exe.c_str(), MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED))
            {
                MoveFileExW(old.c_str(), exe.c_str(), MOVEFILE_REPLACE_EXISTING);
                *error="The new version could not be put in place: download it from the release page";
                return false;
            }
            HINSTANCE r=ShellExecuteW(NULL, L"open", exe.c_str(), NULL, NULL, SW_SHOWNORMAL);
            if((INT_PTR)r<=32)
            {
                *error="The new version is installed but could not be started: start ClaudeReader again";
                return false;
            }
        }
        if(Quit)
            Quit();
        return true;
#else
        (void)file;
        *error="Not supported";
        return false;
#endif
    }

    void Download(std::shared_ptr<UpdateState> s)
    {
#ifdef _WIN32
        SetBusy(s.get(), true);
        s->Cancel=false;
        s->Bar->value(0);
        SetStatus(s.get(), "Downloading "+s->Release.AssetName+"...", false);
        std::string file=DownloadDir()+"/"+s->Release.AssetName;
        std::thread([s, file]()
        {
            std::chrono::steady_clock::time_point last;
            NetResponse r=Net::Download(s->Release.AssetUrl, file, [s, &last](long long done, long long total)
            {
                std::chrono::steady_clock::time_point now=std::chrono::steady_clock::now();
                if(now-last>std::chrono::milliseconds(100))
                {
                    last=now;
                    if(total<=0)
                        total=s->Release.AssetSize;
                    RunInUiThread([s, done, total]()
                    {
                        if(!s->Alive || !s->Busy)
                            return;
                        s->Bar->value(total>0 ? (float)(100.0*done/total) : 0);
                        SetStatus(s.get(), "Downloading "+s->Release.AssetName+": "+Megabytes(done)+
                                  (total>0 ? " of "+Megabytes(total) : std::string()), false);
                    });
                }
                return !s->Cancel;
            });
            //Check the file: its size, and its SHA-256 when GitHub gives it
            std::string error=r.Ok() ? "" : (r.Error=="Canceled" ? r.Error : "Download failed: "+r.Error);
            long long size=0;
            if(error.empty() && (!FileUtf8::Stat(file, &size) || (s->Release.AssetSize>0 && size!=s->Release.AssetSize)))
                error="The download is incomplete, try again";
            if(error.empty() && !s->Release.Sha256.empty() && FileSha256(file)!=s->Release.Sha256)
                error="The downloaded file is damaged (its checksum does not match), try again";
            if(!error.empty())
                FileUtf8::Remove(file);
            RunInUiThread([s, file, error]()
            {
                if(!s->Alive)
                    return;
                if(!error.empty())
                {
                    SetBusy(s.get(), false);
                    SetStatus(s.get(), error=="Canceled" ? "Download canceled." : error, error!="Canceled");
                    return;
                }
                SetStatus(s.get(), Updater::IsInstalled() ? "Installing... ClaudeReader will start again."
                                                          : "Updating... ClaudeReader will start again.", false);
                Fl::flush();
                std::string apply_error;
                if(!Apply(file, &apply_error))
                {
                    SetBusy(s.get(), false);
                    SetStatus(s.get(), apply_error, true);
                }
            });
        }).detach();
#endif
    }

    //! A newer version: what changed, and Update now / Release page / Skip this version / Later
    void ShowUpdate(const Updater::Release &release)
    {
        std::shared_ptr<UpdateState> s(new UpdateState());
        s->Release=release;
        bool can_update=!release.AssetUrl.empty();

        Fl_Double_Window win(620, 470, "Update available");
        s->Window=&win;
        win.color(THEME_EDITOR_BG);
        std::string title="ClaudeReader "+release.Version+" is available";
        Fl_Box *title_box=DialogText(20, 16, 580, 26, 16, THEME_FG_BRIGHT);
        title_box->label(title.c_str());
        std::string subtitle="You have version "+Updater::CurrentVersion()+". What's new:";
        Fl_Box *subtitle_box=DialogText(20, 46, 580, 22, 13, THEME_FG_DIM);
        subtitle_box->label(subtitle.c_str());

        Fl_Text_Buffer buffer;
        buffer.text(release.Notes.empty() ? "See the release page." : release.Notes.c_str());
        Fl_Text_Display notes(20, 74, 580, 270);
        notes.buffer(&buffer);
        notes.box(FL_FLAT_BOX);
        notes.color(THEME_BAR_BG);
        notes.textcolor(THEME_FG);
        notes.textsize(13);
        notes.wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
        notes.scrollbar_width(12);
        notes.scrollbar_align(FL_ALIGN_RIGHT);

        s->Status=DialogText(20, 354, 580, 36, 13, THEME_FG);
        s->Bar=new Fl_Progress(20, 392, 580, 8);
        s->Bar->box(FL_FLAT_BOX);
        s->Bar->color(THEME_INPUT_BG);
        s->Bar->selection_color(THEME_ACCENT);
        s->Bar->minimum(0);
        s->Bar->maximum(100);
        s->Bar->hide();
        if(!can_update)
            SetStatus(s.get(), "Download it from the release page.", false);
        else if(Updater::IsInstalled())
            SetStatus(s.get(), "Update now downloads the installer ("+Megabytes(release.AssetSize)+
                      "), installs it and starts ClaudeReader again.", false);
        else
            SetStatus(s.get(), "Update now downloads the new ClaudeReader ("+Megabytes(release.AssetSize)+
                      "), replaces this one and starts it again.", false);

        s->UpdateButton=DialogButton(20, 422, 120, can_update ? "Update now" : "Download", true);
        s->PageButton=DialogButton(148, 422, 120, "Release page", false);
        s->SkipButton=DialogButton(372, 422, 130, "Skip this version", false);
        s->LaterButton=DialogButton(510, 422, 90, "Later", false);
        s->UpdateButton->callback([](Fl_Widget *, void *p)
        {
            std::shared_ptr<UpdateState> &s=*(std::shared_ptr<UpdateState>*)p;
            if(s->Release.AssetUrl.empty())
            {
                fl_open_uri(s->Release.Page.c_str());
                s->Window->hide();
            }
            else
                Download(s);
        }, &s);
        s->PageButton->callback([](Fl_Widget *, void *p)
        {
            fl_open_uri(((UpdateState*)p)->Release.Page.c_str());
        }, s.get());
        s->SkipButton->callback([](Fl_Widget *, void *p)
        {
            UpdateState *s=(UpdateState*)p;
            SkipVersion(s->Release.Version);
            s->Window->hide();
        }, s.get());
        //Later, or Cancel while downloading
        s->LaterButton->callback([](Fl_Widget *, void *p)
        {
            UpdateState *s=(UpdateState*)p;
            if(s->Busy)
                s->Cancel=true;
            else
                s->Window->hide();
        }, s.get());
        win.callback([](Fl_Widget *, void *p)
        {
            UpdateState *s=(UpdateState*)p;
            s->Cancel=true;
            s->Window->hide();
        }, s.get());
        win.end();
        win.set_modal();
        CenterOnMain(&win);
        win.show();
        ApplyDarkTitleBar(&win);
        while(win.shown())
            Fl::wait();
        s->Cancel=true;
        s->Alive=false;
        notes.buffer(NULL);
    }

    //! Check in a thread, then show the update (or, when asked by the user, that there is none)
    void Check(bool asked, int delay)
    {
        if(Checking.exchange(true))
        {
            if(asked)
                AskedWhileChecking=true;
            return;
        }
        std::thread([asked, delay]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            std::shared_ptr<Updater::Release> release(new Updater::Release());
            std::string error;
            bool ok=Updater::Latest(release.get(), &error);
            RunInUiThread([asked, ok, release, error]()
            {
                Checking=false;
                bool shown=asked || AskedWhileChecking.exchange(false);
                bool newer=ok && Updater::CompareVersions(release->Version, Updater::CurrentVersion())>0;
                if(newer && (shown || release->Version!=SkippedVersion()))
                    ShowUpdate(*release);
                else if(shown)
                {
                    fl_message_title("Check for updates");
                    if(!ok)
                        (CenterNextMessage('!'), fl_alert)("Could not check for updates.\n%s", error.c_str());
                    else
                        (CenterNextMessage('i'), fl_message)("ClaudeReader %s is the latest version.", Updater::CurrentVersion().c_str());
                }
            });
        }).detach();
    }
}

std::string Updater::CurrentVersion()
{
    return CLAUDEREADER_VERSION;
}

int Updater::CompareVersions(const std::string &a, const std::string &b)
{
    auto split=[](std::string v, std::vector<std::string> *main, std::string *pre)
    {
        if(!v.empty() && (v[0]=='v' || v[0]=='V'))
            v.erase(0, 1);
        size_t dash=v.find('-');
        *pre=dash==std::string::npos ? "" : v.substr(dash+1);
        v=v.substr(0, dash);
        size_t start=0;
        while(start<=v.size())
        {
            size_t dot=v.find('.', start);
            if(dot==std::string::npos)
                dot=v.size();
            main->push_back(v.substr(start, dot-start));
            start=dot+1;
        }
    };
    //Numbers compared as numbers, anything else as text
    auto compare=[](const std::string &x, const std::string &y)
    {
        bool xn=!x.empty() && x.find_first_not_of("0123456789")==std::string::npos;
        bool yn=!y.empty() && y.find_first_not_of("0123456789")==std::string::npos;
        if(xn && yn)
        {
            long long dx=atoll(x.c_str()), dy=atoll(y.c_str());
            return dx<dy ? -1 : dx>dy ? 1 : 0;
        }
        return x<y ? -1 : x>y ? 1 : 0;
    };
    std::vector<std::string> ma, mb;
    std::string pa, pb;
    split(a, &ma, &pa);
    split(b, &mb, &pb);
    for(size_t i=0; i<std::max(ma.size(), mb.size()); i++)
    {
        int c=compare(i<ma.size() ? ma[i] : "0", i<mb.size() ? mb[i] : "0");
        if(c)
            return c;
    }
    //A pre-release comes before its release: 1.1-beta.2 < 1.1
    if(pa.empty() || pb.empty())
        return pa.empty() ? (pb.empty() ? 0 : 1) : -1;
    std::vector<std::string> qa, qb;
    std::string unused;
    split(pa, &qa, &unused);
    split(pb, &qb, &unused);
    for(size_t i=0; i<std::max(qa.size(), qb.size()); i++)
    {
        if(i>=qa.size() || i>=qb.size())
            return i>=qa.size() ? -1 : 1;
        int c=compare(qa[i], qb[i]);
        if(c)
            return c;
    }
    return 0;
}

bool Updater::IsInstalled()
{
#ifdef _WIN32
    std::string exe=ExePath();
    size_t slash=exe.find_last_of("\\/");
    return slash!=std::string::npos && FileUtf8::Exists(exe.substr(0, slash+1)+UNINSTALLER_NAME);
#else
    return false;
#endif
}

bool Updater::Latest(Release *release, std::string *error)
{
    const char *custom=getenv("CLAUDEREADER_UPDATE_URL");
    std::string url=custom && *custom ? custom : UPDATE_LATEST_URL;
    NetHeaders headers;
    headers.push_back(std::make_pair("Accept", "application/vnd.github+json"));
    NetResponse r=Net::Get(url, headers);
    if(r.Status==404)
    {
        *error="No release published yet";
        return false;
    }
    if(!r.Ok())
    {
        *error="GitHub: "+r.Error;
        return false;
    }
    Json json;
    if(!ParseJson(r.Body, &json) || json["tag_name"].String.empty())
    {
        *error="Unexpected answer from GitHub";
        return false;
    }
    release->Tag=json["tag_name"].String;
    release->Version=release->Tag[0]=='v' || release->Tag[0]=='V' ? release->Tag.substr(1) : release->Tag;
    release->Page=json["html_url"].String;
    if(release->Page.empty())
        release->Page="https://github.com/" UPDATE_REPOSITORY "/releases/latest";

    //The file for this computer
#ifdef _WIN32
    bool installed=IsInstalled();
    const Json &assets=json["assets"];
    for(size_t i=0; i<assets.Items.size(); i++)
    {
        const std::string &name=assets.Items[i]["name"].String;
        bool wanted=installed ? (name.size()>strlen(UPDATE_INSTALLER_SUFFIX) &&
                                 name.compare(name.size()-strlen(UPDATE_INSTALLER_SUFFIX), std::string::npos, UPDATE_INSTALLER_SUFFIX)==0)
                              : name==UPDATE_PORTABLE_ASSET;
        if(!wanted)
            continue;
        release->AssetName=name;
        release->AssetUrl=assets.Items[i]["browser_download_url"].String;
        release->AssetSize=(long long)assets.Items[i]["size"].Number;
        const std::string &digest=assets.Items[i]["digest"].String;
        if(digest.compare(0, 7, "sha256:")==0)
            release->Sha256=digest.substr(7);
        break;
    }
#endif

    //What changed: the CHANGES of the release (every version newer than this one), or else the release notes
    if(!custom || !*custom)
    {
        NetResponse changes=Net::Get("https://raw.githubusercontent.com/" UPDATE_REPOSITORY "/"+release->Tag+"/CHANGES");
        if(changes.Ok())
            release->Notes=NotesFromChanges(changes.Body, release->Version);
    }
    if(release->Notes.empty())
        release->Notes=json["body"].String;
    return true;
}

bool Updater::CheckAtStartup()
{
    std::unique_ptr<Fl_Preferences> prefs(OpenPrefs());
    int check=1;
    prefs->get("CheckUpdates", check, 1);
    return check!=0;
}

void Updater::SetCheckAtStartup(bool check)
{
    std::unique_ptr<Fl_Preferences> prefs(OpenPrefs());
    prefs->set("CheckUpdates", check ? 1 : 0);
}

void Updater::Start(std::function<void()> quit)
{
    Quit=quit;
#ifdef _WIN32
    //What the last update left: the replaced exe (portable), the downloaded files
    std::string exe=ExePath();
    if(!exe.empty())
        DeleteFileW(Wide(exe+".old").c_str());
    std::string dir=DownloadDir();
    std::vector<std::string> files=FileUtf8::List(dir);
    for(size_t i=0; i<files.size(); i++)
        FileUtf8::Remove(dir+"/"+files[i]);
#endif
    if(CheckAtStartup())
        Check(false, UPDATE_START_DELAY);
}

void Updater::CheckNow()
{
    Check(true, 0);
}
