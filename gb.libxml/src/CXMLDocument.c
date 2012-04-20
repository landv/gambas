/***************************************************************************

  CXMLDocument.c

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

#define __CXMLDOCUMENT_C

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include "main.h"
#include "CXMLDocument.h"

CXMLNODE *XML_CreateNode(CXMLDOCUMENT *doc, xmlNode *node)
{
	CXMLNODE *p;
	
	if (!node)
		return NULL;
	
	p = GB.New(GB.FindClass("XmlNode"), NULL, NULL);
	p->node = node;
	p->doc = doc;
	GB.Ref(doc);
	return p;
}

static void free_document(CXMLDOCUMENT *_object)
{
	if (THIS->doc)
	{
		xmlFreeDoc(THIS->doc);
		THIS->doc = NULL;
	}
}


void XML_InitDocument(CXMLDOCUMENT *_object, xmlDoc *doc, const char *err)
{
	if (!doc)
	{
		GB.Error(err ? err : "Unable to parse XML file");
		return;
	}
	
	free_document(THIS);
	THIS->doc = doc;
}


BEGIN_METHOD_VOID (CXMLDocument_Free)

	free_document(THIS);

END_METHOD


BEGIN_METHOD (CXMLDocument_Open,GB_STRING FileName;)

	const char *path;

	path = GB.RealFileName(STRING(FileName), LENGTH(FileName));
	XML_InitDocument(THIS, xmlParseFile(path), NULL);

END_METHOD

BEGIN_METHOD (CXMLDocument_FromString,GB_STRING Data;)

	XML_InitDocument(THIS, xmlParseDoc((xmlChar *)GB.ToZeroString(ARG(Data))), NULL);

END_METHOD

BEGIN_METHOD (CXMLDocument_HtmlFromString,GB_STRING Data;)

	XML_InitDocument(THIS, htmlParseDoc((xmlChar *)GB.ToZeroString(ARG(Data)),NULL), NULL);

END_METHOD


BEGIN_METHOD (CXMLDocument_Write,GB_STRING FileName; GB_STRING Encoding;)

	char *enc;

	if (!THIS->doc)
	{
		GB.Error("Unable to write NULL document");
		return;
	}

	if (MISSING(Encoding))
		enc="UTF-8";
	else
		enc=GB.ToZeroString(ARG(Encoding));

	xmlSaveFormatFileEnc(GB.ToZeroString(ARG(FileName)), THIS->doc,enc , 1);

END_METHOD

BEGIN_METHOD(CXMLDocument_ToString, GB_STRING Encoding)

	xmlChar *mem;
	int size;
	char *encoding;

	if (!THIS->doc) 
	{
		GB.ReturnVoidString();
		return;
	}

	if (MISSING(Encoding))
		encoding = "UTF-8";
	else
		encoding = GB.ToZeroString(ARG(Encoding));

	xmlDocDumpFormatMemoryEnc(THIS->doc, &mem, &size, encoding, 1);
	
	GB.ReturnNewString((char*)mem,size);
	xmlFree(mem);

END_METHOD

BEGIN_PROPERTY (CXMLDocument_Root)

	GB.ReturnObject(XML_CreateNode(THIS, xmlDocGetRootElement(THIS->doc)));

END_PROPERTY


GB_DESC CXmlDocumentDesc[] =
{

	GB_DECLARE("XmlDocument", sizeof(CXMLDOCUMENT)),

	GB_PROPERTY_READ("Root","XmlNode",CXMLDocument_Root),

	GB_METHOD("_free",NULL,CXMLDocument_Free,NULL),

	GB_METHOD("Open", NULL, CXMLDocument_Open, "(FileName)s"),
	GB_METHOD("FromString", NULL, CXMLDocument_FromString, "(Data)s"),
	GB_METHOD("HtmlFromString", NULL, CXMLDocument_HtmlFromString, "(Data)s"),
	GB_METHOD("Write", NULL, CXMLDocument_Write, "(FileName)s[(Encoding)s]"),
	//GB_METHOD("Write",NULL,CXMLDocument_Write,"(FileName)s[(Encoding)s]"),
	GB_METHOD("ToString", "s", CXMLDocument_ToString, "[(Encoding)s]"),



	GB_END_DECLARE
};






