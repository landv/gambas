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

#include "reader.h"

#include "node.h"
#include "utils.h"
#include "element.h"
#include "document.h"
#include "textnode.h"

#include <memory.h>
#include <stdlib.h>

#define DELETE(_ob) if(_ob) {delete _ob; _ob = 0;}
#define FREE(_ob) if(_ob) {free(_ob); _ob = 0;}
#define UNREF(_ob) if(_ob) GB.Unref(POINTER(&(_ob)))
#define DESTROYPARENT(_ob) if(_ob) {XMLNode_DestroyParent(_ob); _ob = 0;}

void Reader::ClearReader()
{
    //UNREF(foundNode);
    //UNREF(curNode);
    this->keepMemory = false;
    this->pos = 0;
    this->depth = -1;
    this->inTag = false;
    this->inTagName = false;
    this->inAttr = false;
    this->inAttrName = false;
    this->inAttrVal = false;
    this->inNewTag = false;
    this->specialTagLevel = 0;
    this->inEndTag = false;
    this->inXMLProlog = false;
    this->inCommentTag = false;
    this->inCDATATag = false;
    this->inComment = false;
    this->inCDATA = false;
    this->waitClosingElmt = false;
    this->specialTagLevel = 0;
    this->state = 0;

    if(curNode != foundNode)
    {
        DESTROYPARENT(curNode);
    }
    else
    {
        curNode = 0;
    }
    DESTROYPARENT(foundNode);
    curElmt = 0;
    storedDocument = 0;
    FREE(attrName);
            lenAttrName = 0;
    FREE(attrVal);
            lenAttrVal = 0;
    FREE(content);
            lenContent = 0;
    
    if(storedElements)
    {
        
        /*for(vector<Node*>::iterator it = storedElements->begin(); it != storedElements->end(); ++it)
        {
            GB.Unref(POINTER(&(*it)));
        }
        this->storedElements->clear();*/
    }
    
    curAttrEnum = 0;
    
}

void Reader::InitReader()
{
    attrName = 0;
    attrVal = 0;
    content = 0;
    storedDocument = 0;
    storedElements = 0;
    curNode = 0;
    foundNode = 0;

    ClearReader();

    this->flags[NODE_ELEMENT] = true;
    this->flags[NODE_TEXT] = true;
    this->flags[NODE_COMMENT] = true;
    this->flags[NODE_CDATA] = true;
    this->flags[NODE_ATTRIBUTE] = false;
    this->flags[READ_ATTRIBUTE] = false;

    this->flags[READ_END_CUR_ELEMENT] = true;
    this->flags[READ_ERR_EOF] = true;
    FREE(storedElements);

}

void Reader::DestroyReader()
{
    ClearReader();
}

int Reader::ReadChar(char car)
{
    #define APPEND(elmt) if(curElmt == 0){}\
    else {XMLNode_appendChild(curElmt, elmt);}

    
    ++(this->pos);
    
    if(waitClosingElmt)
    {
        if(car != '>') return 0;
        waitClosingElmt = false;
        depth--;
//        this->state = READ_END_CUR_ELEMENT;
        return 0;
    }
    
    if(car == '<' && !inComment)//Début de tag
    {
        if(inTag)//Si on est déjà dans un tag
        {
            throw XMLParseException_New("Invalid tag Name", pos);
        }
        inNewTag = true;
        inTagName = true;
        if(curNode && curNode->type == Node::NodeText) //Si il y avait du texte avant
        {
            DESTROYPARENT(foundNode);
            foundNode = curNode;
            if(keepMemory)
            {
                APPEND(foundNode);
            }
            //const char *trimmedText = curNode->toTextNode()->content;
            //size_t lenTrimmedText = curNode->toTextNode()->lenContent;

            //Trim(trimmedText, lenTrimmedText);

            XMLTextNode_TrimContent((TextNode*)curNode);

            curNode = 0;
            this->state = NODE_TEXT;
            return NODE_TEXT;
        }
    }
    else if(car == '>' && inTag && !inEndTag && !inComment)//Fin de tag (de nouvel élément)
    {
        DESTROYPARENT(foundNode);
        //UNREF(foundNode);
        foundNode = curNode;//On a trouvé un élément complet
        //curNode = 0;
        //GB.Ref(foundNode);
        inTag = false;
        depth++;
        if(keepMemory)
        {
            APPEND(foundNode);
            curElmt = ((Element*)foundNode);
        }
        if(attrName && attrVal)
        {
            XMLElement_AddAttribute(((Element*)curNode), attrName, lenAttrName,
                                               attrVal, lenAttrVal);
            FREE(attrName); lenAttrName = 0; inAttrName = false; inAttr = false;
            FREE(attrVal); lenAttrVal = 0; inAttrVal = false;
        }
        else if(attrName) 
        {
            XMLElement_AddAttribute(((Element*)curNode), attrName, lenAttrName,  "", 0);
            FREE(attrName); lenAttrName = 0; inAttrName = false; inAttr = false;
        }
        this->state = NODE_ELEMENT;
        return NODE_ELEMENT;
    }
    else if(isWhiteSpace(car) && inTag && inTagName && !inComment)// Fin de tagName
    {
        inTagName = false;
        XMLElement_RefreshPrefix((Element*)curNode);
    }
    else if(isNameStartChar(car) && inTag && !inTagName && !inEndTag && !inAttrVal && !inAttrName && !inComment)//Début de nom d'attribut
    {
        if(attrName && attrVal)
        {
            XMLElement_AddAttribute(((Element*)curNode), attrName, lenAttrName,
                                               attrVal, lenAttrVal);
            FREE(attrName); lenAttrName = 0; inAttrName = false; inAttr = false;
            FREE(attrVal); lenAttrVal = 0; inAttrVal = false;
        }
        else if(attrName) 
        {
            XMLElement_AddAttribute(((Element*)curNode), attrName, lenAttrName,  "", 0);
            FREE(attrName); lenAttrName = 0; inAttrName = false; inAttr = false;
        }
        inAttr = true;
        inAttrName = true;
        attrName = (char*)malloc(1);
        *attrName = car;
        lenAttrName = 1;
    }
    else if(car == '=' && inAttrName && !inComment)//Fin du nom d'attribut
    {
        inAttrName = false;
    }
    else if((car == '\'' || car == '"') && inAttr && !inAttrVal && !inComment)//Début de valeur d'attribut
    {
        inAttrVal = true;
        attrVal = 0;
    }
    else if((car == '\'' || car == '"') && inAttr && inAttrVal && !inComment)//Fin de valeur d'attribut
    {
        XMLElement_AddAttribute(((Element*)curNode), attrName, lenAttrName,
                                           attrVal, lenAttrVal);
        FREE(attrName); lenAttrName = 0;
        FREE(attrVal); lenAttrVal = 0;
        inAttr = false;
        inAttrVal = false;
        this->state = READ_ATTRIBUTE;
        return READ_ATTRIBUTE;
    }
    else if(car == '/' && inTag && !inAttrVal && !inComment)//Self-closed element
    {
        inTag = false;
        inTagName = false;
        inEndTag = false;
        if(curElmt) curElmt = (Element*)(curElmt->parent);
        FREE(content); lenContent = 0;
        //depth--;
        waitClosingElmt = true;
        DESTROYPARENT(foundNode);
        foundNode = curNode;
        XMLElement_RefreshPrefix((Element*)curNode);
        this->state = NODE_ELEMENT;
        depth++;
        return NODE_ELEMENT;
    }
    else if(car == '/' && inNewTag && !inComment)//C'est un tag de fin
    {
        inEndTag = true;
        inNewTag = false;
        inTag = true;
    }
    else if(car == '>' && inEndTag && !inComment)//La fin d'un tag de fin
    {
        inTag = false;
        inEndTag = false;
        if(curElmt && lenContent == curElmt->lenTagName)
        {
            if(memcmp(curElmt->tagName, content, lenContent))
                curElmt = (Element*)(curElmt->parent);
        }
        FREE(content); lenContent = 0;
        depth--;
        this->state = READ_END_CUR_ELEMENT;
        return READ_END_CUR_ELEMENT;
    }
    else if(inEndTag)//Tag de fin
    {
        if(!content) 
        {
            content = (char*)malloc(1);
            content[0] = car;
            lenContent = 1;
        }
        else
        {
            content = (char*)realloc(content, lenContent + 1);
            content[lenContent] = car;
            ++lenContent;
        }
        
    }
    else if(inNewTag && car == '!' )//Premier caractère de commentaire
    {
        specialTagLevel = COMMENT_TAG_STARTCHAR_1;
        inCommentTag = true;
        inNewTag = false;
        inTag = false;
    }
    //Caractère de début de CDATA
    else if(inCommentTag && car == '[' && specialTagLevel == COMMENT_TAG_STARTCHAR_1)
    {
        specialTagLevel = CDATA_TAG_STARTCHAR_2;
        inCommentTag = false;
        inCDATATag = true;
    }
    //Caractère de CDATA
    else if(inCDATATag && specialTagLevel >= CDATA_TAG_STARTCHAR_2 && specialTagLevel < CDATA_TAG_STARTCHAR_8
            && (car == '[' || car == 'C' || car == 'D' || car == 'A' || car == 'T'))
    {
        ++specialTagLevel;
        if(specialTagLevel == CDATA_TAG_STARTCHAR_8)
        {
            inCDATATag = false;
            inCDATA = true;
            curNode = XMLCDATA_New();
        }
    }
    //Caractère "]" de fin de CDATA
    else if(curNode && curNode->type == Node::CDATA && car == ']')
    {
        ++specialTagLevel;
        if(specialTagLevel > CDATA_TAG_ENDCHAR_2)//On est allés un peu trop loin, il y a des ] en trop
        {
            --specialTagLevel;
            char *&textContent = ((TextNode*)curNode)->content;
            size_t &lenTextContent = ((TextNode*)curNode)->lenContent;
            textContent = (char*)realloc(textContent, lenTextContent + 1);
            textContent[lenTextContent] = car;
            ++lenTextContent;
        }
    }
    //Fin du CDATA
    else if(curNode && curNode->type == Node::CDATA && car == '>' && specialTagLevel == CDATA_TAG_ENDCHAR_2)
    {
        specialTagLevel = 0;
        inTag = false;
        DESTROYPARENT(foundNode);
        //UNREF(foundNode);
        foundNode = curNode;
        inCDATA = false;
        if(keepMemory)
        {
            APPEND(foundNode);
        }
        curNode = 0;
        this->state = NODE_CDATA;
        return NODE_CDATA;
    }
    //Caractère "-" de début de commentaire
    else if(inCommentTag && car == '-' && specialTagLevel >= COMMENT_TAG_STARTCHAR_1 && specialTagLevel < COMMENT_TAG_STARTCHAR_3  && !inComment)
    {
        ++specialTagLevel;
        if (specialTagLevel == COMMENT_TAG_STARTCHAR_3)//Le tag <!-- est complet, on crée un nouveau node
        {
            inCommentTag = false;
            inComment = true;
            //DESTROYPARENT(curNode);
            //UNREF(curNode);
            curNode = XMLCDATA_New();
            //GB.Ref(curNode);
        }
    }
    //Caractère "-" de fin de commentaire
    else if(curNode && curNode->type == Node::Comment && car == '-')
    {
        ++specialTagLevel;
        if(specialTagLevel > COMMENT_TAG_ENDCHAR_2)//On est allés un peu trop loin, il y a des - en trop
        {
            --specialTagLevel;
            char *&textContent = ((TextNode*)curNode)->content;
            size_t &lenTextContent = ((TextNode*)curNode)->lenContent;
            textContent = (char*)realloc(textContent, lenTextContent + 1);
            textContent[lenTextContent] = car;
            ++lenTextContent;
        }
    }
    //Fin du commentaire
    else if(curNode && curNode->type == Node::Comment && car == '>' && specialTagLevel == COMMENT_TAG_ENDCHAR_2)
    {
        specialTagLevel = 0;
        inTag = false;

        DESTROYPARENT(foundNode);
        //UNREF(foundNode);
        foundNode = curNode;
        inComment = false;
        if(keepMemory)
        {
            APPEND(foundNode);
        }
        curNode = 0;
        this->state = NODE_COMMENT;
        return NODE_COMMENT;
    }
    //Début de prologue XML
    else if(car == '?' && inNewTag && !inComment)
    {
        inXMLProlog = true;
        inNewTag = false;
        inTag = false;
    }
    else if(car == '?' && inXMLProlog && !inComment)
    {
        specialTagLevel = PROLOG_TAG_ENDCHAR;
    }
    else if(car == '>' && inXMLProlog && specialTagLevel == PROLOG_TAG_ENDCHAR && !inComment)
    {
        specialTagLevel = 0;
        inXMLProlog = 0;
    }
    else//Texte
    {
        if(inXMLProlog) return 0;
        if(inNewTag && !inEndTag)//On est dans un tag avec contenu -> on crée l'élément
        {
            Element* newNode = XMLElement_New(&car, 1);
            inTag = true;
            inNewTag = false;
            //DESTROYPARENT(curNode);
            //UNREF(curNode);
            curNode = newNode;
            //GB.Ref(curNode);
        }
        else if(!curNode || (curNode->type != Node::NodeText && !inTag))//Pas de nœud courant -> nœud texte
        {
            if(isWhiteSpace(car)) return 0;
            TextNode* newNode = XMLTextNode_New(&car, 1);
            //DESTROYPARENT(curNode);
            curNode = newNode;
            //GB.Ref(curNode);
        }
        else if(curNode->type == Node::ElementNode && inTag && !inAttr)//Si on est dans le tag d'un élément
        {
            if(!isNameChar(car)) return 0;
            char *&textContent = ((Element*)curNode)->tagName;
            size_t &lenTextContent = ((Element*)curNode)->lenTagName;
            textContent = (char*)realloc(textContent, lenTextContent + 1);
            textContent[lenTextContent] = car;
            ++lenTextContent;
        }
        else if(inAttrName && inAttr)//Nom d'attribut
        {
            if(!attrName)
            {
                attrName = (char*)malloc(1);
                *attrName = car;
                lenAttrName = 1;
            }
            else
            {
                attrName = (char*)realloc(attrName, lenAttrName + 1);
                attrName[lenAttrName] = car;
                ++lenAttrName;
            }
        }
        else if(inAttrVal && inAttr)//Valeur d'attribut
        {
            if(!attrVal)
            {
                attrVal = (char*)malloc(1);
                *attrVal = car;
                lenAttrVal = 1;
            }
            else
            {
                attrVal = (char*)realloc(attrVal, lenAttrVal + 1);
                attrVal[lenAttrVal] = car;
                ++lenAttrVal;
            }
        }
        else if(curNode->type == Node::NodeText)
        {
            char *&textContent = ((TextNode*)curNode)->content;
            size_t &lenTextContent = ((TextNode*)curNode)->lenContent;
            textContent = (char*)realloc(textContent, lenTextContent + 1);
            textContent[lenTextContent] = car;
            ++lenTextContent;
            if(curNode->type == Node::Comment) specialTagLevel = COMMENT_TAG_STARTCHAR_3; //En cas de "-" non significatifs
            else if(inXMLProlog) specialTagLevel = 0;//En cas de "?" non significatifs
        }
    }

    return 0;
}
