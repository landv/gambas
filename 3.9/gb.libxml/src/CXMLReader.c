/***************************************************************************

  CXMLReader.c

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

#define __CXMLREADER_C

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libxml/xmlreader.h>
#include "main.h"
#include "CXMLReader.h"

unsigned char b64value(char car)
{
	if ( (car>=65) && (car<=90) ) return car-65;
	if ( (car>=97) && (car<=122) ) return car-71;
	if ( (car>=48) && (car<=57) ) return car+4;
	if (car=='+') return 62;
	if (car=='/') return 63;
	if (car=='=') return 254;
	return 255;

}

void FromBinHex(char *src,char *dst)
{
	char b;
	unsigned long bucle;
	int zone=0;

	for (bucle=0;bucle<strlen(src);bucle++)
	{
		switch (toupper(src[bucle]))
		{
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				b=src[bucle]-48; break;
			default:
				b=src[bucle]-55;
		}

		switch(zone)
		{
			case 0:
				dst[bucle/2]=b<<4;
				zone=1;
				break;
			default:
				dst[bucle/2]+=b;
				zone=0;
				break;
		}
	}
}

unsigned long FromBase64(char *src,char *dst)
{
	unsigned long bucle,retval=0;
	unsigned char car;
	int zone=0;
	int pads=0;

	for (bucle=0;bucle<strlen(src);bucle++)
	{
		car=b64value(src[bucle]);
		switch (car)
		{
			case 255: break;
			case 254:
				zone=4;
				pads++;
				if (pads==3) return retval-pads;
				break;
			default:
				switch(zone)
				{
					case 0:
						retval+=3;
						dst[retval-3]=car<<2;
						zone++;
						break;

					case 1:
						dst[retval-3]+=(car>>4);
						dst[retval-2]=car<<4;
						zone++;
						break;
					case 2:
						dst[retval-2]+=car>>2;
						dst[retval-1]=car<<6;
						zone++;
						break;
					case 3:
						dst[retval-1]+=car;
						zone=0;
						break;
					case 4:
						return retval;
				}
		}
	}

	return retval-pads;
}


int Check_Reader(CXMLREADER *test)
{
	if (!test->reader)
	{
		GB.Error("No XML file or string to read from");
		return 1;
	}

	if (test->eof)
	{
		GB.Error("Reached end of file");
		return 1;
	}

	return 0;
}

void Free_Reader(CXMLREADER *test)
{
	if (test->buffer)GB.Free(POINTER(&test->buffer));
	if (test->reader)
	{
		xmlTextReaderClose(test->reader);
		xmlFreeTextReader(test->reader);
		test->reader=NULL;
	}
	test->eof=0;
}

BEGIN_METHOD (CXmlReader_Open,GB_STRING FileName)

	Free_Reader(THIS);
    	THIS->reader=xmlReaderForFile(GB.ToZeroString(ARG(FileName)),NULL,0);
	if (!THIS->reader) GB.Error ("Unable to parse XML file");


END_METHOD

BEGIN_METHOD (CXmlReader_FromString,GB_STRING Buffer;GB_STRING URL;)

	if (!LENGTH(Buffer))
	{
		GB.Error ("Unable to parse NULL string");
		return;
	}

	Free_Reader(THIS);

	GB.Alloc(POINTER(&THIS->buffer),LENGTH(Buffer));
	memcpy (THIS->buffer,STRING(Buffer),LENGTH(Buffer));

    	if (!MISSING(URL))
	  THIS->reader=xmlReaderForMemory(THIS->buffer,LENGTH(Buffer),GB.ToZeroString(ARG(URL)),NULL,0);
	else
	  THIS->reader=xmlReaderForMemory(THIS->buffer,LENGTH(Buffer),"",NULL,0);

	if (!THIS->reader) GB.Error ("Unable to parse XML file");


END_METHOD

BEGIN_METHOD_VOID (CXmlReader_Read)

	int retval;

	if (Check_Reader(THIS)) return;

	retval=xmlTextReaderRead(THIS->reader);
	switch(retval)
	{
		case  0:
			THIS->eof=1;
			break;

		case -1:
			Free_Reader(THIS);
			GB.Error("Error parsing XML file");
			break;
	}


END_METHOD

BEGIN_METHOD (CXmlReader_Decode,GB_STRING Data;GB_STRING Encoding;)

	char *dst=NULL;
	unsigned long len;

	if (!strcasecmp(GB.ToZeroString(ARG(Encoding)),"base64") ) {
		if (!LENGTH(Data)) return;

		GB.Alloc (POINTER(&dst),LENGTH(Data) );
		len=FromBase64(GB.ToZeroString(ARG(Data)),dst);
		GB.ReturnNewString(dst,len);
		GB.Free(POINTER(&dst));
		return;
	}

	if (!strcasecmp(GB.ToZeroString(ARG(Encoding)),"binhex") ) {
		if (!LENGTH(Data)) return;
		if (LENGTH(Data)%2) return;

		dst=STRING(Data);
		for(len=0;len<LENGTH(Data);len++)
		{
			switch (toupper(dst[len]))
			{
				case '0': case '1': case '2': case '3': case '4': case '5':
				case '6': case '7': case '8': case '9': case 'A': case 'B':
				case 'C': case 'D': case 'E': case 'F': break;
				default: return;
			}

		}

		dst=NULL;
		GB.Alloc (POINTER(&dst),LENGTH(Data)/2);
		FromBinHex(GB.ToZeroString(ARG(Data)),dst);
		GB.ReturnNewString(dst,LENGTH(Data)/2);
		GB.Free(POINTER(&dst));
		return;
	}


	GB.Error ("Invalid encoding");



END_METHOD


BEGIN_PROPERTY (CXMLReader_Node)

	RETURN_SELF();

END_PROPERTY

BEGIN_METHOD_VOID(CXmlReader_Free)

	Free_Reader(THIS);


END_METHOD



BEGIN_METHOD_VOID(CXmlReader_next)

	char *wenum=(char*)GB.GetEnum();
	int eval;

	if (Check_Reader(THIS))
	{
		GB.StopEnum();
		return;
	}

	if (!wenum[0]) eval=xmlTextReaderMoveToFirstAttribute(THIS->reader);
	else           eval=xmlTextReaderMoveToNextAttribute(THIS->reader);

	if (eval==-1)
	{
		xmlFreeTextReader(THIS->reader);
		THIS->reader=NULL;
		GB.StopEnum();
		GB.Error("Error parsing XML file");
		return;
	}

	if (!eval)
	{
		if (wenum[0]) xmlTextReaderMoveToElement(THIS->reader);
		GB.StopEnum();
		return;
	}

	wenum[0]=1;
	RETURN_SELF();



END_METHOD

BEGIN_PROPERTY(CXmlReader_count)

	int nval;

	if (Check_Reader(THIS)) return;

	nval=xmlTextReaderAttributeCount(THIS->reader);

	if (nval==-1)
	{
		xmlFreeTextReader(THIS->reader);
		THIS->reader=NULL;
		GB.Error("Error parsing XML file");
		return;
	}

	GB.ReturnInteger(nval);

END_METHOD

BEGIN_PROPERTY(CXMLReader_EOF)

	if (!THIS->reader)
	{
		GB.ReturnBoolean(1);
		return;
	}

	GB.ReturnBoolean(THIS->eof);

END_PROPERTY


BEGIN_PROPERTY(CRNODE_BaseUri)

	if (Check_Reader(THIS)) return;
	GB.ReturnNewZeroString((const char *)xmlTextReaderBaseUri(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_Depth)

	if (Check_Reader(THIS)) return;
	GB.ReturnInteger( xmlTextReaderDepth(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_IsDefault)

	if (Check_Reader(THIS)) return;
	GB.ReturnBoolean(xmlTextReaderIsDefault(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_IsEmptyElement)

	if (Check_Reader(THIS)) return;
	GB.ReturnBoolean(xmlTextReaderIsEmptyElement(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_LocalName)

	if (Check_Reader(THIS)) return;
	GB.ReturnNewZeroString((const char *)xmlTextReaderLocalName(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_Name)

	if (Check_Reader(THIS)) return;
	GB.ReturnNewZeroString((const char *)xmlTextReaderName(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_NamespaceUri)

	if (Check_Reader(THIS)) return;
	GB.ReturnNewZeroString((const char *)xmlTextReaderNamespaceUri(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_Prefix)

	if (Check_Reader(THIS)) return;
	GB.ReturnNewZeroString((const char *)xmlTextReaderPrefix(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_QuoteChar)

	char car='\"';

	if (Check_Reader(THIS)) return;

	car=(char)xmlTextReaderQuoteChar(THIS->reader);
	GB.ReturnNewString(&car,1);

END_PROPERTY


BEGIN_PROPERTY(CRNODE_Value)

	char *buf;

	if (Check_Reader(THIS)) return;

	buf=(char *)xmlTextReaderValue(THIS->reader);
	GB.ReturnNewZeroString(buf);
	if (buf) xmlFree(buf);

END_PROPERTY

BEGIN_PROPERTY(CRNODE_Type)

	if (Check_Reader(THIS)) return;
	GB.ReturnInteger( xmlTextReaderNodeType(THIS->reader));

END_PROPERTY

BEGIN_PROPERTY(CRNODE_XmlLang)

	if (Check_Reader(THIS)) return;
	GB.ReturnNewZeroString((char *)xmlTextReaderXmlLang(THIS->reader));

END_PROPERTY


GB_DESC CXmlReaderNodeTypeDesc[] =
{
	GB_DECLARE("XmlReaderNodeType", 0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("None", "i", XML_READER_TYPE_NONE),
	GB_CONSTANT("Element", "i", XML_READER_TYPE_ELEMENT),
	GB_CONSTANT("Attribute", "i",  XML_READER_TYPE_ATTRIBUTE),
	GB_CONSTANT("Text", "i",XML_READER_TYPE_TEXT),
	GB_CONSTANT("CDATA", "i", XML_READER_TYPE_CDATA),
	GB_CONSTANT("EntityReference", "i", XML_READER_TYPE_ENTITY_REFERENCE),
	GB_CONSTANT("Entity", "i",XML_READER_TYPE_ENTITY),
	GB_CONSTANT("ProcessingInstruction", "i",XML_READER_TYPE_PROCESSING_INSTRUCTION),
	GB_CONSTANT("Comment", "i", XML_READER_TYPE_COMMENT),
	GB_CONSTANT("Document", "i", XML_READER_TYPE_DOCUMENT),
	GB_CONSTANT("DocumentType", "i", XML_READER_TYPE_DOCUMENT_TYPE),
	GB_CONSTANT("DocumentFragment", "i", XML_READER_TYPE_DOCUMENT_FRAGMENT),
	GB_CONSTANT("Notation", "i", XML_READER_TYPE_NOTATION),
	GB_CONSTANT("Whitespace", "i",XML_READER_TYPE_WHITESPACE),
	GB_CONSTANT("SignificantWhitespace", "i",XML_READER_TYPE_SIGNIFICANT_WHITESPACE),
	GB_CONSTANT("EndElement", "i", XML_READER_TYPE_END_ELEMENT),
	GB_CONSTANT("EndEntity", "i", XML_READER_TYPE_END_ENTITY),
	GB_CONSTANT("XmlDeclaration", "i",XML_READER_TYPE_XML_DECLARATION),

	GB_END_DECLARE
};

GB_DESC CXmlReaderNodeAttributesDesc[] =
{
	GB_DECLARE(".XmlReader.Node.Attributes", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", ".XmlReader.Node", CXmlReader_next, NULL),
	GB_PROPERTY_READ("Count", "i", CXmlReader_count),

	GB_END_DECLARE
};

GB_DESC CXmlReaderNodeDesc[] =
{
	GB_DECLARE(".XmlReader.Node", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Attributes", ".XmlReader.Node.Attributes",CXMLReader_Node),
	GB_PROPERTY_READ("BaseUri","s",CRNODE_BaseUri),
	GB_PROPERTY_READ("Depth","i",CRNODE_Depth),
	GB_PROPERTY_READ("IsDefault","b",CRNODE_IsDefault),
	GB_PROPERTY_READ("IsEmptyElement","b",CRNODE_IsEmptyElement),
	GB_PROPERTY_READ("LocalName","s",CRNODE_LocalName),
	GB_PROPERTY_READ("Name", "s", CRNODE_Name),
	GB_PROPERTY_READ("NamespaceUri", "s", CRNODE_NamespaceUri),
	GB_PROPERTY_READ("Prefix", "s", CRNODE_Prefix),
	GB_PROPERTY_READ("QuoteChar", "s", CRNODE_QuoteChar),
	GB_PROPERTY_READ("Type","i",CRNODE_Type),
	GB_PROPERTY_READ("Value", "s", CRNODE_Value),
	GB_PROPERTY_READ("XmlLang", "s", CRNODE_XmlLang),

	GB_END_DECLARE
};

GB_DESC CXmlReaderDesc[] =
{
  GB_DECLARE("XmlReader", sizeof(CXMLREADER)),

  GB_STATIC_METHOD("Decode","s",CXmlReader_Decode,"(Data)s(Encoding)s"),

  GB_METHOD("_free",NULL,CXmlReader_Free,NULL),

  GB_METHOD("Open", NULL, CXmlReader_Open,"(FileName)s"),
  GB_METHOD("Close",NULL,CXmlReader_Free,NULL),
  GB_METHOD("FromString",NULL,CXmlReader_FromString,"(Buffer)s[(URL)s]"),

  GB_METHOD("Read",NULL,CXmlReader_Read,NULL),

  GB_PROPERTY_READ("Eof","b",CXMLReader_EOF),
  GB_PROPERTY_READ("Node",".XmlReader.Node",CXMLReader_Node),

  GB_END_DECLARE
};




