#include "explorer.h"

#define DEBUG std::cout << pos << " : " <<
#define DELETE(_ob) if(_ob) {delete _ob; _ob = 0;}
#define UNREF(_ob) if(_ob) GB.Unref(POINTER(&(_ob)))

void Explorer::Init()
{
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
}

int Explorer::MoveNext()
{
    if(!curNode)//Début du document
    {
        curNode = loadedDocument->root;
        return NODE_ELEMENT;
    }
    //Premier enfant
    else if(curNode->isElement() && curNode->toElement()->children->size() > 0)
    {
        curNode = *(curNode->toElement()->children->begin());
        return curNode->getType();
    }
    //Si plus d'enfants, frère suivant
    else
    {
        Node *nextNode = curNode->next();
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
            return READ_ERR_EOF;
        }

    }

    return 0;//Hautement probablement impossible

}

int Explorer::Read()
{
    int code;
    do
    {
        code = MoveNext();
    } while(!flags[code]);

    return code;
}
