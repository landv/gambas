#define GBI_CPP
#include "gbi.h"

fwstring::fwstring()
{
    data = 0;
    len = 0;
    cur = 0;
}

fwstring::fwstring(char *src, size_t &length) : data(src), len(length)
{

}

fwstring::fwstring(fwstring &other)
{

}

