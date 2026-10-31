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
#include "HtmlQuery.h"
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <set>


//------------------------------------Selector parsing------------------------

namespace
{
    struct AttrTest
    {
        std::string name, value;
        char op;    //0: exists, '=': equal, '^': starts with, '$': ends with, '*': contains, '~': word in list
    };

    //! One element test: tag#id.class[attr]:pseudo
    struct Compound
    {
        std::string tag;    //Empty: any
        std::string id;
        std::vector<std::string> classes;
        std::vector<AttrTest> attrs;
        bool first_child, last_child;
        char combinator;    //With the previous compound: ' ' descendant, '>' child (unused for the first one)
    };

    typedef std::vector<Compound> Selector;

    std::string Lower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)tolower(c); });
        return s;
    }

    bool IsNameChar(char c)
    {
        return isalnum((unsigned char)c) || c=='-' || c=='_' || (unsigned char)c>=0x80;
    }

    //! Parse one selector (no comma)
    bool ParseSelector(const std::string &text, Selector *sel, std::string *error)
    {
        size_t i=0, n=text.size();
        char combinator=' ';
        while(i<n)
        {
            //Combinator (white spaces and/or '>')
            bool space=false;
            while(i<n && isspace((unsigned char)text[i])) { i++; space=true; }
            if(i<n && text[i]=='>')
            {
                combinator='>';
                i++;
                while(i<n && isspace((unsigned char)text[i])) i++;
            }
            else if(space)
                combinator=' ';
            if(i>=n)
                break;

            Compound c;
            c.first_child=c.last_child=false;
            c.combinator=combinator;
            combinator=' ';
            bool any=false;
            while(i<n && !isspace((unsigned char)text[i]) && text[i]!='>')
            {
                char ch=text[i];
                if(ch=='*')
                {
                    i++;
                    any=true;
                }
                else if(IsNameChar(ch))
                {
                    size_t s=i;
                    while(i<n && IsNameChar(text[i])) i++;
                    c.tag=Lower(text.substr(s, i-s));
                    any=true;
                }
                else if(ch=='#' || ch=='.')
                {
                    size_t s=++i;
                    while(i<n && IsNameChar(text[i])) i++;
                    if(i==s) { *error="Name expected after '"+std::string(1, ch)+"'"; return false; }
                    if(ch=='#') c.id=text.substr(s, i-s); else c.classes.push_back(text.substr(s, i-s));
                    any=true;
                }
                else if(ch=='[')
                {
                    size_t close=text.find(']', i);
                    if(close==std::string::npos) { *error="Missing ']'"; return false; }
                    std::string inside=text.substr(i+1, close-i-1);
                    i=close+1;
                    AttrTest t;
                    t.op=0;
                    size_t eq=inside.find('=');
                    if(eq==std::string::npos)
                        t.name=inside;
                    else
                    {
                        size_t name_end=eq;
                        if(eq>0 && strchr("^$*~", inside[eq-1]))
                        {
                            t.op=inside[eq-1];
                            name_end=eq-1;
                        }
                        else
                            t.op='=';
                        t.name=inside.substr(0, name_end);
                        t.value=inside.substr(eq+1);
                        //Trim and remove the quotes
                        t.value.erase(0, t.value.find_first_not_of(" \t"));
                        t.value.erase(t.value.find_last_not_of(" \t")+1);
                        if(t.value.size()>=2 && (t.value[0]=='"' || t.value[0]=='\'') && t.value[t.value.size()-1]==t.value[0])
                            t.value=t.value.substr(1, t.value.size()-2);
                    }
                    t.name.erase(0, t.name.find_first_not_of(" \t"));
                    t.name.erase(t.name.find_last_not_of(" \t")+1);
                    t.name=Lower(t.name);
                    c.attrs.push_back(t);
                    any=true;
                }
                else if(ch==':')
                {
                    size_t s=++i;
                    while(i<n && IsNameChar(text[i])) i++;
                    std::string pseudo=Lower(text.substr(s, i-s));
                    if(pseudo=="first-child") c.first_child=true;
                    else if(pseudo=="last-child") c.last_child=true;
                    else { *error="Unsupported pseudo class :"+pseudo; return false; }
                    any=true;
                }
                else
                {
                    *error="Unexpected character '"+std::string(1, ch)+"'";
                    return false;
                }
            }
            if(!any) { *error="Empty selector part"; return false; }
            sel->push_back(c);
        }
        if(sel->empty()) { *error="Empty selector"; return false; }
        return true;
    }

    //! Split a selector list on the commas outside brackets and quotes
    std::vector<std::string> SplitList(const std::string &text)
    {
        std::vector<std::string> parts;
        std::string current;
        int depth=0;
        char quote=0;
        for(size_t i=0; i<text.size(); i++)
        {
            char c=text[i];
            if(quote) { if(c==quote) quote=0; }
            else if(c=='"' || c=='\'') quote=c;
            else if(c=='[') depth++;
            else if(c==']') depth--;
            else if(c==',' && depth==0) { parts.push_back(current); current.clear(); continue; }
            current+=c;
        }
        parts.push_back(current);
        return parts;
    }


    //------------------------------------Matching------------------------

    bool IsElement(const GumboNode *node)
    {
        return node && node->type==GUMBO_NODE_ELEMENT;
    }

    const GumboNode *ElementParent(const GumboNode *node)
    {
        const GumboNode *p=node->parent;
        return IsElement(p) ? p : NULL;
    }

    bool HasWord(const std::string &list, const std::string &word)
    {
        size_t i=0;
        while(i<list.size())
        {
            while(i<list.size() && isspace((unsigned char)list[i])) i++;
            size_t s=i;
            while(i<list.size() && !isspace((unsigned char)list[i])) i++;
            if(i>s && list.compare(s, i-s, word)==0)
                return true;
        }
        return false;
    }

    //! Is the node the first (last) element among its siblings
    bool IsEdgeChild(const GumboNode *node, bool first)
    {
        const GumboNode *parent=node->parent;
        if(!parent || (parent->type!=GUMBO_NODE_ELEMENT && parent->type!=GUMBO_NODE_DOCUMENT))
            return true;
        const GumboVector *children=(parent->type==GUMBO_NODE_ELEMENT) ? &parent->v.element.children : &parent->v.document.children;
        for(unsigned int k=0; k<children->length; k++)
        {
            const GumboNode *child=(const GumboNode*)children->data[first ? k : children->length-1-k];
            if(IsElement(child))
                return child==node;
        }
        return false;
    }

    bool MatchCompound(const GumboNode *node, const Compound &c)
    {
        if(!IsElement(node))
            return false;
        if(!c.tag.empty() && HtmlTag(node)!=c.tag)
            return false;
        if(!c.id.empty() && HtmlAttr(node, "id")!=c.id)
            return false;
        if(!c.classes.empty())
        {
            std::string classes=HtmlAttr(node, "class");
            for(size_t k=0; k<c.classes.size(); k++)
                if(!HasWord(classes, c.classes[k]))
                    return false;
        }
        for(size_t k=0; k<c.attrs.size(); k++)
        {
            const AttrTest &t=c.attrs[k];
            const GumboAttribute *a=gumbo_get_attribute(&node->v.element.attributes, t.name.c_str());
            if(!a)
                return false;
            std::string v=a->value;
            switch(t.op)
            {
                case '=': if(v!=t.value) return false; break;
                case '^': if(v.compare(0, t.value.size(), t.value)!=0) return false; break;
                case '$': if(v.size()<t.value.size() || v.compare(v.size()-t.value.size(), t.value.size(), t.value)!=0) return false; break;
                case '*': if(v.find(t.value)==std::string::npos) return false; break;
                case '~': if(!HasWord(v, t.value)) return false; break;
                default: break;
            }
        }
        if(c.first_child && !IsEdgeChild(node, true))
            return false;
        if(c.last_child && !IsEdgeChild(node, false))
            return false;
        return true;
    }

    //! Match the selector parts [0..idx] ending on node, ancestors limited to the root subtree
    bool MatchFrom(const GumboNode *node, const Selector &sel, int idx, const GumboNode *root)
    {
        if(!MatchCompound(node, sel[idx]))
            return false;
        if(idx==0)
            return true;
        const GumboNode *p=ElementParent(node);
        if(sel[idx].combinator=='>')
            return p && p!=root->parent && MatchFrom(p, sel, idx-1, root);
        for(; p && p!=root->parent; p=ElementParent(p))
            if(MatchFrom(p, sel, idx-1, root))
                return true;
        return false;
    }

    void Collect(const GumboNode *node, const GumboNode *root, const std::vector<Selector> &sels, std::vector<const GumboNode*> *found)
    {
        const GumboVector *children=NULL;
        if(node->type==GUMBO_NODE_ELEMENT || node->type==GUMBO_NODE_TEMPLATE)
            children=&node->v.element.children;
        else if(node->type==GUMBO_NODE_DOCUMENT)
            children=&node->v.document.children;
        if(!children)
            return;
        for(unsigned int k=0; k<children->length; k++)
        {
            const GumboNode *child=(const GumboNode*)children->data[k];
            if(IsElement(child))
            {
                for(size_t s=0; s<sels.size(); s++)
                {
                    if(MatchFrom(child, sels[s], (int)sels[s].size()-1, root))
                    {
                        found->push_back(child);
                        break;
                    }
                }
            }
            Collect(child, root, sels, found);
        }
    }

    void AppendText(const GumboNode *node, std::string *out)
    {
        if(node->type==GUMBO_NODE_TEXT || node->type==GUMBO_NODE_WHITESPACE || node->type==GUMBO_NODE_CDATA)
        {
            out->append(node->v.text.text);
            return;
        }
        if(node->type==GUMBO_NODE_ELEMENT || node->type==GUMBO_NODE_TEMPLATE)
        {
            //Line breaks and blocks separate words
            if(node->v.element.tag==GUMBO_TAG_BR)
                out->append(" ");
            const GumboVector *children=&node->v.element.children;
            for(unsigned int k=0; k<children->length; k++)
                AppendText((const GumboNode*)children->data[k], out);
        }
        else if(node->type==GUMBO_NODE_DOCUMENT)
        {
            const GumboVector *children=&node->v.document.children;
            for(unsigned int k=0; k<children->length; k++)
                AppendText((const GumboNode*)children->data[k], out);
        }
    }
}


//------------------------------------Public functions------------------------

bool HtmlSelect(const GumboNode *root, const std::string &selector, std::vector<const GumboNode*> *found, std::string *error)
{
    std::vector<Selector> sels;
    std::vector<std::string> parts=SplitList(selector);
    for(size_t i=0; i<parts.size(); i++)
    {
        Selector sel;
        if(!ParseSelector(parts[i], &sel, error))
        {
            *error="Invalid selector \""+selector+"\": "+*error;
            return false;
        }
        sels.push_back(sel);
    }
    found->clear();
    Collect(root, root, sels, found);
    return true;
}

std::string HtmlText(const GumboNode *node)
{
    std::string raw, out;
    AppendText(node, &raw);
    bool space=false;
    for(size_t i=0; i<raw.size(); i++)
    {
        if(isspace((unsigned char)raw[i]))
            space=true;
        else
        {
            if(space && !out.empty())
                out+=' ';
            space=false;
            out+=raw[i];
        }
    }
    return out;
}

std::string HtmlAttr(const GumboNode *node, const char *name)
{
    if(!IsElement(node))
        return "";
    const GumboAttribute *a=gumbo_get_attribute(&node->v.element.attributes, name);
    return a ? a->value : "";
}

std::string HtmlTag(const GumboNode *node)
{
    if(!IsElement(node))
        return "";
    if(node->v.element.tag!=GUMBO_TAG_UNKNOWN)
        return gumbo_normalized_tagname(node->v.element.tag);
    //Unknown tag: original name from the source
    GumboStringPiece piece=node->v.element.original_tag;
    gumbo_tag_from_original_text(&piece);
    return Lower(std::string(piece.data, piece.length));
}
