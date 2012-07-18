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
#include "utils.h"

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
    virtual void addStringLen(size_t *len, int indent = 0);
    virtual void addString(char **data, int indent = 0);
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

    void setPrefix(const char *nprefix, size_t nlenPrefix);
    char *prefix;
    size_t lenPrefix;
   
    //Node tree
    void appendChild(Node *newChild);//Adds a new child after the last one
    void prependChild(Node *newChild);
    void removeChild(Node *child);//Removes a child from the node
    void removeKeepChild(Node *child);//Removes a child from the node, but doesn't destroys it
    void replaceChild(Node *oldChild, Node *newChild);
    bool insertAfter(Node *child, Node *newChild);
    bool insertBefore(Node *child, Node *newChild);
    void appendText(const char *data, const size_t lenData);
    void clearChildren();
    
    void appendFromText(char *data, const size_t lenData);
    
    void getGBChildrenByNamespace(const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
    void addGBChildrenByNamespace(const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
    void getGBChildrenByTagName(const char *ctagName, const size_t clenTagName,  GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
    void addGBChildrenByTagName(const char *compTagName, const size_t compLenTagName, GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
    void getGBChildrenByAttributeValue(const char *attrName, const size_t lenAttrName,
                                       const char *attrValue, const size_t lenAttrValue,
                                       GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
    void addGBChildrenByAttributeValue(const char *attrName, const size_t lenAttrName,
                                       const char *attrValue, const size_t lenAttrValue,
                                       GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
    
    Element** getChildrenByTagName(const char *ctagName, const size_t clenTagName, size_t &lenArray, const int depth = -1);
    Element* getFirstChildByTagName(const char *ctagName, const size_t clenTagName, const int depth = -1);
    void addChildrenByTagName(const char *compTagName, const size_t compLenTagName, Element** &array, size_t &lenArray, const int depth = -1);
    void getGBAllChildren(GB_ARRAY *array);
    void addGBAllChildren(GB_ARRAY *array);
    void getGBChildElements(GB_ARRAY *array);
    
    Element* firstChildElement();
    Element* lastChildElement();
    Element* previousElement();
    Element* nextElement();
    
    
    Node *firstChild;
    Node *lastChild;
    size_t childCount;
    
    //Attributes
    void addAttribute(const char *nattrName, const size_t nlenAttrName);//Adds a new attribute
    void addAttribute(const char *nattrName, const size_t nlenAttrName, 
                      const char *nattrVal, const size_t nlenAttrVal);
    Attribute* getAttribute(const char *nattrName, const size_t nlenAttrName, const int mode = GB_STRCOMP_BINARY);//Looks for attribute, and returns its value
    void setAttribute(const char *nattrName, const size_t nlenAttrName,
                       const char *nattrVal, const size_t nlenAttrVal);//Looks for attribute, sets its value or add it if attribute is not found
    bool attributeContains(const char *attrName, size_t lenAttrName, char *value, size_t lenValue);
    Attribute *firstAttribute;
    Attribute *lastAttribute;
    size_t attributeCount;
    
    //String output
    virtual void addStringLen(size_t *len, int indent = 0);
    virtual void addString(char **data, int indent = 0);
    
    //Text Content
    virtual void setTextContent(const char *ncontent, const size_t nlen);
    virtual void addTextContentLen(size_t &len);
    virtual void addTextContent(char *&data);
    
    //Parser
    static void GBfromText(char *data, const size_t lendata, GB_ARRAY *array);
    static Node** fromText(char const *data, const size_t lendata, size_t *nodeCount) throw(XMLParseException);
    
    //Gambas object    
    virtual void NewGBObject();
    
    static const char* singleElements;
    
#ifndef HELEMENT_H
};
#endif

#endif // ELEMENT_H
