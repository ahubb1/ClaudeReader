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
#include "LuaSource.h"
#include "HtmlQuery.h"
#include "Preferences.h" //GetUserDataDir
#include "FileUtf8.h"
#include <FL/filename.H>
#include <FL/fl_utf8.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <algorithm>

extern "C"
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

//Built-in sources scripts (StoreSources.cpp)
extern const char *BuiltinSources[];

#define HTML_NODE_META "ClaudeReader.HtmlNode"
#define HTTP_DEFAULT_CACHE 600   //Seconds: web pages and API answers of the scripts


//------------------------------------http------------------------

static NetHeaders HeadersFromTable(lua_State *L, int idx)
{
    NetHeaders headers;
    if(lua_istable(L, idx))
    {
        lua_pushnil(L);
        while(lua_next(L, idx))
        {
            if(lua_type(L, -2)==LUA_TSTRING && (lua_type(L, -1)==LUA_TSTRING || lua_type(L, -1)==LUA_TNUMBER))
                headers.push_back(std::make_pair(lua_tostring(L, -2), lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }
    return headers;
}

static int PushResponse(lua_State *L, const NetResponse &r)
{
    if(!r.Ok() && r.Status==0)
    {
        lua_pushnil(L);
        lua_pushstring(L, r.Error.c_str());
        return 2;
    }
    lua_pushlstring(L, r.Body.data(), r.Body.size());
    lua_pushinteger(L, r.Status);
    return 2;
}

//! http.get(url[, headers[, cache_seconds]]) -> body, status | nil, error
static int l_http_get(lua_State *L)
{
    std::string url=luaL_checkstring(L, 1);
    NetHeaders headers=HeadersFromTable(L, 2);
    int cache=(int)luaL_optinteger(L, 3, HTTP_DEFAULT_CACHE);
    return PushResponse(L, Net::Get(url, headers, cache));
}

//! http.post(url, body[, headers]) -> body, status | nil, error
static int l_http_post(lua_State *L)
{
    std::string url=luaL_checkstring(L, 1);
    size_t len;
    const char *body=luaL_checklstring(L, 2, &len);
    return PushResponse(L, Net::Post(url, std::string(body, len), HeadersFromTable(L, 3)));
}


//------------------------------------json------------------------

namespace
{
    struct JsonReader
    {
        lua_State *L;
        const char *p, *end;
        std::string error;

        void Skip() { while(p<end && (*p==' ' || *p=='\t' || *p=='\n' || *p=='\r')) p++; }
        bool Fail(const char *msg) { if(error.empty()) error=msg; return false; }

        static void AppendUtf8(std::string *s, unsigned int c)
        {
            if(c<0x80) *s+=(char)c;
            else if(c<0x800) { *s+=(char)(0xC0|(c>>6)); *s+=(char)(0x80|(c&0x3F)); }
            else if(c<0x10000) { *s+=(char)(0xE0|(c>>12)); *s+=(char)(0x80|((c>>6)&0x3F)); *s+=(char)(0x80|(c&0x3F)); }
            else { *s+=(char)(0xF0|(c>>18)); *s+=(char)(0x80|((c>>12)&0x3F)); *s+=(char)(0x80|((c>>6)&0x3F)); *s+=(char)(0x80|(c&0x3F)); }
        }

        bool Hex4(unsigned int *v)
        {
            if(end-p<4) return false;
            *v=0;
            for(int i=0; i<4; i++)
            {
                char c=*p++;
                *v<<=4;
                if(c>='0' && c<='9') *v|=c-'0';
                else if(c>='a' && c<='f') *v|=c-'a'+10;
                else if(c>='A' && c<='F') *v|=c-'A'+10;
                else return false;
            }
            return true;
        }

        bool String()
        {
            p++; //Opening quote
            std::string s;
            while(p<end && *p!='"')
            {
                if(*p=='\\')
                {
                    if(++p>=end) return Fail("Unfinished string");
                    char c=*p++;
                    switch(c)
                    {
                        case 'n': s+='\n'; break;
                        case 't': s+='\t'; break;
                        case 'r': s+='\r'; break;
                        case 'b': s+='\b'; break;
                        case 'f': s+='\f'; break;
                        case 'u':
                        {
                            unsigned int code;
                            if(!Hex4(&code)) return Fail("Invalid \\u escape");
                            //Surrogate pair
                            if(code>=0xD800 && code<0xDC00 && end-p>=6 && p[0]=='\\' && p[1]=='u')
                            {
                                p+=2;
                                unsigned int low;
                                if(!Hex4(&low)) return Fail("Invalid \\u escape");
                                code=0x10000+((code-0xD800)<<10)+(low-0xDC00);
                            }
                            AppendUtf8(&s, code);
                            break;
                        }
                        default: s+=c; break; //  \" \\ \/
                    }
                }
                else
                    s+=*p++;
            }
            if(p>=end) return Fail("Unfinished string");
            p++;
            lua_pushlstring(L, s.data(), s.size());
            return true;
        }

        bool Value(int depth)
        {
            if(depth>200) return Fail("Too deep");
            luaL_checkstack(L, 4, "json");
            Skip();
            if(p>=end) return Fail("Unexpected end");
            char c=*p;
            if(c=='{')
            {
                p++;
                lua_newtable(L);
                Skip();
                if(p<end && *p=='}') { p++; return true; }
                while(true)
                {
                    Skip();
                    if(p>=end || *p!='"') return Fail("Object key expected");
                    if(!String()) return false;
                    Skip();
                    if(p>=end || *p!=':') return Fail("':' expected");
                    p++;
                    if(!Value(depth+1)) return false;
                    lua_rawset(L, -3);
                    Skip();
                    if(p<end && *p==',') { p++; continue; }
                    if(p<end && *p=='}') { p++; return true; }
                    return Fail("',' or '}' expected");
                }
            }
            if(c=='[')
            {
                p++;
                lua_newtable(L);
                Skip();
                if(p<end && *p==']') { p++; return true; }
                for(lua_Integer i=1; ; i++)
                {
                    if(!Value(depth+1)) return false;
                    lua_rawseti(L, -2, i);
                    Skip();
                    if(p<end && *p==',') { p++; continue; }
                    if(p<end && *p==']') { p++; return true; }
                    return Fail("',' or ']' expected");
                }
            }
            if(c=='"')
                return String();
            if(end-p>=4 && !strncmp(p, "true", 4)) { p+=4; lua_pushboolean(L, 1); return true; }
            if(end-p>=5 && !strncmp(p, "false", 5)) { p+=5; lua_pushboolean(L, 0); return true; }
            if(end-p>=4 && !strncmp(p, "null", 4)) { p+=4; lua_pushlightuserdata(L, NULL); return true; }
            //Number
            const char *s=p;
            while(p<end && strchr("+-0123456789.eE", *p)) p++;
            if(p==s) return Fail("Unexpected character");
            std::string num(s, p-s);
            if(num.find_first_of(".eE")==std::string::npos)
                lua_pushinteger(L, strtoll(num.c_str(), NULL, 10));
            else
                lua_pushnumber(L, strtod(num.c_str(), NULL));
            return true;
        }
    };

    void JsonEncode(lua_State *L, int idx, std::string *out, int depth)
    {
        idx=lua_absindex(L, idx);
        if(depth>100)
            luaL_error(L, "json.encode: too deep");
        switch(lua_type(L, idx))
        {
            case LUA_TNIL: *out+="null"; break;
            case LUA_TLIGHTUSERDATA: *out+="null"; break; //json.null
            case LUA_TBOOLEAN: *out+=lua_toboolean(L, idx) ? "true" : "false"; break;
            case LUA_TNUMBER:
            {
                if(lua_isinteger(L, idx))
                    *out+=std::to_string((long long)lua_tointeger(L, idx));
                else
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%.17g", lua_tonumber(L, idx));
                    *out+=buf;
                }
                break;
            }
            case LUA_TSTRING:
            {
                size_t len;
                const char *s=lua_tolstring(L, idx, &len);
                *out+='"';
                for(size_t i=0; i<len; i++)
                {
                    unsigned char c=(unsigned char)s[i];
                    switch(c)
                    {
                        case '"': *out+="\\\""; break;
                        case '\\': *out+="\\\\"; break;
                        case '\n': *out+="\\n"; break;
                        case '\r': *out+="\\r"; break;
                        case '\t': *out+="\\t"; break;
                        default:
                            if(c<0x20) { char buf[8]; snprintf(buf, sizeof(buf), "\\u%04x", c); *out+=buf; }
                            else *out+=(char)c;
                    }
                }
                *out+='"';
                break;
            }
            case LUA_TTABLE:
            {
                lua_Integer n=(lua_Integer)lua_rawlen(L, idx);
                if(n>0)
                {
                    *out+='[';
                    for(lua_Integer i=1; i<=n; i++)
                    {
                        if(i>1) *out+=',';
                        lua_rawgeti(L, idx, i);
                        JsonEncode(L, -1, out, depth+1);
                        lua_pop(L, 1);
                    }
                    *out+=']';
                }
                else
                {
                    *out+='{';
                    bool first=true;
                    lua_pushnil(L);
                    while(lua_next(L, idx))
                    {
                        if(lua_type(L, -2)==LUA_TSTRING)
                        {
                            if(!first) *out+=',';
                            first=false;
                            lua_pushvalue(L, -2);
                            JsonEncode(L, -1, out, depth+1);
                            lua_pop(L, 1);
                            *out+=':';
                            JsonEncode(L, -1, out, depth+1);
                        }
                        lua_pop(L, 1);
                    }
                    *out+='}';
                }
                break;
            }
            default:
                luaL_error(L, "json.encode: unsupported type %s", luaL_typename(L, idx));
        }
    }
}

//! json.decode(text) -> value | nil, error
static int l_json_decode(lua_State *L)
{
    size_t len;
    const char *text=luaL_checklstring(L, 1, &len);
    JsonReader reader;
    reader.L=L;
    reader.p=text;
    reader.end=text+len;
    int top=lua_gettop(L);
    if(!reader.Value(0))
    {
        lua_settop(L, top);
        lua_pushnil(L);
        lua_pushfstring(L, "json.decode: %s at character %d", reader.error.c_str(), (int)(reader.p-text)+1);
        return 2;
    }
    return 1;
}

//! json.encode(value) -> text
static int l_json_encode(lua_State *L)
{
    std::string out;
    luaL_checkany(L, 1);
    JsonEncode(L, 1, &out, 0);
    lua_pushlstring(L, out.data(), out.size());
    return 1;
}


//------------------------------------html------------------------

//! Parsed document, owned by the Lua document userdata
struct HtmlDocument
{
    std::string Source;     //gumbo keeps pointers to it
    GumboOutput *Output;
};

//! Node userdata (the document userdata is kept alive as its user value)
struct HtmlNodeRef
{
    const GumboNode *Node;
};

static int l_htmldoc_gc(lua_State *L)
{
    HtmlDocument *doc=(HtmlDocument*)lua_touserdata(L, 1);
    if(doc && doc->Output)
    {
        gumbo_destroy_output(&kGumboDefaultOptions, doc->Output);
        doc->Output=NULL;
        doc->Source.~basic_string();
    }
    return 0;
}

//! Push a node userdata keeping the document (at stack index doc_idx) alive
static void PushNode(lua_State *L, const GumboNode *node, int doc_idx)
{
    doc_idx=lua_absindex(L, doc_idx);
    HtmlNodeRef *ref=(HtmlNodeRef*)lua_newuserdatauv(L, sizeof(HtmlNodeRef), 1);
    ref->Node=node;
    lua_pushvalue(L, doc_idx);
    lua_setiuservalue(L, -2, 1);
    luaL_setmetatable(L, HTML_NODE_META);
}

//! html.parse(text) -> document node
static int l_html_parse(lua_State *L)
{
    size_t len;
    const char *text=luaL_checklstring(L, 1, &len);
    HtmlDocument *doc=(HtmlDocument*)lua_newuserdatauv(L, sizeof(HtmlDocument), 0);
    new (&doc->Source) std::string(text, len);
    doc->Output=gumbo_parse_with_options(&kGumboDefaultOptions, doc->Source.data(), doc->Source.size());
    lua_newtable(L);
    lua_pushcfunction(L, l_htmldoc_gc);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    PushNode(L, doc->Output->document, -1);
    return 1;
}

static const GumboNode *CheckNode(lua_State *L, int idx)
{
    return ((HtmlNodeRef*)luaL_checkudata(L, idx, HTML_NODE_META))->Node;
}

//! node:select(css) -> { nodes }
static int l_node_select(lua_State *L)
{
    const GumboNode *node=CheckNode(L, 1);
    std::vector<const GumboNode*> found;
    std::string error;
    if(!HtmlSelect(node, luaL_checkstring(L, 2), &found, &error))
        return luaL_error(L, "%s", error.c_str());
    lua_getiuservalue(L, 1, 1); //Document
    lua_createtable(L, (int)found.size(), 0);
    for(size_t i=0; i<found.size(); i++)
    {
        PushNode(L, found[i], -2);
        lua_rawseti(L, -2, (lua_Integer)i+1);
    }
    return 1;
}

//! node:first(css) -> node | nil
static int l_node_first(lua_State *L)
{
    const GumboNode *node=CheckNode(L, 1);
    std::vector<const GumboNode*> found;
    std::string error;
    if(!HtmlSelect(node, luaL_checkstring(L, 2), &found, &error))
        return luaL_error(L, "%s", error.c_str());
    if(found.empty())
    {
        lua_pushnil(L);
        return 1;
    }
    lua_getiuservalue(L, 1, 1);
    PushNode(L, found[0], -1);
    return 1;
}

static int l_node_text(lua_State *L)
{
    lua_pushstring(L, HtmlText(CheckNode(L, 1)).c_str());
    return 1;
}

//! node:attr(name) -> value | nil
static int l_node_attr(lua_State *L)
{
    const GumboNode *node=CheckNode(L, 1);
    const char *name=luaL_checkstring(L, 2);
    if(node->type!=GUMBO_NODE_ELEMENT || !gumbo_get_attribute(&node->v.element.attributes, name))
        lua_pushnil(L);
    else
        lua_pushstring(L, HtmlAttr(node, name).c_str());
    return 1;
}

static int l_node_tag(lua_State *L)
{
    lua_pushstring(L, HtmlTag(CheckNode(L, 1)).c_str());
    return 1;
}


//------------------------------------util------------------------

static int l_util_urlencode(lua_State *L)
{
    lua_pushstring(L, Net::UrlEncode(luaL_checkstring(L, 1)).c_str());
    return 1;
}

static int l_util_trim(lua_State *L)
{
    std::string s=luaL_checkstring(L, 1);
    size_t a=s.find_first_not_of(" \t\r\n");
    size_t b=s.find_last_not_of(" \t\r\n");
    lua_pushstring(L, a==std::string::npos ? "" : s.substr(a, b-a+1).c_str());
    return 1;
}

static int l_util_log(lua_State *L)
{
    int n=lua_gettop(L);
    lua_getglobal(L, "SOURCE");
    lua_getfield(L, -1, "id");
    fprintf(stderr, "[%s]", lua_isstring(L, -1) ? lua_tostring(L, -1) : "source");
    lua_pop(L, 2);
    for(int i=1; i<=n; i++)
        fprintf(stderr, " %s", luaL_tolstring(L, i, NULL));
    fprintf(stderr, "\n");
    return 0;
}


//------------------------------------Source------------------------

static void RegisterModule(lua_State *L, const char *name, const luaL_Reg *functions)
{
    luaL_newlib(L, functions);
    lua_setglobal(L, name);
    (void)functions;
}

#define LUA_MEMORY_LIMIT    ((size_t)256<<20)   //!< Per store
#define LUA_LOAD_BUDGET     200000000LL         //!< Instructions to load a script (a few tenths of a second)
#define LUA_CALL_BUDGET     1000000000LL        //!< Instructions for a search, chapters or pages call (a few seconds)
#define LUA_HOOK_STEP       100000

//! Allocator of the stores: refuses to go over LUA_MEMORY_LIMIT (the script gets a "not enough memory" error)
static void *LimitedAlloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    LuaLimits *limits=(LuaLimits*)ud;
    if(!ptr)
        osize=0;    //Then osize is the type of the new object
    if(nsize==0)
    {
        free(ptr);
        limits->Memory-=osize;
        return NULL;
    }
    if(nsize>osize && limits->Memory+(nsize-osize)>LUA_MEMORY_LIMIT)
        return NULL;
    void *block=realloc(ptr, nsize);
    if(block)
        limits->Memory=limits->Memory-osize+nsize;
    return block;
}

//! Called every LUA_HOOK_STEP instructions: stops a script that runs for too long
static void BudgetHook(lua_State *L, lua_Debug *)
{
    void *ud;
    lua_getallocf(L, &ud);
    LuaLimits *limits=(LuaLimits*)ud;
    limits->Budget-=LUA_HOOK_STEP;
    if(limits->Budget<0)
        luaL_error(L, "the script runs for too long (endless loop?)");
}

//! A Lua state with the limits of the stores
static lua_State *NewLimitedState(LuaLimits *limits)
{
#if LUA_VERSION_NUM>=505
    lua_State *L=lua_newstate(LimitedAlloc, limits, luaL_makeseed(NULL));
#else
    lua_State *L=lua_newstate(LimitedAlloc, limits);
#endif
    if(L)
        lua_sethook(L, BudgetHook, LUA_MASKCOUNT, LUA_HOOK_STEP);
    return L;
}

LuaSource::LuaSource(const std::string &script, const std::string &origin):
    Origin(origin),
    L(NULL),
    Valid(false)
{
    L=NewLimitedState(&Limits);
    if(!L)
    {
        Error="Not enough memory for Lua";
        return;
    }

    //Safe standard libraries only (no io, os, package)
    luaL_requiref(L, LUA_GNAME, luaopen_base, 1);
    luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, 1);
    luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, 1);
    luaL_requiref(L, LUA_MATHLIBNAME, luaopen_math, 1);
    luaL_requiref(L, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    lua_settop(L, 0);
    lua_pushnil(L); lua_setglobal(L, "dofile");
    lua_pushnil(L); lua_setglobal(L, "loadfile");

    static const luaL_Reg http_functions[]={{"get", l_http_get}, {"post", l_http_post}, {NULL, NULL}};
    RegisterModule(L, "http", http_functions);
    static const luaL_Reg json_functions[]={{"decode", l_json_decode}, {"encode", l_json_encode}, {NULL, NULL}};
    RegisterModule(L, "json", json_functions);
    lua_getglobal(L, "json");
    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "null");
    lua_pop(L, 1);
    static const luaL_Reg html_functions[]={{"parse", l_html_parse}, {NULL, NULL}};
    RegisterModule(L, "html", html_functions);
    static const luaL_Reg util_functions[]={{"urlencode", l_util_urlencode}, {"trim", l_util_trim}, {"log", l_util_log}, {NULL, NULL}};
    RegisterModule(L, "util", util_functions);

    //Html nodes methods
    luaL_newmetatable(L, HTML_NODE_META);
    static const luaL_Reg node_methods[]={{"select", l_node_select}, {"first", l_node_first}, {"text", l_node_text},
                                          {"attr", l_node_attr}, {"tag", l_node_tag}, {NULL, NULL}};
    luaL_newlib(L, node_methods);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    //Run the script
    Limits.Budget=LUA_LOAD_BUDGET;
    if(luaL_loadbuffer(L, script.data(), script.size(), origin.c_str())!=LUA_OK || lua_pcall(L, 0, 0, 0)!=LUA_OK)
    {
        Error=lua_tostring(L, -1) ? lua_tostring(L, -1) : "Unknown error";
        lua_pop(L, 1);
        return;
    }

    //Description
    lua_getglobal(L, "SOURCE");
    if(!lua_istable(L, -1))
    {
        Error="SOURCE table missing";
        lua_pop(L, 1);
        return;
    }
    const char *fields[]={"id", "name", "kind", "site", "version"};
    std::string *values[]={&Id, &Name, &Kind, &Site, &Version};
    for(int i=0; i<5; i++)
    {
        lua_getfield(L, -1, fields[i]);
        if(lua_isstring(L, -1))
            *values[i]=lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    lua_getfield(L, -1, "headers");
    Headers=HeadersFromTable(L, lua_gettop(L));
    lua_pop(L, 2);
    if(Kind.empty())
        Kind="manga";
    if(Id.empty() || Name.empty())
    {
        Error="SOURCE.id and SOURCE.name are required";
        return;
    }
    const char *functions[]={"search", "chapters", "pages"};
    for(int i=0; i<(IsTorrent() ? 1 : 3); i++)
    {
        lua_getglobal(L, functions[i]);
        bool ok=lua_isfunction(L, -1);
        lua_pop(L, 1);
        if(!ok)
        {
            Error=std::string("Function ")+functions[i]+"() missing";
            return;
        }
    }
    Valid=true;
}

LuaSource::~LuaSource()
{
    if(L)
        lua_close(L);
}

//! Error handler: message with the traceback
static int Traceback(lua_State *L)
{
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}

//! Push an entry as a table (the fields are text)
void LuaSource::PushEntry(const StoreEntry &entry)
{
    lua_createtable(L, 0, (int)entry.Fields.size());
    for(std::map<std::string, std::string>::const_iterator it=entry.Fields.begin(); it!=entry.Fields.end(); ++it)
    {
        lua_pushlstring(L, it->second.data(), it->second.size());
        lua_setfield(L, -2, it->first.c_str());
    }
}

//! Call a function (pushed before its nargs arguments) returning a list of entries
bool LuaSource::CallEntries(const char *function, int nargs, std::vector<StoreEntry> *entries, std::string *error)
{
    int base=lua_gettop(L)-nargs;
    lua_pushcfunction(L, Traceback);
    lua_insert(L, base);
    Limits.Budget=LUA_CALL_BUDGET;
    int ret=lua_pcall(L, nargs, 1, base);
    lua_remove(L, base);
    if(ret!=LUA_OK)
    {
        *error=Name+": "+(lua_tostring(L, -1) ? lua_tostring(L, -1) : "error");
        lua_pop(L, 1);
        return false;
    }
    if(!lua_istable(L, -1))
    {
        *error=Name+": "+function+"() must return a table";
        lua_pop(L, 1);
        return false;
    }
    entries->clear();
    lua_Integer n=(lua_Integer)lua_rawlen(L, -1);
    for(lua_Integer i=1; i<=n; i++)
    {
        lua_rawgeti(L, -1, i);
        StoreEntry entry;
        if(lua_istable(L, -1))
        {
            lua_pushnil(L);
            while(lua_next(L, -2))
            {
                if(lua_type(L, -2)==LUA_TSTRING && (lua_type(L, -1)==LUA_TSTRING || lua_type(L, -1)==LUA_TNUMBER))
                {
                    lua_pushvalue(L, -1); //tostring on a copy (lua_next needs the key unchanged)
                    entry.Fields[lua_tostring(L, -3)]=lua_tostring(L, -1);
                    lua_pop(L, 1);
                }
                else if(lua_type(L, -2)==LUA_TSTRING && lua_type(L, -1)==LUA_TBOOLEAN)
                    entry.Fields[lua_tostring(L, -2)]=lua_toboolean(L, -1) ? "true" : "false";
                else if(lua_type(L, -2)==LUA_TSTRING && !strcmp(lua_tostring(L, -2), "headers") && lua_istable(L, -1))
                {
                    //Page headers: flattened as header_Name fields
                    NetHeaders headers=HeadersFromTable(L, lua_gettop(L));
                    for(size_t h=0; h<headers.size(); h++)
                        entry.Fields["header_"+headers[h].first]=headers[h].second;
                }
                lua_pop(L, 1);
            }
        }
        else if(lua_isstring(L, -1))
            entry.Fields["url"]=lua_tostring(L, -1); //pages() can return plain URLs
        lua_pop(L, 1);
        entries->push_back(entry);
    }
    lua_pop(L, 1);
    return true;
}

bool LuaSource::Search(const std::string &query, int page, std::vector<StoreEntry> *results, std::string *error)
{
    std::lock_guard<std::mutex> lock(Lock);
    lua_getglobal(L, "search");
    lua_pushstring(L, query.c_str());
    lua_pushinteger(L, page);
    return CallEntries("search", 2, results, error);
}

bool LuaSource::Chapters(const StoreEntry &manga, std::vector<StoreEntry> *chapters, std::string *error)
{
    std::lock_guard<std::mutex> lock(Lock);
    lua_getglobal(L, "chapters");
    PushEntry(manga);
    return CallEntries("chapters", 1, chapters, error);
}

bool LuaSource::Pages(const StoreEntry &chapter, std::vector<StorePage> *pages, std::string *error)
{
    std::vector<StoreEntry> entries;
    {
        std::lock_guard<std::mutex> lock(Lock);
        lua_getglobal(L, "pages");
        PushEntry(chapter);
        if(!CallEntries("pages", 1, &entries, error))
            return false;
    }
    pages->clear();
    for(size_t i=0; i<entries.size(); i++)
    {
        StorePage page;
        page.Url=entries[i].Get("url");
        page.CacheKey=entries[i].Get("key");
        page.Headers=Headers;
        //Per page headers: "header_Name" fields (a headers table is flattened by the script helper)
        for(std::map<std::string, std::string>::const_iterator it=entries[i].Fields.begin(); it!=entries[i].Fields.end(); ++it)
            if(it->first.compare(0, 7, "header_")==0)
                page.Headers.push_back(std::make_pair(it->first.substr(7), it->second));
        if(!page.Url.empty())
            pages->push_back(page);
    }
    return true;
}


//------------------------------------Sources list------------------------

//! How to write a source, copied in the user sources folder
static const char *SourcesReadme=
"ClaudeReader stores\n"
"===================\n"
"\n"
"Each .lua file of this folder is a store (a tab of the Store view). Settings > Stores installs\n"
"stores from the catalog, from a file or from a link, and removes them. A file with the same\n"
"SOURCE.id as a built-in store (mangadex, mangapill, nyaa) replaces it: copy and fix a store\n"
"when its web site changes, without waiting for a new ClaudeReader. Click the refresh button\n"
"of the Store view after editing a file. Test from a command prompt:\n"
"    ClaudeReader --store list\n"
"    ClaudeReader --store <id> search \"query\"\n"
"    ClaudeReader --store <id> chapters <manga id>\n"
"    ClaudeReader --store <id> pages <chapter id>\n"
"\n"
"A store defines:\n"
"    SOURCE = { id=\"...\", name=\"...\", kind=\"manga\" (or \"torrent\"), site=\"https://...\",\n"
"               headers={ Referer=\"https://...\" } }   -- headers: sent with the images\n"
"    function search(query, page)  -- query \"\": popular titles; page: 1, 2...\n"
"        return { {id=, title=, cover=, description=, url=, status=, year=}, ... }\n"
"        -- torrent stores: {id=, title=, magnet=, size=, seeders=, leechers=, date=}\n"
"    function chapters(manga)      -- manga: a table returned by search()\n"
"        return { {id=, title=, number=, volume=, group=, date=}, ... }  -- oldest first\n"
"    function pages(chapter)       -- chapter: a table returned by chapters()\n"
"        return { \"image url\", ... }  -- or { {url=, headers={...}, key=}, ... }\n"
"\n"
"Available functions:\n"
"    http.get(url [, headers [, cache_seconds]])  -> body, status   (nil, error on network failure)\n"
"    http.post(url, body [, headers])             -> body, status\n"
"    json.decode(text) -> value (json.null for null), json.encode(value) -> text\n"
"    html.parse(text) -> document; document/node: select(css) -> {nodes}, first(css) -> node or nil,\n"
"        text(), attr(name), tag()   -- css: tag #id .class [attr] [attr=v] [attr^=v] [attr$=v]\n"
"        [attr*=v] :first-child :last-child, descendant (space) and child (>) combinators, a, b\n"
"    util.urlencode(text), util.trim(text), util.log(...)\n"
"The Lua base, string, table, math and utf8 libraries are available (no file or system access).\n"
"\n"
"See example.lua.txt (rename it to example.lua to try it).\n";

static const char *SourcesExample=
"-- Minimal store: a site with a search page, a manga page listing the chapters and a chapter page with the images\n"
"SOURCE = { id = \"example\", name = \"Example\", kind = \"manga\", site = \"https://example.com\",\n"
"           headers = { Referer = \"https://example.com/\" } }\n"
"\n"
"local BASE = \"https://example.com\"\n"
"\n"
"local function get_page(url)\n"
"    local body, status = http.get(url)\n"
"    if not body or status ~= 200 then error(\"cannot load \" .. url, 0) end\n"
"    return html.parse(body)\n"
"end\n"
"\n"
"function search(query, page)\n"
"    local results = {}\n"
"    for _, card in ipairs(get_page(BASE .. \"/search?q=\" .. util.urlencode(query) .. \"&page=\" .. page):select(\".result\")) do\n"
"        local link = card:first(\"a\")\n"
"        local img = card:first(\"img\")\n"
"        results[#results + 1] = { id = link:attr(\"href\"), title = link:text(), cover = img and img:attr(\"src\") or \"\" }\n"
"    end\n"
"    return results\n"
"end\n"
"\n"
"function chapters(manga)\n"
"    local list = {}\n"
"    for _, a in ipairs(get_page(BASE .. manga.id):select(\".chapters a\")) do\n"
"        list[#list + 1] = { id = a:attr(\"href\"), title = a:text() }\n"
"    end\n"
"    return list\n"
"end\n"
"\n"
"function pages(chapter)\n"
"    local list = {}\n"
"    for _, img in ipairs(get_page(BASE .. chapter.id):select(\".page img\")) do\n"
"        list[#list + 1] = img:attr(\"src\")\n"
"    end\n"
"    return list\n"
"end\n";

std::string UserSourcesDir()
{
    std::string dir=std::string(GetUserDataDir())+"/sources";
    fl_make_path(dir.c_str());
    //Documentation for the store authors
    std::string readme=dir+"/README.txt", example=dir+"/example.lua.txt";
    if(fl_access(readme.c_str(), 0)!=0) //Missing
    {
        FILE *f=fl_fopen(readme.c_str(), "wb");
        if(f) { fputs(SourcesReadme, f); fclose(f); }
    }
    if(fl_access(example.c_str(), 0)!=0)
    {
        FILE *f=fl_fopen(example.c_str(), "wb");
        if(f) { fputs(SourcesExample, f); fclose(f); }
    }
    return dir;
}

std::vector<LuaSource*> LoadStoreSources(std::vector<std::string> *errors)
{
    std::vector<LuaSource*> sources, user_sources;

    //User sources: they replace the built-in ones with the same id, the others come after them
    std::string dir=UserSourcesDir();
    dirent **listing;
    int nb=fl_filename_list(dir.c_str(), &listing, fl_casenumericsort);
    for(int i=0; i<nb; i++)
    {
        std::string name=listing[i]->d_name;
        if(name.size()<5 || name.compare(name.size()-4, 4, ".lua")!=0)
            continue;
        std::string path=dir+"/"+name;
        FILE *f=fl_fopen(path.c_str(), "rb");
        if(!f)
            continue;
        std::string script;
        char chunk[16384];
        size_t n;
        while((n=fread(chunk, 1, sizeof(chunk), f))>0)
            script.append(chunk, n);
        fclose(f);
        LuaSource *source=new LuaSource(script, path);
        if(source->IsValid())
            user_sources.push_back(source);
        else
        {
            errors->push_back(name+": "+source->LoadError());
            delete source;
        }
    }
    if(nb>0)
        fl_filename_free_list(&listing, nb);

    for(int i=0; BuiltinSources[i]; i++)
    {
        LuaSource *source=new LuaSource(BuiltinSources[i], "built-in");
        if(!source->IsValid())
        {
            errors->push_back("built-in source: "+source->LoadError());
            delete source;
            continue;
        }
        //Replaced by a user source?
        LuaSource *replacement=NULL;
        for(size_t k=0; k<user_sources.size() && !replacement; k++)
        {
            if(user_sources[k]->Id==source->Id)
            {
                replacement=user_sources[k];
                user_sources.erase(user_sources.begin()+k);
            }
        }
        if(replacement)
        {
            delete source;
            source=replacement;
        }
        sources.push_back(source);
    }
    sources.insert(sources.end(), user_sources.begin(), user_sources.end());
    return sources;
}


//------------------------------------Installation------------------------

//! Field of the table at the top of the stack, as text ("" if missing)
static std::string FieldText(lua_State *L, const char *name)
{
    lua_getfield(L, -1, name);
    std::string text=lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
    lua_pop(L, 1);
    return text;
}

bool ParseStoreCatalog(const std::string &json, const std::string &index_url, std::vector<StoreCatalogEntry> *entries, std::string *error)
{
    LuaLimits limits;
    lua_State *L=NewLimitedState(&limits);
    if(!L)
    {
        *error="Not enough memory";
        return false;
    }
    limits.Budget=LUA_CALL_BUDGET;
    lua_pushcfunction(L, l_json_decode);
    lua_pushlstring(L, json.data(), json.size());
    bool ok=lua_pcall(L, 1, 2, 0)==LUA_OK && lua_istable(L, -2);
    if(!ok)
        *error="The catalog is not valid: "+std::string(lua_isstring(L, -1) ? lua_tostring(L, -1) : "not a JSON object");
    else
    {
        lua_pop(L, 1);
        lua_getfield(L, -1, "stores");
        if(!lua_istable(L, -1))
        {
            *error="The catalog has no \"stores\" list";
            ok=false;
        }
        else
        {
            //Relative script addresses: from the folder of the catalog
            std::string base=index_url.substr(0, index_url.find_last_of('/')+1);
            entries->clear();
            lua_Integer n=(lua_Integer)lua_rawlen(L, -1);
            for(lua_Integer i=1; i<=n; i++)
            {
                lua_rawgeti(L, -1, i);
                if(lua_istable(L, -1))
                {
                    StoreCatalogEntry entry;
                    entry.Id=FieldText(L, "id");
                    entry.Name=FieldText(L, "name");
                    entry.Version=FieldText(L, "version");
                    entry.Description=FieldText(L, "description");
                    entry.Url=FieldText(L, "url");
                    if(!entry.Url.empty() && entry.Url.compare(0, 7, "http://")!=0 && entry.Url.compare(0, 8, "https://")!=0)
                        entry.Url=base+entry.Url;
                    if(!entry.Id.empty() && !entry.Url.empty())
                        entries->push_back(entry);
                }
                lua_pop(L, 1);
            }
        }
    }
    lua_close(L);
    return ok;
}

std::vector<InstalledStore> ListStores(std::vector<std::string> *errors)
{
    std::vector<std::string> builtin_ids;
    for(int i=0; BuiltinSources[i]; i++)
    {
        LuaSource source(BuiltinSources[i], "built-in");
        if(source.IsValid())
            builtin_ids.push_back(source.Id);
    }
    std::vector<InstalledStore> stores;
    std::vector<LuaSource*> sources=LoadStoreSources(errors);
    for(size_t i=0; i<sources.size(); i++)
    {
        InstalledStore store;
        store.Id=sources[i]->Id;
        store.Name=sources[i]->Name;
        store.Version=sources[i]->Version;
        store.Origin=sources[i]->Origin;
        store.BuiltIn=store.Origin=="built-in";
        store.ReplacesBuiltIn=!store.BuiltIn && std::find(builtin_ids.begin(), builtin_ids.end(), store.Id)!=builtin_ids.end();
        stores.push_back(store);
        delete sources[i];
    }
    return stores;
}

bool InstallStoreScript(const std::string &script, InstalledStore *installed, bool *replaced, std::string *error)
{
    LuaSource source(script, "new store");
    if(!source.IsValid())
    {
        *error="Not a valid store: "+source.LoadError();
        return false;
    }
    //The file name from the id: letters, digits, - and _ only
    std::string file;
    for(size_t i=0; i<source.Id.size(); i++)
    {
        char c=source.Id[i];
        file+=(isalnum((unsigned char)c) || c=='-' || c=='_') ? c : '_';
    }
    std::string path=UserSourcesDir()+"/"+file+".lua";
    *replaced=FileUtf8::Exists(path);
    if(!FileUtf8::Write(path, script))
    {
        *error="Unable to write "+path;
        return false;
    }
    installed->Id=source.Id;
    installed->Name=source.Name;
    installed->Version=source.Version;
    installed->Origin=path;
    return true;
}

std::string StoreCatalogUrl()
{
    const char *url=getenv("CLAUDEREADER_STORE_CATALOG");
    return (url && *url) ? url : STORE_CATALOG_URL;
}

int CompareVersions(const std::string &a, const std::string &b)
{
    size_t i=0, j=0;
    while(i<a.size() || j<b.size())
    {
        long x=0, y=0;
        while(i<a.size() && a[i]!='.')
            x=x*10+(isdigit((unsigned char)a[i]) ? a[i]-'0' : 0), i++;
        while(j<b.size() && b[j]!='.')
            y=y*10+(isdigit((unsigned char)b[j]) ? b[j]-'0' : 0), j++;
        if(x!=y)
            return x<y ? -1 : 1;
        i++;
        j++;
    }
    return 0;
}
