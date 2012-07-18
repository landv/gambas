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

#include "document.h"
#include "element.h"
#include "utils.h"
#include "gbi.h"
#include "CDocument.h"

#include <stdio.h>
#include <stdlib.h>

Document::Document() : GBObject(0)
{
    root = new Element("xml", 3);
    root->parentDocument = this;
    
}

Document::Document(const char *fileName, const size_t lenFileName)
{
    root = 0;
    Open(fileName, lenFileName);
}

Document::~Document()
{
    root->DestroyParent();
}

/***** Node tree *****/
void Document::setRoot(Element *newRoot)
{
    if(root) root->DestroyParent();
    
    if(newRoot->parent) newRoot->parent->removeKeepChild(newRoot);
    
    newRoot->parentDocument = this;
    root = newRoot;
}

void Document::getGBElementsByTagName(const char *ctagName, const size_t clenTagName, GB_ARRAY *array, const int mode, const int depth)
{
    root->getGBChildrenByTagName(ctagName, clenTagName, array, mode, depth);
}

void Document::getGBElementsByNameSpace(const char *cName, const size_t clenName, GB_ARRAY *array, const int mode, const int depth)
{
    root->getGBChildrenByNamespace(cName, clenName, array, mode, depth);
}

void Document::getAllElements(GB_ARRAY *array)
{
    root->getGBAllChildren(array);
}

/***** Document loading *****/
void Document::Open(const char *fileName, const size_t lenFileName) throw(XMLParseException)
{
    char *content; int len;
    
    if(GB.LoadFile(fileName, lenFileName, &content, &len))
    {
        GB.Error("Error loading file.");
        GB.Propagate();
        return;
    }
    
    this->setContent(content, len);
    
}

void Document::setContent(const char *content, size_t len) throw(XMLParseException)
{
    char *posStart = 0, *posEnd = 0;
    
    //On cherche le début du prologue XML
    posStart = (char*)memchrs(content, len, "<?xml ", 6);
    
    if(posStart)//On cherche la fin du prologue XML
        posEnd = (char*)memchrs(posStart, len - (posStart - content), "?>", 2);

    Node** elements = 0;
    size_t elementCount = 0;
    if(posEnd)
    {
        elements = Element::fromText(posEnd, len - (posEnd - content), &elementCount);
    }
    else
    {
        elements = Element::fromText(content, len, &elementCount);
    }

    Element *newRoot = 0;
    Node *node = 0;
    
    for(size_t i = 0; i < elementCount; i++)
    {
        node = elements[i];
        if(node->isElement() && !newRoot)
        {
            newRoot = node->toElement();
        }
        else
        {
            delete node;
        }

    }
    
    if(!newRoot)
    {
        throw XMLParseException("No valid element root found", 0, 0, 0);
    }
    
    this->setRoot(newRoot);
    free(elements);

    //if(!root) throw HTMLParseException(0, 0, "somewhere", "No valid root element found.");
}

/***** String output *****/
void Document::toString(char **output, size_t *len, int indent)
{
    //<?xml version="1.0" encoding="UTF-8"?> //Len = 38
    *len = 38 + (indent >= 0 ? 1 : 0); root->addStringLen(len, indent);
    *output = (char*)malloc(*len);
    memcpy(*output, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 38);
    *output += 38;
    if(indent >= 0) 
    {
        **output = SCHAR_N;
        ++(*output);
    }
    root->addString(output, indent);
    (*output) -= (*len);
}

void Document::toGBString(char **output, size_t *len, int indent)
{
    //<?xml version="1.0" encoding="UTF-8"?> //Len = 38
    *len = 38 + (indent >= 0 ? 1 : 0); root->addStringLen(len, indent);
    *output = GB.TempString(0, *len);
    memcpy(*output, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 38);
    *output += 38;
    if(indent >= 0) 
    {
        **output = SCHAR_N;
        ++(*output);
    }
    root->addString(output, indent);
    (*output) -= (*len);
}

void Document::save(const char *fileName, bool indent)
{
    FILE *newFile = fopen(fileName, "w");
    
    if(!newFile)
    {
        GB.Error("Cannot open file");
        GB.Propagate();
        return;
    }
    
    char *data = 0;
    size_t lenData = 0;
    toString(&data, &lenData, indent ? 0 : -1);
    data = (char*)realloc(data, lenData + 1);
    data[lenData] = 0;
    
    fputs(data, newFile);
    fclose(newFile);
    
}

void Document::NewGBObject()
{
    Node::NoInstanciate = true;
    GBObject = GBI::New<CDocument>("XmlDocument");
    GBObject->doc = this;
    Node::NoInstanciate = false;
}
