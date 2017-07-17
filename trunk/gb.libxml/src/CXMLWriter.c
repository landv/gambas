/***************************************************************************

  CXMLWriter.c

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

#define __CXMLWRITER_C

#include <stdio.h>
#include <string.h>
#include <libxml/xmlwriter.h>
#include "main.h"
#include "CXMLWriter.h"



int Check_Writer(CXMLWRITER *test)
{
	if (!test->writer) 
	{
		GB.Error("No XML file or string to write to");
		return 1;
	}
	return 0;
}

void Free_Writer(CXMLWRITER *test)
{
	if (test->writer)
	{
		xmlTextWriterEndDocument(test->writer);
		xmlFreeTextWriter(test->writer);
		test->writer=NULL;
	}
	if (test->buffer) xmlBufferFree(test->buffer);
	test->buffer=NULL;
}

int Resul_Writer(CXMLWRITER *test,int value)
{
	if (value==-1)
	{
		Free_Writer(test);
		GB.Error("Error writing XML data");
		return -1;
	}
	return 0;
}

BEGIN_METHOD_VOID(CXmlWriter_Free)

	Free_Writer(THIS);

END_METHOD

BEGIN_METHOD (CXmlWriter_Open,GB_STRING FileName; GB_BOOLEAN Indent; GB_STRING Encoding;)

	int res;
	int indent=0;
	char *encoding=NULL;
	
	if (!MISSING(Indent))
		if (VARG(Indent)) indent=1;
	
	if (!MISSING(Encoding))
		encoding=GB.ToZeroString(ARG(Encoding));	
	
	Free_Writer (THIS);
	
	if (!LENGTH(FileName))
	{
		THIS->buffer=xmlBufferCreate();
		THIS->writer = xmlNewTextWriterMemory(THIS->buffer, 0);
		xmlTextWriterSetIndent(THIS->writer,indent);
	}
	else
	{	
		THIS->writer = xmlNewTextWriterFilename(GB.ToZeroString(ARG(FileName)),0); 
		xmlTextWriterSetIndent(THIS->writer,indent);
	}
	
	if (!THIS->writer)
	{
		GB.Error("Unable to write XML file");
		return;
	}
	
	res=xmlTextWriterStartDocument(THIS->writer, NULL,encoding, NULL);
	
	if (res==-1)
	{
		Free_Writer (THIS);
		GB.Error("Unable to write XML file");
		return; 
	}	
	
	

END_METHOD


BEGIN_METHOD (CXmlWriter_StartElement,GB_STRING Name; GB_OBJECT Attributes;GB_STRING Prefix;GB_STRING URI;)

	int bucle;
	int nmax;
	int res;
	char *sname;
	char *svalue;
	char *prefix=NULL;
	char *uri=NULL;
	
	if (!MISSING(Prefix)) prefix=GB.ToZeroString(ARG(Prefix));
	if (!MISSING(URI)) uri=GB.ToZeroString(ARG(URI));
	
	if (Check_Writer(THIS)) return;
	
	if (prefix || uri)
		res=xmlTextWriterStartElementNS(THIS->writer,(xmlChar *)prefix,(xmlChar *)GB.ToZeroString(ARG(Name)),(xmlChar *)uri);
	else
		res=xmlTextWriterStartElement(THIS->writer,(xmlChar *)GB.ToZeroString(ARG(Name)));
	
	if (Resul_Writer(THIS,res)) return;

	if (MISSING(Attributes)) return;
	if (!VARG(Attributes)) return;
	
	nmax=GB.Array.Count(VARG(Attributes));
	
	for (bucle=0;bucle<nmax;bucle+=2)
	{
		sname=*( (char**)GB.Array.Get(VARG(Attributes),bucle)  );
		if (!sname) sname="";
		if (bucle<(nmax-1))
			svalue=*( (char**)GB.Array.Get(VARG(Attributes),bucle+1)  );
		else
			svalue="";
		
		res=xmlTextWriterWriteAttribute(THIS->writer,(xmlChar *)sname,(xmlChar *)svalue);
		if (Resul_Writer(THIS,res)) return;
	}
	

END_METHOD

BEGIN_METHOD_VOID (CXmlWriter_EndElement)

	if (Check_Writer(THIS)) return;
	
	Resul_Writer(THIS,xmlTextWriterEndElement(THIS->writer));
	
	
END_METHOD

BEGIN_METHOD(CXmlWriter_Element,GB_STRING Name;GB_STRING Value;GB_STRING Prefix;GB_STRING URI;)

	xmlChar *name,*value;
	int resul;
	char *prefix=NULL;
	char *uri=NULL;
	
	if (!MISSING(Prefix)) prefix=GB.ToZeroString(ARG(Prefix));
	if (!MISSING(URI)) uri=GB.ToZeroString(ARG(URI));
	
	if (Check_Writer(THIS)) return;

	name=(xmlChar *)GB.ToZeroString(ARG(Name));
	if (!MISSING(Value))
	{
		value=(xmlChar *)GB.ToZeroString(ARG(Value));
		
		if ( prefix || uri )
			resul=xmlTextWriterWriteElementNS(THIS->writer,(xmlChar *)prefix,name,(xmlChar *)uri,value);
		else 
			resul=xmlTextWriterWriteElement(THIS->writer,name,value);
	}
	else
	{
		if ( prefix || uri )
			resul=xmlTextWriterStartElementNS(THIS->writer,(xmlChar *)prefix,name,(xmlChar *)uri);
		else
			resul=xmlTextWriterStartElement(THIS->writer,name);
		if (resul != -1) resul=xmlTextWriterEndElement(THIS->writer);
	}
		
	
	Resul_Writer(THIS,resul);
	
END_METHOD

BEGIN_METHOD(CXmlWriter_Text,GB_STRING Name;)

	xmlChar *name;

	if (Check_Writer(THIS)) return;
	name=(xmlChar *)GB.ToZeroString(ARG(Name));	
	Resul_Writer(THIS,xmlTextWriterWriteString(THIS->writer,name));

END_METHOD

BEGIN_METHOD(CXmlWriter_Base64,GB_STRING Name;)

	if (Check_Writer(THIS)) return;
	Resul_Writer(THIS,xmlTextWriterWriteBase64 (THIS->writer,STRING(Name),0,LENGTH(Name)));

END_METHOD

BEGIN_METHOD(CXmlWriter_BinHex,GB_STRING Name;)

	if (Check_Writer(THIS)) return;
	Resul_Writer(THIS,xmlTextWriterWriteBinHex (THIS->writer,STRING(Name),0,LENGTH(Name)));

END_METHOD

BEGIN_METHOD(CXmlWriter_CDATA,GB_STRING Name;)

	if (Check_Writer(THIS)) return;	
	Resul_Writer(THIS,xmlTextWriterWriteCDATA(THIS->writer,(xmlChar *)GB.ToZeroString(ARG(Name))));

END_METHOD


BEGIN_METHOD(CXmlWriter_Attribute,GB_STRING Name;GB_STRING Value;GB_STRING Prefix;GB_STRING URI;)

	char *name,*value;
	char *prefix=NULL;
	char *uri=NULL;
	int res;
	
	if (!MISSING(Prefix)) prefix=GB.ToZeroString(ARG(Prefix));
	if (!MISSING(URI)) uri=GB.ToZeroString(ARG(URI));

	if (Check_Writer(THIS)) return;

	name=GB.ToZeroString(ARG(Name));
	value=GB.ToZeroString(ARG(Value));
		
	if (prefix || uri)
		res=xmlTextWriterWriteAttributeNS(THIS->writer,(xmlChar *)prefix,(xmlChar *)name,(xmlChar *)uri,(xmlChar *)value);
	else
		res=xmlTextWriterWriteAttribute(THIS->writer,(xmlChar *)name,(xmlChar *)value);
	
	Resul_Writer(THIS,res);


END_METHOD

BEGIN_METHOD(CXmlWriter_WritePI,GB_STRING Target;GB_STRING Content;)

	char *target,*content;

	if (Check_Writer(THIS)) return;
	target=GB.ToZeroString(ARG(Target));
	content=GB.ToZeroString(ARG(Content));	
	Resul_Writer(THIS,xmlTextWriterWritePI(THIS->writer,(xmlChar *)target,(xmlChar *)content));


END_METHOD



				     
BEGIN_METHOD(CXmlWriter_Comment,GB_STRING Comment)

	if (Check_Writer(THIS)) return;
	Resul_Writer(THIS,xmlTextWriterWriteComment(THIS->writer,(xmlChar *)GB.ToZeroString(ARG(Comment))));

END_METHOD

BEGIN_METHOD_VOID(CXmlWriter_EndDocument)

	if (Check_Writer(THIS)) return;
	
	xmlTextWriterEndDocument(THIS->writer);
	xmlFreeTextWriter(THIS->writer);
	THIS->writer=NULL;
	if (!THIS->buffer)
	{
		GB.ReturnVoidString();
		return;
	}
	GB.ReturnNewZeroString((char *)THIS->buffer->content);
	xmlBufferFree(THIS->buffer);
	THIS->buffer=NULL;

END_METHOD


BEGIN_PROPERTY(CXMLWriter_DTD)

	RETURN_SELF();

END_PROPERTY


BEGIN_METHOD(CXmlWriter_StartDTD,GB_STRING Name;GB_STRING PubID;GB_STRING SysID;)

	char *name,*pubid=NULL,*sysid=NULL;

	if (Check_Writer(THIS)) return;

	name=GB.ToZeroString(ARG(Name));
	if (!MISSING(PubID)) pubid=GB.ToZeroString(ARG(PubID));
	if (!MISSING(SysID)) pubid=GB.ToZeroString(ARG(SysID));
	Resul_Writer(THIS,xmlTextWriterStartDTD(THIS->writer,(xmlChar *)name,(xmlChar *)pubid,(xmlChar *)sysid));
	
END_METHOD

BEGIN_METHOD_VOID (CXmlWriter_EndDTD)

	if (Check_Writer(THIS)) return;
	
	Resul_Writer(THIS,xmlTextWriterEndDTD(THIS->writer));
	
END_METHOD

BEGIN_METHOD (CXmlWriter_DTDElement,GB_STRING Name;GB_STRING Content;)

	char *name,*content;
	int resul;

	if (Check_Writer(THIS)) return;
	
	name=GB.ToZeroString(ARG(Name));
	if (!MISSING(Content))
	{
		content=GB.ToZeroString(ARG(Content));
		resul=xmlTextWriterWriteDTDElement (THIS->writer,(xmlChar *)name,(xmlChar *)content);
	}
	else
	{
		resul=xmlTextWriterStartDTDElement (THIS->writer,(xmlChar *)name);
		if (resul != 1) resul=xmlTextWriterEndDTDElement(THIS->writer);
	}
	
	Resul_Writer(THIS,resul);

END_METHOD


BEGIN_METHOD (CXmlWriter_DTDInternalEntity,GB_STRING Name;GB_STRING Content;GB_BOOLEAN IsParameter;)

	char *name,*content;
	int isparameter=0;
	
	if (Check_Writer(THIS)) return;
	
	name=GB.ToZeroString(ARG(Name));
	content=GB.ToZeroString(ARG(Content));
	if (!MISSING(IsParameter)) isparameter=VARG(IsParameter);
	Resul_Writer(THIS,xmlTextWriterWriteDTDInternalEntity(THIS->writer,isparameter,(xmlChar *)name,(xmlChar *)content));
	
END_METHOD


BEGIN_METHOD (CXmlWriter_DTDAttList,GB_STRING Name;GB_STRING Content;)

	char *name,*content;
	
	if (Check_Writer(THIS)) return;
	
	name=GB.ToZeroString(ARG(Name));
	content=GB.ToZeroString(ARG(Content));
	Resul_Writer(THIS,xmlTextWriterWriteDTDAttlist(THIS->writer,(xmlChar *)name,(xmlChar *)content));

	
END_METHOD



GB_DESC CXmlWriterDTDDesc[] =
{
	// TOOD: external entity
	// Namespace
	GB_DECLARE(".XmlWriter.DTD", 0), GB_VIRTUAL_CLASS(),
	
	GB_METHOD("Start",NULL,CXmlWriter_StartDTD,"(Name)s[(PubID)s(SysID)s]"),
	GB_METHOD("Element",NULL,CXmlWriter_DTDElement,"(Name)s[(Content)s]"),
	GB_METHOD("InternalEntity",NULL,CXmlWriter_DTDInternalEntity,"(Name)s(Content)s[(IsParameter)b]"),
	GB_METHOD("AttList",NULL,CXmlWriter_DTDAttList,"(Name)s(Content)s"),
	GB_METHOD("End",NULL,CXmlWriter_EndDTD,NULL),

	GB_END_DECLARE
};

GB_DESC CXmlWriterDesc[] =
{
  GB_DECLARE("XmlWriter", sizeof(CXMLWRITER)),

  GB_PROPERTY_READ("DTD",".XmlWriter.DTD",CXMLWriter_DTD),
  
  GB_METHOD("_free",NULL,CXmlWriter_Free,NULL),

  GB_METHOD("Open", NULL, CXmlWriter_Open,"(FileName)s[(Indent)b(Encoding)s]"),
  
  GB_METHOD("StartElement",NULL,CXmlWriter_StartElement,"(Name)s[(Attributes)String[];(Prefix)s(URI)s]"),
  GB_METHOD("Attribute",NULL,CXmlWriter_Attribute,"(Name)s(Value)s[(Prefix)s(URI)s]"),
  GB_METHOD("Element",NULL,CXmlWriter_Element,"(Name)s[(Value)s(Prefix)s(URI)s]"),
  GB_METHOD("Comment",NULL,CXmlWriter_Comment,"(Comment)s"),
  GB_METHOD("Text",NULL,CXmlWriter_Text,"(Data)s"),
  GB_METHOD("Base64",NULL,CXmlWriter_Base64,"(Data)s"),
  GB_METHOD("BinHex",NULL,CXmlWriter_BinHex,"(Data)s"),
  GB_METHOD("CDATA",NULL,CXmlWriter_CDATA,"(Data)s"),
  GB_METHOD("EndElement",NULL,CXmlWriter_EndElement,NULL),
  GB_METHOD("EndDocument","s",CXmlWriter_EndDocument,NULL),
  GB_METHOD("PI",NULL,CXmlWriter_WritePI,"(Target)s(Content)s"),

  GB_END_DECLARE
};




