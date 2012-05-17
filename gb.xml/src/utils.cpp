#include "utils.h"


/*
#define PUSH_BACK(car) if(car > 0) {deststr[j] = (car); j++;}

void utf8toWStr(fwstring &dest, const char *src, const uint len)
{
    register wchar_t *deststr = new wchar_t[len];
    register wchar_t w = 0;
    register unsigned char bytes = 0;
    register wchar_t err = L'�';
    register size_t j = 0;
    register size_t i = 0;
    register unsigned char c;
    for (; i < len; i++){
        c = (unsigned char)src[i];
        if (c <= 0x7f){//first byte
            if (bytes){
                PUSH_BACK(err);
                bytes = 0;
            }
            PUSH_BACK((wchar_t)c);
        }
        else if (c <= 0xbf){//second/third/etc byte
            if (bytes){
                w = ((w << 6)|(c & 0x3f));
                bytes--;
                if (bytes == 0)
                     PUSH_BACK(w)
            }
            else
                 PUSH_BACK(err);
        }
        else if (c <= 0xdf){//2byte sequence start
            bytes = 1;
            w = c & 0x1f;
        }
        else if (c <= 0xef){//3byte sequence start
            bytes = 2;
            w = c & 0x0f;
        }
        else if (c <= 0xf7){//3byte sequence start
            bytes = 3;
            w = c & 0x07;
        }
        else{
             PUSH_BACK(err);
            bytes = 0;
        }
    }
    if (bytes)
         PUSH_BACK(err);

    dest.erase();
    dest.append(deststr, j);

    //std::cout << i << "/" << j << ">" << dest.size() << "/" << src.size() <<endl;
    delete[] deststr;
}

void utf8toWStr(fwstring& dest, const fwstring& src){
    return utf8toWStr(dest, src.c_str(), src.size());
}

#undef PUSH_BACK

void wstrToUtf8(fwstring& dest, const fwstring& src){
    dest.clear();
    for (size_t i = 0; i < src.size(); i++){
        wchar_t w = src[i];
        if (w <= 0x7f)
            dest.push_back((char)w);
        else if (w <= 0x7ff){
            dest.push_back(0xc0 | ((w >> 6)& 0x1f));
            dest.push_back(0x80| (w & 0x3f));
        }
        else if (w <= 0xffff){
            dest.push_back(0xe0 | ((w >> 12)& 0x0f));
            dest.push_back(0x80| ((w >> 6) & 0x3f));
            dest.push_back(0x80| (w & 0x3f));
        }
        else if (w <= 0x10ffff){
            dest.push_back(0xf0 | ((w >> 18)& 0x07));
            dest.push_back(0x80| ((w >> 12) & 0x3f));
            dest.push_back(0x80| ((w >> 6) & 0x3f));
            dest.push_back(0x80| (w & 0x3f));
        }
        else
            dest.push_back('?');
    }
}*/

/*
fwstring (const fwstring& str)
{
    fwstring result;
    wstrToUtf8(result, str);
    return result;
}

fwstring (const fwstring& str)
{
    fwstring result;
    utf8toWStr(result, str);
    return result;
}

fwstring (const char *src, const uint &len)
{
    fwstring result;
    utf8toWStr(result, src, len);
    return result;
}*/

fwstring Html$(fwstring text)
{
    fwstring str, s;
    for(unsigned int i = 0; i < text.length(); i++)
    {
        s = text.at(i);
        if(s == "<")
        {
            str += "&lt;";
        }
        else if(s == ">")
        {
            str += "&gt;";
        }
        else if(s == "&")
        {
            /*if((text.substr(i, 6) == "&nbsp;"))
            {str+= "&nbsp"; i +=5;} //On ignore les espaces
            else*/ str += "&amp;";
        }
        else if(s == "\"")
        {
            str += "&quot;";
        }
        else
        {
            str += s;
        }
    }

    return str;
}


/* http://www.w3.org/TR/REC-xml/#NT-NameStartChar

  NameStartChar	::=  ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] |
  [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] |
  [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
  */

#define INTER(min, max) (car >= min && car <= max)
#define INTERCAR(min, max) (car >= *(min) && car <= *(max))
#define CAR(c) (car == *(c))

bool isNameStartChar(fwstring &s)
{
    return isNameStartChar(s.at(0));
}
bool isNameStartChar(const wchar_t s)
{
    register const wchar_t car = s;
    if(INTER(CHAR_a, CHAR_z)) return true;

    return CAR(":") || INTERCAR("A", "Z") || CAR("_") || INTERCAR("a", "z") || CAR("Ø") ||
            INTER(0xC0, 0xD6) || INTER(0xD8, 0xF6) || INTER(0xF8, 0x2FF) ||
            INTER(0x370, 0x37D) || INTER(0x37F, 0x1FFF) || INTER(0x200C, 0x200D) ||
            INTER(0x2070, 0x218F) || INTER(0x2C00, 0x2FEF) || INTER(0x3001, 0xD7FF) ||
            INTER(0xF900, 0xFDCF) || INTER(0xFDF0, 0xFFFD) || INTER(0x10000, 0xEFFFF);


}

/* http://www.w3.org/TR/REC-xml/#NT-NameChar

  NameChar ::= 	NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]

  */

bool isNameChar(const wchar_t s)
{
    register const wchar_t car = s;
    if(INTER(CHAR_a, CHAR_z)) return true;
    if(isNameStartChar(s)) return true;
    return  CAR("-") || CAR(".") || INTERCAR("0", "9") ||
            (car == 0xB7) || INTER(0x0300, 0x036F) || INTER(0x203F, 0x2040);
}

bool isNameChar(fwstring &s)
{
    return isNameChar(s.at(0));
}

/* http://www.w3.org/TR/REC-xml/#NT-S

    S ::= (#x20 | #x9 | #xD | #xA)+

  */

bool isWhiteSpace(fwstring &s)
{
    return isWhiteSpace(s.at(0));
}

bool isWhiteSpace(const wchar_t s)
{
    register const wchar_t car = s;

    return (car == 0x20) || (car == 0x9) || (car == 0xD) || (car == 0xA);
}

bool isWhiteSpace(const char s)
{
    register const char car = s;

    return (car == 0x20) || (car == 0x9) || (car == 0xD) || (car == 0xA);
}



const void* memrchrs(const void *source, size_t lensource, const void *comp, size_t lencomp)
{
    char *pos = (char*)source;
    do
    {
        pos = (char*)(memrchr((void*)pos, ((char*)(comp))[lencomp - 1], lensource));
        if(!pos) return 0;
        if(pos - lencomp < source) return 0;
        if(memcmp(pos - lencomp, comp, lencomp) != 0) continue;
        return pos;
    }while(1);
}



fwstring::fwstring()
{
    data = 0;
    len = 0;
}


fwstring::fwstring(char *src, size_t length) : data(src), len(length)
{

}

fwstring::fwstring(char *src, int &length) : data(src), len((unsigned int)length)
{

}

wchar_t fwstring::increment(size_t &cur) const
{
    register unsigned char c = (unsigned char)data[cur];
    if (c <= 0x7f){//first byte
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
        cur += 2;
        return (((c & 0x1f) << 6) | (c2 & 0x3f));
    }
    else if (c <= 0xef  && c >= 0xbf && cur + 2 < len){//3byte sequence start
        register wchar_t c2 = (unsigned char)data[cur+1];
        register wchar_t c3 = (unsigned char)data[cur+2];
        cur += 3;
        return (((((c & 0x1f) << 6) | (c2 & 0x3f)) << 6)| (c3 & 0x3f));
        //w = c & 0x0f;
    }
    else if (c <= 0xf7  && c >= 0xbf && cur + 3 < len){//4byte sequence start
        register wchar_t c2 = (unsigned char)data[cur+1];
        register wchar_t c3 = (unsigned char)data[cur+2];
        register wchar_t c4 = (unsigned char)data[cur+3];
        cur += 4;
        return (((((((c & 0x1f) << 6) | (c2 & 0x3f)) << 6)| (c3 & 0x3f)) << 6) | (c4 & 0x3f));
        //w = c & 0x07;
    }
    else
    {
       return CHAR_ERROR;
    }
}
/*
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
}*/

string fwstring::toStdString() const
{
    return string(data, len);
}
fwstring fwstring::toString() const
{
    return fwstring(data, len);
}

/*
bool fwstring::incrementCompare(const wchar_t* text, size_t tlen)
{

}
*/

fwstring* fwstring::copyString(uint start, uint end) const
{
    register const size_t len = end - start;
    char *ndata = (char*)malloc(sizeof(char) * len);
    memcpy(ndata, data + start, len);
    return new fwstring(ndata, len);
}


fwstring::fwstring(const fwstring &other)
{
    len = other.length();
    //if(!len) {data = 0; return;}
    //DEBUG << len << endl;
    //data = new char[len];
    data = other.data;
    //memcpy((void*)data, (void*)other.data, len);
}

fwstring::~fwstring()
{

}

bool fwstring::operator==(const char* other)
{
    return strcmp(other, data);
}

fwstring& fwstring::operator=(char s)
{
    data = new char;
    *data = s;
    len = 1;

    return *this;
}

fwstring& fwstring::operator=(const fwstring other)
{
    data = other.data;
    len = other.len;

    return *this;
}

fwstring operator+(fwstring &char1, const char* char2)
{
    fwstring str;
    register size_t char2len = strlen(char2);
    str.len = char1.len + char2len;
    str.data = (char*)malloc(sizeof(char)*str.len);
    memcpy(str.data, char1.data, char1.len);
    memcpy(str.data + char1.len, char2, char2len);
    return str;
}

fwstring operator+(const char* char1,  fwstring &char2)
{
    fwstring str;
    register size_t char1len = strlen(char1);
    str.len = char1len + char2.len;
    str.data = (char*)malloc(sizeof(char)*str.len);
    memcpy(str.data, char1, char1len);
    memcpy(str.data + char1len, char2.data, char2.len);
    return str;
}

fwstring& fwstring::operator +=(const char* other)
{
    if(!len) *this = other;
    register size_t otherlen = strlen(other);
    register size_t newlen = len + otherlen;
    //DEBUG << sizeof(char)*newlen << endl;
    data = (char *)realloc(data, sizeof(char)*newlen);
    memcpy(data + len, other, otherlen);
    //delete data;
    len = newlen;
    return *this;
}

fwstring& fwstring::operator+=(const fwstring other)
{
    if(!len) {*this = other; return *this;}
    register size_t newlen = len + other.len;
    //DEBUG << sizeof(char)*newlen << endl;
    data = (char *)realloc(data, sizeof(char)*newlen);
    memcpy(data + len, other.data, other.len);
    //delete data;
    len = newlen;
    return *this;
}

fwstring& fwstring::operator+=(const char other)
{
    //DEBUG << (void*)data << " " << len << endl;
    if(!len || !data)
    {
        data = (char*)malloc(sizeof(char*));
        *data = other;
        len = 1;
        return *this;
    }
    char *newdata = (char*)malloc(sizeof(char) * (len + 1));
    memcpy(newdata, data, len);
    free(data);
    data = newdata;
    //data = (char *)realloc(data, sizeof(char) * (len + 1));
    data[len] = other;
    len++;//DEBUG << (void*)data << " " << len << endl;
    return *this;
}


bool operator<(fwstring a, fwstring b)
{
    return memcmp(a.data, b.data, Mini<size_t>(a.len, b.len)) < 0;
}

bool operator>(fwstring a, fwstring b)
{
    return memcmp(a.data, b.data, Mini<size_t>(a.len, b.len)) > 0;
}

fwstring::fwstring(const string &other)
{
    len = other.length();
    data = (char*)malloc(sizeof(char)*len);
    memcpy(data, other.c_str(), len);
}


fwstring::fwstring(const char *src)
{
    len = strlen(src);
    data = (char*)malloc(sizeof(char)*len);
    memcpy(data, src, len);

}

fwstring::fwstring(const char src)
{
    len = 1;
    data = new char;
    *data = src;
}

fwstring operator+(fwstring &char1, fwstring &char2)
{
    fwstring str;
    str.len = char1.len + char2.len;
    str.data = (char*)malloc(sizeof(char)*str.len);
    memcpy(str.data, char1.data, char1.len);
    memcpy(str.data + char1.len, char2.data, char2.len);
    return str;
}


fwstring operator+(const char char1, fwstring char2)
{
    fwstring str;
    str.len = char2.len + 1;
    str.data = (char*)malloc(sizeof(char)*str.len);
    str.data[0] = char1;
    memcpy(str.data + 1, char2.data, char2.len);
    return str;
}

fwstring fwstring::substr(size_t start, size_t nlen)
{
    if(nlen == npos) nlen = len - start;
    char *ndata = (char*)malloc(sizeof(char) * nlen);
    memcpy(ndata, data + start, nlen);
    fwstring str(data, nlen);

    return str;

}
fwstring* fwstring::ssubstr(size_t start, size_t nlen)
{
    if(nlen == npos) nlen = len - start;
    fwstring *str = new fwstring(data + start, nlen);

    return str;

}

ostream &operator<<( ostream &out, fwstring &str )
{
    out << str.toStdString();
    return out;
}
ostream &operator<<( ostream &out, fwstring str )
{
    out << str.toStdString();
    return out;
}

fwstring::fwstring(int num, const char *s)
{
    size_t slen = strlen(s);
    register int i = 0;
    len = slen*num;
    data = (char*)malloc(sizeof(char)*len);
    for(i = 0; i < num; i++)
    {
        memcpy(data + (i*slen), s, slen);
    }
}

fwstring::fwstring(int num, const char s)
{
    len = num;
    data = (char*)malloc(sizeof(char)*len);
    memset(data, s, len);
}

bool operator==(fwstring const &a, fwstring const& b)
{
    if(a.len != b.len) return false;
    if(a.data == 0 && b.data == 0) return true;
    if(a.data == 0 || b.data == 0) return false;

    DEBUG << "comp : " << (void*)(a.data) << " == " << (void*)(b.data) << endl;

    return (memcmp(a.data, b.data, a.len) == 0);
}

bool operator==(fwstring const &a, const char* b)
{
    size_t blen = strlen(b);
    if(a.len != blen) return false;
    //DEBUG << "comp : " << a << " == " << b << " -> "<<memcmp(a.data, b, a.len) << endl;
    return (memcmp(a.data, b, a.len) == 0);

}

bool operator!=(fwstring const &a, fwstring const& b)
{
    if(a.len != b.len) return true;

    return memcmp(a.data, b.data, a.len) != 0;
}

void fwstring::erase()
{
    data = 0;
    len = 0;
}

size_t fwstring::rfind(const char s)
{
    register void* pos = memrchr(data, s, len);
    if(pos == 0) return npos;
    return (char*)pos - data;

}

size_t fwstring::rfind(const char *s)
{
    const register void* pos = memrchrs(data, len, s, strlen(s));
    if(pos == 0) return npos;
    return (char*)pos - data;

}

size_t fwstring::rfind(const fwstring &s)
{
    const register void* pos = memrchrs(data, len, s.data, s.len);
    if(pos == 0) return npos;
    return (char*)pos - data;
}

fwstring* fwstring::copy()
{
    char *ndata = (char*)malloc(sizeof(char)*len);
    memcpy(ndata, data, len);
    return new fwstring(ndata, len);
}
