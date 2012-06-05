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
    Document* GetOwnerDocument();
    Document *parentDocument;
    Element *parent;
    Node *nextNode;
    Node *previousNode;
    
    //Node Types
    enum Type {ElementNode, NodeText, Comment, CDATA, AttributeNode};
    virtual Type getType() = 0;//Returns the type of the node
    bool isElement();
    bool isText();
    bool isComment();
    bool isTextNode();
    Element* toElement();
    TextNode* toTextNode();
    
    //String output
    virtual void addStringLen(size_t *len, int indent = 0) = 0;//Calculates the node's string representation length, and adds it to len (recursive)
    virtual void addString(char **data, int indent = 0) = 0;//Puts the string represenetation into data, and increments it (recursive)
    void toString(char **output, size_t *len);//Converts the node to its string representation
    void toGBString(char *&output, size_t &len, int indent = 0);
    
    virtual void setTextContent(const char *ncontent, const size_t nlen) = 0;//Sets the plain text conent of a node
    virtual void addTextContentLen(size_t &len) = 0;
    virtual void addTextContent(char *&data) = 0;
    void GBTextContent(char *&output, size_t &len);
    //Gambas object
    virtual void NewGBObject() = 0;//Instanciates a new Gambas XmlElement object linked to the element
    CNode *GBObject;
};

#endif // NODE_H
