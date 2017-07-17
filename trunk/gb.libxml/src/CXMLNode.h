/***************************************************************************

  CXMLNode.h

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

#ifndef __CXMLNODE_H
#define __CXMLNODE_H

#include <libxml/tree.h>
#include "gambas.h"

#ifndef __CXMLNODE_C


extern GB_DESC CXmlNodeAttributesDesc[];
extern GB_DESC CXmlNodeChildrenDesc[];
extern GB_DESC CXmlNodeDesc[];

#else

#define THIS ((CXMLNODE *)_object)

#endif

typedef 
	struct 
	{
		GB_BASE ob;
		xmlNode *node;
		struct _CXMLDOCUMENT *doc;
	}
	CXMLNODE;

#endif
