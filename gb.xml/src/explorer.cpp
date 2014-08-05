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

#include "node.h"
#include "document.h"
#include "element.h"
#include "gbinterface.h"
#include <memory.h>



Explorer::Explorer() : loadedDocument(0)
{
    Init();
}

Explorer::~Explorer()
{
    Clear();
    delete[] flags;
}

void Explorer::Init()
{
    this->flags = new bool[FLAGS_COUNT];
    memset(this->flags, false, FLAGS_COUNT);
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
    Clear();
    loadedDocument = doc;
    CNode *obj = XMLNode_GetGBObject(loadedDocument);
    GB.Ref(obj);
    //GB.Ref(doc);

    Read();

}

void Explorer::Clear()
{
    if(loadedDocument)
    {
        CNode *obj = XMLNode_GetGBObject(loadedDocument);
        GB.Unref(POINTER(&obj));
    }
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
    else if(curNode->type == Node::ElementNode && ((Element*)curNode)->childCount > 0 && !endElement)
    {
        curNode = (((Element*)curNode)->firstChild);
        return curNode->type;
    }
    //Si plus d'enfants, frère suivant
    else
    {
        Node *nextNode = curNode->nextNode;
        endElement = false;
        if(nextNode)
        {
            curNode = nextNode;
            return nextNode->type;
        }
        //si plus d'enfants ni de frère, on remonte
        else if(curNode->parent && curNode != loadedDocument->root && curNode->parent != loadedDocument)
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
