/***************************************************************************

  CXMLDocument.c

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



#define __CXMLDOCUMENT_C

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include "main.h"
#include "CXMLDocument.h"


void free_document(CXMLDOCUMENT *test)
{

	int bucle;

	if (test->nchildren)
	{
		for (bucle=0;bucle<test->nchildren;bucle++)
			((CXMLNODE*)test->children[bucle])->parent=NULL;

		GB.Free(POINTER(&test->children));
		test->nchildren=0;
	}

	if (test->doc)
	{
		xmlFreeDoc(test->doc);
		test->doc=NULL;
	}

	if (test->node)
	{
		GB.Unref(POINTER(&test->node));
	}


}

void Doc_AddChild(void *_object,CXMLNODE *chd)
{
	THIS->nchildren++;

	if (!THIS->children)
		GB.Alloc(POINTER(&THIS->children),sizeof(CXMLNODE*));
	else
		GB.Realloc(POINTER(&THIS->children),THIS->nchildren*sizeof(CXMLNODE*));

	// BM: BUG?
	chd->parent=THIS->doc;
	THIS->children[THIS->nchildren-1]=chd;
}

void Doc_RemoveChild(void *_object,CXMLNODE *chd)
{
	int myloop,myloop2;

	for (myloop=0; myloop<THIS->nchildren; myloop++)
	{
		if (THIS->children[myloop] == chd )
		{
			THIS->nchildren--;
			for (myloop2=myloop;myloop2<THIS->nchildren;myloop2++)
				THIS->children[myloop2]=THIS->children[myloop2+1];

			if (!THIS->nchildren)
				GB.Free(POINTER(&THIS->children));
			else
				GB.Realloc(POINTER(&THIS->children),THIS->nchildren*sizeof(CXMLNODE*));

			return;
		}
	}
}


BEGIN_METHOD_VOID (CXMLDocument_Free)

	free_document(THIS);

END_METHOD


BEGIN_METHOD (CXMLDocument_Open,GB_STRING FileName;)

	const char *path;

	free_document(THIS);

	path = GB.RealFileName(STRING(FileName), LENGTH(FileName));

	THIS->doc=xmlParseFile(path);
	if (!THIS->doc)
	{
		GB.Error("Unable to parse XML file");
		return;
	}

	GB.New(POINTER(&THIS->node),GB.FindClass("XmlNode"),NULL,NULL);
	THIS->node->node=xmlDocGetRootElement(THIS->doc);
	Doc_AddChild(THIS,THIS->node);
	GB.Ref((void*)THIS->node);

END_METHOD

BEGIN_METHOD (CXMLDocument_FromString,GB_STRING Data;)


	free_document(THIS);

	THIS->doc=xmlParseDoc((xmlChar *)GB.ToZeroString(ARG(Data)));
	if (!THIS->doc)
	{
		GB.Error("Unable to parse XML data");
		return;
	}

	GB.New(POINTER(&THIS->node),GB.FindClass("XmlNode"),NULL,NULL);
	THIS->node->node=xmlDocGetRootElement(THIS->doc);
	Doc_AddChild(THIS,THIS->node);
	GB.Ref((void*)THIS->node);

END_METHOD

BEGIN_METHOD (CXMLDocument_HtmlFromString,GB_STRING Data;)


	free_document(THIS);

	THIS->doc=htmlParseDoc((xmlChar *)GB.ToZeroString(ARG(Data)),NULL);
	if (!THIS->doc)
	{
		GB.Error("Unable to parse XML data");
		return;
	}

	GB.New(POINTER(&THIS->node),GB.FindClass("XmlNode"),NULL,NULL);
	THIS->node->node=xmlDocGetRootElement(THIS->doc);
	Doc_AddChild(THIS,THIS->node);
	GB.Ref((void*)THIS->node);

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
		enc=GB.ToZeroString(ARG(FileName));

	xmlSaveFormatFileEnc(GB.ToZeroString(ARG(FileName)), THIS->doc,enc , 1);

END_METHOD

BEGIN_METHOD (CXMLDocument_ToString,GB_STRING Encoding;)

	xmlChar *mem;
	int size;

	if (!THIS->doc) return;

	xmlDocDumpFormatMemory(THIS->doc,&mem ,&size , 1);

	GB.ReturnNewString((char*)mem,size);

END_METHOD

BEGIN_PROPERTY (CXMLDocument_Root)

	GB.ReturnObject(THIS->node);

END_PROPERTY

BEGIN_PROPERTY (CXMLDocument_Encoding)



END_PROPERTY



GB_DESC CXmlDocumentDesc[] =
{

	GB_DECLARE("XmlDocument", sizeof(CXMLDOCUMENT)),

	GB_PROPERTY_READ("Encoding","s",CXMLDocument_Encoding),
	GB_PROPERTY_READ("Root","XmlNode",CXMLDocument_Root),

	GB_METHOD("_free",NULL,CXMLDocument_Free,NULL),

	GB_METHOD("Open",NULL,CXMLDocument_Open,"(FileName)s"),
	GB_METHOD("FromString",NULL,CXMLDocument_FromString,"(Data)s"),
	GB_METHOD("HtmlFromString",NULL,CXMLDocument_HtmlFromString,"(Data)s"),
	GB_METHOD("Write",NULL,CXMLDocument_Write,"(FileName)s[(Encoding)s]"),

	GB_METHOD("Write",NULL,CXMLDocument_Write,"(FileName)s[(Encoding)s]"),
	GB_METHOD("ToString","s",CXMLDocument_ToString,"[(Encoding)s]"),



	GB_END_DECLARE
};






