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

#ifndef ELEMENT_H
#define ELEMENT_H

#include "node.h"

class Attribute : public Node
{
public:
    Attribute();
    Attribute(const char *nattrName, const size_t nlenAttrName);
    Attribute(const char *nattrName, const size_t nlenAttrName, 
              const char *nattrVal, const size_t nlenAttrVal);
    ~Attribute();
    
    
    void setName(const char *nattrName, const size_t nlenAttrName);
    void setValue(const char *nattrVal, const size_t nlenAttrVal);
    char *attrName;
    size_t lenAttrName;
    char *attrValue;
    size_t lenAttrValue;
    
    virtual Node::Type getType();
    virtual void addStringLen(size_t &len, int indent = -1);
    virtual void addString(char *&data, int indent = -1);
    virtual void setTextContent(const char *ncontent, const size_t nlen);
    virtual void addTextContentLen(size_t &len);
    virtual void addTextContent(char *&data);
    virtual void NewGBObject();
};

class Element : public Node
{
public:
    Element();
    Element(const char *ntagName, size_t nlenTagName);
    virtual ~Element();
    virtual Type getType();
    
    //Tag Name
    void setTagName(const char *ntagName, size_t nlenTagName);//(re)defines the tag name
    bool isSingle();
    char *tagName;
    size_t lenTagName;
    undefbool single;

    void setPrefix(const char *nprefix, size_t nlenPrefix);
    char *prefix;
    size_t lenPrefix;

    char* localName;
    size_t lenLocalName;
   
    void refreshPrefix();
    
    //Attributes
    void addAttribute(const char *nattrName, const size_t nlenAttrName);//Adds a new attribute
    void addAttribute(const char *nattrName, const size_t nlenAttrName, 
                      const char *nattrVal, const size_t nlenAttrVal);
    Attribute* getAttribute(const char *nattrName, const size_t nlenAttrName, const int mode = GB_STRCOMP_BINARY);//Looks for attribute, and returns its value
    void setAttribute(const char *nattrName, const size_t nlenAttrName,
                       const char *nattrVal, const size_t nlenAttrVal);//Looks for attribute, sets its value or add it if attribute is not found
    bool attributeContains(const char *attrName, size_t lenAttrName, const char *value, size_t lenValue);
    void removeAttribute(const char *attrName, size_t lenAttrName);
    Attribute *firstAttribute;
    Attribute *lastAttribute;
    size_t attributeCount;
    
    //String output
    virtual void addStringLen(size_t &len, int indent = -1);
    virtual void addString(char *&content, int indent = -1);
    
    //Text Content
    virtual void setTextContent(const char *ncontent, const size_t nlen);
    virtual void addTextContentLen(size_t &len);
    virtual void addTextContent(char *&data);
    
        
    //Gambas object    
    virtual void NewGBObject();
    
    static const char* singleElements[];
    
#ifndef HELEMENT_H
};
#endif

#endif // ELEMENT_H
