#include "CWriter.h"

#undef THIS
#define THIS (static_cast<Writer*>(_object)) 

inline void WriteData(wstring &data, void *ob)
{
    GB_FUNCTION *writeFunc = new GB_FUNCTION;
    
    GB.GetFunction(writeFunc, ob, "_Write", "(Data)s", "");
    string str = WStringToString(data);
    GB.Push(1, GB_T_STRING, str.c_str(), str.length());
    GB.Call(writeFunc, 1, 1);
}

BEGIN_METHOD_VOID(CWriter_new)


END_METHOD



BEGIN_METHOD_VOID(CWriter_free)

END_METHOD

BEGIN_METHOD(CWriter_write, GB_STRING data)

std::cout << "Write !";

END_METHOD

BEGIN_METHOD(CWriter_element, GB_STRING tagName; GB_STRING content)

wstring data = L"<" + STRING(tagName) + L">" + STRING(content) + L"</" + STRING(tagName) + L">";
WriteData(data, THIS);

END_METHOD

BEGIN_PROPERTY(CWriter_stream)

if(READ_PROPERTY)
{
    GB.ReturnObject(THIS->stream);   
}
else
{
    THIS->stream = PROP(GB_STREAM);
}

END_PROPERTY

GB_DESC CWriterDesc[] =
{
    GB_DECLARE("XmlWriter", sizeof(Writer)),

    GB_METHOD("_new", "", CWriter_new, ""),
    GB_METHOD("_free", "", CWriter_free, ""),
    
    GB_PROPERTY("OutputStream", "Stream", )

    GB_METHOD("_Write", "", CWriter_write, "(Data)s"),

    GB_METHOD("Element", "", CWriter_element, "(TagName)s[(Content)s]"), 

    GB_END_DECLARE
};
