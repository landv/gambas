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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "main.h"
#include "utils.h"

class Element;
struct CDocument;
class XMLParseException;

class Document 
{
public:
    Document();
    Document(const char *fileName, const size_t lenFileName);
    virtual ~Document();
    
    //Document loading
    void Open(const char *fileName, const size_t lenFileName) throw(XMLParseException);
    virtual void setContent(char *content, size_t len) throw(XMLParseException);
    
    //String output
    virtual void toString(char **output, size_t *len, int indent = -1);
    virtual void toGBString(char **output, size_t *len, int indent = -1);
    void save(const char *fileName, bool indent = false);
    
    
    //Node tree
    void setRoot(Element *newRoot);
    void getGBElementsByTagName(const char *ctagName, const size_t clenTagName,  GB_ARRAY *array, const int mode = GB_STRCOMP_BINARY, const int depth = -1);
    void getAllElements(GB_ARRAY *array);
    
    Element *root;
    CDocument *GBObject;
    
    virtual void NewGBObject();
};

#endif // DOCUMENT_H
