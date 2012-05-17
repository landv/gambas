#include "reader.h"

//#define DEBUG std::cout << pos << " : " <<
#define DELETE(_ob) if(_ob) {delete _ob; _ob = 0;}
#define UNREF(_ob) if(_ob) GB.Unref(POINTER(&(_ob)))

void Reader::ClearReader()
{
    //UNREF(foundNode);
    //UNREF(curNode);
    this->keepMemory = false;
    this->pos = 0;
    this->depth = 0;
    this->inTag = false;
    this->inAttr = false;
    this->inAttrName = false;
    this->inAttrVal = false;
    this->inNewTag = false;
    this->specialTagLevel = 0;
    this->inEndTag = false;
    this->inXMLProlog = false;
    this->inCommentTag = false;
    this->specialTagLevel = 0;

    foundNode = 0;
    curNode = 0;
    curElmt = 0;
    storedDocument = 0;
    DELETE(attrName)
    DELETE(attrVal)
    DELETE(content)
    
    if(storedElements)
    {
        storedElements->clear();
        /*for(vector<Node*>::iterator it = storedElements->begin(); it != storedElements->end(); ++it)
        {
            GB.Unref(POINTER(&(*it)));
        }
        this->storedElements->clear();*/
    }

    this->curAttrNameEnum = 0;
    
}

void Reader::InitReader()
{
    attrName = 0;
    attrVal = 0;
    content = 0;
    storedDocument = 0;
    storedElements = 0;

    ClearReader();

    this->flags[NODE_ELEMENT] = true;
    this->flags[NODE_TEXT] = true;
    this->flags[NODE_COMMENT] = true;
    this->flags[NODE_CDATA] = true;
    this->flags[NODE_ATTRIBUTE] = false;
    this->flags[READ_ATTRIBUTE] = false;

    this->flags[READ_END_CUR_ELEMENT] = true;
    this->flags[READ_ERR_EOF] = true;
    this->storedElements = new vector<Node*>;

}

void Reader::DestroyReader()
{
    ClearReader();
    delete this->storedElements;
}

int Reader::ReadChar(char car)
{
    #define APPEND(elmt) if(curElmt == 0){storedElements->push_back(elmt); }\
    else {curElmt->appendChild(elmt);}

    //DEBUG << car << endl;
    
    this->pos++;
    
    if(car == CHAR_STARTTAG)//Début de tag
    {
        inNewTag = true;
        //DEBUG "Début de tag" << endl;
        if(curNode && curNode->isText()) //Si il y avait du texte avant
        {
            DEBUG "Élément de texte terminé : " << (curNode->textContent()) << endl;
            //if(foundNode) GB.Unref(POINTER(&foundNode));
            foundNode = curNode;
            if(keepMemory)
            {
                APPEND(foundNode);
            }
            curNode = 0;
            return NODE_TEXT;
        }
    }
    else if(car == CHAR_ENDTAG && inTag && !inEndTag)//Fin de tag (de nouvel élément)
    {
        //DEBUG "Nouvel élément : " << (*(curNode->toElement()->tagName)) << endl;
        //UNREF(foundNode);
        foundNode = curNode;//On a trouvé un élément complet
        //curNode = 0;
        //GB.Ref(foundNode);
        inTag = false;
        depth++;
        if(keepMemory)
        {
            APPEND(foundNode);
            curElmt = foundNode->toElement();
        }
        if(attrName && attrVal)
        {
            curNode->toElement()->setAttribute(*attrName, *attrVal);
            delete attrName; attrName = 0; inAttrName = false; inAttr = false;
            delete attrVal; attrVal = 0; inAttrVal = false;
        }
        else if(attrName) {curNode->toElement()->setAttribute(*attrName, "");
            delete attrName; attrName = 0; inAttrName = false; inAttr = false;}
        return NODE_ELEMENT;
    }
    else if(isWhiteSpace(car) && inTag && !inEndTag && !inAttrVal)//Début de nom d'attribut
    {
        if(attrName && attrVal)
        {
            curNode->toElement()->setAttribute(*attrName, *attrVal);
            delete attrName; attrName = 0; inAttrName = false; inAttr = false;
            delete attrVal; attrVal = 0; inAttrVal = false;
        }
        else if(attrName) {curNode->toElement()->setAttribute(*attrName, "");
            delete attrName; attrName = 0; inAttrName = false; inAttr = false;}
        inAttr = true;
        inAttrName = true;
    }
    else if(car == CHAR_EQUAL && inAttrName)//Fin du nom d'attribut
    {
        inAttrName = false;
    }
    else if((car == CHAR_SINGLEQUOTE || car == CHAR_DOUBLEQUOTE) && inAttr && !inAttrVal)//Début de valeur d'attribut
    {
        inAttrVal = true;
        attrVal = new fwstring;
    }
    else if((car == CHAR_SINGLEQUOTE || car == CHAR_DOUBLEQUOTE) && inAttr && inAttrVal)//Fin de valeur d'attribut
    {
        curNode->toElement()->setAttribute(*attrName, *attrVal);
        delete attrName; attrName = 0; delete attrVal; attrVal = 0;
        inAttr = false;
        inAttrVal = false;
        return READ_ATTRIBUTE;
    }
    else if(car == CHAR_SLASH && inNewTag)//C'est un tag de fin
    {
        //DEBUG "Tag de fermeture" << endl;
        inEndTag = true;
        inNewTag = false;
        inTag = true;
    }
    else if(car == CHAR_ENDTAG && inEndTag)//La fin d'un tag de fin
    {
        inTag = false;
        inEndTag = false;
        //DEBUG "Fin de l'élément : " << (*content) << endl;
        if(curElmt && *(curElmt->toElement()->tagName) == (*content))
        {
            curElmt = curElmt->parent;
        }
        delete content;
        content = 0;
        if(depth > 0) depth--;
        return READ_END_CUR_ELEMENT;
    }
    else if(inEndTag)//Tag de fin
    {
        if(!content) content = new fwstring();
        *content += car;
    }
    else if(inNewTag && !curNode && car == CHAR_EXCL )//Premier caractère de commentaire
    {
        specialTagLevel = COMMENT_TAG_STARTCHAR_1;
        inCommentTag = true;
        inNewTag = false;
        inTag = false;
    }
    //Caractère "-" de début de commentaire
    else if(inCommentTag && car == CHAR_DASH && specialTagLevel >= COMMENT_TAG_STARTCHAR_1 && specialTagLevel < COMMENT_TAG_STARTCHAR_3)
    {
        specialTagLevel++;
        if (specialTagLevel == COMMENT_TAG_STARTCHAR_3)//Le tag <!-- est complet, on crée un nouveau node
        {
            inCommentTag = false;
            //UNREF(curNode);
            curNode = new CommentNode;
            //GB.Ref(curNode);
        }
    }
    //Caractère "-" de fin de commentaire
    else if(curNode && curNode->isComment() && car == CHAR_DASH)
    {
        specialTagLevel++;
        if(specialTagLevel > COMMENT_TAG_ENDCHAR_2)//On est allés un peu trop loin, il y a des - en trop
        {
            specialTagLevel--;
            *(curNode->toTextNode()->content) += car;
        }
    }
    //Fin du commentaire
    else if(curNode && curNode->isComment() && car == CHAR_ENDTAG && specialTagLevel == COMMENT_TAG_ENDCHAR_2)
    {
        specialTagLevel = 0;
        inTag = false;
        //UNREF(foundNode);
        foundNode = curNode;
        if(keepMemory)
        {
            APPEND(foundNode);
        }
        curNode = 0;
        return NODE_COMMENT;
    }
    //Début de prologue XML
    else if(car == CHAR_PI && inNewTag)
    {
        inXMLProlog = true;
        inNewTag = false;
        inTag = false;
    }
    else if(car == CHAR_PI && inXMLProlog)
    {
        specialTagLevel = PROLOG_TAG_ENDCHAR;
    }
    else if(car == CHAR_ENDTAG && inXMLProlog && specialTagLevel == PROLOG_TAG_ENDCHAR)
    {
        specialTagLevel = 0;
        inXMLProlog = 0;
    }
    else//Texte
    {
        if(inXMLProlog) return 0;
        if(inNewTag && !inEndTag)//On est dans un tag avec contenu -> on crée l'élément
        {
            Node* newNode = new Element;
            inTag = true;
            inNewTag = false;
            //UNREF(curNode);
            curNode = newNode;
            //GB.Ref(curNode);
            *(curNode->toElement()->tagName) += car;
        }
        else if(!curNode || (!curNode->isText() && !inTag))//Pas de nœud courant -> nœud texte
        {
            if(isWhiteSpace(car)) return 0;
            //DEBUG "Nouvel élément texte " << endl;
            Node* newNode = new TextNode;
            if(curNode) 
            { /*if(curNode->isElement()) this->curNode->toElement()->appendChild((newNode));*/
                //GB.Unref(POINTER(&curNode));
            }
            curNode = newNode;
            //GB.Ref(curNode);
            *(curNode->toTextNode()->content) += car;
        }
        else if(curNode->isElement() && inTag && !inAttr)//Si on est dans le tag d'un élément
        {
            *(curNode->toElement()->tagName) += car;
        }
        else if(inAttrName && inAttr)//Nom d'attribut
        {
            if(!attrName) attrName = new fwstring();
            *attrName += car;
        }else if(inAttrVal && inAttr)//Valeur d'attribut
        {
            if(!attrVal) attrVal = new fwstring();
            *attrVal += car;
        }
        else if(curNode->isText())
        {
            *(curNode->toTextNode()->content) += car;
            if(curNode->isComment()) specialTagLevel = COMMENT_TAG_STARTCHAR_3; //En cas de "-" non significatifs
            else if(inXMLProlog) specialTagLevel = 0;//En cas de "?" non significatifs
        }
    }

    return 0;
}
