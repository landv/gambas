/***************************************************************************

  (c) 2012 Adrien Prokopowicz <prokopy@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include "CDocument.h"
#include "CNode.h"

#include "node.h"
#include "document.h"
#include "element.h"
#include "utils.h"
#include "serializer.h"

#define GBTHIS (static_cast<CDocument*>(_object))
#define THIS ((Document*)(static_cast<CDocument*>(_object)->node))

BEGIN_METHOD(CDocument_new, GB_STRING fileName)

if(XMLNode_NoInstanciate()) return;

bool isHtml = false;
if(GB.Is(_object, GB.FindClass("HtmlDocument")))//Called as inherited HtmlDocument constructor
{
    if(CheckHtmlInterface())
    {
        isHtml = true;
    }
};

try
{
    if(!MISSING(fileName))
    {
        if(isHtml)
        {
            GBTHIS->node = (Node*)XMLDocument_NewFromFile(STRING(fileName), LENGTH(fileName), HTMLDocumentType);
        }
        else
        {
            GBTHIS->node = (Node*)XMLDocument_NewFromFile(STRING(fileName), LENGTH(fileName));
        }
    }
    else
    {
        if(isHtml)
        {
            GBTHIS->node = (Node*)HTML.HtmlDocument_New();
        }
        else
        {
            GBTHIS->node = (Node*)XMLDocument_New();
        }
    }

    THIS->GBObject = (CDocument*)(_object);

}
catch(XMLParseException &e)
{
    GB.Error(e.errorWhat);
    GB.ReturnNull();
    XMLParseException_Cleanup(&e);
}

END_METHOD

BEGIN_METHOD_VOID(CDocument_free)

XMLNode_Free(GBTHIS->node);

END_METHOD

BEGIN_METHOD(CDocument_fromString, GB_STRING content)

try
{
    XMLDocument_SetContent(THIS, STRING(content), LENGTH(content));
}
catch(XMLParseException &e)
{
    GB.Error(e.errorWhat);
    XMLParseException_Cleanup(&e);
}

END_METHOD

BEGIN_METHOD(CDocument_tostring, GB_BOOLEAN indent)

    char *str = 0;
    size_t len = 0;
    GBserializeNode((Node*)THIS, str, len, VARG(indent) == -1 ? 0 : -1);
    
    GB.ReturnString(str);

END_METHOD

BEGIN_PROPERTY(CDocument_content)

if(READ_PROPERTY)
{
    char *str = 0;
    size_t len = 0;
    
    GBserializeNode((Node*)THIS, str, len);
    
    GB.ReturnString(str);
}
else
{
    try
    {
        XMLDocument_SetContent(THIS, PSTRING(), PLENGTH());
    }
    catch(XMLParseException &e)
    {
        GB.Error(e.errorWhat);
        XMLParseException_Cleanup(&e);
    }
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_root)

if(READ_PROPERTY)
{
    XML_ReturnNode((Node*)(THIS->root));
}
else
{
    XMLDocument_SetRoot(THIS, ((Element*)(VPROPOBJ(CNode)->node)));
}

END_PROPERTY

BEGIN_METHOD(CDocument_open, GB_STRING fileName)

try
{
    XMLDocument_Open(THIS, STRING(fileName), LENGTH(fileName));
}
catch(XMLParseException &e)
{
    GB.Error(e.errorWhat);
    XMLParseException_Cleanup(&e);
}

END_METHOD

BEGIN_METHOD(CDocument_save, GB_STRING fileName; GB_BOOLEAN indent)

XMLDocument_Save(THIS, GB.ToZeroString(ARG(fileName)), VARG(indent));

END_METHOD

BEGIN_METHOD(CDocument_getElementsByTagName, GB_STRING tagName; GB_INTEGER mode; GB_INTEGER depth;)

GB_ARRAY array;

XMLNode_getGBChildrenByTagName(THIS, STRING(tagName), LENGTH(tagName), &array, VARGOPT(mode, GB_STRCOMP_BINARY), VARGOPT(depth, -1));

GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CDocument_getElementsByNamespace, GB_STRING name; GB_INTEGER mode; GB_INTEGER depth;)

GB_ARRAY array;

XMLNode_getGBChildrenByTagName(THIS, STRING(name), LENGTH(name), &array, VARGOPT(mode, GB_STRCOMP_BINARY), VARGOPT(depth, -1));

GB.ReturnObject(array);

END_METHOD

BEGIN_PROPERTY(CDocument_getAll)

GB_ARRAY array;

XMLNode_getGBAllChildren(THIS, &array);

GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CDocument_createElement, GB_STRING tagName)

Element *elmt = XMLElement_New(STRING(tagName), LENGTH(tagName));

XML_ReturnNode(elmt);

END_METHOD

GB_DESC CDocumentDesc[] =
{
    GB_DECLARE("XmlDocument", sizeof(CDocument)),

    GB_METHOD("_new", "", CDocument_new, "[(FileName)s]"),
    GB_METHOD("_free", "", CDocument_free, ""),
    
    GB_METHOD("CreateElement", "XmlElement", CDocument_createElement, "(TagName)s"),
    GB_PROPERTY("Root", "XmlElement", CDocument_root),
    GB_PROPERTY_READ("All", "XmlNode[]", CDocument_getAll),
    GB_METHOD("GetElementsByTagName", "XmlElement[]", CDocument_getElementsByTagName, "(TagName)s[(Mode)i(Depth)i]"),
    GB_METHOD("GetElementsByNamespace", "XmlElement[]", CDocument_getElementsByNamespace, "(Namespace)s[(Mode)i(Depth)i]"),

    
    
    GB_PROPERTY("Content", "s", CDocument_content),
    GB_METHOD("FromString", "", CDocument_fromString, "(Data)s"),
    GB_METHOD("HtmlFromString", "", CDocument_fromString, "(Data)s"),
    GB_METHOD("ToString", "s", CDocument_tostring, "[(Indent)b]"),
    
    GB_METHOD("Open", "", CDocument_open, "(FileName)s"),
    GB_METHOD("Save", "", CDocument_save, "(FileName)s[(Indent)b]"),
    GB_METHOD("Write", "", CDocument_save, "(FileName)s[(Indent)b]"),
    

    GB_END_DECLARE
};
