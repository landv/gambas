#ifndef MAIN_H
#define MAIN_H

#include "gbi.h"

#include <string>
#include <list>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <exception>
using namespace std;

#define uint unsigned int

#define DELETE(_ob) if(_ob) {delete _ob; _ob = 0;}
#define UNREF(_ob) if(_ob) GB.Unref(POINTER(&(_ob)))

#define SCHAR_N 0xA // \n
#define SCHAR_R 0xD // \r

#define CHAR_STARTTAG 0x3C // <
#define CHAR_ENDTAG 0x3E // >

#define CHAR_SINGLEQUOTE 0x27 // '
#define CHAR_DOUBLEQUOTE 0x22 // "

#define CHAR_SLASH 0x2F
#define CHAR_BACKSLASH 0x5C
#define CHAR_EQUAL 0x3D // =
#define CHAR_AND 0x26 // &
#define CHAR_PI 0x3F // ?



template<class T>
std::wstring toString(T i)
{
    std::wstringstream ss;
    std::wstring s;
    ss << i;
    s = ss.str();
    return s;
}

template<class T>
inline T Maxi(T a, T b)
{
    if(a > b)
        return a;
    return b;
}
template<class T>
inline T Mini(T a, T b)
{
    if(a < b)
        return a;
    return b;
}

std::string WStringToString(const std::wstring& s);

std::wstring StringToWString(const char *src, const uint &len);
std::wstring StringToWString(const std::string& s);


ostream &operator<<( ostream &out, std::wstring &str );
ostream &operator<<( ostream &out, std::wstring *str );

wstring Html$(wstring text);

#ifndef __MAIN_CPP
extern "C" GB_INTERFACE GB;
#endif

#undef STRING
#define STRING(_arg) (CSTRING(_arg) != 0 ? StringToWString(CSTRING(_arg), VARG(_arg).len) : wstring())
#define CSTRING(_arg) (VARG(_arg).addr + VARG(_arg).start)

#undef PSTRING
#define PSTRING(_arg) (CPSTRING(_arg) != 0 ? StringToWString(CPSTRING(_arg), VPROP(GB_STRING).len) : wstring())
#define CPSTRING(_arg) (VPROP(GB_STRING).addr + VPROP(GB_STRING).start)

#define STRINGOPT(arg, def) (MISSING(arg) ? def : STRING(arg))

#define VARGOBJ(type, arg) reinterpret_cast<type*>(VARG(arg))

bool isNameStartChar(wstring &s);
bool isNameChar(wstring &s);
bool isNameStartChar(const wchar_t s);
bool isNameChar(const wchar_t s);
bool isWhiteSpace(wstring &s);
bool isWhiteSpace(const wchar_t s);



#endif // MAIN_H
