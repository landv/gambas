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
#include <string>

bool Node::NoInstanciate = false;

Node::Node() : parentDocument(0), parent(0), nextNode(0), previousNode(0), GBObject(0), userData(0)
{
    firstChild = 0;
    lastChild = 0;
    childCount = 0;
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
    GB.Array.New(array, GB.FindClass("XmlNode"), childCount);
    if(!this->isElement()) return;
    int i = 0;
    for(Node *node = this->firstChild; node != 0; node = node->nextNode)
    {
        *(reinterpret_cast<void **>((GB.Array.Get(*array, i)))) = node->GetGBObject();
        GB.Ref(node->GBObject);
        ++i;
    }
}

/***** Node tree *****/
void Node::appendChild(Node *newChild)
{
    childCount++;
    if(!lastChild)//No child
    {
        firstChild = newChild;
        lastChild = firstChild;
        lastChild->previousNode = 0;
        lastChild->nextNode = 0;
        newChild->parent = this;
        return;
    }
    
    newChild->previousNode = lastChild;
    lastChild->nextNode = newChild;
    lastChild = newChild;
    lastChild->nextNode = 0;
    newChild->parent = this;

    
}

void Node::prependChild(Node *newChild)
{
    childCount++;
    if(!lastChild)//No child
    {
        firstChild = newChild;
        lastChild = firstChild;
        lastChild->previousNode = 0;
        lastChild->nextNode = 0;
        newChild->parent = this;
        return;
    }
    
    newChild->nextNode = firstChild;
    firstChild->previousNode = newChild;
    firstChild = newChild;
    firstChild->previousNode = 0;
    newChild->parent = this;
}

void Node::removeKeepChild(Node *child)
{
    if(child == firstChild) firstChild = child->nextNode;
    if(child == lastChild) lastChild = child->previousNode;
    if(child->nextNode) child->nextNode->previousNode = child->previousNode;
    if(child->previousNode) child->previousNode->nextNode = child->nextNode;
    childCount--;
}

void Node::removeChild(Node *child)
{
    removeKeepChild(child);
    child->DestroyParent();
}

void Node::appendFromText(char *data, const size_t lenData)
{
    size_t nodeCount = 0;
    Node **nodes = Element::fromText(data, lenData, &nodeCount);
    for(size_t i = 0; i < nodeCount; i++)
    {
        appendChild(nodes[i]);
    }
    free(nodes);
}

void Node::addGBChildrenByTagName(const char *compTagName, const size_t compLenTagName, GB_ARRAY *array, const int mode, const int depth)
{
    if(depth == 0) return;
    if(!this->isElement()) return;
    if(mode == GB_STRCOMP_NOCASE || mode == GB_STRCOMP_LANG + GB_STRCOMP_NOCASE)
    {
        if(compLenTagName == this->toElement()->lenTagName)
        {
            if(strncasecmp(compTagName, this->toElement()->tagName, compLenTagName) == 0)
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
                GB.Ref(GBObject);
            }
        }
    }
    else if(mode == GB_STRCOMP_LIKE)
    {
        if(GB.MatchString(compTagName, compLenTagName, this->toElement()->tagName, this->toElement()->lenTagName))
        {
            *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
            GB.Ref(GBObject);
        }
    }
    else
    {
        if(compLenTagName == this->toElement()->lenTagName)
        {
            if(memcmp(compTagName, this->toElement()->tagName, compLenTagName) == 0)
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
                GB.Ref(GBObject);
            }
        }
    }
    if(depth == 1) return;
    
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addGBChildrenByTagName(compTagName, compLenTagName, array, mode, depth - 1);
        }
    }
}

void Node::addGBChildrenByNamespace(const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode, const int depth)
{
    if(depth == 0) return;
    if(mode == GB_STRCOMP_NOCASE || mode == GB_STRCOMP_LANG + GB_STRCOMP_NOCASE)
    {
        if(lenNamespace == this->toElement()->lenPrefix)
        {
            if(strncasecmp(cnamespace, this->toElement()->prefix, this->toElement()->lenPrefix) == 0)
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
                GB.Ref(GBObject);
            }
        }
    }
    else if(mode == GB_STRCOMP_LIKE)
    {
        if(GB.MatchString(cnamespace, lenNamespace, this->toElement()->prefix, this->toElement()->lenPrefix))
        {
            *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
            GB.Ref(GBObject);
        }
    }
    else
    {
        if(lenNamespace == this->toElement()->lenPrefix)
        {
            if(memcmp(cnamespace, this->toElement()->prefix, lenNamespace) == 0)
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
                GB.Ref(GBObject);
            }
        }
    }
    if(depth == 1) return;

    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addGBChildrenByTagName(cnamespace, lenNamespace, array, mode, depth - 1);
        }
    }
}

void Node::addGBAllChildren(GB_ARRAY *array)
{
    *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
    GB.Ref(GBObject);
    
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addGBAllChildren(array);
        }
    }
            
}

void Node::getGBChildrenByTagName(const char *ctagName, const size_t clenTagName, GB_ARRAY *array, const int mode, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    addGBChildrenByTagName(ctagName, clenTagName, array, mode, depth);
}

void Node::getGBChildrenByNamespace(const char *cnamespace, const size_t lenNamespace, GB_ARRAY *array, const int mode, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    addGBChildrenByNamespace(cnamespace, lenNamespace, array, mode, depth);
}

void Node::getGBAllChildren(GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), 0);
    addGBAllChildren(array);
}

void Node::getGBChildrenByAttributeValue(const char *attrName, const size_t lenAttrName,
                                                 const char *attrValue, const size_t lenAttrValue,
                                                 GB_ARRAY *array, const int mode, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), 0);
    addGBChildrenByAttributeValue(attrName, lenAttrName, attrValue, lenAttrValue, array, mode, depth);
}

void Node::addGBChildrenByAttributeValue(const char *attrName, const size_t lenAttrName,
                                                 const char *attrValue, const size_t lenAttrValue,
                                                 GB_ARRAY *array, const int mode, const int depth)
{
    Attribute *attr = this->toElement()->getAttribute(attrName, lenAttrName, mode);
    if(attr)
    {
        if(mode == GB_STRCOMP_NOCASE || mode == GB_STRCOMP_LANG + GB_STRCOMP_NOCASE)
        {
            if(attr->lenAttrValue == lenAttrValue)
            {
                if(!strncasecmp(attr->attrValue, attrValue, lenAttrValue))
                {
                    *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
                    GB.Ref(GBObject);
                }
            }
        }
        else if(mode == GB_STRCOMP_LIKE)
        {
            if(GB.MatchString(attr->attrValue, attr->lenAttrValue, attrValue, lenAttrValue))
            {
                *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
                GB.Ref(GBObject);
            }
        }
        else
        {
            if(attr->lenAttrValue == lenAttrValue)
            {
                if(!memcmp(attr->attrValue, attrValue, lenAttrValue))
                {
                    *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
                    GB.Ref(GBObject);
                }
            }
        }
    }
    if(depth == 1) return;
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addGBChildrenByAttributeValue(attrName, lenAttrName, attrValue, lenAttrValue, array, mode, depth - 1);
        }
    }
            
}

void Node::getGBChildElements(GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(!node->isElement()) continue;
        *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = node->GetGBObject();
        GB.Ref(node->GBObject);
    }
}

void Node::addChildrenByTagName(const char *compTagName, const size_t compLenTagName, Element** &array, size_t &lenArray, const int depth)
{
    if(depth == 0) return;
    if(this->isElement())
    {
        if(compLenTagName == this->toElement()->lenTagName)
        {
            if(memcmp(compTagName, this->toElement()->tagName, compLenTagName) == 0) 
            {
                array = (Element**)realloc(array, sizeof(Element*) * (lenArray + 1));
                array[lenArray] = this->toElement();
                ++lenArray;
            }
        }
    }
    if(depth == 1) return;
    
    if(SUPPORT_CHILDREN(this))
    {
        for(Node *node = firstChild; node != 0; node = node->nextNode)
        {
            if(node->isElement())
            {
                node->toElement()->addChildrenByTagName(compTagName, compLenTagName, array, lenArray,depth - 1);
            }
        }
    }
}

Element** Node::getChildrenByTagName(const char *ctagName, const size_t clenTagName, size_t &lenArray, const int depth)
{
    lenArray = 0;
    Element **array = 0;
    addChildrenByTagName(ctagName, clenTagName, array, lenArray, depth);
    return array;
}

Element* Node::getFirstChildByTagName(const char *ctagName, const size_t clenTagName, const int depth)
{
    if(depth == 0) return 0;
    if(this->isElement())
    {
        if(this->toElement()->lenTagName == clenTagName) 
        {
            if(!memcmp(this->toElement()->tagName, ctagName, clenTagName)) return this->toElement();
        }
    }
    if(depth == 1) return 0;
    if(!SUPPORT_CHILDREN(this)) return 0;
    Element *elmt = 0;
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            elmt = (it)->toElement()->getFirstChildByTagName(ctagName, clenTagName, depth - 1);
            if(elmt) return elmt;
        }
    }
    return 0;
}

Element* Node::firstChildElement()
{
    Node *child = firstChild;
    while(child != 0)
    {
        if(child->isElement()) return (Element*)child;
        child = child->nextNode;
    }
    
    return 0;
}

Element* Node::lastChildElement()
{
    Node *child = lastChild;
    while(child != 0)
    {
        if(child->isElement()) return (Element*)child;
        child = child->previousNode;
    }
    
    return 0;
}

Element* Node::nextElement()
{
    Node *child = this->nextNode;
    while(child != 0)
    {
        if(child->isElement()) return (Element*)child;
        child = child->nextNode;
    }
    
    return 0;
}

Element* Node::previousElement()
{
    Node *child = this->previousNode;
    while(child != 0)
    {
        if(child->isElement()) return (Element*)child;
        child = child->previousNode;
    }
    
    return 0;
}

bool Node::insertAfter(Node *child, Node *newChild)
{
    if(child->parent != this) return false;
    newChild->nextNode = child->nextNode;
    newChild->previousNode = child;
    if(child->nextNode)
    {
        child->nextNode->previousNode = newChild;
    }
    if(child == lastChild)
    {
        lastChild = newChild;
    }
    child->nextNode = newChild;
    newChild->parent = this;
    childCount++;
    return true;
}

bool Node::insertBefore(Node *child, Node *newChild)
{
    if(child->parent != this) return false;
    newChild->nextNode = child;
    newChild->previousNode = child->previousNode;
    if(child->previousNode)
    {
        child->previousNode->nextNode = newChild;
    }
    if(child == firstChild)
    {
        firstChild = newChild;
    }
    child->previousNode = newChild;
    newChild->parent = this;
    childCount++;
    return true;
}

void Node::replaceChild(Node *oldChild, Node *newChild)
{
    if(insertBefore(oldChild, newChild))
        removeChild(oldChild);
}


void Node::appendText(const char *data, const size_t lenData)
{
    if(lastChild && lastChild->isTextNode())
    {
        TextNode *text = lastChild->toTextNode();
        text->content = (char*)realloc(text->content, lenData + text->lenContent);
        memcpy(text->content + text->lenContent, data, lenData);
        text->lenContent += lenData;
    }
    else
    {
        TextNode *text = new TextNode(data, lenData);
        appendChild(text);
    }
}

void Node::clearChildren()
{
    if(childCount == 0) return;
    register Node* prevChild = 0;
    register Node* child = 0;
    for(child = firstChild->nextNode; child != 0; child = child->nextNode)
    {
        prevChild = child->previousNode;
        prevChild->nextNode = 0;
        prevChild->previousNode = 0;
        prevChild->DestroyParent();
    }
    lastChild->nextNode = 0;
    lastChild->previousNode = 0;
    lastChild->DestroyParent();
    
    childCount = 0;
    lastChild = 0;
    firstChild = 0;
}

/*****    Parser     *****/

void Node::GBfromText(char *data, const size_t lendata, GB_ARRAY *array)
{
    size_t nodeCount;
    size_t i = 0;
    Node **nodes = fromText(data, lendata, &nodeCount);
    GB.Array.New(array, GB.FindClass("XmlNode"), nodeCount);
    
    for(i = 0; i < nodeCount; ++i)
    {
        *(reinterpret_cast<void **>((GB.Array.Get(*array, i)))) = nodes[i]->GetGBObject();
        GB.Ref(nodes[i]->GBObject);
    }
    
    free(nodes);
}

//Ajoute 'elmt' à la liste
#define APPEND(_elmt) if(curElement == 0)\
{\
    (*nodeCount)++;\
    elements = (Node**)realloc(elements, sizeof(Node*) * (*nodeCount));\
    elements[(*nodeCount) - 1] = _elmt;\
}\
else \
{\
    curElement->appendChild(_elmt);\
}

Node** Node::fromText(char const *data, const size_t lendata, size_t *nodeCount) throw(XMLParseException)
{
    *nodeCount = 0;
    if(!lendata || !data) return 0; //Empty ?
    
    const char *endData = data + lendata;
    
    Node **elements = 0;//Elements to return
    Element *curElement = 0;//Current element
    
    
    register char s = 0;//Current byte (value)
    register char const *pos = data;//Current byte (position)
    register wchar_t ws = 0;//Current character (value)
    
    char *tag = 0;//First '<' character found
    
    while(pos < endData)//Start
    {
        tag = (char*)memchr(pos, CHAR_STARTTAG, endData - pos);//On cherche un début de tag
        
        if(tag && (tag - pos) != 0)//On ajoute le texte, s'il existe
        {
            //Checking length
            char const *textpos = pos;
            size_t textlen = tag - pos;
            Trim(textpos, textlen);
            if(textlen != 0)
            {
                TextNode *text = new TextNode;
                text->setEscapedTextContent(textpos, textlen);
                APPEND(text);
            }
        }
        
        if(!tag)
        {
            if(pos < endData)//Il reste du texte
            {
                //Checking length
                char const *textpos = pos;
                size_t textlen = endData - pos;
                Trim(textpos, textlen);
                if(textlen != 0)
                {
                    TextNode *text = new TextNode;
                    text->setEscapedTextContent(textpos, textlen);
                    APPEND(text);
                }
            }
            break;
        }
        
        tag++;
        pos = tag;//On avance au caractère trouvé
        
        //On analyse le contenu du tag
        ws = nextUTF8Char(pos, endData - pos);//On prend le premier caractère
        
        if(!isNameStartChar(ws))//Ce n'est pas un tagName, il y a quelque chose ...
        {
            if(ws == CHAR_SLASH)//C'est un élément de fin
            {
                if(!curElement)//Pas d'élément courant
                {
                    //ERREUR : CLOSING TAG WHEREAS NONE IS OPEN
                    throw(XMLParseException("Closing tag whereas none is open",
                                            data, lendata, pos - 1));
                    
                }
                if((endData) < pos + curElement->lenTagName)//Impossible que les tags correspondent
                {
                    //ERREUR : TAG MISMATCH
                    throw(XMLParseException("Tag mismatch",
                    data, lendata, pos - 1));
                }
                /*else if(curElement->prefix)//Gestion  du préfixe
                {
                    if((endData) < pos + curElement->lenTagName + curElement->lenPrefix + 1)//Impossible que les tags correspondent
                    {
                        //ERREUR : TAG MISMATCH
                        throw(XMLParseException("Tag mismatch",
                        data, lendata, pos - 1));
                    }
                    else if(memcmp(pos, curElement->prefix, curElement->lenPrefix) != 0 ||
                            *(pos + curElement->lenPrefix) != ':' ||
                            memcmp(pos + curElement->lenPrefix + 1, curElement->tagName, curElement->lenTagName) != 0)
                    {
                        //ERREUR : TAG MISMATCH
                        throw(XMLParseException("Tag mismatch",
                        data, lendata, pos - 1));
                    }
                    else
                    {
                        pos += curElement->lenTagName + curElement->lenPrefix + 1;
                        curElement = (Element*)(curElement->parent);
                        tag = (char*)memchr(pos, CHAR_ENDTAG, endData - pos);//On cherche la fin du ta
                        pos = tag + 1;//On avance à la fin du tag

                        continue;
                    }
                }*/
                //Les tags ne correspondent pas
                else if(memcmp(pos, curElement->tagName, curElement->lenTagName) != 0)
                {
                    //ERREUR : TAG MISMATCH
                    throw(XMLParseException("Tag mismatch",
                    data, lendata, pos - 1));
                }
                else//Les tags correspondent, on remonte
                {
                    pos += curElement->lenTagName;
                    curElement = (Element*)(curElement->parent);
                    tag = (char*)memchr(pos, CHAR_ENDTAG, endData - pos);//On cherche la fin du ta
                    pos = tag + 1;//On avance à la fin du tag
                    
                    continue;
                }
            }
            else if(ws == CHAR_EXCL)//Ce serait un commentaire ou un CDATA
            {
                if(memcmp(pos, "--", 2) == 0)//C'est bien un commentaire
                {
                    pos += 2;//On va au début du contenu du commentaire
                    tag = (char*)memchrs(pos, endData - pos, "-->", 3);
                    if(!tag)//Commentaire sans fin
                    {
                        //ERREUR : NEVER-ENDING COMMENT
                        throw(XMLParseException("Never-ending comment",
                        data, lendata, pos - 1));
                    }
                    
                    CommentNode *comment = new CommentNode;
                    comment->setEscapedTextContent(pos, tag - pos);
                    APPEND(comment);
                    pos = tag + 3;
                    continue;
                }
                else if(memcmp(pos, "[CDATA[", 7) == 0)//C'est un CDATA
                {
                    pos += 7;//On va au début du contenu du cdata
                    tag = (char*)memchrs(pos, endData - pos, "]]>", 3);
                    if(!tag)//Cdata sans fin
                    {
                        //ERREUR : UNENDED CDATA
                        throw(XMLParseException("Never-ending CDATA",
                        data, lendata, pos - 1));
                    }
                    
                    CDATANode *cdata = new CDATANode;
                    cdata->setEscapedTextContent(pos, tag - pos);
                    APPEND(cdata);
                    pos = tag + 3;
                    continue;
                }
                else if(memcmp(pos, "DOCTYPE", 7) == 0)//Doctypes are silently ignored for the moment
                {
                    pos += 7;
                    tag = (char*)memchr(pos, '>', endData - pos);
                    if(!tag)//Doctype sans fin
                    {
                        throw(XMLParseException("Never-ending DOCTYPE",
                        data, lendata, pos - 1));
                    }
                    
                    pos = tag + 1;
                    continue;
                }
                else// ... ?
                {
                    //ERREUR : INVALID TAG
                    throw(XMLParseException("Invalid Tag",
                    data, lendata, pos - 1));
                }
            }
            else// ... ?
            {
                //ERREUR : INVALID TAG
                throw(XMLParseException("Invalid Tag",
                data, lendata, pos - 1));
            }
        }//Si tout va bien, on a un nouvel élément
        else
        {
            while(isNameChar(nextUTF8Char(pos, endData - pos)))//On cherche le tagName
            {
                if(pos > endData)
                {
                    //ERREUR : NEVER-ENDING TAG
                    throw(XMLParseException("Never-ending tag",
                    data, lendata, pos - 1));
                }
            }
            pos--;

            Element *elmt = new Element(tag, pos - tag);
            APPEND(elmt);
            curElement = elmt;
            s = *pos;
            
            while(pos < endData)//On gère le contenu de l'élément (attributs)
            {
                if(s == CHAR_ENDTAG) break;//Fin de l'élément
                if(s == CHAR_SLASH) //Élément auto-fermant
                {
                    pos++;
                    curElement = (Element*)(curElement->parent);//Pas d'enfants, on remonte
                    break;
                }
                
                if(isNameStartChar(s))//Début d'attribut
                {
                    const char *attrNamestart = pos;
                    while(isNameChar(nextUTF8Char(pos, endData - pos)) && pos < endData){}//On parcourt le nom d'attribut
                    pos--;
                    const char *attrNameEnd = pos;
                    s = *pos;
                    while(isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs
                    
                    if(s != CHAR_EQUAL)
                    {
                        elmt->addAttribute(attrNamestart, attrNameEnd - attrNamestart);
                        if(s == CHAR_ENDTAG) break;//Fin de l'élément
                        else if (s == CHAR_SLASH)//Élément auto-fermant
                        {
                            pos++;
                            curElement = curElement->parent->toElement();//Pas d'enfants, on remonte
                            break;
                        }
                        else
                        {
                            //ERREUR : INVALID TAG
                            throw(XMLParseException("Invalid tag",
                            data, lendata, pos - 1));
                        }
                    }
                    
                    pos++; s = *pos;
                    
                    while(isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs
                    
                    char delimiter = s;
                    if(delimiter != CHAR_DOUBLEQUOTE && delimiter != CHAR_SINGLEQUOTE)
                    {
                        //ERREUR : EXPECTED ATTRIBUTE DELIMITER
                        throw(XMLParseException("Expected attribute delimiter",
                        data, lendata, pos - 1));
                    }
                    pos++;
                    
                    char* delimiterPos = (char*)memchr(pos, delimiter, endData - pos);
                    
                    elmt->addAttribute(attrNamestart, attrNameEnd - attrNamestart,
                                       pos, delimiterPos - pos);
                    pos = delimiterPos;
                    
                }
                
                pos++; s = *pos;
            }
            
        }
        pos++;
        
    }

    
    return elements;
    
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
void Node::toString(char *&output, size_t &len, int indent)
{
    len = 0; addStringLen(len, indent);
    output = (char*)malloc(sizeof(char) * (len));
    addString(output, indent);
    output -= len;
}

void Node::toGBString(char *&output, size_t &len, int indent)
{
    len = 0; addStringLen(len, indent);
    output = GB.TempString(0, len);
    addString(output, indent);
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
