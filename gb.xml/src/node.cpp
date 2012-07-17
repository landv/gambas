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

#include "node.h"
#include "element.h"
#include <string>

bool Node::NoInstanciate = false;

Node::Node() : parentDocument(0), parent(0), nextNode(0), previousNode(0), GBObject(0), userData(0)
{
}

Node::~Node()
{
    if(userData)
    {
        GB.Unref(POINTER(&userData));
    }
}

CNode* Node::GetGBObject()
{
    if(!GBObject)
    {
        NewGBObject();
    }
    
    return GBObject;
}

void Node::DestroyGBObject()
{
    if((!parent) && (!parentDocument))
    {
        delete this;
    }
    else
    {
        GBObject = 0;
    }
}

void Node::DestroyParent()
{
    if(!GBObject)
    {
        delete this;
    }
    else 
    {
        
        parent = 0;
        parentDocument = 0;
    }
}

/***** Node tree *****/
Document* Node::GetOwnerDocument()
{
    Node *node = this;
    while(node->parent && !node->parentDocument)
        node = (Node*)(node->parent);
    return node->parentDocument;
}

void Node::getGBChildren(GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), this->isElement() ? this->toElement()->childCount : 0);
    if(!this->isElement()) return;
    int i = 0;
    for(Node *node = this->toElement()->firstChild; node != 0; node = node->nextNode)
    {
        *(reinterpret_cast<void **>((GB.Array.Get(*array, i)))) = node->GetGBObject();
        GB.Ref(node->GBObject);
        ++i;
    }
}

/***** Node types *****/

bool Node::isElement()
{
    return getType() == ElementNode;
}

Element *Node::toElement()
{
    if(isElement()) return reinterpret_cast<Element*>(this);
    return 0;
}

bool Node::isText()
{
    return getType() == NodeText || getType() == Comment || getType() == CDATA;
}

bool Node::isComment()
{
    return getType() == Comment;
}

bool Node::isTextNode()
{
    return getType() == NodeText;
}

bool Node::isCDATA()
{
    return getType() == CDATA;
}

TextNode* Node::toTextNode()
{
    if(isText()) return reinterpret_cast<TextNode*>(this);
    return 0;
}

/***** String output *****/
void Node::toString(char **output, size_t *len)
{
    *len = 0; addStringLen(len);
    *output = (char*)malloc(sizeof(char) * (*len));
    addString(output);
    (*output) -= (*len);
}

void Node::toGBString(char *&output, size_t &len, int indent)
{
    len = 0; addStringLen(&len, indent);
    output = GB.TempString(0, len);
    addString(&output, indent);
    output -= len;
}

void Node::GBTextContent(char *&output, size_t &len)
{
    len = 0; addTextContentLen(len);
    output = GB.TempString(0, len);
    addTextContent(output);
    output -= len;
}

GB_VARIANT* Node::getUserData(const char *key, const size_t lenkey)
{
    if(!userData) return 0;
    GB_VARIANT *srcValue = new GB_VARIANT;
    if (GB.Collection.Get(userData, key, lenkey, srcValue)) return 0;
    return srcValue;
}

void Node::addUserData(const char *key, const size_t lenkey, GB_VARIANT *value)
{
    if(!userData)
    {
        GB.Collection.New(POINTER(&userData), GB_COMP_BINARY);
    }
    
    GB.Collection.Set(userData, key, lenkey, value);
}
