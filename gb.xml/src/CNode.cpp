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

#include "CNode.h"
#include "node.h"
#include "gbi.h"
#include "document.h"
#include "element.h"

#define THIS (static_cast<CNode*>(_object)->node)

#define NODE_BASE 0
#define NODE_ELEMENT 1
#define NODE_TEXT 2
#define NODE_COMMENT 3
#define NODE_CDATA 4
#define NODE_ATTRIBUTE 5

BEGIN_METHOD_VOID(CNode_new)
if(Node::NoInstanciate) return;


END_METHOD

BEGIN_METHOD_VOID(CNode_free)

THIS->DestroyGBObject();

END_METHOD

BEGIN_METHOD_VOID(CNode_tostring)

    char *str = 0;
    size_t len = 0;
    
    THIS->toGBString(str, len);
    
    GB.ReturnString(str);

END_METHOD

BEGIN_PROPERTY(CNode_type)

switch(THIS->getType())
{
    case Node::ElementNode:
        GB.ReturnInteger(NODE_ELEMENT);break;
    case Node::Comment:
        GB.ReturnInteger(NODE_COMMENT);break;
    case Node::NodeText:
        GB.ReturnInteger(NODE_TEXT);break;
    case Node::CDATA:
        GB.ReturnInteger(NODE_CDATA);break;
    default:
        GB.ReturnInteger(NODE_BASE);
}

END_PROPERTY

BEGIN_PROPERTY(CNode_isElement)

GB.ReturnBoolean(THIS->isElement());

END_PROPERTY

BEGIN_PROPERTY(CNode_isText)

GB.ReturnBoolean(THIS->isTextNode());

END_PROPERTY

BEGIN_PROPERTY(CNode_isComment)

GB.ReturnBoolean(THIS->getType() == Node::Comment);

END_PROPERTY

BEGIN_PROPERTY(CNode_isCDATA)

GB.ReturnBoolean(THIS->getType() == Node::CDATA);

END_PROPERTY

BEGIN_PROPERTY(CNode_element)

GBI::Return((Node*)(THIS->toElement()));

END_PROPERTY

BEGIN_PROPERTY(CNode_ownerDocument)

GBI::Return(THIS->GetOwnerDocument());

END_PROPERTY

BEGIN_PROPERTY(CNode_parent)

GBI::Return((Node*)(THIS->parent));

END_PROPERTY

BEGIN_PROPERTY(CNode_previous)

GBI::Return((Node*)(THIS->previousNode));

END_PROPERTY

BEGIN_PROPERTY(CNode_next)

GBI::Return((Node*)(THIS->nextNode));

END_PROPERTY

BEGIN_PROPERTY(CNode_textContent)


if(READ_PROPERTY)
{
    char *data; size_t len;
    THIS->GBTextContent(data, len);
    GB.ReturnString(data);
}
else
{
    THIS->setTextContent(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CNode_name)

if(!READ_PROPERTY)
{
    if(THIS->isElement())
    {
        THIS->toElement()->setTagName(PSTRING(), PLENGTH());
    }
    return;
}

switch (THIS->getType())
{
    case Node::ElementNode:
        GB.ReturnNewString(THIS->toElement()->tagName, THIS->toElement()->lenTagName);break;
    case Node::NodeText:
        GB.ReturnNewZeroString("#text");break;
    case Node::Comment:
        GB.ReturnNewZeroString("#comment");break;
    case Node::CDATA:
        GB.ReturnNewZeroString("#cdata");break;
    case Node::AttributeNode:
        GB.ReturnNewString(((Attribute*)THIS)->attrName, ((Attribute*)THIS)->lenAttrName);break;
    default:
        GB.ReturnNewZeroString("");
}

END_PROPERTY


BEGIN_METHOD(CNode_newElement, GB_STRING name; GB_STRING value)

if(!THIS->isElement()) return;
Element *elmt = new Element(STRING(name), LENGTH(name));
if(!MISSING(value)) elmt->setTextContent(STRING(value), LENGTH(value));
THIS->toElement()->appendChild(elmt);

END_METHOD

BEGIN_METHOD(CNode_setAttribute, GB_STRING attr; GB_STRING val)

if(!THIS->isElement()) return;
THIS->toElement()->setAttribute(STRING(attr), LENGTH(attr),
                                STRING(val), LENGTH(val));

END_METHOD


BEGIN_METHOD(CElementAttributes_get, GB_STRING name)

if(!THIS->isElement()) return;

Attribute *attr = THIS->toElement()->getAttribute(STRING(name), LENGTH(name));

GB.ReturnNewString(attr->attrValue, attr->lenAttrValue);

END_METHOD

BEGIN_METHOD(CElementAttributes_put, GB_STRING value; GB_STRING name)

if(!THIS->isElement()) return;

THIS->toElement()->setAttribute(STRING(name), LENGTH(name),
                                STRING(value), LENGTH(value));

END_METHOD

BEGIN_PROPERTY(CElementAttributes_count)

if(!THIS->isElement()) 
{
    GB.ReturnInteger(0);
    return;
}

if(READ_PROPERTY)
{
    GB.ReturnInteger(THIS->toElement()->attributeCount);
}

END_PROPERTY

BEGIN_METHOD_VOID(CElementAttributes_next)

if(!THIS->isElement()) {GB.StopEnum(); return;}

Attribute *attr = *reinterpret_cast<Attribute**>((GB.GetEnum()));
if(attr == 0)
{
    attr = THIS->toElement()->firstAttribute;
    *reinterpret_cast<Attribute**>(GB.GetEnum()) = attr;
}
else
{
    attr = (Attribute*)attr->nextNode;
    *reinterpret_cast<Attribute**>(GB.GetEnum()) = attr;
}

if(attr == 0) {GB.StopEnum(); return;}

GBI::Return(attr);


END_METHOD

GB_DESC CElementAttributeNodeDesc[] =
{
    GB_DECLARE("_XmlAttrNode", sizeof(CNode)), GB_INHERITS("XmlNode"),

    GB_END_DECLARE
};

GB_DESC CElementAttributesDesc[] =
{
    GB_DECLARE(".XmlElementAttributes", 0), GB_VIRTUAL_CLASS(),
    GB_METHOD("_get", "s", CElementAttributes_get, "(Name)s"),
    GB_METHOD("_put", "s", CElementAttributes_put, "(Value)s(Name)s"),
    GB_METHOD("_next", "XmlNode", CElementAttributes_next, ""),
    GB_PROPERTY_READ("Count", "i", CElementAttributes_count),
    GB_END_DECLARE
};

GB_DESC CNodeDesc[] =
{
    GB_DECLARE("XmlNode", sizeof(CNode)), GB_NOT_CREATABLE(),

    GB_METHOD("_new", "", CNode_new, ""),
    GB_METHOD("_free", "", CNode_free, ""),
    
    GB_CONSTANT("ElementNode", "i", NODE_ELEMENT),
    GB_CONSTANT("TextNode", "i", NODE_TEXT),
    GB_CONSTANT("CommentNode", "i", NODE_COMMENT),
    GB_CONSTANT("CDATASectionNode", "i", NODE_CDATA),
    GB_CONSTANT("AttributeNode", "i", NODE_ATTRIBUTE),

    GB_PROPERTY_READ("Type", "i", CNode_type),
    GB_PROPERTY_READ("IsElement", "b", CNode_isElement),
    GB_PROPERTY_READ("IsText", "b", CNode_isText),
    GB_PROPERTY_READ("IsComment", "b", CNode_isComment),
    GB_PROPERTY_READ("IsCDATA", "b", CNode_isCDATA),
    
    GB_PROPERTY_READ("Element", "XmlElement", CNode_element),
    GB_PROPERTY_READ("OwnerDocument", "XmlDocument", CNode_ownerDocument),
    GB_PROPERTY_READ("Parent", "XmlElement", CNode_parent),
    GB_PROPERTY_READ("Previous", "XmlNode", CNode_previous),
    GB_PROPERTY_READ("Next", "XmlNode", CNode_next),
    
    GB_METHOD("ToString", "s", CNode_tostring, "[(Indent)b]"),
    GB_PROPERTY("TextContent", "s", CNode_textContent),
    GB_PROPERTY("Value", "s", CNode_textContent),
    GB_PROPERTY("Name", "s", CNode_name),
    
    //Obsolete methods
    GB_METHOD("NewElement", "", CNode_newElement, "(Name)s[(Value)s]"),
    GB_METHOD("NewAttribute", "", CNode_setAttribute, "(Name)s(Value)s"),
    GB_PROPERTY_SELF("Attributes", ".XmlElementAttributes"),

    GB_END_DECLARE
};
