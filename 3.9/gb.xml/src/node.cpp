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
#include "textnode.h"
#include "parser.h"
#include "document.h"
#include "CNode.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

bool Node_NoInstanciate = false;//If true, newly-created Gambas objects won't instanciate a new node

void XMLNode_Init(Node *node, Node::Type nodeType)
{
    memset(node, 0, sizeof(Node));
    node->type = nodeType;
}

void XMLNode_Free(Node *&node)//TODO: Handle per-node type freeing
{
    if(!node) return;
    if(node->userData)
    {
        GB.Unref(POINTER(&(node->userData)));
        node->userData = 0;
    }
    switch(node->type)
    {
    case Node::ElementNode:
        XMLElement_Free((Element*)node);
        break;
    case Node::DocumentNode:
        XMLDocument_Release((Document*)node);
        break;
    case Node::NodeText:
    case Node::CDATA:
    case Node::Comment:
        XMLTextNode_Free((TextNode*)node);
        break;
    default:
        return;
        break;
    }
    node = 0;
}

CNode* XMLNode_GetGBObject(Node *node)
{
    if(!node->GBObject)
    {
        XMLNode_NewGBObject(node);
    }
    return node->GBObject;
}

void XMLNode_NewGBObject(Node *node)
{
    Node_NoInstanciate = true;
    switch(node->type)
    {
    case Node::ElementNode:
        node->GBObject = (CNode*)GB.New(GB.FindClass("XmlElement"), 0, 0);
        break;
    case Node::CDATA:
        node->GBObject = (CNode*)GB.New(GB.FindClass("XmlCDataNode"), 0, 0);
        break;
    case Node::Comment:
        node->GBObject = (CNode*)GB.New(GB.FindClass("XmlCommentNode"), 0, 0);
        break;
    case Node::NodeText:
        node->GBObject = (CNode*)GB.New(GB.FindClass("XmlTextNode"), 0, 0);
        break;
    case Node::DocumentNode:
        node->GBObject = (CNode*)GB.New(GB.FindClass("XmlDocument"), 0, 0);
        break;
    case Node::AttributeNode:
        node->GBObject = (CNode*)GB.New(GB.FindClass("XmlNode"), 0, 0);
        break;
    default:
        fprintf(stderr, "FATAL : tried to create a Gambas object with invalid type.");
        exit(EXIT_FAILURE);
        break;
    }
    node->GBObject->node = node;
    Node_NoInstanciate = false;
}

void XMLNode_DestroyGBObject(Node *&node)
{
    if((!node->parent) && (!node->parentDocument))
    {
        XMLNode_Free(node);
    }
    else
    {
        node->GBObject = 0;
    }
}

void XMLNode_DestroyParent(Node *node)
{
    if(!node->GBObject)
    {
        XMLNode_Free(node);
    }
    else
    {
        node->parent = 0;
        node->parentDocument = 0;
    }
}

/***** Node tree *****/

Document* XMLNode_GetOwnerDocument(Node *node)
{
    while(node->parent && !node->parentDocument)
        node = (Node*)(node->parent);
    return node->parentDocument;
}

void XMLNode_getGBChildren(Node *node, GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), node->childCount);
    if(!(SUPPORT_CHILDREN(node))) return;
    int i = 0;
    for(Node *tNode = node->firstChild; tNode != 0; tNode = tNode->nextNode)
    {
        *(reinterpret_cast<void **>((GB.Array.Get(*array, i)))) = XMLNode_GetGBObject(tNode);
        GB.Ref(tNode->GBObject);
        ++i;
    }
}

/***** Node tree *****/
void XMLNode_appendChild(Node *node, Node *newChild)
{
    (node->childCount)++;
    if(!(node->lastChild))//No child
    {
        node->firstChild = newChild;
        node->lastChild = newChild;
        node->lastChild->previousNode = 0;
        node->lastChild->nextNode = 0;
        newChild->parent = node;
        return;
    }
    
    newChild->previousNode = node->lastChild;
    node->lastChild->nextNode = newChild;
    node->lastChild = newChild;
    node->lastChild->nextNode = 0;
    newChild->parent = node;
    
}

void XMLNode_prependChild(Node *node, Node *newChild)
{
    node->childCount++;
    if(!node->lastChild)//No child
    {
        node->firstChild = newChild;
        node->lastChild = node->firstChild;
        node->lastChild->previousNode = 0;
        node->lastChild->nextNode = 0;
        newChild->parent = node;
        return;
    }
    
    newChild->nextNode = node->firstChild;
    node->firstChild->previousNode = newChild;
    node->firstChild = newChild;
    node->firstChild->previousNode = 0;
    newChild->parent = node;
}

void XMLNode_removeKeepChild(Node *node, Node *child)
{
    if(child == node->firstChild) node->firstChild = child->nextNode;
    if(child == node->lastChild) node->lastChild = child->previousNode;
    if(child->nextNode) child->nextNode->previousNode = child->previousNode;
    if(child->previousNode) child->previousNode->nextNode = child->nextNode;
    node->childCount--;
}

void XMLNode_removeChild(Node *node, Node *child)
{
    XMLNode_removeKeepChild(node, child);
    XMLNode_DestroyParent(child);
}

GB_VALUE *aft_args;
int aft_argsCount;

void XMLNode_appendFromTextSubstCallback(int index, char* *str, int *len)
{
    if(index < 1 || index > aft_argsCount) return;
    size_t nlen;

    XML_Format(&(aft_args[index - 1]), *str, nlen);
    *len = (int)nlen;

}

void XMLNode_substAppendFromText(Node *node, const char *data, const size_t lenData, GB_VALUE *args, int argsCount)
{
    char *newData;
    size_t lenNewData;

    aft_args = args;
    aft_argsCount = argsCount;

    newData = GB.SubstString(data, lenData, XMLNode_appendFromTextSubstCallback);
    lenNewData = GB.StringLength(newData);

    XMLNode_appendFromText(node, newData, lenNewData);
}

void XMLNode_appendFromText(Node *node, const char *data, const size_t lenData)
{
    size_t nodeCount = 0;
    Document *parentDoc = XMLNode_GetOwnerDocument(node);

    Node **nodes = parse(data, lenData, &nodeCount, parentDoc ? parentDoc->docType : XMLDocumentType);
    for(size_t i = 0; i < nodeCount; i++)
    {
        XMLNode_appendChild(node, nodes[i]);
    }
    free(nodes);
}

void XMLNode_addGBChildrenByTagName(Node *node, const char *compTagName, const size_t compLenTagName, GB_ARRAY *array, const int mode, const int depth)
{
    if(depth == 0) return;
    if(node->type == Node::ElementNode)
    {
        if(mode == GB_STRCOMP_NOCASE || mode == GB_STRCOMP_LANG + GB_STRCOMP_NOCASE)
        {
            if(compLenTagName == ((Element*)node)->lenTagName)
            {
                if(strncasecmp(compTagName, ((Element*)node)->tagName, compLenTagName) == 0)
                {
                    *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(node);
                    GB.Ref(node->GBObject);
                }
            }
        }
        else if(mode == GB_STRCOMP_LIKE)
        {
            if(GB.MatchString(compTagName, compLenTagName, ((Element*)node)->tagName, ((Element*)node)->lenTagName))
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(node);
                GB.Ref(node->GBObject);
            }
        }
        else
        {
            if(compLenTagName == ((Element*)node)->lenTagName)
            {
                if(memcmp(compTagName, ((Element*)node)->tagName, compLenTagName) == 0)
                {
                    *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(node);
                    GB.Ref(node->GBObject);
                }
            }
        }
    }
    if(depth == 1) return;
    
    for(Node *tNode = node->firstChild; tNode != 0; tNode = tNode->nextNode)
    {
        if(tNode->type == Node::ElementNode)
        {
            XMLNode_addGBChildrenByTagName(tNode, compTagName, compLenTagName, array, mode, depth - 1);
        }
    }
}

void XMLNode_addGBChildrenByNamespace(Node *node, const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode, const int depth)
{
    if(depth == 0) return;
    if(node->type != Node::ElementNode)
    {
        if(mode == GB_STRCOMP_NOCASE || mode == GB_STRCOMP_LANG + GB_STRCOMP_NOCASE)
        {
            if(lenNamespace == ((Element*)node)->lenTagName)
            {
                if(strncasecmp(cnamespace, ((Element*)node)->tagName, lenNamespace) == 0)
                {
                    *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(node);
                    GB.Ref(node->GBObject);
                }
            }
        }
        else if(mode == GB_STRCOMP_LIKE)
        {
            if(GB.MatchString(cnamespace, lenNamespace, ((Element*)node)->tagName, ((Element*)node)->lenTagName))
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(node);
                GB.Ref(node->GBObject);
            }
        }
        else
        {
            if(lenNamespace == ((Element*)node)->lenTagName)
            {
                if(memcmp(cnamespace, ((Element*)node)->tagName, lenNamespace) == 0)
                {
                    *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(node);
                    GB.Ref(node->GBObject);
                }
            }
        }
    }
    if(depth == 1) return;

    for(Node *tNode = node->firstChild; tNode != 0; tNode = node->nextNode)
    {
        if(tNode->type == Node::ElementNode)
        {
            XMLNode_addGBChildrenByTagName(tNode, cnamespace, lenNamespace, array, mode, depth - 1);
        }
    }
}

void XMLNode_addGBAllChildren(Node *node, GB_ARRAY *array)
{
    if(SUPPORT_CHILDREN(node))
    {
        for(Node *tNode = node->firstChild; tNode != 0; tNode = tNode->nextNode)
        {
            *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(tNode);
            GB.Ref(tNode->GBObject);
            XMLNode_addGBAllChildren(tNode, array);
        }
    }
            
}

void XMLNode_getGBChildrenByTagName(Node *node, const char *ctagName, const size_t clenTagName, GB_ARRAY *array, const int mode, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    XMLNode_addGBChildrenByTagName(node, ctagName, clenTagName, array, mode, depth);
}

void XMLNode_getGBChildrenByNamespace(Node *node, const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    XMLNode_addGBChildrenByNamespace(node, cnamespace, lenNamespace, array, mode, depth);
}

void XMLNode_getGBAllChildren(Node *node, GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), 0);
    XMLNode_addGBAllChildren(node, array);
}

void XMLNode_getGBChildrenByAttributeValue(Node *node, const char *attrName, const size_t lenAttrName,
                                                 const char *attrValue, const size_t lenAttrValue,
                                                 GB_ARRAY *array, const int mode, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    XMLNode_addGBChildrenByAttributeValue(node, attrName, lenAttrName, attrValue, lenAttrValue, array, mode, depth);
}

void XMLNode_addGBChildrenByAttributeValue(Node *node, const char *attrName, const size_t lenAttrName,
                                                 const char *attrValue, const size_t lenAttrValue,
                                                 GB_ARRAY *array, const int mode, const int depth)
{
    if(node->type == Node::ElementNode)
    {
        Attribute *attr = XMLElement_GetAttribute((Element*)node, attrName, lenAttrName);
        if(attr)
        {
            if(GB_MatchString(attr->attrValue, attr->lenAttrValue, attrValue, lenAttrValue, mode))
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(node);
                GB.Ref(node->GBObject);
            }
        }
    }

    if(depth == 1) return;
    for(Node *tNode = node->firstChild; tNode != 0; tNode = tNode->nextNode)
    {
        if(tNode->type == Node::ElementNode)
        {
            XMLNode_addGBChildrenByAttributeValue(tNode, attrName, lenAttrName, attrValue, lenAttrValue, array, mode, depth - 1);
        }
    }
            
}

void XMLNode_getGBChildElements(Node *node, GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    for(Node *tNode = node->firstChild; tNode != 0; tNode = tNode->nextNode)
    {
        if(!SUPPORT_CHILDREN(tNode)) continue;
        *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = XMLNode_GetGBObject(tNode);
        GB.Ref(tNode->GBObject);
    }
}

void XMLNode_addChildrenByTagName(Node *node, const char *compTagName, const size_t compLenTagName, Element** &array, size_t &lenArray, const int depth)
{
    if(depth == 0) return;
    if(node->type == Node::ElementNode)
    {
        if(compLenTagName == ((Element*)node)->lenTagName)
        {
            if(memcmp(compTagName, ((Element*)node)->tagName, compLenTagName) == 0)
            {
                array = (Element**)realloc(array, sizeof(Element*) * (lenArray + 1));
                array[lenArray] = ((Element*)node);
                ++lenArray;
            }
        }
    }
    if(depth == 1) return;
    
    if(SUPPORT_CHILDREN(node))
    {
        for(Node *tNode = node->firstChild; tNode != 0; tNode = tNode->nextNode)
        {
            XMLNode_addChildrenByTagName(tNode, compTagName, compLenTagName, array, lenArray,depth - 1);
        }
    }
}

Element** XMLNode_getChildrenByTagName(Node *node, const char *ctagName, const size_t clenTagName, size_t &lenArray, const int depth)
{
    lenArray = 0;
    Element **array = 0;
    XMLNode_addChildrenByTagName(node, ctagName, clenTagName, array, lenArray, depth);
    return array;
}

Element* XMLNode_getFirstChildByTagName(const Node *node, const char *ctagName, const size_t clenTagName, const int depth)
{
    if(depth == 0) return 0;
    if(node->type == Node::ElementNode)
    {
        if(((Element*)node)->lenTagName == clenTagName)
        {
            if(!memcmp(((Element*)node)->tagName, ctagName, clenTagName)) return ((Element*)node);
        }
    }
    if(depth == 1) return 0;
    if(!SUPPORT_CHILDREN(node)) return 0;
    Element *elmt = 0;
    for(Node *it = node->firstChild; it != 0; it = it->nextNode)
    {
        if((it)->type == Node::ElementNode)
        {
            elmt = XMLNode_getFirstChildByTagName(it, ctagName, clenTagName, depth - 1);
            if(elmt) return elmt;
        }
    }
    return 0;
}

Element* XMLNode_firstChildElement(Node *node)
{
    Node *child = node->firstChild;
    while(child != 0)
    {
        if(child->type == Node::ElementNode) return (Element*)child;
        child = child->nextNode;
    }
    
    return 0;
}

Element* XMLNode_lastChildElement(Node *node)
{
    Node *child = node->lastChild;
    while(child != 0)
    {
        if(child->type == Node::ElementNode) return (Element*)child;
        child = child->previousNode;
    }
    
    return 0;
}

Element* XMLNode_nextElement(Node *node)
{
    Node *child = node->nextNode;
    while(child != 0)
    {
        if(child->type == Node::ElementNode) return (Element*)child;
        child = child->nextNode;
    }
    
    return 0;
}

Element* XMLNode_previousElement(const Node *node)
{
    Node *child = node->previousNode;
    while(child != 0)
    {
        if(child->type == Node::ElementNode) return (Element*)child;
        child = child->previousNode;
    }
    
    return 0;
}

bool XMLNode_insertAfter(Node *node, Node *child, Node *newChild)
{
    if(child->parent != node) return false;
    newChild->nextNode = child->nextNode;
    newChild->previousNode = child;
    if(child->nextNode)
    {
        child->nextNode->previousNode = newChild;
    }
    if(child == node->lastChild)
    {
        node->lastChild = newChild;
    }
    child->nextNode = newChild;
    newChild->parent = node;
    node->childCount++;
    return true;
}

bool XMLNode_insertBefore(Node *node, Node *child, Node *newChild)
{
    if(child->parent != node) return false;
    newChild->nextNode = child;
    newChild->previousNode = child->previousNode;
    if(child->previousNode)
    {
        child->previousNode->nextNode = newChild;
    }
    if(child == node->firstChild)
    {
        node->firstChild = newChild;
    }
    child->previousNode = newChild;
    newChild->parent = node;
    node->childCount++;
    return true;
}

void XMLNode_replaceChild(Node *node, Node *oldChild, Node *newChild)
{
    if(XMLNode_insertBefore(node, oldChild, newChild))
        XMLNode_removeChild(node, oldChild);
}


void XMLNode_appendText(Node *node, const char *data, const size_t lenData)
{
    if(node->lastChild && node->lastChild->type == Node::NodeText)
    {
        TextNode *text = (TextNode*)node->lastChild;
        text->content = (char*)realloc(text->content, lenData + text->lenContent);
        memcpy(text->content + text->lenContent, data, lenData);
        text->lenContent += lenData;
    }
    else
    {
        TextNode *text = XMLTextNode_New(data, lenData);
        XMLNode_appendChild(node, text);
    }
}

void XMLNode_clearChildren(Node *node)
{
    if(node->childCount == 0) return;
    register Node* prevChild = 0;
    register Node* child = 0;
    for(child = node->firstChild->nextNode; child != 0; child = child->nextNode)
    {
        prevChild = child->previousNode;
        prevChild->nextNode = 0;
        prevChild->previousNode = 0;
        XMLNode_DestroyParent(prevChild);
    }
    node->lastChild->nextNode = 0;
    node->lastChild->previousNode = 0;
    XMLNode_DestroyParent(node->lastChild);
    
    node->childCount = 0;
    node->lastChild = 0;
    node->firstChild = 0;
}

void XMLNode_setTextContent(Node *node, const char *content, const size_t lenContent)
{
    switch(node->type)
    {
    case Node::ElementNode:
        XMLElement_SetTextContent((Element*)node, content, lenContent);
        break;
    case Node::AttributeNode:
        XMLAttribute_SetValue((Attribute*)node, content, lenContent);
    default:
        return;
    }
}

GB_VARIANT* XMLNode_getUserData(Node *node, const char *key, const size_t lenkey)
{
    if(!node->userData) return 0;
    GB_VARIANT *srcValue = new GB_VARIANT;
    if (GB.Collection.Get(node->userData, key, lenkey, srcValue)) return 0;
    return srcValue;
}

void XMLNode_addUserData(Node *node, const char *key, const size_t lenkey, GB_VARIANT *value)
{
    if(!node->userData)
    {
        GB.Collection.New(POINTER(&(node->userData)), GB_COMP_BINARY);
    }
    
    GB.Collection.Set(node->userData, key, lenkey, value);
}


bool XMLNode_NoInstanciate()
{
    return Node_NoInstanciate;
}

void XML_ReturnNode(Node *node)
{
    if(!node)
    {
        GB.ReturnNull(); return;
    }
    if(!(node->GBObject))
    {
        XMLNode_NewGBObject(node);
    }
    GB.ReturnObject(node->GBObject);
}


Element *XMLNode_getFirstChildByAttributeValue(Node *node, const char *attrName, const size_t lenAttrName, const char *attrValue, const size_t lenAttrValue, const int mode, const int depth)
{
    if(depth == 0) return 0;

    if(SUPPORT_CHILDREN(node))
    {
        for(Node *child = node->firstChild; child != 0; child = child->nextNode)
        {
            if(child->type == Node::ElementNode)
            {
                Element *elmt;
                Attribute *attr;
                attr = XMLElement_GetAttribute((Element*)child, attrName, lenAttrName);
                if(attr)
                {
                    if(GB_MatchString(attr->attrValue, attr->lenAttrValue, attrValue, lenAttrValue, mode))
                    {
                        return (Element*)child;
                    }
                }
                elmt = XMLNode_getFirstChildByAttributeValue(child, attrName, lenAttrName, attrValue, lenAttrValue, mode, depth - 1);
                if(elmt) return elmt;
            }
        }
    }

    return 0;
}
