/***************************************************************************

  CXMLDocument.h

  libxml wrapper

  (c) 2004 Daniel Campos Fernández <danielcampos@netcourrier.com> 

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CXMLDOCUMENT_H
#define __CXMLDOCUMENT_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include "gambas.h"
#include "CXMLNode.h"

#ifndef __CXMLDOCUMENT_C

extern GB_DESC CXmlDocumentDesc[];

#else

#define THIS ((CXMLDOCUMENT *)_object)

#endif

typedef struct 
{
    GB_BASE ob;
    xmlDoc *doc; 
    CXMLNODE *node;
    void **children;
    int nchildren;
    
}  CXMLDOCUMENT;

void Doc_AddChild(void *_object,CXMLNODE *chd);
void Doc_RemoveChild(void *_object,CXMLNODE *chd);

#endif

