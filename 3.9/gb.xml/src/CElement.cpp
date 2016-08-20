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

#include "CElement.h"

#include "node.h"
#include "element.h"
#include "textnode.h"
#include "parser.h"


#include <stdlib.h>

#define THIS ((Element*)(static_cast<CNode*>(_object)->node))
#define THISNODE (static_cast<CNode*>(_object)->node)

BEGIN_METHOD(CElement_new, GB_STRING tagName)

if(XMLNode_NoInstanciate()) return;
if(!MISSING(tagName))
{
    THISNODE = XMLElement_New(STRING(tagName), LENGTH(tagName));
}
else
{
THISNODE = XMLElement_New();
}
    THIS->GBObject = static_cast<CNode*>(_object);

END_METHOD

BEGIN_PROPERTY(CElement_tagName)

if(READ_PROPERTY)
{
    if(!THIS->tagName || !THIS->lenTagName)
    {
        GB.ReturnNull();
        return;
    }
    else
    {
        GB.ReturnNewString(THIS->tagName, THIS->lenTagName);
    }
}
else
{
    XMLElement_SetTagName(THIS, PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CElement_prefix)

if(READ_PROPERTY)
{
    if(!THIS->prefix || !THIS->lenPrefix)
    {
        GB.ReturnNull();
        return;
    }
    GB.ReturnNewString(THIS->prefix, THIS->lenPrefix);
}
else
{
    XMLElement_SetPrefix(THIS, PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_METHOD(CElement_appendChild, GB_OBJECT newChild)

    if(!VARGOBJ(CNode, newChild))
    {
        GB.Error("Null object");
        return;
    }
    XMLNode_appendChild(THIS, VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_getAttribute, GB_STRING attrName; GB_INTEGER mode)

Attribute *attr = XMLElement_GetAttribute(THIS, STRING(attrName), LENGTH(attrName), VARG(mode));
    if(attr)
    {
        GB.ReturnNewString(attr->attrValue, attr->lenAttrValue);
    }
    else
    {
        GB.ReturnNull();
    }

END_METHOD

BEGIN_METHOD(CElement_removeAttribute, GB_STRING attrName)

    XMLElement_RemoveAttribute(THIS, STRING(attrName), LENGTH(attrName));

END_METHOD

BEGIN_METHOD(CElement_setAttribute, GB_STRING attrName; GB_STRING attrValue)

    XMLElement_SetAttribute(THIS, STRING(attrName), LENGTH(attrName),
                       STRING(attrValue), LENGTH(attrValue));

END_METHOD

BEGIN_METHOD(CElement_appendFromText, GB_STRING data; GB_VALUE param[0])

try
{
    if(GB.NParam() > 0)
    {
        XMLNode_substAppendFromText(THIS, STRING(data), LENGTH(data), ARG(param[0]), GB.NParam());
    }
    else
    {
        XMLNode_appendFromText(THIS, STRING(data), LENGTH(data));
    }
}
catch(XMLParseException &e)
{
    GB.Error(e.errorWhat);
    XMLParseException_Cleanup(&e);
}
END_METHOD

BEGIN_METHOD(CElement_appendChildren, GB_OBJECT children)

GB_ARRAY array = reinterpret_cast<GB_ARRAY>(VARG(children));

for(int i = 0; i < GB.Array.Count(array); i++)
{
    XMLNode_appendChild(THIS, (*(reinterpret_cast<CNode**>(GB.Array.Get(array, i))))->node);
}

END_METHOD

BEGIN_METHOD(CElement_prependChild, GB_OBJECT newChild)

XMLNode_prependChild(THIS, VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_removeChild, GB_OBJECT oldChild)

XMLNode_removeChild(THIS, VARGOBJ(CNode, oldChild)->node);

END_METHOD

BEGIN_METHOD(CElement_replaceChild, GB_OBJECT oldChild; GB_OBJECT newChild)

XMLNode_replaceChild(THIS, VARGOBJ(CNode, oldChild)->node,
                        VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_insertAfter, GB_OBJECT child; GB_OBJECT newChild)

XMLNode_insertAfter(THIS, VARGOBJ(CNode, child)->node, VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_insertBefore, GB_OBJECT child; GB_OBJECT newChild)

XMLNode_insertBefore(THIS, VARGOBJ(CNode, child)->node, VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_appendText, GB_STRING data)

XMLNode_appendText(THIS, STRING(data), LENGTH(data));

END_METHOD

BEGIN_METHOD_VOID(CElement_clearChildren)

XMLNode_clearChildren(THIS);

END_METHOD

BEGIN_PROPERTY(CElement_previousElement)

XML_ReturnNode(XMLNode_previousElement(THIS));

END_PROPERTY

BEGIN_PROPERTY(CElement_nextElement)

XML_ReturnNode(XMLNode_nextElement(THIS));

END_PROPERTY

BEGIN_METHOD(CElement_isAttributeSet, GB_STRING name)

GB.ReturnBoolean((bool)(XMLElement_GetAttribute(THIS, STRING(name), LENGTH(name))));

END_METHOD

BEGIN_PROPERTY(CElement_allChildNodes)

GB_ARRAY array;
XMLNode_getGBAllChildren(THIS, &array);

GB.ReturnObject(array);

END_PROPERTY

BEGIN_PROPERTY(CElement_firstChildElement)

XML_ReturnNode(XMLNode_firstChildElement(THIS));

END_PROPERTY

BEGIN_PROPERTY(CElement_lastChildElement)

XML_ReturnNode(XMLNode_lastChildElement(THIS));

END_PROPERTY

BEGIN_PROPERTY(CElement_childElements)

GB_ARRAY array;
XMLNode_getGBChildElements(THIS, &array);
GB.ReturnObject(array);

END_PROPERTY

BEGIN_METHOD(CElement_fromText, GB_STRING data)

GB_ARRAY array;
try
{
    GBparseXML(STRING(data), LENGTH(data), &array);
}
catch(XMLParseException &e)
{
    GB.Error(e.errorWhat);
    XMLParseException_Cleanup(&e);
}
GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CElement_getChildrenByTagName, GB_STRING tagName; GB_INTEGER mode ; GB_INTEGER depth;)

GB_ARRAY array;
XMLNode_getGBChildrenByTagName(THIS, STRING(tagName), LENGTH(tagName), &array, VARGOPT(mode, GB_STRCOMP_BINARY), VARGOPT(depth, -1));
GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CElement_getChildrenByNamespace, GB_STRING name; GB_INTEGER mode ; GB_INTEGER depth;)

GB_ARRAY array;
XMLNode_getGBChildrenByNamespace(THIS, STRING(name), LENGTH(name), &array, VARGOPT(mode, GB_STRCOMP_BINARY), VARGOPT(depth, -1));
GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CElement_getChildrenByAttributeValue, GB_STRING name; GB_STRING value; GB_INTEGER mode ; GB_INTEGER depth;)

GB_ARRAY array;
XMLNode_getGBChildrenByAttributeValue(THIS, STRING(name), LENGTH(name),
                                    STRING(value), LENGTH(value),
                                    &array, VARGOPT(mode, GB_STRCOMP_BINARY), VARGOPT(depth, -1));
GB.ReturnObject(array);

END_METHOD

BEGIN_PROPERTY(CElement_firstChild)

XML_ReturnNode(THIS->firstChild);

END_METHOD

BEGIN_PROPERTY(CElement_lastChild)

XML_ReturnNode(THIS->lastChild);

END_PROPERTY

BEGIN_METHOD(CElement_normalizeAttributeContent, GB_STRING data)

if(!LENGTH(data))
{
    GB.ReturnNull();
    return;
}

char *escapedData; size_t lenEscapedData;

XMLText_escapeAttributeContent(STRING(data), LENGTH(data), escapedData, lenEscapedData);

GB.ReturnNewString(escapedData, lenEscapedData);

if(escapedData != STRING(data)) free(escapedData);

END_METHOD



GB_DESC CElementDesc[] =
{
    GB_DECLARE("XmlElement", sizeof(CNode)), GB_INHERITS("XmlNode"),
    
    GB_METHOD("_new", "", CElement_new, "[(TagName)s]"),
    
    GB_METHOD("AppendChild", "", CElement_appendChild, "(NewChild)XmlNode"),
    GB_METHOD("AppendChildren", "", CElement_appendChildren, "(NewChildren)XmlNode[]"),
    GB_METHOD("PrependChild", "", CElement_prependChild, "(NewChild)XmlNode"),
    GB_METHOD("InsertAfter", "", CElement_insertAfter, "(Child)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("InsertBefore", "", CElement_insertBefore, "(Child)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("RemoveChild", "", CElement_removeChild, "(OldChild)XmlNode"),
    GB_METHOD("ReplaceChild", "", CElement_replaceChild, "(OldChild)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("ClearChildren", "", CElement_clearChildren, ""),
    
    GB_METHOD("AppendText", "", CElement_appendText, "(Data)s"),
    GB_METHOD("AppendFromText", "", CElement_appendFromText, "(Data)s(Arguments)."),
    
    GB_METHOD("GetAttribute", "s", CElement_getAttribute, "(Name)s[(Mode)i]"),
    GB_METHOD("RemoveAttribute", "s", CElement_removeAttribute, "(Name)s"),
    GB_METHOD("SetAttribute", "", CElement_setAttribute, "(Name)s(Value)s"),
    
    GB_METHOD("IsAttributeSet", "b", CElement_isAttributeSet, "(Name)s"),
    
    GB_PROPERTY_READ("ChildElements", "XmlElement[]", CElement_childElements),
    GB_PROPERTY_READ("AllChildNodes", "XmlNode[]", CElement_allChildNodes),
    GB_PROPERTY_READ("FirstChild", "XmlNode", CElement_firstChild),
    GB_PROPERTY_READ("LastChild", "XmlNode", CElement_lastChild),
    GB_PROPERTY_READ("FirstChildElement", "XmlElement", CElement_firstChildElement),
    GB_PROPERTY_READ("LastChildElement", "XmlElement", CElement_lastChildElement),
    
    GB_PROPERTY("TagName", "s", CElement_tagName),
    GB_PROPERTY("Prefix", "s", CElement_prefix),
    GB_PROPERTY("PreviousElement", "XmlElement", CElement_previousElement),
    GB_PROPERTY("NextElement", "XmlElement", CElement_nextElement),

    GB_METHOD("GetChildrenByNamespace", "XmlElement[]", CElement_getChildrenByNamespace, "(Namespace)s[(Mode)i(Depth)i]"),
    GB_METHOD("GetChildrenByTagName", "XmlElement[]", CElement_getChildrenByTagName, "(TagName)s[(Mode)i(Depth)i]"),
    GB_METHOD("GetChildrenByAttributeValue", "XmlElement[]", CElement_getChildrenByAttributeValue, "(Attribute)s(Value)s[(Mode)i(Depth)i]"),
    
    GB_STATIC_METHOD("FromText", "XmlNode[]", CElement_fromText, "(Data)s"),
    GB_STATIC_METHOD("NormalizeAttributeContent", "s", CElement_normalizeAttributeContent, "(Data)s"),
    

    GB_END_DECLARE
};

/*
GB_DESC CElementDesc[] =
{
    GB_DECLARE("XmlElement", sizeof(CElement)), GB_INHERITS("XmlNode"),
    GB_METHOD("_new", "", CElement_new, "[(TagName)s]"),
    GB_METHOD("_free", "", CElement_free, ""),
    GB_METHOD("AppendChild", "", CElement_AppendChild, "(NewChild)XmlNode"),
    GB_METHOD("AppendChildren", "", CElement_AppendChildren, "(NewChildren)XmlNode[]"),
    GB_METHOD("PrependChild", "", CElement_prependChild, "(NewChild)XmlNode"),
    GB_METHOD("InsertAfter", "", CElement_insertAfter, "(Child)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("InsertBefore", "", CElement_insertBefore, "(Child)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("AppendText", "", CElement_AppendText, "(NewText)s"),
    GB_METHOD("AppendFromText", "", CElement_appendFromText, "(Data)s"),
    GB_METHOD("RemoveChild", "", CElement_removeChild, "(OldChild)XmlNode"),
    GB_METHOD("ReplaceChild", "", CElement_replaceChild, "(OldChild)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("NewElement", "", CElement_newElement, "(Name)s[(Value)s]"),

    GB_PROPERTY("TagName", "s", CElement_tagName),

    GB_PROPERTY_READ("PreviousSibling", "XmlElement", CElement_previousSibling),
    GB_PROPERTY_READ("NextSibling", "XmlElement", CElement_nextSibling),

    GB_METHOD("GetAttribute", "s", CElement_getAttribute, "(Name)s"),
    GB_METHOD("SetAttribute", "", CElement_setAttribute, "(Name)s(Value)s"),
    GB_METHOD("NewAttribute", "", CElement_setAttribute, "(Name)s(Value)s"),
    GB_METHOD("IsAttributeSet", "b", CElement_isAttributeSet, "(Name)s"),

    GB_PROPERTY_READ("ChildNodes", "XmlNode[]", CElement_childNodes),
    GB_PROPERTY_READ("Children", "XmlNode[]", CElement_childNodes),
    GB_PROPERTY_READ("ChildElements", "XmlElement[]", CElement_childElements),
    GB_PROPERTY_READ("AllChildNodes", "XmlNode[]", CElement_allChildNodes),
    GB_PROPERTY_READ("FirstChildElement", "XmlElement", CElement_firstChildElement),
    GB_PROPERTY_READ("LastChildElement", "XmlElement", CElement_lastChildElement),

    GB_METHOD("MatchXPathFilter", "b", CElement_matchXPathFilter ,"(Filter)s"),
    GB_METHOD("GetChildrenByTagName", "XmlElement[]", CElement_getChildrenByTagName, "(TagName)s[(Depth)i]"),
    GB_METHOD("GetChildrenByAttributeValue", "XmlElement[]", CElement_getChildrenByAttributeValue, "(Attribute)s(Value)s[(Depth)i]"),

    GB_STATIC_METHOD("FromText", "XmlNode[]", CElement_fromText, "(Data)s"),

    GB_END_DECLARE
};*/
