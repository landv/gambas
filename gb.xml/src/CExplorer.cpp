#define CEXPLORER_CPP
#include "CExplorer.h"

#undef THIS
#define THIS (static_cast<Explorer*>(_object))

BEGIN_METHOD(CExplorerReadFlags_get, GB_INTEGER flag)

int flag = VARG(flag);
if(flag > FLAGS_COUNT || flag < 0) return;
GB.ReturnBoolean(THIS->flags[flag]);

END_METHOD

BEGIN_METHOD(CExplorerReadFlags_put, GB_BOOLEAN value; GB_INTEGER flag)

int flag = VARG(flag);
if(flag > FLAGS_COUNT || flag < 0 || flag == READ_ERR_EOF) return;
THIS->flags[flag] = VARG(value);

END_METHOD

BEGIN_PROPERTY(CExplorer_Node)

GB.ReturnObject(THIS->curNode);

END_PROPERTY

BEGIN_METHOD_VOID(CExplorer_Read)

GB.ReturnInteger(THIS->Read());

END_METHOD

BEGIN_METHOD(CExplorer_new, GB_OBJECT doc)

THIS->Init();
if(!MISSING(doc))
{
    THIS->Load(VARGOBJ(Document, doc));
}

END_METHOD

BEGIN_METHOD_VOID(CExplorer_free)

THIS->Clear();

END_METHOD

BEGIN_METHOD(CExplorer_open, GB_STRING path)

Document *doc = GBI::New<Document>();
char *content; int len;

if(GB.LoadFile(CSTRING(path), LENGTH(path), &content, &len))
{
    GB.Error("Error loading file.");
    return;
}
try
{
    doc->setContent(StringToWString(std::string(content,len)));
    GB.ReleaseFile(content, len);
}
catch(HTMLParseException &e)
{
    GB.Error(e.what());
}
THIS->Load(doc);

END_METHOD

BEGIN_PROPERTY(CExplorer_eof)

GB.ReturnBoolean(THIS->eof);

END_PROPERTY

BEGIN_PROPERTY(CExplorer_state)

GB.ReturnInteger(THIS->state);

END_PROPERTY

GB_DESC CExplorerReadFlagsDesc[] =
{
    GB_DECLARE(".XmlExplorerReadFlags", 0), GB_VIRTUAL_CLASS(),

    GB_METHOD("_get", "b", CExplorerReadFlags_get, "(Flag)i"),
    GB_METHOD("_put", "b", CExplorerReadFlags_put, "(Value)b(Flag)i"),

    GB_END_DECLARE
};

GB_DESC CExplorerDesc[] =
{
    GB_DECLARE("XmlExplorer", sizeof(Explorer)),

    GB_METHOD("_new", "", CExplorer_new, "[(Document)XmlDocument]"),
    GB_METHOD("_free", "", CExplorer_free, ""),
    GB_PROPERTY_SELF("ReadFlags", ".XmlExplorerReadFlags"),
    GB_PROPERTY_READ("Node", "XmlNode", CExplorer_Node),
    GB_PROPERTY_READ("Eof", "b", CExplorer_eof),
    GB_PROPERTY_READ("State", "i", CExplorer_state),
    GB_METHOD("Read", "i", CExplorer_Read, ""),
    GB_METHOD("Open", "", CExplorer_open, "(Path)s"),

    GB_END_DECLARE
};
