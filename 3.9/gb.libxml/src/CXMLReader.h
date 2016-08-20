/***************************************************************************

  CXMLReader.h

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

#ifndef __CXMLREADER_H
#define __CXMLREADER_H

#include "gambas.h"
#include <libxml/xmlreader.h>

#ifndef __CXMLREADER_C

extern GB_DESC CXmlReaderNodeTypeDesc[];
extern GB_DESC CXmlReaderNodeDesc[];
extern GB_DESC CXmlReaderNodeAttributesDesc[];
extern GB_DESC CXmlReaderDesc[];

#else

#define THIS ((CXMLREADER *)_object)

#endif


typedef struct 
{
    GB_BASE ob;
    xmlTextReaderPtr reader;
    char *buffer;
    int eof;
    
}  CXMLREADER;


#endif
