/***************************************************************************

  CXSLT.c

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

#define __CXSLT_C


#include "main.h"
#include "CXSLT.h"


#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>


#include "../document.h"
#include "../document.cpp"
#include "../element.h"
#include "../element.cpp"
#include "../node.h"
#include "../node.cpp"
#include "../textnode.h"
#include "../textnode.cpp"

BEGIN_METHOD(CXSLT_Transform,GB_OBJECT inputDoc;GB_OBJECT inputStyleSheet)

    Document *doc = VARGOBJ(Document,inputDoc),
             *stylesheet = VARGOBJ(Document,inputStyleSheet);
	
    if (GB.CheckObject(doc)) return;
    if (GB.CheckObject(stylesheet)) return;
	
    if (!doc->root->children->size())
	{
		GB.Error("Void document");
		return;
	}
	
    if (!stylesheet->root->children->size())
	{
		GB.Error("Void Style Sheet");
		return;
	}

    xsltStylesheetPtr sheet = 0;
    xmlDoc *xmlStyleSheet = xmlParseDoc((xmlChar*)(WStringToString(stylesheet->getContent()).c_str()));
	

    if(!(sheet=xsltParseStylesheetDoc(xmlStyleSheet)))
	{
		GB.Error("Invalid style sheet");
		return;
	}

    xmlDoc *xmlInputDoc = xmlParseDoc((xmlChar*)(WStringToString(doc->getContent()).c_str()));

	
    xmlDoc *xmlOutDoc;
    xmlChar *buffer = 0;
    int size;

    xmlOutDoc = xsltApplyStylesheet(sheet, xmlInputDoc, NULL);
    if (!xmlOutDoc)
    {
        GB.Error("Unable to apply style sheet");
    }
    xmlDocDumpFormatMemoryEnc(xmlOutDoc ,&buffer, &size, "UTF-8", 1);

    Document *outDoc = GBI::New<Document>("XmlDocument");
    try{
    outDoc->setContent(StringToWString(string((char*)(buffer),size)));
    }
    catch(HTMLParseException &e)
    {
        outDoc->setContent(L"<?xml version=\"1.0\"?><xml></xml>");
        std::cerr << "XSLT Warning : error when parsing output document : " << endl << e.what() << endl;
    }
	
    GB.ReturnObject(outDoc);
		
END_METHOD



GB_DESC CXsltDesc[] =
{
  GB_DECLARE("Xslt", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD ("Transform","XmlDocument",CXSLT_Transform,"(Document)XmlDocument;(StyleSheet)XmlDocument;"),

  GB_END_DECLARE
};




