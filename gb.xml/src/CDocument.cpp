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

#include "gbi.h"
#include "document.h"
#include "element.h"
#include "utils.h"

#define THIS (static_cast<CDocument*>(_object)->doc)

BEGIN_METHOD(CDocument_new, GB_STRING fileName)

if(Node::NoInstanciate) return;

if(!MISSING(fileName))
{
    THIS = new Document(STRING(fileName), LENGTH(fileName));
}
else
{
    THIS = new Document();
}
    
THIS->GBObject = (CDocument*)(_object);

END_METHOD

BEGIN_METHOD_VOID(CDocument_free)

delete THIS;

END_METHOD

BEGIN_METHOD(CDocument_fromString, GB_STRING content)

try
{
    THIS->setContent(STRING(content), LENGTH(content));
}
catch(XMLParseException &e)
{
    GB.Error(e.what());
}

END_METHOD

BEGIN_METHOD_VOID(CDocument_tostring)

    char *str = 0;
    size_t len = 0;
    
    THIS->toGBString(&str, &len);
    
    GB.ReturnString(str);

END_METHOD

BEGIN_PROPERTY(CDocument_content)

if(READ_PROPERTY)
{
    char *str = 0;
    size_t len = 0;
    
    THIS->toGBString(&str, &len);
    
    GB.ReturnString(str);
}
else
{
    try
    {
        THIS->setContent(PSTRING(), PLENGTH());
    }
    catch(XMLParseException &e)
    {
        GB.Error(e.what());
    }
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_root)

    GBI::Return((Node*)(THIS->root));

END_PROPERTY

BEGIN_METHOD(CDocument_open, GB_STRING fileName)

try
{
    THIS->Open(STRING(fileName), LENGTH(fileName));
}
catch(XMLParseException &e)
{
    GB.Error(e.what());
}

END_METHOD

BEGIN_METHOD(CDocument_save, GB_STRING fileName)

THIS->save(GB.ToZeroString(ARG(fileName)));

END_METHOD

BEGIN_METHOD(CDocument_getElementsByTagName, GB_STRING tagName; GB_INTEGER depth)

GB_ARRAY array;

THIS->getGBElementsByTagName(STRING(tagName), LENGTH(tagName), &array, VARGOPT(depth, -1));

GB.ReturnObject(array);

END_METHOD

BEGIN_PROPERTY(CDocument_getAll)

GB_ARRAY array;

THIS->getAllElements(&array);

GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CDocument_createElement, GB_STRING tagName)

Element *elmt = new Element(STRING(tagName), LENGTH(tagName));

GBI::Return(elmt);

END_METHOD

GB_DESC CDocumentDesc[] =
{
    GB_DECLARE("XmlDocument", sizeof(CDocument)),

    GB_METHOD("_new", "", CDocument_new, "[(FileName)s]"),
    GB_METHOD("_free", "", CDocument_free, ""),
    
    GB_METHOD("CreateElement", "XmlElement", CDocument_createElement, "(TagName)s"),
    GB_PROPERTY_READ("Root", "XmlElement", CDocument_root),
    GB_PROPERTY_READ("All", "XmlElement[]", CDocument_getAll),
    GB_METHOD("GetElementsByTagName", "XmlElement[]", CDocument_getElementsByTagName, "(TagName)s[(Depth)i]"),
    
    
    
    GB_PROPERTY("Content", "s", CDocument_content),
    GB_METHOD("FromString", "", CDocument_fromString, "(Data)s"),
    GB_METHOD("HtmlFromString", "", CDocument_fromString, "(Data)s"),
    GB_METHOD("ToString", "s", CDocument_tostring, ""),
    
    GB_METHOD("Open", "", CDocument_open, "(FileName)s"),
    GB_METHOD("Save", "", CDocument_save, "(FileName)s"),
    GB_METHOD("Write", "", CDocument_save, "(FileName)s"),
    

    GB_END_DECLARE
};
