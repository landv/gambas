#include "CDocument.h"


/*========== Document */

#undef THIS
#define THIS (static_cast<Document*>(_object))

BEGIN_METHOD(CDocument_new, GB_STRING path)

if(!MISSING(path))
{
    char *content; int len;

    if(GB.LoadFile(CSTRING(path), LENGTH(path), &content, &len))
    {
        GB.Error("Error loading file.");
        return;
    }
    try
    {
        THIS->setContent(StringToWString(std::string(content,len)));
        GB.ReleaseFile(content, len);
    }
    catch(HTMLParseException &e)
    {
        GB.Error(e.what());
    }
}
else
{
Element *root = GBI::New<Element>();
GB.Ref(root);
root->setTagName(L"xml");
THIS->setRoot(root);


}
THIS->virt = new Document::Virtual(THIS);

END_METHOD

BEGIN_METHOD(CDocument_open, GB_STRING path)

    char *content; int len;
    if(GB.LoadFile(CSTRING(path), LENGTH(path), &content, &len)) return;
    try
    {
        THIS->setContent(StringToWString(std::string(content,len)));
    }
    catch(HTMLParseException &e)
    {
        GB.Error(e.what());
    }

END_METHOD

BEGIN_METHOD_VOID(CDocument_free)

GB.Unref(POINTER(&(THIS->root)));

END_METHOD

BEGIN_PROPERTY(CDocument_content)

if(READ_PROPERTY)
{
    GB.ReturnNewZeroString(WStringToString(THIS->getContent()).c_str());
}
else
{
if(PLENGTH() <= 0) return;
    try
    {
        THIS->setContent(PSTRING());
    }
    catch(HTMLParseException &e)
    {
        GB.Error(e.what());
    }
}

END_PROPERTY

BEGIN_METHOD(CDocument_fromString, GB_STRING data)

    if(LENGTH(data) <= 0) return;
    try
    {
        THIS->setContent(STRING(data));
    }
    catch(HTMLParseException &e)
    {
        GB.Error(e.what());
    }

END_METHOD

BEGIN_METHOD(CDocument_toString, GB_BOOLEAN indent)

    GB.ReturnNewZeroString(WStringToString(THIS->getContent(VARGOPT(indent, false))).c_str());

END_METHOD

BEGIN_PROPERTY(CDocument_root)

GB.ReturnObject(THIS->getRoot());

END_PROPERTY

BEGIN_METHOD(CDocument_createElement, GB_STRING tagName)

if(LENGTH(tagName) <= 0) return;
GB.ReturnObject(THIS->createElement(MISSING(tagName) ? L"" : STRING(tagName)));

END_METHOD

BEGIN_PROPERTY(CDocument_All)

GB.ReturnObject(THIS->getAll()->array);

END_PROPERTY


BEGIN_METHOD(CDocument_getElementsByTagName, GB_STRING className; GB_INTEGER depth)

GB.ReturnObject(THIS->getGBElementsByTagName(STRING(className), VARGOPT(depth, -1))->array);

END_METHOD

BEGIN_METHOD(CDocument_save, GB_STRING fileName)

THIS->save(STRING(fileName));

END_METHOD



GB_DESC CDocumentDesc[] =
{
    GB_DECLARE("XmlDocument", sizeof(Document)),

    GB_METHOD("_new", "", CDocument_new, "[(Path)s]"),
    GB_METHOD("_free", "", CDocument_free, ""),

    GB_PROPERTY("Content", "s", CDocument_content),
    GB_PROPERTY_READ("Root", "XmlElement", CDocument_root),

    GB_METHOD("GetElementsByTagName", "XmlElement[]", CDocument_getElementsByTagName, "(TagName)s[(Depth)i]"),
    GB_METHOD("CreateElement", "XmlElement", CDocument_createElement, "[(TagName)s]"),
    GB_METHOD("Open", "", CDocument_open, "(Path)s"),
    GB_METHOD("FromString", "", CDocument_fromString, "(Data)s"),
    GB_METHOD("HtmlFromString", "", CDocument_fromString, "(Data)s"),
    GB_METHOD("ToString", "s", CDocument_toString, "[(Indent)b]"),
    GB_METHOD("Save", "", CDocument_save, "(FileName)s"),
    GB_METHOD("Write", "", CDocument_save, "(FileName)s"),

    GB_PROPERTY("All", "XmlNode[]", CDocument_All),


    GB_END_DECLARE
};
