/***************************************************************************

  CXMLDocument.h

  (c) 2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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

typedef 
	struct _CXMLDOCUMENT
	{
		GB_BASE ob;
		xmlDoc *doc; 
	}  
	CXMLDOCUMENT;

CXMLNODE *XML_CreateNode(CXMLDOCUMENT *doc, xmlNode *node);
void XML_InitDocument(CXMLDOCUMENT *_object, xmlDoc *doc, const char *err);

#endif

