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
#include "../gb.xml.h"
#include "CXSLT.h"

extern GB_INTERFACE GB;
extern XML_INTERFACE XML;

#include <iostream>
#include <memory.h>
#include <libxml2/libxml/xinclude.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

void XSLT_Transform(Document* doc, Document* stylesheet,char* &outDocument, size_t &outDocumentLen) throw(XSLTException)
{
    if(!doc->childCount) throw XSLTException("Void document");
    if(!stylesheet->childCount) throw XSLTException("Void style sheet");

    xsltStylesheetPtr sheet = 0;

    char *StyleSheetOutput = NULL;
    size_t StyleSheetLen = 0;

    XML.SerializeXMLNode((Node*)stylesheet, StyleSheetOutput, StyleSheetLen, -1);

    StyleSheetOutput =(char*)realloc(StyleSheetOutput, StyleSheetLen + 1);
    StyleSheetOutput[StyleSheetLen] = 0;


    xmlDoc *xmlStyleSheet = xmlParseDoc((xmlChar*)(StyleSheetOutput));

    free(StyleSheetOutput);

    if(!(sheet=xsltParseStylesheetDoc(xmlStyleSheet))) throw XSLTException("Invalid style sheet");

    char *DocumentOutput;
    size_t DocumentLen;


    XML.SerializeXMLNode((Node*)doc, DocumentOutput, DocumentLen, -1);

    DocumentOutput =(char*)realloc(DocumentOutput, DocumentLen + 1);
    DocumentOutput[DocumentLen] = 0;


    xmlDoc *xmlInputDoc = xmlParseDoc((xmlChar*)(DocumentOutput));
    if(!xmlInputDoc) throw XSLTException("Unable to parse input document");

    free(DocumentOutput);

    xmlDoc *xmlOutDoc;

    xmlOutDoc = xsltApplyStylesheet(sheet, xmlInputDoc, NULL);
    if (!xmlOutDoc) throw XSLTException("Unable to apply style sheet");

    int size;
    xmlDocDumpFormatMemoryEnc(xmlOutDoc ,(xmlChar**)&outDocument, &size, "UTF-8", 1);
    outDocumentLen = size;

    xsltFreeStylesheet(sheet);
    xmlFreeDoc(xmlOutDoc);

    xmlFreeDoc(xmlInputDoc);
}

BEGIN_METHOD(CXSLT_Transform,GB_OBJECT inputDoc;GB_OBJECT inputStyleSheet)

if (GB.CheckObject(VARGOBJ(CDocument,inputDoc))) return;
if (GB.CheckObject(VARGOBJ(CDocument,inputStyleSheet))) return;

    Document *doc = (Document*)(VARGOBJ(CDocument,inputDoc)->node),
             *stylesheet = (Document*)(VARGOBJ(CDocument,inputStyleSheet)->node);

    char *buffer = 0;
    size_t size;

    try
    {
        XSLT_Transform(doc, stylesheet, buffer, size);
    }
    catch(XSLTException &ex)
    {
        GB.Error(ex.what());
        GB.ReturnNull();
        return;
    }

    Document *outDoc = XML.XMLDocument_New();
    
    try
    {
        XML.XMLDocument_SetContent(outDoc, (char*)(buffer),size);
    }
    catch(XMLParseException e)
    {
        std::cerr << "XSLT Warning : error when parsing output document : " << e.errorWhat << std::endl;
    }


    free(buffer);
    XML.ReturnNode(outDoc);

		
END_METHOD

BEGIN_METHOD(CXSLT_TransformToString,GB_OBJECT inputDoc;GB_OBJECT inputStyleSheet)

if (GB.CheckObject(VARGOBJ(CDocument,inputDoc))) return;
if (GB.CheckObject(VARGOBJ(CDocument,inputStyleSheet))) return;

    Document *doc = (Document*)(VARGOBJ(CDocument,inputDoc)->node),
             *stylesheet = (Document*)(VARGOBJ(CDocument,inputStyleSheet)->node);

    char *buffer = 0;
    size_t size;

    try
    {
        XSLT_Transform(doc, stylesheet, buffer, size);
    }
    catch(XSLTException &ex)
    {
        GB.Error(ex.what());
        GB.ReturnNull();
        return;
    }

    GB.ReturnNewString(buffer, size);

    free(buffer);

END_METHOD




GB_DESC CXsltDesc[] =
{
  GB_DECLARE("Xslt", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD ("Transform","XmlDocument",CXSLT_Transform,"(Document)XmlDocument;(StyleSheet)XmlDocument;"),
    GB_STATIC_METHOD ("TransformToString","s",CXSLT_TransformToString,"(Document)XmlDocument;(StyleSheet)XmlDocument;"),

  GB_END_DECLARE
};


XSLTException::XSLTException(const char *error) throw()
{
    this->error = error;
}

const char *XSLTException::what()
{
    return this->error;
}
