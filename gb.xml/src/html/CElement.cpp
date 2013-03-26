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

#include "../CElement.h"
#include "helement.h"
#include "../gbi.h"

/*========== Element */

#undef THIS
#undef THISNODE
#define THIS (static_cast<CNode*>(_object)->node->toElement())
#define THISNODE (static_cast<CNode*>(_object)->node)

BEGIN_METHOD_VOID(CElement_new)

if(Node::NoInstanciate) 
{
    delete THISNODE;
}

END_METHOD

BEGIN_PROPERTY(CElement_id)

if(READ_PROPERTY)
{
    Attribute *id = THIS->getId();
    if(id)
    {
        GB.ReturnNewString(id->attrValue, id->lenAttrValue);
    }
    else
    {
        GB.ReturnNull();
    }
}
else
{
THIS->setId(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CElement_className)

if(READ_PROPERTY) 
{
    Attribute *className = THIS->getClassName();
    if(className)
    {
        GB.ReturnNewString(className->attrValue, className->lenAttrValue);
    }
    else
    {
        GB.ReturnNull();
    }
}
else 
{
THIS->setClassName(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_METHOD(CElement_matchFilter, GB_STRING filter)

GB.ReturnBoolean(THIS->matchFilter(STRING(filter), LENGTH(filter)));

END_METHOD

BEGIN_METHOD(CElement_getChildrenByFilter, GB_STRING filter; GB_INTEGER depth)

GB_ARRAY array;

THIS->getGBChildrenByFilter(STRING(filter), LENGTH(filter), &array, VARGOPT(depth, -1));

GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CElement_getChildById, GB_STRING id; GB_INTEGER depth)

GBI::Return(THIS->getChildById(STRING(id), LENGTH(id), VARGOPT(depth, -1)));

END_METHOD

BEGIN_METHOD(CElement_getChildrenByClassName, GB_STRING className; GB_INTEGER depth)

    GB_ARRAY array;

    THIS->getGBChildrenByClassName(STRING(className), LENGTH(className), &array, VARGOPT(depth, -1));

    GB.ReturnObject(array);

END_METHOD

GB_DESC CElementDesc[] =
{
    GB_DECLARE("XmlElement", sizeof(CNode)),
    
    GB_METHOD("_new", "", CElement_new, ""),

    GB_PROPERTY("Id", "s", CElement_id),
    GB_PROPERTY("ClassName", "s", CElement_className),

    GB_METHOD("MatchFilter", "b", CElement_matchFilter, "(Filter)s"),
    GB_METHOD("GetChildrenByFilter", "XmlElement[]", CElement_getChildrenByFilter, "(Filter)s[(Depth)i]"),

    GB_METHOD("GetChildById", "XmlElement", CElement_getChildById, "(Id)s[(Depth)i]"),
    GB_METHOD("GetChildrenByClassName", "XmlElement[]", CElement_getChildrenByClassName, "(ClassName)s[(Depth)i]"),



    GB_END_DECLARE
};
