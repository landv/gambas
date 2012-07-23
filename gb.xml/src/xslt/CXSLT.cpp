/***************************************************************************

  CXSLT.c

  (c) 2004 Daniel Campos Fernández <danielcampos@netcourrier.com>
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

#define __CXSLT_C


#include "main.h"
#include "CXSLT.h"


#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include "../CDocument.h"
#include "../document.h"
#include "../document.cpp"
#include "../element.h"
#include "../element.cpp"
#include "../node.h"
#include "../node.cpp"
#include "../textnode.h"
#include "../textnode.cpp"
#include "../gbi.cpp"
#include "../utils.cpp"

BEGIN_METHOD(CXSLT_Transform,GB_OBJECT inputDoc;GB_OBJECT inputStyleSheet)

if (GB.CheckObject(VARGOBJ(CDocument,inputDoc))) return;
if (GB.CheckObject(VARGOBJ(CDocument,inputStyleSheet))) return;
    Document *doc = VARGOBJ(CDocument,inputDoc)->doc,
             *stylesheet = VARGOBJ(CDocument,inputStyleSheet)->doc;
	

	
    if (!doc->childCount)
	{
		GB.Error("Void document");
		return;
	}
	
    if (!stylesheet->childCount)
	{
		GB.Error("Void Style Sheet");
		return;
	}

    xsltStylesheetPtr sheet = 0;
    
    char *StyleSheetOutput;
    size_t StyleSheetLen;
    stylesheet->toString(StyleSheetOutput, StyleSheetLen);
    
    StyleSheetOutput =(char*)realloc(StyleSheetOutput, StyleSheetLen + 1);
    StyleSheetOutput[StyleSheetLen] = 0;
    
    
    xmlDoc *xmlStyleSheet = xmlParseDoc((xmlChar*)(StyleSheetOutput));
	

    if(!(sheet=xsltParseStylesheetDoc(xmlStyleSheet)))
	{
		GB.Error("Invalid style sheet");
		return;
	}
    
    char *DocumentOutput;
    size_t DocumentLen;
    doc->toString(DocumentOutput, DocumentLen);
    
    DocumentOutput =(char*)realloc(DocumentOutput, DocumentLen + 1);
    DocumentOutput[DocumentLen] = 0;
    
    xmlDoc *xmlInputDoc = xmlParseDoc((xmlChar*)(DocumentOutput));

	
    xmlDoc *xmlOutDoc;
    xmlChar *buffer = 0;
    int size;

    xmlOutDoc = xsltApplyStylesheet(sheet, xmlInputDoc, NULL);
    if (!xmlOutDoc)
    {
        GB.Error("Unable to apply style sheet");
    }
    
    xmlDocDumpFormatMemoryEnc(xmlOutDoc ,&buffer, &size, "UTF-8", 1);

    Document *outDoc = new Document;
    
    try
    {
        outDoc->setContent((char*)(buffer),size);
    }
    catch(XMLParseException &e)
    {
        outDoc->setContent("<?xml version=\"1.0\"?><xml></xml>", 32);
        std::cerr << "XSLT Warning : error when parsing output document : " << endl << e.what() << endl;
        //return;
    }
        
    GBI::Return(outDoc);
		
END_METHOD



GB_DESC CXsltDesc[] =
{
  GB_DECLARE("Xslt", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD ("Transform","XmlDocument",CXSLT_Transform,"(Document)XmlDocument;(StyleSheet)XmlDocument;"),

  GB_END_DECLARE
};
