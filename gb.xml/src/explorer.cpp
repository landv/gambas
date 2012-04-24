#include "explorer.h"

#define DEBUG std::cout << pos << " : " <<


void Explorer::Init()
{
    this->flags = new bool[FLAGS_COUNT];
    this->flags[NODE_ELEMENT] = true;
    this->flags[NODE_TEXT] = true;
    this->flags[NODE_COMMENT] = true;
    this->flags[NODE_CDATA] = true;
    this->flags[READ_END_CUR_ELEMENT] = true;
    this->flags[READ_ERR_EOF] = true;
    Clear();
}

void Explorer::Load(Document *doc)
{
    UNREF(loadedDocument);
    loadedDocument = doc;
    GB.Ref(doc);

}

void Explorer::Clear()
{
    UNREF(loadedDocument);
    loadedDocument = 0;
    curNode = 0;
    this->eof = false;
    this->endElement = false;
}

int Explorer::MoveNext()
{
    if(eof) return READ_ERR_EOF;
    if(!curNode)//Début du document
    {
        curNode = loadedDocument->root;
        return NODE_ELEMENT;
    }
    //Premier enfant
    else if(curNode->isElement() && curNode->toElement()->children->size() > 0 && !endElement)
    {
        curNode = *(curNode->toElement()->children->begin());
        return curNode->getType();
    }
    //Si plus d'enfants, frère suivant
    else
    {
        Node *nextNode = curNode->next();
        endElement = false;
        if(nextNode)
        {
            curNode = nextNode;
            return nextNode->getType();
        }
        //si plus d'enfants ni de frère, on remonte
        if(curNode->parent)
        {
            curNode = curNode->parent;
            endElement = true;
            return READ_END_CUR_ELEMENT;
        }
        else//Plus de parent = fin du document
        {
            this->eof = true;
            return READ_ERR_EOF;
        }

    }

    return 0;//Hautement probablement impossible

}

int Explorer::Read()
{
    do
    {
        state = MoveNext();
    } while(!this->flags[state]);

    return state;
}
