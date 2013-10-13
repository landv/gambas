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

#include "htmlelement.h"
#include "cssfilter.h"
#include "../gbinterface.h"

void HTMLElement_AddGBChildrenByFilter(Element *elmt, char *filter, size_t lenFilter, GB_ARRAY *array, int depth = -1);

bool HTMLElement_HasClassName(const Element *elmt, const char *className, size_t lenClassName)
{
    return XML.XMLElement_AttributeContains(elmt, "class", 5, className, lenClassName);
}

Element* HTMLElement_GetChildById(Element *elmt, char *id, size_t lenId, int depth)
{
    return XML.XMLNode_getFirstChildByAttributeValue(elmt, "id", 2, id, lenId, 0, depth);
}

void HTMLElement_GetGBChildrenByClassName(Element *elmt, char* className, size_t lenClassName, GB_ARRAY *array, int depth)
{
    XML.XMLNode_getGBChildrenByAttributeValue(elmt, "class", 5, className, lenClassName, array, 0, depth);
}

void HTMLElement_SetId(Element *elmt, const char *value, size_t len)
{
    XML.XMLElement_SetAttribute(elmt, "id", 2, value, len);
}

void HTMLElement_SetClassName(Element *elmt, const char *value, size_t len)
{
    XML.XMLElement_SetAttribute(elmt, "class", 5, value, len);
}

Attribute* HTMLElement_GetClassName(const Element *elmt)
{
    return XML.XMLElement_GetAttribute(elmt, "class", 5, 0);
}

Attribute* HTMLElement_GetId(const Element *elmt)
{
    return XML.XMLElement_GetAttribute(elmt, "id", 2, 0);
}

void HTMLElement_GetGBChildrenByFilter(Element *elmt, char *filter, size_t lenFilter, GB_ARRAY *array, int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    HTMLElement_AddGBChildrenByFilter(elmt, filter, lenFilter, array, depth);
}

void HTMLElement_AddGBChildrenByFilter(Element *elmt, char *filter, size_t lenFilter, GB_ARRAY *array, int depth)
{
    if(depth == 0) return;
    for(Node *node = elmt->firstChild; node != 0; node = node->nextNode)
    {
        if(node->type == Node::ElementNode)
        {
            if(HTMLElement_MatchFilter((Element*)node, filter, lenFilter))
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XML.XMLNode_GetGBObject(node);
                GB.Ref(node->GBObject);
            }
            HTMLElement_AddGBChildrenByFilter((Element*)(node), filter, lenFilter, array, depth - 1);
        }
    }
}

const size_t lenSingleElements[] =      {2 , 3  ,4    ,5     ,4    ,4    ,2  ,7       ,5     ,2  ,5      ,4    ,2     ,6      ,5     ,3};
const char* singleElements[] = {"br", "img", "meta", "input", "area", "base", "co", "command", "embed", "hr", "keygen", "link", "param", "source", "track", "wbr"};
#define COUNT_SINGLEELEMENTS 16


bool HTMLElement_IsSingle(Element *elmt)
{
    for(int i = 0; i < COUNT_SINGLEELEMENTS; i++)
    {
        if(elmt->lenTagName == lenSingleElements[i])
        {
            if(!strncasecmp(singleElements[i], elmt->tagName, elmt->lenTagName))
            {
                return true;
            }
        }
    }
    return false;
}
