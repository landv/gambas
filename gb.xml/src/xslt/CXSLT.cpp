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

#include "../gb.xml.h"
#include "../gbinterface.h"

extern GB_INTERFACE GB;
extern XML_INTERFACE XML;

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

BEGIN_METHOD(CXSLT_Transform,GB_OBJECT inputDoc;GB_OBJECT inputStyleSheet)

if (GB.CheckObject(VARGOBJ(CDocument,inputDoc))) return;
if (GB.CheckObject(VARGOBJ(CDocument,inputStyleSheet))) return;

    Document *doc = (Document*)(VARGOBJ(CDocument,inputDoc)->node),
             *stylesheet = (Document*)(VARGOBJ(CDocument,inputStyleSheet)->node);

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
    
    char *StyleSheetOutput = NULL;
    size_t StyleSheetLen = 0;

    XML.SerializeXMLNode((Node*)stylesheet, StyleSheetOutput, StyleSheetLen, -1);
    
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


    XML.SerializeXMLNode((Node*)doc, DocumentOutput, DocumentLen, -1);
    
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

    Document *outDoc = XML.XMLDocument_New();
    
    /*try
    {*/
        XML.XMLDocument_SetContent(outDoc, (char*)(buffer),size);
    //}
    /*catch(XMLParseException &e)
    {
        XML.XMLDocument_SetContent(outDoc, "<?xml version=\"1.0\"?><xml></xml>", 32);
        std::cerr << "XSLT Warning : error when parsing output document : " << std::endl << e.what() << std::endl;
    }*/
        
    XML.ReturnNode(outDoc);
		
END_METHOD



GB_DESC CXsltDesc[] =
{
  GB_DECLARE("Xslt", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD ("Transform","XmlDocument",CXSLT_Transform,"(Document)XmlDocument;(StyleSheet)XmlDocument;"),

  GB_END_DECLARE
};
