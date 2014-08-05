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

#include "node.h"
#include "element.h"
#include "utils.h"
#include "CDocument.h"
#include "parser.h"
#include "serializer.h"

#include <stdio.h>
#include <stdlib.h>

Document* XMLDocument_New()
{
    Document *newDoc = (Document*)malloc(sizeof(Document));

    XMLNode_Init((Node*)newDoc, Node::DocumentNode);

    newDoc->root = XMLElement_New("xml", 3);
    newDoc->root->parentDocument = newDoc;
    newDoc->parentDocument = newDoc;
    newDoc->docType = XMLDocumentType;
    XMLNode_appendChild((Node*)newDoc, newDoc->root);

    return newDoc;
    
}

Document* XMLDocument_NewFromFile(const char *fileName, const size_t lenFileName, const DocumentType docType)
{
    Document *newDoc = (Document*)malloc(sizeof(Document));

    XMLNode_Init((Node*)newDoc, Node::DocumentNode);

    newDoc->root = 0;
    newDoc->parentDocument = newDoc;
    newDoc->docType = docType;

    XMLDocument_Open(newDoc, fileName, lenFileName);

    return newDoc;
}

void XMLDocument_Release(Document *doc)
{
    XMLNode_clearChildren((Node*)doc);
    free(doc);
}

/***** Node tree *****/
void XMLDocument_SetRoot(Document *doc, Element *newRoot)
{
    if(!doc->root)
    {
        XMLNode_appendChild(doc, newRoot);
    }
    else
    {
        XMLNode_replaceChild(doc, doc->root, newRoot);
    }
    doc->root = newRoot;
}

/***** Document loading *****/
void XMLDocument_Open(Document *doc, const char *fileName, const size_t lenFileName) throw(XMLParseException)
{
    char *content; int len;
    
    if(GB.LoadFile(fileName, lenFileName, &content, &len))
    {
        GB.Error("Error loading file.");
        GB.Propagate();
        return;
    }

    
    XMLDocument_SetContent(doc, content, len);
    
}

void XMLDocument_SetContent(Document *doc, const char *content, size_t len) throw(XMLParseException)
{
    char *posStart = 0, *posEnd = 0;
    
    if(doc->docType == XMLDocumentType)
    {
        //On cherche le début du prologue XML
        posStart = (char*)memchrs(content, len, "<?xml ", 6);

        if(posStart)//On cherche la fin du prologue XML
        {
            posEnd = (char*)memchrs(posStart, len - (posStart - content), "?>", 2);
            posEnd += 2;
        }
    }
    else
    {
        //On cherche le début du prologue XML
        posStart = (char*)memchrs(content, len, "<!DOCTYPE ", 10);

        //On cherche la fin du prologue XML
        if(posStart)
        {
            posEnd = (char*)memchr(posStart, '>', len - (posStart - content));

            if(posEnd)
            {
                posEnd += 1;
                //HTML5 ? (<!DOCTYPE html>)
                doc->docType = (posEnd - posStart == 98) ? XHTMLDocumentType : HTMLDocumentType;
                if(doc->docType == HTMLDocumentType) doc->docType = (!memcmp(posStart, "html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"", 98)) ? XHTMLDocumentType : HTMLDocumentType;
            }

        }

     }

    Node** elements = 0;
    size_t elementCount = 0;

    if(posEnd)
    {
        elements = parse(posEnd, len - (posEnd - content), &elementCount, doc->docType);
    }
    else
    {
        elements = parse(content, len, &elementCount, doc->docType);
    }

    Node *newRoot = 0;
    Node *node = 0;
    
    XMLNode_clearChildren((Node*)doc);
    doc->root = 0;
    for(size_t i = 0; i < elementCount; i++)
    {
        node = elements[i];
        if(node->type == Node::ElementNode)
        {
            if(!newRoot)
            {
                newRoot = node;
            }
            else
            {
                if(doc->docType == XMLDocumentType)//Strict document
                {
                    throw XMLParseException_New("Extra root element", 0, 0, 0);
                }
            }
                
        }
            XMLNode_appendChild((Node*)doc, node);

    }
    
    
    free(elements);
    if(newRoot) doc->root = (Element*)newRoot;
}



void XMLDocument_Save(Document *doc, const char *fileName, bool indent)
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
    serializeNode(doc, data, lenData, indent ? 0 : -1);
    data = (char*)realloc(data, lenData + 1);
    data[lenData] = 0;
    
    fputs(data, newFile);
    fclose(newFile);
    free(data);
    
}
