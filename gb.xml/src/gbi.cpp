#define GBI_CPP
#include "gbi.h"

#include "element.h"
#include "document.h"

void GBI::InitClasses()
{
}

void GBI::Return(fwstring str)
{
    if(!str.len)
    {
        GB.ReturnNull();
        return;
    }
    GB.ReturnNewString(str.data, str.len);
    //DEBUG << str.toStdString() << endl;
    //GB.ReturnNewZeroString(str.data);
}

void GBI::Return(Node *ob)
{
    //DEBUG << ob << " " << ob->GetGBObject() << endl;
    if(!(ob->GetGBObject()))
    {
        ob->NewGBObject();
        GB.Ref(ob->GetGBObject());
    }

    GB.ReturnObject(ob->GetGBObject());
}

void GBI::Return(Document *ob)
{

    if(!ob->relob)
    {
        ob->NewGBObject();
    }

    GB.ReturnObject(ob->relob);
}
