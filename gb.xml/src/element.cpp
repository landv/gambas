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
#include "gbi.h"
#include <stdlib.h>
#include "CNode.h"
#include "utils.h"
#include "textnode.h"

/*************************************** Element ***************************************/

const char* Element::singleElements = "br\0hr\0area\0base\0br\0co\0command\0embed\0hr\0img\0input\0keygen\0link\0meta\0param\0source\0track\0wbr\0\0";
#define LEN_SINGLEELEMENTS 89

Element::Element() : Node()
{
    tagName = 0;
    lenTagName = 0;
    firstAttribute = 0;
    lastAttribute = 0;
    attributeCount = 0;
    prefix = 0;
    lenPrefix = 0;
    localName = 0;
    lenLocalName = 0;
}

Element::Element(const char *ntagName, size_t nlenTagName) : Element()
{
    setTagName(ntagName, nlenTagName);

}

Element::~Element()
{
    //Releasing tag name
    if(tagName) free(tagName);
    free(prefix);
    free(localName);
    
    //Releasing children
    if(firstChild)
    {
        for(Node *node = firstChild->nextNode; node != 0; node = node->nextNode)
        {
            removeChild(node->previousNode);
        }
        removeChild(lastChild);
    }
    
    //Releasing attributes
    if(firstAttribute)
    {
        for(Attribute *attr = (Attribute*)(firstAttribute->nextNode); attr != 0; attr = (Attribute*)(attr->nextNode))
        {
            delete attr->previousNode;
        }
        delete lastAttribute;
    }
}

Node::Type Element::getType()
{
    return Node::ElementNode;
}



/***** TagName *****/

void Element::setTagName(const char *ntagName, size_t nlenTagName)
{
    lenTagName = nlenTagName;
    tagName = (char*)realloc(tagName, sizeof(char) * lenTagName);
    memcpy(tagName, ntagName, lenTagName);

refreshPrefix();

}

void Element::setPrefix(const char *nprefix, size_t nlenPrefix)
{
    if(nlenPrefix)
    {
        tagName = (char*)realloc(tagName, nlenPrefix + lenLocalName + 1);
        memcpy(tagName, nprefix, nlenPrefix);
        *(tagName + nlenPrefix) = ':';
        memcpy(tagName + nlenPrefix + 1, localName, lenLocalName);
    }
    else if(lenPrefix)
    {
        tagName = (char*)realloc(tagName, lenLocalName);
        memcpy(tagName, localName, lenLocalName);
    }


    lenPrefix = nlenPrefix;
    prefix = (char*)realloc(prefix, lenPrefix);
    if(nlenPrefix) memcpy(prefix, nprefix, nlenPrefix);
}

void Element::refreshPrefix()
{
    if(!lenTagName)
    {
        free(localName);
        localName = 0;
        lenLocalName = 0;
        free(prefix);
        prefix = 0;
        lenPrefix = 0;
        return;
    }
    register char* pos = (char*)memrchr(tagName, ':', lenTagName);//Prefix
    if(pos)
    {
        lenLocalName = (tagName + lenTagName) - (pos + 1);
        lenPrefix = pos - tagName;
        localName = (char*)realloc(localName, lenLocalName);
        prefix = (char*)realloc(prefix, lenPrefix);
        memcpy(prefix, tagName, lenPrefix);
        memcpy(localName, pos + 1, lenLocalName);
    }
    else
    {
        lenLocalName = lenTagName;
        localName = (char*)realloc(localName, sizeof(char) * lenTagName);
        memcpy(localName, tagName, lenTagName);
        free(prefix);
        prefix = 0;
        lenPrefix = 0;
    }
}

bool Element::isSingle()
{
    const char *start = Element::singleElements;
    char *end = (char*)memchr(start, 0, LEN_SINGLEELEMENTS);
    unsigned char lenTag = end - start;
    
    while(end < singleElements + LEN_SINGLEELEMENTS)
    {
        if(lenTag == lenTagName)
        {
            if(!memcmp(tagName, start, lenTagName))
            {
                return true;
            }
        }
        
        start = end + 1;
        end = (char*)memchr(start, 0, LEN_SINGLEELEMENTS - (singleElements - start));
        lenTag = end - start;
    }
    
    return false;
}

/***** Attributes *****/

void Element::addAttribute(const char *nattrName, const size_t nlenAttrName)
{
    attributeCount++;
    Attribute *newAttribute = new Attribute(nattrName, nlenAttrName);
    if(!lastAttribute)//No attribute
    {
        firstAttribute = newAttribute;
        lastAttribute = firstAttribute;
        lastAttribute->previousNode = 0;
        lastAttribute->nextNode = 0;
        return;
    }
    
    newAttribute->previousNode = lastAttribute;
    lastAttribute->nextNode = newAttribute;
    lastAttribute = newAttribute;
    lastAttribute->nextNode = 0;
}

void Element::addAttribute(const char *nattrName, const size_t nlenAttrName, 
                           const char *nattrVal, const size_t nlenAttrVal)
{
    attributeCount++;
    Attribute *newAttribute = new Attribute(nattrName, nlenAttrName,
                                            nattrVal, nlenAttrVal);
    newAttribute->parent = this;
    if(!lastAttribute)//No attribute
    {
        firstAttribute = newAttribute;
        lastAttribute = firstAttribute;
        lastAttribute->previousNode = 0;
        lastAttribute->nextNode = 0;
        return;
    }
    
    newAttribute->previousNode = lastAttribute;
    lastAttribute->nextNode = newAttribute;
    lastAttribute = newAttribute;
    lastAttribute->nextNode = 0;
}

Attribute* Element::getAttribute(const char *nattrName, const size_t nlenAttrName, const int mode)
{
    for(Attribute *node = (Attribute*)(firstAttribute); node != 0; node = (Attribute*)(node->nextNode))
    {
        if(mode == GB_STRCOMP_NOCASE || mode == GB_STRCOMP_LANG + GB_STRCOMP_NOCASE)
        {
            if(!(nlenAttrName == node->lenAttrName)) continue;
            if(strncasecmp(nattrName, node->attrName, nlenAttrName) == 0) return node;
        }
        else if(mode == GB_STRCOMP_LIKE)
        {
            if(GB.MatchString(node->attrName, node->lenAttrName, nattrName, nlenAttrName)) return node;
        }
        else
        {
            if(!(nlenAttrName == node->lenAttrName)) continue;
            if(memcmp(nattrName, node->attrName, nlenAttrName) == 0) return node;
        }
    }
    return 0;
}

void Element::setAttribute(const char *nattrName, const size_t nlenAttrName, 
                           const char *nattrVal, const size_t nlenAttrVal)
{
    Attribute *attr = getAttribute(nattrName, nlenAttrName);
    if(!attr)
    {
        addAttribute(nattrName, nlenAttrName, nattrVal, nlenAttrVal);
    }
    else
    {
        attr->setValue(nattrVal, nlenAttrVal);
    }
}

bool Element::attributeContains(const char *attrName, size_t lenAttrName, const char *value, size_t lenValue)
{
        Attribute *attr = getAttribute(attrName, lenAttrName);
        if(!attr) return false;
        char *pos = (char*)memchr(attr->attrValue, CHAR_SPACE ,attr->lenAttrValue);
        char *oldPos = attr->attrValue;
        
        while(pos)
        {
            if((pos + 1) == lenValue + oldPos) //(pos + 1) - oldPos == lenValue
            {
                if(!memcmp(value, pos + 1, lenValue)) return true;
            }
            oldPos = pos + 1;
            pos = (char*)memchr(pos, CHAR_SPACE ,attr->lenAttrValue - (attr->attrValue - pos));
        }
        
        if(((attr->attrValue + attr->lenAttrValue))  == lenValue + oldPos)
        {
            if(!memcmp(value, oldPos + 1, lenValue)) return true;
        }
        
        return false;
        
    
}

/***** String output *****/
void Element::addStringLen(size_t &len, int indent)
{
    // (indent) '<' + prefix:tag + (' ' + attrName + '=' + '"' + attrValue + '"') + '>' \n
    // + children + (indent) '</' + tag + '>" \n
    // Or, singlElement :
    // (indent) '<' + prefix:tag + (' ' + attrName + '=' + '"' + attrValue + '"') + ' />' \n
    if(isSingle())
    {
        len += (4 + lenTagName + (prefix ? lenPrefix + 1 : 0));
        if(indent >= 0) len += indent + 1;
    }
    else
    {
        len += (5 + ((lenTagName + (prefix ? lenPrefix + 1 : 0)) * 2));
        if(indent >= 0) len += indent * 2 + 2;
        for(Node *child = firstChild; child != 0; child = child->nextNode)
        {
            child->addStringLen(len, indent >= 0 ? indent + 1 : -1);
        }
    }
    
    for(Attribute *attr = (Attribute*)(firstAttribute); attr != 0; attr = (Attribute*)(attr->nextNode))
    {
        len += 4 + attr->lenAttrName + attr->lenAttrValue;
    }
    
    
    
}

void Element::addString(char *&content, int indent)
{
    //register char *content = data;
    bool single = isSingle();
#define ADD(_car) *content = _car; content++;
    
    //Opening tag
    if(indent > 0)
    {
        memset(content, CHAR_SPACE, indent);
        content += indent;
    }
    ADD(CHAR_STARTTAG);
    if(prefix)
    {
        memcpy(content, prefix, lenPrefix); content += lenPrefix;
        ADD(':');
    }
    memcpy(content, tagName, lenTagName); content += lenTagName;
    
    //Attributes
    for(register Attribute *attr = (Attribute*)firstAttribute; attr != 0; attr = (Attribute*)(attr->nextNode))
    {
        ADD(CHAR_SPACE);
        memcpy(content, attr->attrName, attr->lenAttrName); content += attr->lenAttrName;
        
        ADD(CHAR_EQUAL);
        ADD(CHAR_DOUBLEQUOTE);
        memcpy(content, attr->attrValue, attr->lenAttrValue); content += attr->lenAttrValue;
        ADD(CHAR_DOUBLEQUOTE);
    }
    
    if(single)
    {
        ADD(CHAR_SPACE);
        ADD(CHAR_SLASH);
    }
    ADD(CHAR_ENDTAG);
    if(indent >= 0) { ADD(SCHAR_N); }
    
    if(!single)
    {
    
        //Content
        for(register Node *child = firstChild; child != 0; child = child->nextNode)
        {
            child->addString(content, indent >= 0 ? indent + 1 : -1);
        }
        
        if(indent > 0)
        {
            memset(content, CHAR_SPACE, indent);
            content += indent;
        }
        
        //Ending Tag    
        ADD(CHAR_STARTTAG);
        ADD(CHAR_SLASH);
        if(prefix)
        {
            memcpy(content, prefix, lenPrefix); content += lenPrefix;
            ADD(':');
        }
        memcpy(content, tagName, lenTagName); content += lenTagName;
        ADD(CHAR_ENDTAG); 
        if(indent >= 0) { ADD(SCHAR_N); }
    
    }
    
    //data = content;
    
}

/***** Text Content *****/

void Element::setTextContent(const char *ncontent, const size_t nlen)
{
    Node *newText = 0;
    if(nlen == 0) return;
    
    clearChildren();
    
    if(!newText)
    {
        newText = new TextNode(ncontent, nlen);
        appendChild(newText);
    }
    else
    {
        newText->setTextContent(ncontent, nlen);
    }
}

void Element::addTextContentLen(size_t &len)
{
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isComment()) continue;
        node->addTextContentLen(len);
    }
}

void Element::addTextContent(char *&data)
{
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isComment()) continue;
        node->addTextContent(data);
    }
}




/***** Gambas object *****/

void Element::NewGBObject()
{
    NoInstanciate = true;
    GBObject = GBI::New<CNode>("XmlElement");
    GBObject->node = this;
    NoInstanciate = false;
}

/*************************************** Attribute ***************************************/

Attribute::Attribute() : Node()
{
    attrName = 0;
    attrValue = 0;
    lenAttrName = 0;
    lenAttrValue = 0;
}

Attribute::Attribute(const char *nattrName, const size_t nlenAttrName) : Node()
{
    attrValue = 0;
    lenAttrValue = 0;
    
    lenAttrName = lenAttrName;
    attrName = (char*)malloc(sizeof(char)*lenAttrName);
    memcpy(attrName, nattrName, lenAttrName);
}

Attribute::Attribute(const char *nattrName, const size_t nlenAttrName, 
                     const char *nattrVal, const size_t nlenAttrVal) : Node()
{
    
    lenAttrName = nlenAttrName;
    lenAttrValue = nlenAttrVal;
    
    attrName = (char*)malloc(sizeof(char)*(lenAttrName));
    memcpy(attrName, nattrName, lenAttrName);
    
    attrValue = (char*)malloc(lenAttrValue);
    memcpy(attrValue, nattrVal, lenAttrValue);
}

Attribute::~Attribute()
{
    if(attrName) free(attrName);
    if(attrValue) free(attrValue);
}

void Attribute::setName(const char *nattrName, const size_t nlenAttrName)
{
    lenAttrName = nlenAttrName;
    if(attrName)
    {
        attrName = (char*)realloc(attrName, sizeof(char) * lenAttrName);
    }
    else
    {
        attrName = (char*)malloc(sizeof(char) * lenAttrName);
    }
    memcpy(attrName, nattrName, lenAttrName);
}

void Attribute::setValue(const char *nattrVal, const size_t nlenAttrVal)
{
    lenAttrValue = nlenAttrVal;
    if(!nattrVal)
    {
        free(attrValue);
        attrValue = 0;
        return;
    }
    if(attrValue)
    {
        attrValue = (char*)realloc(attrValue, sizeof(char) * lenAttrValue);
    }
    else
    {
        attrValue = (char*)malloc(sizeof(char) * lenAttrValue);
    }
    memcpy(attrValue, nattrVal, lenAttrValue);
}

Node::Type Attribute::getType()
{
    return Node::AttributeNode;
}

void Attribute::addStringLen(size_t &len, int indent)
{
    
}

void Attribute::addString(char *&data, int indent)
{
    
}

void Attribute::setTextContent(const char *ncontent, const size_t nlen)
{
    setValue(ncontent, nlen);
}

void Attribute::addTextContentLen(size_t &len)
{
    len += lenAttrValue;
}

void Attribute::addTextContent(char *&data)
{
    memcpy(data, attrValue, lenAttrValue);
    data += lenAttrValue;
}

void Attribute::NewGBObject()
{
    NoInstanciate = true;
    GBObject = GBI::New<CNode>("_XmlAttrNode");
    GBObject->node = this;
    NoInstanciate = false;
}

