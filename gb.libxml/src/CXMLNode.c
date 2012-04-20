/***************************************************************************

  CXMLNode.c

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

#define __CXMLNODE_C

#include <stdio.h>
#include <libxml/tree.h>
#include "main.h"
#include "CXMLDocument.h"
#include "CXMLNode.h"

/*int CXMLNode_check(void *_object)
{
	return !THIS->parent;
}*/


BEGIN_PROPERTY(CXMLNode_Next)

	if (!THIS->node->next)
		GB.ReturnNull();
	else
		GB.ReturnObject(XML_CreateNode(THIS->doc, THIS->node->next));

END_PROPERTY

BEGIN_PROPERTY(CXMLNode_Prev)

	if (!THIS->node->prev)
		GB.ReturnNull();
	else
		GB.ReturnObject(XML_CreateNode(THIS->doc, THIS->node->prev));
	
END_PROPERTY

BEGIN_PROPERTY(CXMLNode_Parent)

	if (!THIS->node->parent)
		GB.ReturnNull();
	else
		GB.ReturnObject(XML_CreateNode(THIS->doc, THIS->node->parent));
	
END_PROPERTY


BEGIN_PROPERTY(CXMLNode_Name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString((const char *)THIS->node->name);
	else
		xmlNodeSetName(THIS->node, (const xmlChar *)GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CXMLNode_Value)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString((const char *)xmlNodeGetContent(THIS->node));
	else
		fprintf(stderr, "*NOT IMPLEMENTED*");

END_PROPERTY


BEGIN_PROPERTY(CXmlNode_c_count)

	int nval=0;
	xmlNode *ch;

	ch=THIS->node->children;

	while(ch)
	{
		nval++;
		ch=ch->next;
	}

	GB.ReturnInteger(nval);

END_METHOD

/***************************************************************
   "Creating" functions
   *************************************************************/
BEGIN_METHOD(CXMLNode_AddAttr,GB_STRING Name; GB_STRING Value)

  	char *name,*value;

	name=GB.ToZeroString(ARG(Name));
	value=GB.ToZeroString(ARG(Value));

	if (!xmlNewProp(THIS->node,BAD_CAST name,BAD_CAST value))
		GB.Error("Unable to add XML Attribute");

END_METHOD

BEGIN_METHOD(CXMLNode_AddElement,GB_STRING Name; GB_STRING Value)

	char *name,*value;

	name=GB.ToZeroString(ARG(Name));
	value=GB.ToZeroString(ARG(Value));


	if (!xmlNewChild(THIS->node, NULL, BAD_CAST name, BAD_CAST value))
		GB.Error("Unable to add XML Element");

END_METHOD

/**************************************************************
	NODE CHILDREN
***************************************************************/
BEGIN_METHOD(CXmlNode_c_get,GB_INTEGER Element)

	int nval=0;
	int nMax;
	xmlNode *ch;

	nMax = VARG(Element);

	ch = THIS->node->children;

	if (!ch)
	{
		GB.Error("Out of bounds");
		return;
	}

	for (nval=0;nval<nMax;nval++)
	{
		ch=ch->next;
		if (!ch) break;
	}

	if (!ch)
	{
		GB.Error("Out of bounds");
		return;
	}

	GB.ReturnObject(XML_CreateNode(THIS->doc, ch));

END_METHOD

/******************************************************************
		NODE ATTRIBUTES
*******************************************************************/

BEGIN_METHOD_VOID(CXmlNode_a_next)

	xmlNodePtr attr;
	int bucle;
	long *wenum;

	wenum=(long*)GB.GetEnum();

	attr=(xmlNodePtr)THIS->node->properties;

	if (!attr)
	{
		GB.StopEnum();
		return;
	}

	for(bucle=0;bucle<(*wenum);bucle++)
	{
		attr=attr->next;
		if (!attr)
		{
			GB.StopEnum();
			return;
		}
	}

	(*wenum)++;

	GB.ReturnObject(XML_CreateNode(THIS->doc, attr));

END_METHOD

BEGIN_PROPERTY(CXmlNode_a_count)

	int nval=0;
	xmlAttr *ch;

	ch=THIS->node->properties;

	while(ch)
	{
		nval++;
		ch=ch->next;
	}

	GB.ReturnInteger(nval);

END_METHOD


BEGIN_METHOD_VOID(CXMLNode_Free)

	GB.Unref(POINTER(&THIS->doc));

END_METHOD

GB_DESC CXmlNodeChildrenDesc[] =
{
	GB_DECLARE(".XmlNode.Children", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", "XmlNode",CXmlNode_c_get, "(Element)i"),
	GB_PROPERTY_READ("Count", "i", CXmlNode_c_count),

	GB_END_DECLARE
};


GB_DESC CXmlNodeAttributesDesc[] =
{
	GB_DECLARE(".XmlNode.Attributes", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "XmlNode", CXmlNode_a_next, NULL),
	GB_PROPERTY_READ("Count", "i", CXmlNode_a_count),

	GB_END_DECLARE
};



GB_DESC CXmlNodeDesc[] =
{
  GB_DECLARE("XmlNode", sizeof(CXMLNODE)), GB_NOT_CREATABLE(),

  GB_NOT_CREATABLE(),
  //GB_HOOK_CHECK(CXMLNode_check),

  GB_CONSTANT("ElementNode", "i", XML_ELEMENT_NODE),
  GB_CONSTANT("AttributeNode", "i", XML_ATTRIBUTE_NODE),
  GB_CONSTANT("TextNode", "i", XML_TEXT_NODE),
  GB_CONSTANT("CDataSectionNode", "i", XML_CDATA_SECTION_NODE),
  GB_CONSTANT("EntityRefNode", "i", XML_ENTITY_REF_NODE),
  GB_CONSTANT("EntityNode", "i", XML_ENTITY_NODE),
  GB_CONSTANT("PiNode", "i", XML_PI_NODE),
  GB_CONSTANT("CommentNode", "i", XML_COMMENT_NODE),
  GB_CONSTANT("DocumentNode", "i", XML_DOCUMENT_NODE),
  GB_CONSTANT("DocumentTypeNode", "i", XML_DOCUMENT_TYPE_NODE),
  GB_CONSTANT("DocumentFragNode", "i", XML_DOCUMENT_FRAG_NODE),
  GB_CONSTANT("NotationNode", "i", XML_NOTATION_NODE),
  GB_CONSTANT("HtmlDocumentNode", "i", XML_HTML_DOCUMENT_NODE),
  GB_CONSTANT("DtdNode", "i", XML_DTD_NODE),
  GB_CONSTANT("ElementDecl", "i", XML_ELEMENT_DECL),
  GB_CONSTANT("AttributeDecl", "i", XML_ATTRIBUTE_DECL),
  GB_CONSTANT("EntityDecl", "i", XML_ENTITY_DECL),
  GB_CONSTANT("NamespaceDecl", "i", XML_NAMESPACE_DECL),
  GB_CONSTANT("XIncludeStart", "i", XML_XINCLUDE_START),
  GB_CONSTANT("XIncludeEnd", "i", XML_XINCLUDE_END),
  GB_CONSTANT("DocbDocumentNode", "i", XML_DOCB_DOCUMENT_NODE),

  GB_PROPERTY("Name","s",CXMLNode_Name),
  GB_PROPERTY("Value","s",CXMLNode_Value),

  GB_PROPERTY_READ("Parent","XmlNode",CXMLNode_Parent),
  GB_PROPERTY_READ("Next","XmlNode",CXMLNode_Next),
  GB_PROPERTY_READ("Previous","XmlNode",CXMLNode_Prev),
  GB_PROPERTY_SELF("Children",".XmlNode.Children"),
  GB_PROPERTY_SELF("Attributes",".XmlNode.Attributes"),

  GB_METHOD("NewAttribute",NULL,CXMLNode_AddAttr,"(Name)s(Value)s"),
  GB_METHOD("NewElement",NULL,CXMLNode_AddElement,"(Name)s(Value)s"),

  GB_METHOD("_free",NULL,CXMLNode_Free,NULL),

  GB_END_DECLARE
};








