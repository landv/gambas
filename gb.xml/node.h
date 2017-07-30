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

#include "gb.xml.h"

#define SUPPORT_CHILDREN(__node) ((__node->type == Node::ElementNode) || (__node->type == Node::DocumentNode))

class Element;
class TextNode;
class Document;

struct CNode;

void XMLNode_Init(Node* node, Node::Type nodeType);
void XMLNode_Free(Node* &node);

void XMLNode_DestroyGBObject(Node* &node);
void XMLNode_DestroyParent(Node *node);

void XMLNode_NewGBObject(Node *node);
CNode* XMLNode_GetGBObject(Node *node);

//Node tree
void XMLNode_getGBChildren(Node *node, GB_ARRAY *array);
Document* XMLNode_GetOwnerDocument(Node *node);
void XMLNode_appendChild(Node *node, Node *newChild);
void XMLNode_prependChild(Node *node, Node *newChild);
void XMLNode_removeChild(Node *node, Node *child);
void XMLNode_removeKeepChild(Node *node, Node *child);
void XMLNode_replaceChild(Node *node, Node *oldChild, Node *newChild);
bool XMLNode_insertAfter(Node *node, Node *child, Node *newChild);
bool XMLNode_insertBefore(Node *node, Node *child, Node *newChild);
void XMLNode_appendText(Node *node, const char *data, const size_t lenData);
void XMLNode_clearChildren(Node *node);
void XMLNode_appendFromText(Node *node, const char *data, const size_t lenData);

//Searching elements
void XMLNode_getGBChildrenByNamespace(Node *node, const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
void XMLNode_addGBChildrenByNamespace(Node *node, const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
void XMLNode_getGBChildrenByTagName(Node *node, const char *ctagName, const size_t clenTagName,  GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
void XMLNode_addGBChildrenByTagName(Node *node, const char *compTagName, const size_t compLenTagName, GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
void XMLNode_getGBChildrenByAttributeValue(Node *node, const char *attrName, const size_t lenAttrName,
                                   const char *attrValue, const size_t lenAttrValue,
                                   GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
void XMLNode_addGBChildrenByAttributeValue(Node *node, const char *attrName, const size_t lenAttrName,
                                   const char *attrValue, const size_t lenAttrValue,
                                   GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
Element* XMLNode_getFirstChildByAttributeValue(Node *node, const char *attrName, const size_t lenAttrName,
                                               const char *attrValue, const size_t lenAttrValue, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
Element** XMLNode_getChildrenByTagName(Node *node, const char *ctagName, const size_t clenTagName, size_t &lenArray, const int depth = -1);
Element* XMLNode_getFirstChildByTagName(const Node *node, const char *ctagName, const size_t clenTagName, const int depth = -1);
void XMLNode_addChildrenByTagName(Node *node, const char *compTagName, const size_t compLenTagName, Element** &array, size_t &lenArray, const int depth = -1);
void XMLNode_getGBAllChildren(Node *node, GB_ARRAY *array);
void XMLNode_addGBAllChildren(Node *node, GB_ARRAY *array);
void XMLNode_getGBChildElements(Node *node, GB_ARRAY *array);

Element* XMLNode_firstChildElement(Node *node);
Element* XMLNode_lastChildElement(Node *node);
Element* XMLNode_previousElement(const Node *node);
Element* XMLNode_nextElement(Node *node);

void XMLNode_setTextContent(Node *node, const char *content, const size_t lenContent);//Sets the plain text conent of a node



bool XMLNode_NoInstanciate();

void XML_ReturnNode(Node *node);

#endif // NODE_H

#if !defined(NODE_GBINTERFACE) && defined(GBINTERFACE_H)
#define NODE_GBINTERFACE
void XMLNode_substAppendFromText(Node *node, const char *data, const size_t lenData, GB_VALUE *args, int argsCount);
GB_VARIANT *XMLNode_getUserData(Node *node, const char *key, const size_t lenkey);
void XMLNode_addUserData(Node *node, const char *key, const size_t lenkey, GB_VARIANT *value);
#endif
