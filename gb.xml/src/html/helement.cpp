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

#include "helement.h"
#include "../utils.h"

//void Element::getClassNames()
//{
//    return split(this->getClassName(), " ");
//}

bool Element::hasClassName(const char *className, size_t lenClassName)
{
    return attributeContains("class", 5, className, lenClassName);
    
}

void Element::getGBChildrenByClassName(char* className, size_t lenClassName, GB_ARRAY *array, int depth)
{
    getGBChildrenByAttributeValue("class", 5, className, lenClassName, array, depth);
}

void Element::setId(const char *value, size_t len)
{
    setAttribute("id", 2, value, len);
}

void Element::setClassName(const char *value, size_t len)
{
    setAttribute("class", 5, value, len);
}

Attribute* Element::getClassName()
{
    return getAttribute("class", 5);
}

#define CHAR_UNIVERSAL 0x2A // *
#define CHAR_PSEUDOCLASS 0x3A // :
#define CHAR_ID 0x23 // #
#define CHAR_CLASSNAME 0x2E // .
#define CHAR_STARTATTRIBUTE 0x5B // [
#define CHAR_ENDATTRIBUTE 0x5DE // ]

#define CHAR_CONTAINS 0x7E // ~
#define CHAR_BEGINS 0x5E // ^
#define CHAR_ENDS 0x24 // $

bool Element::matchSubFilter(const char *filter, size_t lenFilter)
{
    if(!lenFilter) return true;
    Trim(filter, lenFilter);
    if(!lenFilter) return true;
    char s = 0;
    char const *pos = 0;
    
    for(pos = filter + 1; pos < filter + lenFilter; ++pos)
    {
        if(!isNameChar(*pos))//Autre chose qu'un nom
        {
            break;
        }
    }

    bool cond = (pos != filter + lenFilter);//Si il y a autre chose à évaluer

    s = *filter;
    if(s == CHAR_UNIVERSAL)//Sélecteur universel
    {
        if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
        return true;
    }
    if(s == CHAR_PSEUDOCLASS)//Pseudo-classe
    {
        register size_t lenSubStr = pos - filter;
        
        if(lenSubStr == 11 && !memcmp(filter, "first-child", 11))
        {
            if(!parent) return false;
            if(cond) return this->parent->firstChildElement() == this && matchSubFilter(pos, lenFilter - (pos - filter));
            return this->parent->firstChildElement() == this;
        }
        if(lenSubStr == 10 && !memcmp(filter, "last-child", 10))
        {
            if(!parent) return false;
            if(cond) return this->parent->lastChildElement() == this && matchSubFilter(pos, lenFilter - (pos - filter));
            return this->parent->lastChildElement() == this;
        }
        return false;
    }
    if(isNameStartChar(s))//C'est un nom de tag (rien au début)
    {
        if(!(lenTagName + filter == pos)) return false;//lenTagName == pos - filter
        if(memcmp(tagName, filter, lenTagName)) return false;
        if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
        return true;
    }
    else if(s == CHAR_ID)//C'est un id
    {
        Attribute *id = getId();
        if(!(id->lenAttrValue + filter == pos)) return false;
        if(memcmp(pos, id->attrValue, pos - filter)) return false;
        if(cond) return (matchSubFilter(pos, lenFilter - (pos - filter)));
        return true;
    }
    else if(s == CHAR_CLASSNAME)//ClassName
    {
        if(!hasClassName(filter, pos - filter)) return false;
        if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
        return true;
    }
    else if(s == CHAR_STARTATTRIBUTE)//Attribut
    {
        //Syntax : [foo="bar"]
        char const *endPos = (char*)memchr(filter, CHAR_ENDATTRIBUTE, lenFilter);//On cherche le crochet fermant
        
        endPos = endPos ? endPos : filter + lenFilter - 1;
        pos = (endPos + 1);
        cond = (pos < filter + lenFilter);

        char *equalPos = (char*)memchr(filter, CHAR_EQUAL, lenFilter);//On cherche le signe égal

        if(equalPos)//Si trouvé
        {
            s = *(equalPos - 1);//Le signe avant le signe égal
            char const *attrName = filter + 1; size_t lenAttrName = (equalPos - filter - 1);
            char const *attrValue = equalPos + 2; size_t lenAttrValue = (endPos - equalPos - 3);
            if(s == CHAR_CONTAINS)//Comparaison ~=
            {
                if(!attributeContains(attrName, lenAttrName - 1, attrValue, lenAttrValue)) return false;
                if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
                return true;
            }
            if(s == CHAR_BEGINS)
            {
                Attribute *attr = getAttribute(attrName, lenAttrName - 1);
                if(!attr) return false;
                if(attr->lenAttrValue < lenAttrValue) return false;
                if(memcmp(attr->attrValue, attrValue, lenAttrValue)) return false;
                if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
                return true;
            }
            if(s == CHAR_ENDS)
            {
                Attribute *attr = getAttribute(attrName, lenAttrName - 1);
                if(!attr) return false;
                if(attr->lenAttrValue < lenAttrValue) return false;
                if(memcmp(attr->attrValue + attr->lenAttrValue - lenAttrValue, attrValue, lenAttrValue)) return false;
                if(cond) matchSubFilter(pos, lenFilter - (pos - filter));
                return true;
            }
            if(s == CHAR_UNIVERSAL)
            {
                Attribute *attr = getAttribute(attrName, lenAttrName - 1);
                if(!attr) return false;
                if(attr->lenAttrValue < lenAttrValue) return false;
                if(!memchrs(attr->attrValue, attr->lenAttrValue, attrValue, lenAttrValue)) return false;
                if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
                return true;
            }
            
            Attribute *attr = getAttribute(attrName, lenAttrName);
            if(!attr) return false;
            if(attr->lenAttrValue != lenAttrValue) return false;
            if(memcmp(attr->attrValue, attrValue, lenAttrValue)) return false;
            
            //Valeur de l'attribut
            if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
            return true;
        }
        
        Attribute *attr = getAttribute(filter + 1, endPos - filter - 1);
        if(!attr) return false;
        //Si l'attribut est défini
        if(cond) return matchSubFilter(pos, lenFilter - (pos - filter));
        return true;

    }

    return false;
}

#define CHAR_COMMA 0x2C // ,
#define CHAR_PLUS 0x2B // +
#define CHAR_GREATER 0x3E // >


bool Element::matchFilter(const char *filter, size_t lenFilter)
{
    if(!lenFilter) return true;
    Trim(filter, lenFilter);
    char *pos;

    pos = (char*)memrchr(filter, CHAR_COMMA, lenFilter);
    if(pos)
    {
        return matchFilter(filter, (pos - filter)) || 
                matchFilter(pos, lenFilter - (pos + 1 - filter));
    }

    pos = (char*)memrchr(filter, CHAR_GREATER, lenFilter);
    if(pos)
    {
        Element *elmt = this->parent->toElement();
        if(!elmt) return false;
        return elmt->matchFilter(filter, (pos - filter))  &&
                matchFilter(pos, lenFilter - (pos + 1 - filter));
    }

    pos = (char*)memrchr(filter, CHAR_PLUS, lenFilter);
    if(pos)
    {
        Element *elmt = this->previousElement();
        if(!elmt) return false;
        return elmt->matchFilter(filter, (pos - filter))  &&
                matchFilter(pos, lenFilter - (pos + 1 - filter));
    }

    pos = (char*)memrchr(filter, CHAR_SPACE, lenFilter);
    if(pos)
    {
        if(!matchFilter(pos, lenFilter - (pos + 1 - filter))) return false;
        Element *elmt = this->parent->toElement();
        while(elmt)
        {
            if(elmt->matchFilter(filter, (pos - filter))) return true;
            elmt = elmt->parent->toElement();
        }

        return false;
    }

    return matchSubFilter(filter, lenFilter);


}

void Element::addGBChildrenByFilter(char *filter, const size_t lenFilter, GB_ARRAY *array, const int depth)
{
    if(depth == 0) return;
    if(matchFilter(filter, lenFilter))
    {
        *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
        GB.Ref(GBObject);
    }
    if(depth == 1) return;
    
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addGBChildrenByFilter(filter, lenFilter, array, depth - 1);
        }
    }
}

void Element::getGBChildrenByFilter(char *filter, size_t lenFilter, GB_ARRAY *array, int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    addGBChildrenByFilter(filter, lenFilter, array, depth);
}
