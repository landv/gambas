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
    firstChild = 0;
    lastChild = 0;
    childCount = 0;
    firstAttribute = 0;
    lastAttribute = 0;
    attributeCount = 0;
}

Element::Element(const char *ntagName, size_t nlenTagName) : Node()
{
    lenTagName = nlenTagName;
    tagName = (char*)malloc(sizeof(char) * lenTagName);
    memcpy(tagName, ntagName, lenTagName);
    
    //DEBUG << std::string(tagName, lenTagName) << endl;
    
    firstChild = 0;
    lastChild = 0;
    childCount = 0;
    firstAttribute = 0;
    lastAttribute = 0;
}

Element::~Element()
{
    //Releasing tag name
    if(tagName) free(tagName);
    
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

/***** Node tree *****/
void Element::appendChild(Node *newChild)
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

void Element::prependChild(Node *newChild)
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

void Element::removeKeepChild(Node *child)
{
    if(child == firstChild) firstChild = child->nextNode;
    if(child == lastChild) lastChild = child->previousNode;
    if(child->nextNode) child->nextNode->previousNode = child->previousNode;
    if(child->previousNode) child->previousNode->nextNode = child->nextNode;
    childCount--;
}

void Element::removeChild(Node *child)
{
    removeKeepChild(child);
    child->DestroyParent();
}

void Element::appendFromText(char *data, const size_t lenData)
{
    size_t nodeCount = 0;
    Node **nodes = Element::fromText(data, lenData, &nodeCount);
    for(size_t i = 0; i < nodeCount; i++)
    {
        appendChild(nodes[i]);
    }
    free(nodes);
}

void Element::addGBChildrenByTagName(const char *compTagName, const size_t compLenTagName, GB_ARRAY *array, const int depth)
{
    if(depth == 0) return;
    if(compLenTagName == lenTagName)
    {
        if(memcmp(compTagName, tagName, lenTagName) == 0) 
        {
            *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = GetGBObject();
            GB.Ref(GBObject);
        }
    }
    if(depth == 1) return;
    
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addGBChildrenByTagName(compTagName, compLenTagName, array, depth - 1);
        }
    }
}

void Element::addGBAllChildren(GB_ARRAY *array)
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

void Element::getGBChildrenByTagName(const char *ctagName, const size_t clenTagName, GB_ARRAY *array, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), 0);
    addGBChildrenByTagName(ctagName, clenTagName, array, depth);
}

void Element::getGBAllChildren(GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), 0);
    addGBAllChildren(array);
}

void Element::getGBChildrenByAttributeValue(const char *attrName, const size_t lenAttrName,
                                                 const char *attrValue, const size_t lenAttrValue,
                                                 GB_ARRAY *array, const int depth)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), 0);
    addGBChildrenByAttributeValue(attrName, lenAttrName, attrValue, lenAttrValue, array, depth);
}

void Element::addGBChildrenByAttributeValue(const char *attrName, const size_t lenAttrName,
                                                 const char *attrValue, const size_t lenAttrValue,
                                                 GB_ARRAY *array, const int depth)
{
    Attribute *attr = getAttribute(attrName, lenAttrName);
    if(attr)
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
    
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addGBAllChildren(array);
        }
    }
            
}

void Element::getGBChildren(GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlNode"), childCount);
    int i = 0;
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        *(reinterpret_cast<void **>((GB.Array.Get(*array, i)))) = node->GetGBObject();
        GB.Ref(node->GBObject);
        ++i;
    }
}

void Element::getGBChildElements(GB_ARRAY *array)
{
    GB.Array.New(array, GB.FindClass("XmlElement"), childCount);
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(!node->isElement()) continue;
        *(reinterpret_cast<void **>((GB.Array.Add(*array)))) = node->GetGBObject();
        GB.Ref(node->GBObject);
    }
}

void Element::addChildrenByTagName(const char *compTagName, const size_t compLenTagName, Element** &array, size_t &lenArray, const int depth)
{
    if(depth == 0) return;
    if(compLenTagName == lenTagName)
    {
        if(memcmp(compTagName, tagName, lenTagName) == 0) 
        {
            array = (Element**)realloc(array, sizeof(Element*) * (lenArray + 1));
            array[lenArray] = this;
            ++lenArray;
        }
    }
    if(depth == 1) return;
    
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isElement())
        {
            node->toElement()->addChildrenByTagName(compTagName, compLenTagName, array, lenArray,depth - 1);
        }
    }
}

Element** Element::getChildrenByTagName(const char *ctagName, const size_t clenTagName, size_t &lenArray, const int depth)
{
    lenArray = 0;
    Element **array = 0;
    addChildrenByTagName(ctagName, clenTagName, array, lenArray, depth);
    return array;
}

Element* Element::getFirstChildByTagName(const char *ctagName, const size_t clenTagName, const int depth)
{
    if(depth == 0) return 0;
    if(lenTagName == clenTagName) 
    {
        if(!memcmp(tagName, ctagName, clenTagName)) return this;
    }
    if(depth == 1) return 0;
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

Element* Element::firstChildElement()
{
    Node *child = firstChild;
    while(child != 0)
    {
        if(child->isElement()) return (Element*)child;
        child = child->nextNode;
    }
    
    return 0;
}

Element* Element::nextSibling()
{
    Node *child = this->nextNode;
    while(child != 0)
    {
        if(child->isElement()) return (Element*)child;
        child = child->nextNode;
    }
    
    return 0;
}

Element* Element::previousSibling()
{
    Node *child = this->previousNode;
    while(child != 0)
    {
        if(child->isElement()) return (Element*)child;
        child = child->previousNode;
    }
    
    return 0;
}

bool Element::insertAfter(Node *child, Node *newChild)
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
    child->parent = this;
    childCount++;
    return true;
}

bool Element::insertBefore(Node *child, Node *newChild)
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
    child->parent = this;
    childCount++;
    return true;
}

void Element::replaceChild(Node *oldChild, Node *newChild)
{
    if(insertBefore(oldChild, newChild))
        removeChild(oldChild);
}


/***** TagName *****/

void Element::setTagName(const char *ntagName, size_t nlenTagName)
{
    
    lenTagName = nlenTagName;
    if(tagName) 
    {
        tagName = (char*)realloc(tagName, sizeof(char) * lenTagName);
    }
    else
    {
        tagName = (char*)malloc(sizeof(char) * lenTagName);
    }
    
    memcpy(tagName, ntagName, lenTagName);
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

Attribute* Element::getAttribute(const char *nattrName, const size_t nlenAttrName)
{
    for(Attribute *node = (Attribute*)(firstAttribute); node != 0; node = (Attribute*)(node->nextNode))
    {
        if(!(nlenAttrName == node->lenAttrName)) continue;
        if(memcmp(nattrName, node->attrName, nlenAttrName) == 0) return node;
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

bool Element::attributeContains(const char *attrName, size_t lenAttrName, char *value, size_t lenValue)
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
void Element::addStringLen(size_t *len)
{
    // '<' + tag + (' ' + attrName + '=' + '"' + attrValue + '"') + '>' 
    // + children + '</' + tag + '>"
    // Or, singlElement :
    // '<' + tag + (' ' + attrName + '=' + '"' + attrValue + '"') + ' />' 
    
    if(isSingle())
    {
        (*len) += (4 + lenTagName);
    }
    else
    {
        (*len) += (5 + (lenTagName * 2));
        for(Node *child = firstChild; child != 0; child = child->nextNode)
        {
            child->addStringLen(len);
        }
    }
    
    for(Attribute *attr = (Attribute*)(firstAttribute); attr != 0; attr = (Attribute*)(attr->nextNode))
    {
        (*len) += 4 + attr->lenAttrName + attr->lenAttrValue;
    }
    
}

void Element::addString(char **data)
{
    register char *content = (*data);
    bool single = isSingle();
#define ADD(_car) *content = _car; content++;
    
    //Opening tag
    ADD(CHAR_STARTTAG);
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
    
    if(!single)
    {
    
        //Content
        for(register Node *child = firstChild; child != 0; child = child->nextNode)
        {
            child->addString(&content);
        }
        
        //Ending Tag    
        ADD(CHAR_STARTTAG);
        ADD(CHAR_SLASH);
        memcpy(content, tagName, lenTagName); content += lenTagName;
        ADD(CHAR_ENDTAG); 
    
    }
    
    *data = content;
    
}

/***** Text Content *****/

void Element::setTextContent(const char *ncontent, const size_t nlen)
{
    Node *newText = 0;
    
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        if(node->isTextNode() && !newText)
        {
            newText = node;
        }
        else
        {
            removeChild(node);
        }
    }
    
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
        node->addTextContentLen(len);
    }
}

void Element::addTextContent(char *&data)
{
    for(Node *node = firstChild; node != 0; node = node->nextNode)
    {
        node->addTextContent(data);
    }
}

void Element::appendText(const char *data, const size_t lenData)
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

/*****    Parser     *****/

void Element::GBfromText(char *data, const size_t lendata, GB_ARRAY *array)
{
    size_t nodeCount;
    size_t i = 0;
    Node **nodes = fromText(data, lendata, &nodeCount);
    GB.Array.New(array, GB.FindClass("XmlElement"), nodeCount);
    
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

Node** Element::fromText(char *data, const size_t lendata, size_t *nodeCount)
{
    *nodeCount = 0;
    if(!lendata || !data) return 0; //Empty ?
    
    const char *endData = data + lendata;
    
    Node **elements = 0;//Elements to return
    Element *curElement = 0;//Current element
    
    
    register char s = 0;//Current byte (value)
    register char *pos = data;//Current byte (position)
    register wchar_t ws = 0;//Current character (value)
    
    char *tag = 0;//First '<' character found
    
    while(pos < endData)//Start
    {
        tag = (char*)memchr(pos, CHAR_STARTTAG, endData - pos);//On cherche un début de tag
        
        if(tag && (tag - pos) != 0)//On ajoute le texte, s'il existe
        {
            TextNode *text = new TextNode(pos, tag - pos);
            APPEND(text);
        }
        
        if(!tag) break;
        
        tag++;
        pos = tag;//On avance au caractère trouvé
        
        //On analyse le contenu du tag
        ws = nextUTF8Char(pos, endData - pos);//On prend le premier caractère
        //DEBUG << ws << endl;
        
        if(!isNameStartChar(ws))//Ce n'est pas un tagName, il y a quelque chose ...
        {
            if(ws == CHAR_SLASH)//C'est un élément de fin
            {
                if(!curElement)//Pas d'élément courant
                {
                    //ERREUR : CLOSING TAG WHEREAS NONE IS OPEN
                }
                if((endData) < pos + curElement->lenTagName)//Impossible que les tags correspondent
                {
                    //ERREUR : TAG MISMATCH
                }
                //Les tags ne correspondent pas
                else if(memcmp(pos, curElement->tagName, curElement->lenTagName) != 0)
                {
                    //ERREUR : TAG MISMATCH
                }
                else//Les tags correspondent, on remonte
                {
                    pos += curElement->lenTagName;
                    curElement = curElement->parent;
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
                        //ERREUR : UNENDED COMMENT
                    }
                    
                    CommentNode *comment = new CommentNode(pos, tag - pos);
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
                    }
                    
                    CDATANode *cdata = new CDATANode(pos, tag - pos);
                    APPEND(cdata);
                    continue;
                }
                else// ... ?
                {
                    //ERREUR : INVALID TAG
                }
            }
            else// ... ?
            {
                //ERREUR : INVALID TAG
            }
        }//Si tout va bien, on a un nouvel élément
        else
        {
            while(isNameChar(nextUTF8Char(pos, endData - pos)))//On cherche le tagName
            {
                if(pos > endData)
                {
                    //ERREUR : UNENDED TAG
                }
            }
            pos--;
            
            //DEBUG << pos - tag << endl;
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
                    curElement = curElement->parent;//Pas d'enfants, on remonte
                    break;
                }
                
                if(isNameStartChar(s))//Début d'attribut
                {
                    char *attrNamestart = pos;
                    while(isNameChar(nextUTF8Char(pos, endData - pos)) && pos < endData){}//On parcourt le nom d'attribut
                    pos--;
                    char *attrNameEnd = pos;
                    s = *pos;
                    //DEBUG << "S : " << s << endl;
                    //DEBUG << "ATTR : " << *(data.copyString(attrNamestart, attrNameEnd)) << endl;
                    
                    while(isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs
                    
                    if(s != CHAR_EQUAL)
                    {
                        elmt->addAttribute(attrNamestart, attrNameEnd - attrNamestart);
                        if(s == CHAR_ENDTAG) break;//Fin de l'élément
                        else if (s == CHAR_SLASH)//Élément auto-fermant
                        {
                            pos++;
                            curElement = curElement->parent;//Pas d'enfants, on remonte
                            break;
                        }
                        else
                        {
                            //ERREUR : INVALID TAG
                            //throw HTMLParseException(i, 0, "", "INVALID TAG");
                        }
                    }
                    
                    pos++; s = *pos;
                    
                    while(isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs
                    
                    char delimiter = s;
                    if(delimiter != CHAR_DOUBLEQUOTE && delimiter != CHAR_SINGLEQUOTE)
                    {
                        //ERREUR : EXPECTED ATTRIBUTE DELIMITER
                        //throw HTMLParseException(0, 0, "", "EXPECTED ATTRIBUTE DELIMITER");
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
    
    //DEBUG << i << endl;
    
    return elements;
    
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

void Attribute::addStringLen(size_t *len)
{
    
}

void Attribute::addString(char **data)
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
