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

std::wstring StringToWString(const std::string& s);

ostream &operator<<( ostream &out, std::wstring &str );
ostream &operator<<( ostream &out, std::wstring *str );

wstring Html$(wstring text);

#ifndef __MAIN_CPP
extern "C" GB_INTERFACE GB;
#endif

#undef STRING
#define STRING(_arg) (CSTRING(_arg) != 0 ? StringToWString(string(CSTRING(_arg), VARG(_arg).len)) : wstring())
#define CSTRING(_arg) (VARG(_arg).addr + VARG(_arg).start)

#undef PSTRING
#define PSTRING(_arg) (CPSTRING(_arg) != 0 ? StringToWString(string(CPSTRING(_arg), VPROP(GB_STRING).len)) : wstring())
#define CPSTRING(_arg) (VPROP(GB_STRING).addr + VPROP(GB_STRING).start)

#define STRINGOPT(arg, def) (MISSING(arg) ? def : STRING(arg))

#define VARGOBJ(type, arg) reinterpret_cast<type*>(VARG(arg))



#endif // MAIN_H
