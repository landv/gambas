/***************************************************************************

  main.c

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

#define __MAIN_C

#include <stdio.h>
#include "main.h"
#include "CXMLReader.h"
#include "CXMLWriter.h"
#include "CXMLNode.h"
#include "CXMLDocument.h"



//extern "C" {


GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{

  CXmlReaderNodeTypeDesc,
  CXmlReaderNodeAttributesDesc,
  CXmlReaderNodeDesc,
  CXmlReaderDesc,

  CXmlWriterDTDDesc,
  CXmlWriterDesc,

  CXmlNodeAttributesDesc,
  CXmlNodeChildrenDesc,
  CXmlNodeDesc,
  CXmlDocumentDesc,

  NULL
};


int EXPORT GB_INIT(void)
{
	return -1;
}

void EXPORT GB_EXIT()
{

}

//}



