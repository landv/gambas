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

#include "explorer.h"
#include "document.h"
#include "element.h"


//#define DEBUG std::cout << pos << " : " <<

Explorer::Explorer()
{
    Init();
}

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
    //UNREF(loadedDocument);
    Clear();
    loadedDocument = doc;
    //GB.Ref(doc);

}

void Explorer::Clear()
{
    //UNREF(loadedDocument);
    loadedDocument = 0;
    curNode = 0;
    this->eof = false;
    this->endElement = false;
}

int Explorer::MoveNext()
{
    if(eof) return READ_ERR_EOF;
    if(!loadedDocument)
    {
        GB.Error("No document loaded");
        GB.Propagate();
        return READ_ERR_EOF;
    }
    if(!curNode)//Début du document
    {
        curNode = (Node*)(loadedDocument->root);
        return NODE_ELEMENT;
    }
    //Premier enfant
    else if(curNode->isElement() && curNode->toElement()->childCount > 0 && !endElement)
    {
        curNode = (curNode->toElement()->firstChild);
        return curNode->getType();
    }
    //Si plus d'enfants, frère suivant
    else
    {
        Node *nextNode = curNode->nextNode;
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

unsigned char Explorer::Read()
{
    do
    {
        state = MoveNext();
    } while(!this->flags[state]);

    return state;
}
