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

#include "element.h"

#include "node.h"
#include "utils.h"
#include "textnode.h"

#include <stdlib.h>
#include <memory.h>

/*************************************** Element ***************************************/

Element* XMLElement_New()
{
    Element *newElement = (Element*)malloc(sizeof(Element));
    memset(newElement, 0, sizeof(Element));
    XMLNode_Init(newElement, Node::ElementNode);
    return newElement;
}

Element* XMLElement_New(const char *ntagName, size_t nlenTagName)
{
    Element *newElement = XMLElement_New();
    XMLElement_SetTagName(newElement, ntagName, nlenTagName);
    return newElement;
}

void XMLElement_Free(Element *elmt)
{
    //Releasing tag name
    if(elmt->tagName) free(elmt->tagName);
    free(elmt->prefix);
    free(elmt->localName);

    //Releasing children
    XMLNode_clearChildren(elmt);

    //Releasing attributes
    if(elmt->firstAttribute)
    {
        for(Attribute *attr = (Attribute*)(elmt->firstAttribute->nextNode); attr != 0; attr = (Attribute*)(attr->nextNode))
        {
            XMLAttribute_Free((Attribute*)(attr->previousNode));
        }
        XMLAttribute_Free(elmt->lastAttribute);
    }
    free(elmt);
}



/***** TagName *****/

void XMLElement_SetTagName(Element *elmt, const char *ntagName, size_t nlenTagName)
{
    elmt->lenTagName = nlenTagName;
    elmt->tagName = (char*)realloc(elmt->tagName, sizeof(char) * elmt->lenTagName);
    memcpy(elmt->tagName, ntagName, nlenTagName);
    XMLElement_RefreshPrefix(elmt);

}

void XMLElement_SetPrefix(Element *elmt, const char *nprefix, size_t nlenPrefix)
{
    if(nlenPrefix)
    {
        elmt->tagName = (char*)realloc(elmt->tagName, nlenPrefix + elmt->lenLocalName + 1);
        memcpy(elmt->tagName, nprefix, nlenPrefix);
        *(elmt->tagName + nlenPrefix) = ':';
        memcpy(elmt->tagName + nlenPrefix + 1, elmt->localName, elmt->lenLocalName);
    }
    else if(elmt->lenPrefix)
    {
        elmt->tagName = (char*)realloc(elmt->tagName, elmt->lenLocalName);
        memcpy(elmt->tagName, elmt->localName, elmt->lenLocalName);
    }


    elmt->lenPrefix = nlenPrefix;
    elmt->prefix = (char*)realloc(elmt->prefix, nlenPrefix);
    if(nlenPrefix) memcpy(elmt->prefix, nprefix, nlenPrefix);
}

void XMLElement_RefreshPrefix(Element *elmt)
{
    if(!elmt->lenTagName)
    {
        free(elmt->localName);
        elmt->localName = 0;
        elmt->lenLocalName = 0;
        free(elmt->prefix);
        elmt->prefix = 0;
        elmt->lenPrefix = 0;
        return;
    }
    register char* pos = (char*)memrchr(elmt->tagName, ':', elmt->lenTagName);//Prefix
    if(pos)
    {
        elmt->lenLocalName = (elmt->tagName + elmt->lenTagName) - (pos + 1);
        elmt->lenPrefix = pos - elmt->tagName;
        elmt->localName = (char*)realloc(elmt->localName, elmt->lenLocalName);
        elmt->prefix = (char*)realloc(elmt->prefix, elmt->lenPrefix);
        memcpy(elmt->prefix, elmt->tagName, elmt->lenPrefix);
        memcpy(elmt->localName, pos + 1, elmt->lenLocalName);
    }
    else
    {
        elmt->lenLocalName = elmt->lenTagName;
        elmt->localName = (char*)realloc(elmt->localName, sizeof(char) * elmt->lenTagName);
        memcpy(elmt->localName, elmt->tagName, elmt->lenTagName);
        free(elmt->prefix);
        elmt->prefix = 0;
        elmt->lenPrefix = 0;
    }
}

/***** Attributes *****/

Attribute* XMLElement_AddAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName)
{
    elmt->attributeCount++;
    Attribute *newAttribute = XMLAttribute_New(nattrName, nlenAttrName);
    newAttribute->parent = elmt;
    if(!elmt->lastAttribute)//No attribute
    {
        elmt->firstAttribute = newAttribute;
        elmt->lastAttribute = elmt->firstAttribute;
        elmt->lastAttribute->previousNode = 0;
        elmt->lastAttribute->nextNode = 0;
        return newAttribute;
    }
    
    newAttribute->previousNode = elmt->lastAttribute;
    elmt->lastAttribute->nextNode = newAttribute;
    elmt->lastAttribute = newAttribute;
    elmt->lastAttribute->nextNode = 0;
    return newAttribute;
}

Attribute* XMLElement_AddAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName,
                           const char *nattrVal, const size_t nlenAttrVal)
{
    elmt->attributeCount++;
    Attribute *newAttribute = XMLAttribute_New(nattrName, nlenAttrName,
                                            nattrVal, nlenAttrVal);
    newAttribute->parent = elmt;
    if(!elmt->lastAttribute)//No attribute
    {
        elmt->firstAttribute = newAttribute;
        elmt->lastAttribute = elmt->firstAttribute;
        elmt->lastAttribute->previousNode = 0;
        elmt->lastAttribute->nextNode = 0;
        return newAttribute;
    }
    
    newAttribute->previousNode = elmt->lastAttribute;
    elmt->lastAttribute->nextNode = newAttribute;
    elmt->lastAttribute = newAttribute;
    elmt->lastAttribute->nextNode = 0;
    return newAttribute;
}

Attribute* XMLElement_GetAttribute(const Element *elmt, const char *nattrName, const size_t nlenAttrName, const int mode)
{
    for(Attribute *attr = elmt->firstAttribute; attr != 0; attr = (Attribute*)(attr->nextNode))
    {
        if(GB_MatchString(attr->attrName, attr->lenAttrName, nattrName, nlenAttrName, mode))
            return attr;
    }
    return 0;
}

void XMLElement_SetAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName,
                           const char *nattrVal, const size_t nlenAttrVal)
{
    Attribute *attr = XMLElement_GetAttribute(elmt, nattrName, nlenAttrName);
    if(!attr)
    {
        XMLElement_AddAttribute(elmt, nattrName, nlenAttrName, nattrVal, nlenAttrVal);
    }
    else
    {
        XMLAttribute_SetValue(attr, nattrVal, nlenAttrVal);
    }
}



bool XMLElement_AttributeContains(const Element *elmt, const char *attrName, size_t lenAttrName, const char *value, size_t lenValue)
{
        Attribute *attr = XMLElement_GetAttribute(elmt, attrName, lenAttrName);
        if(!attr) return false;
        char *pos = (char*)memchr(attr->attrValue, ' ' ,attr->lenAttrValue);
        char *oldPos = attr->attrValue;
        
        while(pos)
        {
            if((pos + 1) == lenValue + oldPos) //(pos + 1) - oldPos == lenValue
            {
                if(!memcmp(value, pos + 1, lenValue)) return true;
            }
            oldPos = pos + 1;
            pos = (char*)memchr(pos, ' ' ,attr->lenAttrValue - (attr->attrValue - pos));
        }
        
        if(((attr->attrValue + attr->lenAttrValue))  == lenValue + oldPos)
        {
            if(!memcmp(value, oldPos, lenValue)) return true;
        }
        
        return false;
        

}

void XMLElement_RemoveAttribute(Element *elmt, const char *attrName, size_t lenAttrName)
{
    XMLElement_RemoveAttribute(elmt, XMLElement_GetAttribute(elmt, attrName,lenAttrName));
}

void XMLElement_RemoveAttribute(Element *elmt, Attribute *attr)
{
    if(!attr) return;
    if(attr->parent != elmt) return;
    if(attr == elmt->firstAttribute) elmt->firstAttribute = (Attribute*)(attr->nextNode);
    if(attr == elmt->lastAttribute) elmt->lastAttribute = (Attribute*)(attr->previousNode);
    if(attr->nextNode) attr->nextNode->previousNode = attr->previousNode;
    if(attr->previousNode) attr->previousNode->nextNode = attr->nextNode;
    elmt->attributeCount--;
    XMLAttribute_Free(attr);
}

void XMLElement_SetTextContent(Element *elmt, const char *content, size_t lenContent)
{
    if(!lenContent) return;

    XMLNode_clearChildren(elmt);

    TextNode *newChild = XMLTextNode_New(content, lenContent);

    XMLNode_appendChild(elmt, newChild);

}

/*************************************** Attribute ***************************************/

Attribute* XMLAttribute_New()
{
    Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
    XMLNode_Init(newAttr, Node::AttributeNode);
    newAttr->attrName = 0;
    newAttr->attrValue = 0;
    newAttr->lenAttrName = 0;
    newAttr->lenAttrValue = 0;
    return newAttr;
}

Attribute* XMLAttribute_New(const char *nattrName, const size_t nlenAttrName)
{
    Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
    XMLNode_Init(newAttr, Node::AttributeNode);
    newAttr->attrValue = 0;
    newAttr->lenAttrValue = 0;
    
    newAttr->lenAttrName = nlenAttrName;
    newAttr->attrName = (char*)malloc(sizeof(char)*nlenAttrName);
    memcpy(newAttr->attrName, nattrName, nlenAttrName);
    return newAttr;
}

Attribute* XMLAttribute_New(const char *nattrName, const size_t nlenAttrName,
                     const char *nattrVal, const size_t nlenAttrVal)
{
    Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
    XMLNode_Init(newAttr, Node::AttributeNode);

    newAttr->lenAttrName = nlenAttrName;
    newAttr->lenAttrValue = nlenAttrVal;
    
    newAttr->attrName = (char*)malloc(sizeof(char)*(nlenAttrName));
    memcpy(newAttr->attrName, nattrName, nlenAttrName);
    
    newAttr->attrValue = (char*)malloc(nlenAttrVal);
    memcpy(newAttr->attrValue, nattrVal, nlenAttrVal);

    return newAttr;
}

void XMLAttribute_Free(Attribute *attr)
{
    if(attr->attrName) free(attr->attrName);
    if(attr->attrValue) free(attr->attrValue);

    free(attr);
    attr = 0;
}

void XMLAttribute_SetName(Attribute *attr, const char *nattrName, const size_t nlenAttrName)
{
    attr->lenAttrName = nlenAttrName;
    attr->attrName = (char*)realloc(attr->attrName, sizeof(char) * attr->lenAttrName);
    memcpy(attr->attrName, nattrName, attr->lenAttrName);
}

void XMLAttribute_SetValue(Attribute *attr, const char *nattrVal, const size_t nlenAttrVal)
{
    attr->lenAttrValue = nlenAttrVal;
    if((!nlenAttrVal) && attr->attrValue)
    {
        free(attr->attrValue);
        attr->attrValue = 0;
        return;
    }
    attr->attrValue = (char*)realloc(attr->attrValue, sizeof(char) * attr->lenAttrValue);
    memcpy(attr->attrValue, nattrVal, attr->lenAttrValue);
}
