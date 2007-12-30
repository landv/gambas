/***************************************************************************

  CPdfDocument.cpp

  gb.pdf Component

  (C) 2005 Daniel Campos Fernández <danielcampos@netcourrier.com>


  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

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

#define __CPDFDOCUMENT_C

#include "CPdfDocument.h"

#include "gambas.h"
#include "main.h"

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <PDFDoc.h>
#include <Stream.h>
#include <ErrorCodes.h>
#include <Page.h>
#include <Catalog.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#include <goo/GooList.h>
#include <Outline.h>
#include <Link.h>

/*****************************************************************************

 PDF document

******************************************************************************/

BEGIN_METHOD_VOID(PDFDOCUMENT_new)

	THIS->scale = 1;
	THIS->rotation = 0;

END_METHOD


void free_all(void *_object)
{
	if (THIS->doc)
	{
		delete THIS->doc;
		THIS->doc=NULL;
	}

	if (THIS->dev)
	{
		delete THIS->dev;
		THIS->dev=NULL;
	}

	if (THIS->buf)
	{
		GB.ReleaseFile(&THIS->buf,THIS->len);
		THIS->buf=NULL;
	}

	THIS->index=NULL;
}

BEGIN_METHOD_VOID (PDFDOCUMENT_free)

	free_all(_object);

END_METHOD

BEGIN_PROPERTY(PDFDOCUMENT_scale)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->scale);
	else
		THIS->scale = VPROP(GB_FLOAT);

END_PROPERTY

BEGIN_PROPERTY(PDFDOCUMENT_rotation)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->rotation);
	else
		THIS->rotation = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_METHOD (PDFDOCUMENT_open, GB_STRING File;)

	SplashColor white;
	PDFDoc *test;
	MemStream *stream;
	Object obj;
	Outline *outline;
	GooList *items;
	char *buf=NULL;
	long len=0;
	int ret;


	//if (!LENGTH(File)) { GB.Error ("Invalid file name"); return; }

	if ( GB.LoadFile(STRING(File),LENGTH(File),&buf,&len) ) return;

	obj.initNull();
	stream=new MemStream(buf,0,(Guint)len,&obj);
	test=new PDFDoc (stream,0,0);

	if (!test->isOk())
	{
		GB.ReleaseFile(&buf,len);
		ret=test->getErrorCode();
		delete test;
		test=NULL;
		if (ret == errEncrypted)
			GB.Error("Document is encrypted");
		else
			GB.Error("Document seems to be not a PDF file");
	}

	if (test)
	{

		free_all(_object);

		THIS->doc=test;
		THIS->buf=buf;
		THIS->len=len;

#if POPPLER_VERSION_0_5
		white[0] = 0xFF; white[1] = 0xFF; white[2] = 0xFF;
		THIS->dev=new SplashOutputDev(splashModeRGB8, 3, gFalse, white);
#else
		white.rgb8 = splashMakeRGB8 (0xff, 0xff, 0xff);
		THIS->dev=new SplashOutputDev(splashModeRGB8, gFalse, white);
#endif
		
		THIS->dev->startDoc(THIS->doc->getXRef ());

		THIS->indexpos=0;
		outline=THIS->doc->getOutline();
		if (outline) THIS->index=outline->getItems();
	}

END_METHOD

BEGIN_METHOD_VOID(PDFDOCUMENT_close)

	free_all(_object);

END_METHOD

BEGIN_METHOD(PDFDOCUMENT_get,GB_INTEGER index;)

	if ( (VARG(index)<1) || ( VARG(index)>THIS->doc->getNumPages() ) )
	{
		GB.Error("Invalid page number");
		GB.ReturnNull();
		return;
	}

	THIS->page=THIS->doc->getCatalog()->getPage(VARG(index));
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(PDFDOCUMENT_count)

	GB.ReturnInteger( THIS->doc->getNumPages() );

END_PROPERTY

BEGIN_PROPERTY(PDFDOCUMENT_index)

	if (THIS->index) RETURN_SELF();
	else             GB.ReturnNull();

END_PROPERTY

BEGIN_PROPERTY(PDFDOCUMENT_info)


END_PROPERTY

/*****************************************************************************

PDF document information

******************************************************************************/


BEGIN_PROPERTY(PDFINFO_title)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_format)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_author)


END_PROPERTY

BEGIN_PROPERTY(PDFINFO_subject)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_keywords)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_creator)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_producer)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_linearized)


END_PROPERTY

BEGIN_PROPERTY(PDFINFO_layout)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_mode)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_permissions)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_creation)



END_PROPERTY

BEGIN_PROPERTY(PDFINFO_modification)



END_PROPERTY


/*****************************************************************************

PDF document index

******************************************************************************/

BEGIN_PROPERTY(PDFINDEX_title)

	OutlineItem *item;
	LinkAction *link_action;

	item = (OutlineItem *)THIS->index->get (THIS->indexpos);
	link_action = item->getAction ();


END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_type)



END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_page)



END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_left)



END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_top)



END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_right)


END_PROPERTY

BEGIN_PROPERTY (PDFINDEX_target)



END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_bottom)



END_PROPERTY

BEGIN_METHOD_VOID(PDFINDEX_root)

	Outline *outline;

	outline=THIS->doc->getOutline();
	if (outline) THIS->index=outline->getItems();
	THIS->indexpos=0;

END_METHOD

BEGIN_METHOD_VOID(PDFINDEX_next)



END_METHOD

BEGIN_METHOD_VOID(PDFINDEX_child)


END_METHOD

BEGIN_METHOD_VOID(PDFINDEX_parent)



END_METHOD

/*****************************************************************************

 PDF pages

******************************************************************************/

BEGIN_PROPERTY (PDFPAGE_width)

#if POPPLER_VERSION_0_5
	GB.ReturnInteger((long)THIS->page->getMediaWidth());
#else
	GB.ReturnInteger((long)THIS->page->getWidth());
#endif

END_PROPERTY

BEGIN_PROPERTY (PDFPAGE_height)

#if POPPLER_VERSION_0_5
	GB.ReturnInteger((long)THIS->page->getMediaHeight());
#else
	GB.ReturnInteger((long)THIS->page->getHeight());
#endif

END_PROPERTY

static uint32_t *get_page_data(CPDFDOCUMENT *_object, int x, int y, int *width, int *height, double scale, int rotation)
{
	SplashBitmap *map;
	uint32_t *data;
	uint32_t vl1,vl2,vl3;
	int i;
	int w, h;

	w = *width;
	h = *height;

#if POPPLER_VERSION_0_5
	if (w < 0)
		w = (long)THIS->page->getMediaWidth();

	if (h < 0)
		h = (long)THIS->page->getMediaHeight();

	if (x<0) x=0;
	if (y<0) y=0;
	if (w<1) w=1;
	if (h<1) h=1;
	if ( (x+w) > (long)THIS->page->getMediaWidth() ) w=(long)THIS->page->getMediaWidth()-x;
	if ( (y+h) > (long)THIS->page->getMediaHeight() ) h=(long)THIS->page->getMediaHeight()-y;
#else
	if (w < 0)
		w = (long)THIS->page->getWidth();

	if (h < 0)
		h = (long)THIS->page->getHeight();

	if (x<0) x=0;
	if (y<0) y=0;
	if (w<1) w=1;
	if (h<1) h=1;
	if ( (x+w) > (long)THIS->page->getWidth() ) w=(long)THIS->page->getWidth()-x;
	if ( (y+h) > (long)THIS->page->getHeight() ) h=(long)THIS->page->getHeight()-y;
#endif

	w = (long)(w * scale);
	h = (long)(h * scale);

	if (rotation)
	{
		double a = rotation * M_PI / 180;
		double ww, hh;

		ww = w * fabs(cos(a)) + h * fabs(sin(a));
		hh = w * fabs(sin(a)) + h * fabs(cos(a));

		w = (int)(ww + 0.5);
		h = (int)(hh + 0.5);
	}

	THIS->page->displaySlice(THIS->dev,72.0*scale,72.0*scale,
			   rotation,
#if POPPLER_VERSION_0_5
				 gTrue,
#endif
			   gTrue,
			   x,y,w,h,
			   NULL,
			   THIS->doc->getCatalog ());

	map=THIS->dev->getBitmap();
	
#if POPPLER_VERSION_0_5
	data=(uint32_t*)map->getDataPtr();
#else
	data=(uint32_t*)map->getDataPtr().rgb8;
#endif

	*width = w;
	*height = h;

	return data;
}

BEGIN_METHOD(PDFPAGE_image, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	GB_IMAGE img = NULL;
	uint32_t *data;
	int x,y, w, h;

	x = VARGOPT(x, 0);
	y = VARGOPT(y, 0);
	w = VARGOPT(w, -1);
	h = VARGOPT(h, -1);

	data = get_page_data(THIS, x, y, &w, &h, THIS->scale, THIS->rotation);

#if POPPLER_VERSION_0_5
	GB.Image.Create(&img, data, w, h, GB_IMAGE_RGB);
#else
	GB.Image.Create(&img, data, w, h, GB_IMAGE_BGRX);
#endif
	GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD(PDFPAGE_picture, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	GB_IMAGE img = NULL;
	uint32_t *data;
	int x,y, w, h;

	x = VARGOPT(x, 0);
	y = VARGOPT(y, 0);
	w = VARGOPT(w, -1);
	h = VARGOPT(h, -1);

	data = get_page_data(THIS, x, y, &w, &h, THIS->scale, THIS->rotation);

	GB.Picture.Create(&img, data, w, h, GB_IMAGE_BGRX);
	GB.ReturnObject(img);

END_METHOD



BEGIN_METHOD(PDFPAGE_select,GB_INTEGER x;GB_INTEGER y;GB_INTEGER w;GB_INTEGER h;)



END_METHOD


/*****************************************************************************

 Bookmarks of a PDF page

******************************************************************************/
BEGIN_PROPERTY (PDFPAGELINKS_count)


END_PROPERTY

BEGIN_METHOD (PDFPAGELINKS_get,GB_INTEGER ind;)



END_METHOD

BEGIN_PROPERTY (PDFPAGELINKDATA_uri)



END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_left)



END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_top)



END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_right)


END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_bottom)


END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_page)


END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINKDATA_title)



END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINK_type)


END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINK_width)


END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINK_height)



END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINK_left)



END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINK_top)



END_PROPERTY

/*****************************************************************************

 Finding a text in a PDF page

******************************************************************************/
BEGIN_METHOD (PDFPAGE_find,GB_STRING Text;)



END_METHOD


BEGIN_METHOD (PDFPAGERESULT_get,GB_INTEGER Index;)



END_METHOD

BEGIN_PROPERTY (PDFPAGERESULT_count)



END_PROPERTY

BEGIN_PROPERTY (PDFPAGERESULT_width)



END_PROPERTY

BEGIN_PROPERTY (PDFPAGERESULT_height)



END_PROPERTY

BEGIN_PROPERTY (PDFPAGERESULT_left)



END_PROPERTY

BEGIN_PROPERTY (PDFPAGERESULT_top)


END_PROPERTY

/**********************************************************************

Gambas Interface

***********************************************************************/


GB_DESC PdfResultItemDesc[]=
{
	GB_DECLARE(".PdfResultItem",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Left","i",PDFPAGERESULT_left),
	GB_PROPERTY_READ("Top","i",PDFPAGERESULT_top),
	GB_PROPERTY_READ("Width","i",PDFPAGERESULT_width),
	GB_PROPERTY_READ("Height","i",PDFPAGERESULT_height),

	GB_END_DECLARE
};

GB_DESC PdfResultDesc[]=
{
	GB_DECLARE(".PdfResult",0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get",".PdfResultItem",PDFPAGERESULT_get,"(Index)i"),
	GB_PROPERTY_READ("Count","i",PDFPAGERESULT_count),

	GB_END_DECLARE
};


GB_DESC PdfLinkDataDesc[]=
{
	GB_DECLARE(".PdfLinkData",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Title","s",PDFPAGELINKDATA_title),
	GB_PROPERTY_READ("Target","s",PDFPAGELINKDATA_uri),
	GB_PROPERTY_READ("Page","i",PDFPAGELINKDATA_page),
	GB_PROPERTY_READ("Left","i",PDFPAGELINKDATA_left),
	GB_PROPERTY_READ("Top","i",PDFPAGELINKDATA_top),
	GB_PROPERTY_READ("Right","i",PDFPAGELINKDATA_right),
	GB_PROPERTY_READ("Bottom","i",PDFPAGELINKDATA_bottom),

	GB_END_DECLARE
};

GB_DESC PdfLinkDesc[]=
{
	GB_DECLARE(".PdfLink",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Type","i",PDFPAGELINK_type),
	GB_PROPERTY_READ("Left","i",PDFPAGELINK_left),
	GB_PROPERTY_READ("Top","i",PDFPAGELINK_top),
	GB_PROPERTY_READ("Width","i",PDFPAGELINK_width),
	GB_PROPERTY_READ("Height","i",PDFPAGELINK_height),
	GB_PROPERTY_SELF("Data",".PdfLinkData"),

	GB_END_DECLARE
};

GB_DESC PdfIndexDesc[]=
{
	GB_DECLARE(".PdfIndex",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Title","s",PDFINDEX_title),
	GB_PROPERTY_READ("Page","i",PDFINDEX_page),
	GB_PROPERTY_READ("Type","i",PDFINDEX_type),
	GB_PROPERTY_READ("Left","i",PDFINDEX_left),
	GB_PROPERTY_READ("Top","i",PDFINDEX_top),
	GB_PROPERTY_READ("Right","i",PDFINDEX_right),
	GB_PROPERTY_READ("Bottom","i",PDFINDEX_bottom),
	GB_PROPERTY_READ("Target","s",PDFINDEX_target),

	GB_METHOD("MoveNext","b",PDFINDEX_next,NULL),
	GB_METHOD("MoveChild","b",PDFINDEX_child,NULL),
	GB_METHOD("MoveParent","b",PDFINDEX_parent,NULL),
	GB_METHOD("MoveRoot",NULL,PDFINDEX_root,NULL),

	GB_END_DECLARE
};


GB_DESC PdfPageDesc[]=
{
	GB_DECLARE(".PdfPage",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Width","f",PDFPAGE_width),
	GB_PROPERTY_READ("Height","f",PDFPAGE_height),
	GB_PROPERTY_SELF("Result",".PdfResult"),

	GB_METHOD("GetImage","Image",PDFPAGE_image,"[(X)i(Y)i(Width)i(Height)i]"),
	GB_METHOD("GetPicture","Picture",PDFPAGE_picture,"[(X)i(Y)i(Width)i(Height)i]"),
	GB_METHOD("Find","b",PDFPAGE_find,"(Text)s"),
	GB_METHOD("Select","s",PDFPAGE_select,"(X)i(Y)i[(W)i(H)i]"),

	GB_METHOD("_get",".PdfLink",PDFPAGELINKS_get,"(Index)i"),
	GB_PROPERTY_READ("Count","i",PDFPAGELINKS_count),

	GB_END_DECLARE
};

GB_DESC PdfDocumentInfo[] =
{
	GB_DECLARE(".PdfInfo",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Title","s",PDFINFO_title),
	GB_PROPERTY_READ("Format","s",PDFINFO_format),
	GB_PROPERTY_READ("Author","s",PDFINFO_author),
	GB_PROPERTY_READ("Subject","s",PDFINFO_subject),
	GB_PROPERTY_READ("Keywords","s",PDFINFO_keywords),
	GB_PROPERTY_READ("Creator","s",PDFINFO_creator),
	GB_PROPERTY_READ("Producer","s",PDFINFO_producer),
	GB_PROPERTY_READ("Creation","d",PDFINFO_creation),
	GB_PROPERTY_READ("Modification","s",PDFINFO_modification),
	GB_PROPERTY_READ("Linearized","b",PDFINFO_linearized),
	GB_PROPERTY_READ("Layout","i",PDFINFO_layout),
	GB_PROPERTY_READ("Mode","i",PDFINFO_mode),
	GB_PROPERTY_READ("Permissions","i",PDFINFO_permissions),

	GB_END_DECLARE
};

GB_DESC PdfLayoutDesc[] =
{

  GB_DECLARE("PdfLayout", 0), GB_NOT_CREATABLE(),

  GB_CONSTANT("Unset","i",0),
  GB_CONSTANT("SinglePage","i",0),
  GB_CONSTANT("OneColumn","i",0),
  GB_CONSTANT("TwoColumnLeft","i",0),
  GB_CONSTANT("TwoColumnRight","i",0),
  GB_CONSTANT("TwoPageLeft","i",0),
  GB_CONSTANT("TwoPageRight","i",0),

  GB_END_DECLARE
};

GB_DESC PdfPermissionsDesc[] =
{

  GB_DECLARE("PdfPermissions", 0), GB_NOT_CREATABLE(),

  GB_CONSTANT("Print","i",0),
  GB_CONSTANT("Modify","i",0),
  GB_CONSTANT("Copy","i",0),
  GB_CONSTANT("AddNotes","i",0),
  GB_CONSTANT("All","i",0),

  GB_END_DECLARE
};

GB_DESC PdfModeDesc[] =
{
	GB_DECLARE("PdfPageMode",0), GB_NOT_CREATABLE(),

	GB_CONSTANT("Unset","i",0),
	GB_CONSTANT("None","i",0),
	GB_CONSTANT("UseOutlines","i",0),
	GB_CONSTANT("UseThumbs","i",0),
	GB_CONSTANT("FullScreen","i",0),
	GB_CONSTANT("UseOC","i",0),
	GB_CONSTANT("UseAttachments","i",0),

	GB_END_DECLARE
};

GB_DESC PdfDocumentDesc[] =
{

  GB_DECLARE("PdfDocument", sizeof(CPDFDOCUMENT)),


  GB_CONSTANT("Unknown","i",0),        /* unknown action */
  GB_CONSTANT("Goto","i",0),         /* go to destination */
  GB_CONSTANT("GotoRemote","i",0), /* go to destination in new file */
  GB_CONSTANT("Launch","i",0),          /* launch app or open doc. */
  GB_CONSTANT("Uri","i",0),                /* URI */
  GB_CONSTANT("Named","i",0),            /* named action*/
  GB_CONSTANT("Movie","i",0),            /* movie action */

  GB_METHOD("_new", NULL, PDFDOCUMENT_new, NULL),
  GB_METHOD("_free", NULL, PDFDOCUMENT_free, NULL),

  GB_METHOD("Open",NULL,PDFDOCUMENT_open,"(File)s"),
  GB_METHOD("Close",NULL,PDFDOCUMENT_close,NULL),
  GB_METHOD("_get",".PdfPage",PDFDOCUMENT_get,"(Index)i"),

	GB_PROPERTY("Scale", "f", PDFDOCUMENT_scale),
	GB_PROPERTY("Rotation", "i", PDFDOCUMENT_rotation),

  GB_PROPERTY_READ("Count","i",PDFDOCUMENT_count),
  GB_PROPERTY_READ("Index",".PdfIndex",PDFDOCUMENT_index),
  GB_PROPERTY_READ("Info",".PdfInfo",PDFDOCUMENT_info),

  GB_END_DECLARE
};


