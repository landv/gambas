#define GBI_CPP
#include "gbi.h"

#include "element.h"
#include "document.h"

fwstring::fwstring()
{
    data = 0;
    len = 0;
    cur = 0;
    wcur = 0;
}

void GBI::InitClasses()
{
    Element::ClassName = GB.FindClass("XmlElement");
    TextNode::ClassName = GB.FindClass("XmlTextNode");
    CommentNode::ClassName = GB.FindClass("XmlCommentNode");
    CDATANode::ClassName = GB.FindClass("XmlCDATANode");
    Node::ClassName = GB.FindClass("XmlNode");
    AttrNode::ClassName = GB.FindClass("_XmlAttrNode");
    Document::ClassName = GB.FindClass("XmlDocument");
}

fwstring::fwstring(char *src, size_t &length) : data(src), len(length), cur(0), wcur(0)
{

}

fwstring::fwstring(char *src, int &length) : data(src), len((unsigned int)length), cur(0), wcur(0)
{}

fwstring::fwstring(const fwstring &other)
{
    data = new char[other.len];
    len = other.len;
    cur = 0;

    for(register size_t i = 0; i < len; i++)
    {
        other.data[i] = data[i];
    }
}

wchar_t fwstring::increment()
{
    register unsigned char c = (unsigned char)data[cur];
    if (c <= 0x7f){//first byte
        wcur++;
        cur++;
        return (wchar_t)c;
    }
    /*else if (c <= 0xbf){//second/third/etc byte
        if (bytes){
            w = ((w << 6)|(c & 0x3f));
            bytes--;
            if (bytes == 0)
                 PUSH_BACK(w)
        }
        else
             PUSH_BACK(err);
    }*/
    else if (c <= 0xdf && c >= 0xbf && cur + 1 < len){//2byte sequence start
        register wchar_t c2 = (unsigned char)data[cur+1];
        wcur++;
        cur += 2;
        return (((c & 0x1f) << 6) | (c2 & 0x3f));
    }
    else if (c <= 0xef  && c >= 0xbf && cur + 2 < len){//3byte sequence start
        register wchar_t c2 = (unsigned char)data[cur+1];
        register wchar_t c3 = (unsigned char)data[cur+2];
        wcur++;
        cur += 3;
        return (((((c & 0x1f) << 6) | (c2 & 0x3f)) << 6)| (c3 & 0x3f));
        //w = c & 0x0f;
    }
    else if (c <= 0xf7  && c >= 0xbf && cur + 3 < len){//4byte sequence start
        register wchar_t c2 = (unsigned char)data[cur+1];
        register wchar_t c3 = (unsigned char)data[cur+2];
        register wchar_t c4 = (unsigned char)data[cur+3];
        wcur++;
        cur += 4;
        return (((((((c & 0x1f) << 6) | (c2 & 0x3f)) << 6)| (c3 & 0x3f)) << 6) | (c4 & 0x3f));
        //w = c & 0x07;
    }
    else
    {
       return CHAR_ERROR;
    }
}

wchar_t fwstring::increment(size_t count)
{
   register unsigned char c = 0;
   size_t i = wcur;
   while(count - 1 > wcur - i)
   {
       c = (unsigned char)data[cur];
       if (c <= 0x7f){//first byte
           wcur++;
           cur++;
       }
       else if (c <= 0xdf && c >= 0xbf && cur + 1 < len){//2byte sequence start
           wcur++;
           cur += 2;
       }
       else if (c <= 0xef  && c >= 0xbf && cur + 2 < len){//3byte sequence start
           wcur++;
           cur += 3;
       }
       else if (c <= 0xf7  && c >= 0xbf && cur + 3 < len){//4byte sequence start
           wcur++;
           cur += 4;
       }
   }

   return increment();
}

void fwstring::resetCounter()
{
    cur = 0;
    wcur = 0;
}

std::string fwstring::toString() const
{
    return std::string(data, len);
}

bool fwstring::incrementCompare(const wchar_t* text, size_t tlen)
{
    register size_t i = 0;

    while(cur < len && i < tlen)
    {
        if(increment() != text[i]) return false;
        i++;
    }

    return true;

}
