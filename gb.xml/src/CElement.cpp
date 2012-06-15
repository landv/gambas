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
#include "element.h"
#include "gbi.h"

#define THIS (static_cast<CNode*>(_object)->node->toElement())
#define THISNODE (static_cast<CNode*>(_object)->node)

BEGIN_METHOD(CElement_new, GB_STRING tagName)

if(Node::NoInstanciate) return;

if(!MISSING(tagName))
{
    THISNODE = new Element(STRING(tagName), LENGTH(tagName));
}
else
{
    THISNODE = new Element;
}
    THIS->GBObject = static_cast<CNode*>(_object);

END_METHOD

BEGIN_PROPERTY(CElement_tagName)

if(READ_PROPERTY)
{
    GB.ReturnNewString(THIS->tagName, THIS->lenTagName);
}
else
{
    THIS->setTagName(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_METHOD(CElement_appendChild, GB_OBJECT newChild)

    THIS->appendChild(VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_getAttribute, GB_STRING attrName)

    Attribute *attr = THIS->getAttribute(STRING(attrName), LENGTH(attrName));
    if(attr)
    {
        GB.ReturnNewString(attr->attrName, attr->lenAttrName);
    }
    else
    {
        GB.ReturnNull();
    }

END_METHOD

BEGIN_METHOD(CElement_setAttribute, GB_STRING attrName; GB_STRING attrValue)

    THIS->setAttribute(STRING(attrName), LENGTH(attrName), 
                       STRING(attrValue), LENGTH(attrValue));

END_METHOD

BEGIN_METHOD(CElement_appendFromText, GB_STRING data)

THIS->appendFromText(STRING(data), LENGTH(data));

END_METHOD

BEGIN_METHOD(CElement_appendChildren, GB_OBJECT children)

GB_ARRAY array = reinterpret_cast<GB_ARRAY>(VARG(children));

for(int i = 0; i < GB.Array.Count(array); i++)
{
    THIS->appendChild((*(reinterpret_cast<CNode**>(GB.Array.Get(array, i))))->node);
}

END_METHOD

BEGIN_METHOD(CElement_prependChild, GB_OBJECT newChild)

THIS->prependChild(VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_removeChild, GB_OBJECT oldChild)

THIS->removeChild(VARGOBJ(CNode, oldChild)->node);

END_METHOD

BEGIN_METHOD(CElement_replaceChild, GB_OBJECT oldChild; GB_OBJECT newChild)

THIS->replaceChild(VARGOBJ(CNode, oldChild)->node,
                        VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_insertAfter, GB_OBJECT child; GB_OBJECT newChild)

THIS->insertAfter(VARGOBJ(CNode, child)->node, VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_insertBefore, GB_OBJECT child; GB_OBJECT newChild)

THIS->insertBefore(VARGOBJ(CNode, child)->node, VARGOBJ(CNode, newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_appendText, GB_STRING data)

THIS->appendText(STRING(data), LENGTH(data));

END_METHOD

BEGIN_PROPERTY(CElement_previousSibling)

GBI::Return(THIS->previousSibling());

END_PROPERTY

BEGIN_PROPERTY(CElement_nextSibling)

GBI::Return(THIS->nextSibling());

END_PROPERTY

BEGIN_METHOD(CElement_isAttributeSet, GB_STRING name)

GB.ReturnBoolean((bool)(THIS->getAttribute(STRING(name), LENGTH(name))));

END_METHOD

BEGIN_PROPERTY(CElement_childNodes)

GB_ARRAY array;
THIS->getGBChildren(&array);

GB.ReturnObject(array);

END_PROPERTY

BEGIN_PROPERTY(CElement_allChildNodes)

GB_ARRAY array;
THIS->getGBAllChildren(&array);

GB.ReturnObject(array);

END_PROPERTY

BEGIN_PROPERTY(CElement_firstChildElement)

GBI::Return(THIS->firstChildElement());

END_PROPERTY

BEGIN_PROPERTY(CElement_lastChildElement)

GBI::Return(THIS->lastChildElement());

END_PROPERTY

BEGIN_PROPERTY(CElement_childElements)

GB_ARRAY array;
THIS->getGBChildElements(&array);
GB.ReturnObject(array);

END_PROPERTY

BEGIN_METHOD(CElement_fromText, GB_STRING data)

GB_ARRAY array;
Element::GBfromText(STRING(data), LENGTH(data), &array);
GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CElement_getChildrenByTagName, GB_STRING tagName; GB_INTEGER depth)

GB_ARRAY array;
THIS->getGBChildrenByTagName(STRING(tagName), LENGTH(tagName), &array, VARGOPT(depth, -1));
GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CElement_getChildrenByAttributeValue, GB_STRING name; GB_STRING value; GB_INTEGER depth)

GB_ARRAY array;
THIS->getGBChildrenByAttributeValue(STRING(name), LENGTH(name), 
                                    STRING(value), LENGTH(value),
                                    &array, VARGOPT(depth, -1));
GB.ReturnObject(array);

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
    
    GB_METHOD("AppendText", "", CElement_appendText, "(Data)s"),
    GB_METHOD("AppendFromText", "", CElement_appendFromText, "(Data)s"),
    
    GB_METHOD("GetAttribute", "s", CElement_getAttribute, "(Name)s"),
    GB_METHOD("SetAttribute", "", CElement_setAttribute, "(Name)s(Value)s"),
    
    GB_METHOD("IsAttributeSet", "b", CElement_isAttributeSet, "(Name)s"),
    
    GB_PROPERTY_READ("ChildNodes", "XmlNode[]", CElement_childNodes),
    GB_PROPERTY_READ("Children", "XmlNode[]", CElement_childNodes),
    GB_PROPERTY_READ("ChildElements", "XmlElement[]", CElement_childElements),
    GB_PROPERTY_READ("AllChildNodes", "XmlNode[]", CElement_allChildNodes),
    GB_PROPERTY_READ("FirstChildElement", "XmlElement", CElement_firstChildElement),
    GB_PROPERTY_READ("LastChildElement", "XmlElement", CElement_lastChildElement),
    
    GB_PROPERTY("TagName", "s", CElement_tagName),
    GB_PROPERTY("PerviousSibling", "XmlElement", CElement_previousSibling),
    GB_PROPERTY("NextSibling", "XmlElement", CElement_nextSibling),
    
    GB_METHOD("GetChildrenByTagName", "XmlElement[]", CElement_getChildrenByTagName, "(TagName)s[(Depth)i]"),
    GB_METHOD("GetChildrenByAttributeValue", "XmlElement[]", CElement_getChildrenByAttributeValue, "(Attribute)s(Value)s[(Depth)i]"),
    
    GB_STATIC_METHOD("FromText", "XmlNode[]", CElement_fromText, "(Data)s"),
    

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
