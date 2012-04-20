#define MAIN_CPP
#ifndef __HMAIN_CPP
#include "main.h"
#include "CDocument.h"
#include "CElement.h"
#include "CNode.h"
#include "CTextNode.h"
#include "CReader.h"
#include "CExplorer.h"
#endif

std::string WStringToString(const std::wstring& s)
{
std::string temp(s.length(), ' ');
std::copy(s.begin(), s.end(), temp.begin());
return temp;
}

std::wstring StringToWString(const std::string& s)
{
std::wstring temp(s.length(),L' ');
std::copy(s.begin(), s.end(), temp.begin());
return temp;
}

wstring Html$(wstring text)
{
    wstring str, s;
    for(unsigned int i = 0; i < text.length(); i++)
    {
        s = text.at(i);
        if(s == L"<")
        {
            str += L"&lt;";
        }
        else if(s == L">")
        {
            str += L"&gt;";
        }
        else if(s == L"&")
        {
            if((text.substr(i, 6) == L"&nbsp;"))
            {str+= L"&nbsp"; i +=5;} //On ignore les espaces
            else str += L"&amp;";
        }
        else if(s == L"\"")
        {
            str += L"&quot;";
        }
        else
        {
            str += s;
        }
    }

    return str;
}


ostream &operator<<( ostream &out, std::wstring &str )
{
    out << WStringToString(str);
    return out;
}

ostream &operator<<( ostream &out, std::wstring *str )
{
    out << *str;
    return out;
}

#ifndef __HMAIN_CPP

GB_INTERFACE GB EXPORT;

extern "C"{
GB_DESC *GB_CLASSES[] EXPORT =
{
  CDocumentDesc, CNodeDesc, CElementAttributesDesc, CElementDesc, CTextNodeDesc,
    CCommentNodeDesc, CCDATANodeDesc, CReaderDesc, CReaderNodeDesc, CReaderNodeTypeDesc,
    CReaderNodeAttributesDesc, CReaderReadFlagsDesc, CExplorerDesc, CExplorerReadFlagsDesc, 0
};

int EXPORT GB_INIT(void)
{

  return -1;
}

void EXPORT GB_EXIT()
{

}
}
#endif
