/***************************************************************************

  CXSLT.c

  XSLT component

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



#define __CXSLT_C

#include <stdio.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include "main.h"
#include "CXSLT.h"
#include "../CXMLDocument.h"
#include "../CXMLNode.h"


void Doc_AddChild(void *_object,CXMLNODE *chd)
{
	CXMLDOCUMENT* mythis=(CXMLDOCUMENT*)_object;

	mythis->nchildren++;
	
	if (!mythis->children)
		GB.Alloc((void**)&mythis->children,sizeof(CXMLNODE*));
	else
		GB.Realloc((void**)&mythis->children,mythis->nchildren*sizeof(CXMLNODE*));	

	chd->parent=_object;
	mythis->children[mythis->nchildren-1]=chd;
}


BEGIN_METHOD(CXSLT_Transform,GB_OBJECT Document;GB_OBJECT StyleSheet;)

	CXMLDOCUMENT *doc;
	CXMLDOCUMENT *st;
	CXMLDOCUMENT *out=NULL;
	xsltStylesheetPtr sheet=NULL;
	
	doc=VARG(Document);
	st=VARG(StyleSheet);
	
	if (GB.CheckObject ((void*)doc)) return;
	if (GB.CheckObject ((void*)st)) return;
	
	if (!doc->doc)
	{
		GB.Error("Void document");
		return;
	}
	
	if (!st->doc)
	{
		GB.Error("Void Style Sheet");
		return;
	}
	
	if(!(sheet=xsltParseStylesheetDoc (st->doc)))
	{
		GB.Error("Invalid style sheet");
		return;
	}
	
	GB.New((void**)&out,GB.FindClass("XmlDocument"),NULL,NULL);
	out->doc=xsltApplyStylesheet(sheet, doc->doc, NULL);
	GB.New((void**)&out->node,GB.FindClass("XmlNode"),NULL,NULL);
	
	if (!out->doc)
	{
		GB.Unref((void**)&out);
		GB.Error("Unable to apply Style Sheet");
		return;
	}
	
	out->node->node=xmlDocGetRootElement(out->doc);
	Doc_AddChild(out,out->node);
	GB.Ref((void*)out->node);
	GB.ReturnObject((void*)out);
	//xsltFreeStylesheet(sheet);


END_METHOD



GB_DESC CXsltDesc[] =
{
  GB_DECLARE("Xslt", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD ("Transform","XmlDocument",CXSLT_Transform,"(Document)XmlDocument;(StyleSheet)XmlDocument;"),

  GB_END_DECLARE
};




