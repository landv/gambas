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
#include "document.h"
#include "element.h"
#include "textnode.h"
#include "serializer.h"
#include <stdlib.h>

#define THISOBJ ((CNode*)_object)
#define THIS (static_cast<CNode*>(_object)->node)

#define NODE_BASE 0
#define NODE_ELEMENT 1
#define NODE_TEXT 2
#define NODE_COMMENT 3
#define NODE_CDATA 4
#define NODE_ATTRIBUTE 5
#define NODE_DOCUMENT 6

BEGIN_METHOD_VOID(CNode_new)

if(XMLNode_NoInstanciate()) return;


END_METHOD

BEGIN_METHOD_VOID(CNode_free)

XMLNode_DestroyGBObject(THIS);

END_METHOD

BEGIN_METHOD(CNode_tostring, GB_BOOLEAN indent)

    char *str = 0;
    size_t len = 0;
    
    GBserializeNode(THIS, str, len, VARG(indent) ? 0 : -1);
    
    GB.ReturnString(str);

END_METHOD

BEGIN_PROPERTY(CNode_type)

switch(THIS->type)
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

GB.ReturnBoolean(THIS->type == Node::ElementNode);

END_PROPERTY

BEGIN_PROPERTY(CNode_isText)

GB.ReturnBoolean(THIS->type == Node::NodeText);

END_PROPERTY

BEGIN_PROPERTY(CNode_isComment)

GB.ReturnBoolean(THIS->type == Node::Comment);

END_PROPERTY

BEGIN_PROPERTY(CNode_isCDATA)

GB.ReturnBoolean(THIS->type == Node::CDATA);

END_PROPERTY

BEGIN_PROPERTY(CNode_element)

XML_ReturnNode(THIS);

END_PROPERTY

BEGIN_PROPERTY(CNode_ownerDocument)

XML_ReturnNode((Node*)XMLNode_GetOwnerDocument((Node*)THIS));

END_PROPERTY

BEGIN_PROPERTY(CNode_parent)

XML_ReturnNode((Node*)(THIS->parent));

END_PROPERTY

BEGIN_PROPERTY(CNode_previous)

XML_ReturnNode((Node*)(THIS->previousNode));

END_PROPERTY

BEGIN_PROPERTY(CNode_next)

XML_ReturnNode((Node*)(THIS->nextNode));

END_PROPERTY

BEGIN_PROPERTY(CNode_textContent)


if(READ_PROPERTY)
{
    char *data; size_t len;
    GBGetXMLTextContent(THIS, data, len);
    GB.ReturnString(data);
}
else
{
    XMLNode_setTextContent(THIS, PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CNode_name)

if(!READ_PROPERTY)
{
    if(THIS->type == Node::ElementNode)
    {
        XMLElement_SetTagName((Element*)THIS, PSTRING(), PLENGTH());
    }
    return;
}

switch (THIS->type)
{
    case Node::ElementNode:
        GB.ReturnNewString(((Element*)THIS)->tagName, ((Element*)THIS)->lenTagName);break;
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

if(!SUPPORT_CHILDREN(THIS)) return;
Element *elmt = XMLElement_New(STRING(name), LENGTH(name));
if(!MISSING(value)) XMLElement_SetTextContent(elmt, STRING(value), LENGTH(value));
XMLNode_appendChild(THIS, elmt);

END_METHOD

BEGIN_METHOD(CNode_setAttribute, GB_STRING attr; GB_STRING val)

if(THIS->type != Node::ElementNode) return;

XMLElement_SetAttribute(((Element*)THIS), STRING(attr), LENGTH(attr),
                                STRING(val), LENGTH(val));

END_METHOD


BEGIN_METHOD(CElementAttributes_get, GB_STRING name)

if(THIS->type != Node::ElementNode) return;

Attribute *attr = XMLElement_GetAttribute((Element*)THIS, STRING(name), LENGTH(name));

if(attr)
{
    GB.ReturnNewString(attr->attrValue, attr->lenAttrValue);
}
else
{
    GB.ReturnNull();
}

END_METHOD

BEGIN_METHOD(CElementAttributes_put, GB_STRING value; GB_STRING name)

if(THIS->type != Node::ElementNode) return;

XMLElement_SetAttribute((Element*)THIS, STRING(name), LENGTH(name),
                                STRING(value), LENGTH(value));

END_METHOD

BEGIN_PROPERTY(CElementAttributes_count)

if(THIS->type != Node::ElementNode)
{
    GB.ReturnInteger(0);
    return;
}

if(READ_PROPERTY)
{
    GB.ReturnInteger(((Element*)THIS)->attributeCount);
}

END_PROPERTY

BEGIN_METHOD_VOID(CElementAttributes_next)

if(THIS->type != Node::ElementNode) {GB.StopEnum(); return;}

Attribute *attr = *reinterpret_cast<Attribute**>((GB.GetEnum()));
if(attr == 0)
{
    attr = ((Element*)THIS)->firstAttribute;
    *reinterpret_cast<Attribute**>(GB.GetEnum()) = attr;
}
else
{
    attr = (Attribute*)attr->nextNode;
    *reinterpret_cast<Attribute**>(GB.GetEnum()) = attr;
}

THISOBJ->curAttrEnum = attr;

if(attr == 0) {GB.StopEnum(); return;}


XML_ReturnNode(attr);


END_METHOD


BEGIN_PROPERTY(CElementAttributes_name)

if(!THISOBJ->curAttrEnum)
{
    GB.Error("No enumerated attribute available");
    GB.ReturnNull();
    return;
}

GB.ReturnNewString(THISOBJ->curAttrEnum->attrName, THISOBJ->curAttrEnum->lenAttrName);

END_PROPERTY

BEGIN_PROPERTY(CElementAttributes_value)

if(!THISOBJ->curAttrEnum)
{
    GB.Error("No enumerated attribute available");
    GB.ReturnNull();
    return;
}

GB.ReturnNewString(THISOBJ->curAttrEnum->attrValue, THISOBJ->curAttrEnum->lenAttrValue);

END_PROPERTY

BEGIN_PROPERTY(CNode_childNodes)

GB_ARRAY array;
XMLNode_getGBChildren(THIS, &array);

GB.ReturnObject(array);

END_PROPERTY

BEGIN_METHOD(CNode_getUserData, GB_STRING key)

GB_VARIANT *value = XMLNode_getUserData(THIS, STRING(key), LENGTH(key));

if(value) 
{
    GB.ReturnVariant(&(value->value));
    delete value;
}
else
{
    GB.ReturnNull();
}

END_METHOD

BEGIN_METHOD(CNode_setUserData, GB_STRING key; GB_VARIANT value)

XMLNode_addUserData(THIS, STRING(key), LENGTH(key), ARG(value));

END_METHOD

BEGIN_METHOD(CNode_escapeContent, GB_STRING data)

if(!LENGTH(data))
{
    GB.ReturnNull();
    return;
}

char *escapedData; size_t lenEscapedData;

XMLText_escapeContent(STRING(data), LENGTH(data), escapedData, lenEscapedData);

GB.ReturnNewString(escapedData, lenEscapedData);

if(escapedData != STRING(data)) free(escapedData);

END_METHOD

BEGIN_METHOD(CNode_unEscapeContent, GB_STRING data)

if(!LENGTH(data))
{
    GB.ReturnNull();
    return;
}

char *unescapedData; size_t lenUnEscapedData;

XMLText_unEscapeContent(STRING(data), LENGTH(data), unescapedData, lenUnEscapedData);

GB.ReturnNewString(unescapedData, lenUnEscapedData);

if(unescapedData != STRING(data)) free(unescapedData);

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
    GB_PROPERTY_READ("Name", "s", CElementAttributes_name),
    GB_PROPERTY_READ("Value", "s", CElementAttributes_value),
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
    GB_PROPERTY_READ("ChildNodes", "XmlNode[]", CNode_childNodes),
    GB_PROPERTY_READ("Children", "XmlNode[]", CNode_childNodes),
    GB_PROPERTY_READ("Parent", "XmlElement", CNode_parent),
    GB_PROPERTY_READ("Previous", "XmlNode", CNode_previous),
    GB_PROPERTY_READ("Next", "XmlNode", CNode_next),
    GB_PROPERTY_READ("PreviousSibling", "XmlNode", CNode_previous),
    GB_PROPERTY_READ("NextSibling", "XmlNode", CNode_next),
    
    GB_METHOD("ToString", "s", CNode_tostring, "[(Indent)b]"),
    GB_METHOD("GetUserData", "v", CNode_getUserData, "(Key)s"),
    GB_METHOD("SetUserData", "", CNode_setUserData, "(Key)s(Value)v"),
    GB_PROPERTY("TextContent", "s", CNode_textContent),
    GB_PROPERTY("Value", "s", CNode_textContent),
    GB_PROPERTY("Name", "s", CNode_name),

    GB_STATIC_METHOD("Serialize", "s", CNode_escapeContent, "(Data)s"),
    GB_STATIC_METHOD("Deserialize", "s", CNode_unEscapeContent, "(Data)s"),

    GB_METHOD("NewElement", "", CNode_newElement, "(Name)s[(Value)s]"),
    GB_METHOD("NewAttribute", "", CNode_setAttribute, "(Name)s(Value)s"),
    GB_PROPERTY_SELF("Attributes", ".XmlElementAttributes"),

    GB_END_DECLARE
};
