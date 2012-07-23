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

#ifndef NODE_H
#define NODE_H

#include <memory.h>
#include "main.h"
#include "utils.h"

#define SUPPORT_CHILDREN(__node) ((__node->getType() == Node::ElementNode) || (__node->getType() == Node::DocumentNode))

class Element;
class TextNode;
class Document;

struct CNode;

class Node
{
public:
    static bool NoInstanciate;//If true, newly-created Gambas objects won't instanciate a new node
    
    Node();
    virtual ~Node();
    void DestroyGBObject();//Removes the link between the Gambas object and the node, and deletes the node if there isn't any other link
    void DestroyParent();//Removes the link between the parent node and the node, and deletes the node if there isn't any other link
    CNode *GetGBObject();
    
    //Node tree
    void getGBChildren(GB_ARRAY *array);
    Document* GetOwnerDocument();
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
    Document *parentDocument;
    Node *parent;
    Node *nextNode;
    Node *previousNode;
    
    //Node Types
    enum Type {ElementNode, NodeText, Comment, CDATA, AttributeNode, DocumentNode};
    virtual Type getType() = 0;//Returns the type of the node
    bool isElement();
    bool isText();
    bool isComment();
    bool isCDATA();
    bool isTextNode();
    Element* toElement();
    TextNode* toTextNode();
    
    //Parser
    static void GBfromText(char *data, const size_t lendata, GB_ARRAY *array);
    static Node** fromText(char const *data, const size_t lendata, size_t *nodeCount) throw(XMLParseException);    
    
    //String output
    virtual void addStringLen(size_t &len, int indent = -1) = 0;//Calculates the node's string representation length, and adds it to len (recursive)
    virtual void addString(char *&data, int indent = -1) = 0;//Puts the string represenetation into data, and increments it (recursive)
    void toString(char *&output, size_t &len,int indent = -1);//Converts the node to its string representation
    void toGBString(char *&output, size_t &len, int indent = -1);
    
    virtual void setTextContent(const char *ncontent, const size_t nlen) = 0;//Sets the plain text conent of a node
    virtual void addTextContentLen(size_t &len) = 0;
    virtual void addTextContent(char *&data) = 0;
    void GBTextContent(char *&output, size_t &len);
    //Gambas object
    virtual void NewGBObject() = 0;//Instanciates a new Gambas XmlElement object linked to the element
    CNode *GBObject;
    
    //User data
    GB_COLLECTION userData;
    GB_VARIANT *getUserData(const char *key, const size_t lenkey);
    void addUserData(const char *key, const size_t lenkey, GB_VARIANT *value);
};

#endif // NODE_H
