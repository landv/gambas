/***************************************************************************

  CPdfDocument.cpp

  (C) 2005-2007 Daniel Campos Fernández <dcamposf@gmail.com>

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
#include <TextOutputDev.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#include <goo/GooList.h>
#include <Outline.h>
#include <Link.h>
#include <Gfx.h>
#include <GlobalParams.h>
#include <UnicodeMap.h>


/***************************************************************************/

static CPDFRECT *create_rect(void)
{
	return (CPDFRECT *)GB.New(GB.FindClass("PdfRect"), NULL, NULL);
}

BEGIN_PROPERTY(PdfRect_X)

	GB.ReturnFloat(THIS_RECT->x);

END_PROPERTY

BEGIN_PROPERTY(PdfRect_Y)

	GB.ReturnFloat(THIS_RECT->y);

END_PROPERTY

BEGIN_PROPERTY(PdfRect_Width)

	GB.ReturnFloat(THIS_RECT->w);

END_PROPERTY

BEGIN_PROPERTY(PdfRect_Height)

	GB.ReturnFloat(THIS_RECT->h);

END_PROPERTY

BEGIN_PROPERTY(PdfRect_Right)

	GB.ReturnFloat(THIS_RECT->x + THIS_RECT->w);

END_PROPERTY

BEGIN_PROPERTY(PdfRect_Bottom)

	GB.ReturnFloat(THIS_RECT->y + THIS_RECT->h);

END_PROPERTY



/****************************************************************************

 Translations from Poppler universe to Gambas universe

****************************************************************************/

static void return_unicode_string(Unicode *unicode, int len)
{
	static UnicodeMap *uMap = NULL;
	
	GooString gstr;
	char buf[8]; /* 8 is enough for mapping an unicode char to a string */
	int i, n;

	if (uMap == NULL) 
	{
		GooString *enc = new GooString("UTF-8");
		uMap = globalParams->getUnicodeMap(enc);
		uMap->incRefCnt();
		delete enc;
	}
		
	for (i = 0; i < len; ++i) {
		n = uMap->mapUnicode(unicode[i], buf, sizeof(buf));
		gstr.append(buf, n);
	}

	GB.ReturnNewZeroString(gstr.getCString());
}


static void aux_return_string_info(void *_object, const char *key)
{
	Object obj;
	Object dst;
	GooString *goo_value;
	Dict *info_dict;
	char *tmpstr;

	THIS->doc->getDocInfo (&obj);
	if (!obj.isDict()) { GB.ReturnNewZeroString(""); return; }
		
	info_dict=obj.getDict();
	info_dict->lookup ((char *)key, &dst);
	if (!dst.isString ()) { GB.ReturnNewZeroString(""); }
	else {
		goo_value = dst.getString();

		if (goo_value->hasUnicodeMarker())
		{
			GB.ConvString (&tmpstr,goo_value->getCString()+2,goo_value->getLength()-2,"UTF-16BE","UTF-8");
			GB.ReturnNewZeroString(tmpstr);		
		}		
		else
			GB.ReturnNewString(goo_value->getCString(),goo_value->getLength());		
	}
	dst.free();
	obj.free();		
}

static void aux_return_date_info(void *_object, const char *key)
{
	// TODO: Y2K effect
	GB_DATE_SERIAL ds;
	GB_DATE ret;
	Object obj;
	Object dst;
	GooString *goo;
	Dict *info_dict;
	char *datestr=NULL,*tofree=NULL;
	int nnum;

	GB.ReturnDate(NULL);
	
	THIS->doc->getDocInfo (&obj);
	if (!obj.isDict()) return;

	info_dict=obj.getDict();
	info_dict->lookup ((char *)key, &dst);
	if (dst.isString ())
	{
		goo = dst.getString();
		if (goo->hasUnicodeMarker())
			GB.ConvString (&datestr,goo->getCString()+2,goo->getLength()-2,"UTF-16BE","UTF-8");
		else
		{
			datestr = GB.NewString(goo->getCString(),goo->getLength());
			tofree=datestr;		
		}

		if (datestr)
		{
			if (datestr[0] == 'D' && datestr[1] == ':') datestr += 2;
			nnum=sscanf(datestr, "%4d%2d%2d%2d%2d%2d",&ds.year, &ds.month, &ds.day, &ds.hour, &ds.min, &ds.sec);
			if (nnum == 6)
			{
				if (!GB.MakeDate(&ds,&ret))
					GB.ReturnDate(&ret);
			}		
		}
		
	}

	if (tofree) GB.FreeString(&tofree);
	dst.free();
	obj.free();
}

static LinkDest *get_dest(LinkAction *act)
{
	if (!act)
		return 0;
	
	switch (act->getKind())
	{
		case actionGoTo: return ((LinkGoTo*)act)->getDest();
		case actionGoToR: return ((LinkGoToR*)act)->getDest();
		default: return 0;
	}
}

static uint32_t aux_get_page_from_action(void *_object, LinkAction *act)
{
	Ref pref;       
	LinkDest *dest = get_dest(act);
	#if POPPLER_VERSION_0_6
	GooString *name;
	#else
	UGooString *name;
	#endif

	if (!dest)
	{
		// try to use NamedDest to get dest
		if (!act)
			return 0;
		if (act->getKind () == actionGoTo)
		{
			name = ((LinkGoTo*)act)->getNamedDest();
			if (name)
				dest = THIS->doc->findDest(name);
		}
	}

	if (!dest)
		return 0;

	if (dest->isPageRef() )
	{
		pref= dest->getPageRef();
		return THIS->doc->findPage(pref.num, pref.gen);
	}
	else
		return dest->getPageNum();
}


static void aux_get_dimensions_from_action(LinkAction *act, CPDFRECT *rect)
{
	LinkDest *dest = get_dest(act);
	if (!dest)
		return;
	
	rect->x = dest->getLeft();
	rect->w = dest->getRight() - rect->x;
	rect->y = dest->getTop();
	rect->h = dest->getBottom() - rect->y;
}

static double aux_get_zoom_from_action(LinkAction *act)
{
	LinkDest *dest = get_dest(act);
	if (dest)
		return dest->getZoom();
	else
		return 1;
}

static char* aux_get_target_from_action(LinkAction *act)
{
	char *vl=NULL;
	char *uni=NULL;	
	GooString *tmp=NULL;

	switch (act->getKind())
	{
		case actionGoToR:
			tmp=((LinkGoToR*)act)->getFileName(); break;

		case actionLaunch:
			tmp=((LinkLaunch*)act)->getFileName(); break;

		case actionURI:
			tmp=((LinkURI*)act)->getURI(); break;
			
		case actionNamed:
			tmp=((LinkNamed*)act)->getName(); break;

		case actionMovie:
			#if POPPLER_VERSION_0_8
			tmp=((LinkMovie*)act)->getAnnotTitle(); break;
			#else
			tmp=((LinkMovie*)act)->getTitle(); break;
			#endif

		default:
			break;
	}

	if (!tmp) return NULL;

	if (tmp->hasUnicodeMarker())
	{
			GB.ConvString (&uni,tmp->getCString()+2,tmp->getLength()-2,"UTF-16BE","UTF-8");
			vl = GB.AddString(vl, uni, 0);	
	}	
	else
			vl = GB.AddString(vl,tmp->getCString(),tmp->getLength());
	

	return vl;

}

/*****************************************************************************

 PDF document

******************************************************************************/


static void free_all(void *_object)
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
		GB.ReleaseFile(THIS->buf,THIS->len);
		THIS->buf=NULL;
	}

	if (THIS->Found)
	{		
		GB.FreeArray(POINTER(&THIS->Found));
		THIS->Found=NULL;
	}

	if (THIS->links)
	{
		delete THIS->links;	
		THIS->links=NULL;
	}

	if (THIS->pindex)
	{		
		GB.FreeArray(POINTER(&THIS->pindex));
		GB.FreeArray(POINTER(&THIS->oldindex));
		THIS->pindex=NULL;
		THIS->oldindex=NULL;
	}

	THIS->index=NULL;
	THIS->currpage=-1;
}

BEGIN_METHOD_VOID (PDFDOCUMENT_free)

	free_all(_object);

END_METHOD

BEGIN_PROPERTY(PDFDOCUMENT_scale)

	if (READ_PROPERTY){ GB.ReturnFloat(THIS->scale); return; }
	
	if (VPROP(GB_FLOAT)>0) { THIS->scale = VPROP(GB_FLOAT); return; }

	GB.Error("Zoom must be a positive value");

END_PROPERTY

BEGIN_PROPERTY(PDFDOCUMENT_rotation)

	int32_t rot;

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->rotation);
		return;
	}
	
	rot=VPROP(GB_INTEGER);

	while (rot<0) rot+=360;
	while (rot>=360) rot-=360;

	switch (rot)
	{
		case 0:
		case 90:
		case 180:
		case 270: 
			THIS->rotation = VPROP(GB_INTEGER);
			break;
	}

END_PROPERTY


int32_t open_document (void *_object, char *sfile, int32_t lfile)
{
	SplashColor white;
	PDFDoc *test;
	MemStream *stream;
	Object obj;
	Outline *outline;
	char *buf=NULL;
	int32_t len=0;
	int32_t ret;


	if ( GB.LoadFile(sfile,lfile,&buf,&len) ) return -1;

	obj.initNull();
	stream=new MemStream(buf,0,(Guint)len,&obj);
	test=new PDFDoc (stream,0,0);

	if (!test->isOk())
	{
		GB.ReleaseFile(buf,len);
		ret=test->getErrorCode();
		delete test;
		test=NULL;
		if (ret == errEncrypted) return -2;
		return -3;
	}

	free_all(_object);

	THIS->doc=test;
	THIS->buf=buf;
	THIS->len=len;

	white[0] = 0xFF; white[1] = 0xFF; white[2] = 0xFF;
	THIS->dev=new SplashOutputDev(splashModeRGB8, 3, gFalse, white);

	#if POPPLER_VERSION_0_20
	THIS->dev->startDoc(THIS->doc);
	#else
	THIS->dev->startDoc(THIS->doc->getXRef ());
	#endif
	
	outline=THIS->doc->getOutline();
	if (outline) THIS->index=outline->getItems();
	
	//if (THIS->index)
	//	if (!THIS->index->getLength()) THIS->index=NULL;

	THIS->currindex=0;
	THIS->currpage=-1;

	return 0;

}


BEGIN_METHOD(PDFDOCUMENT_new, GB_STRING File)

	THIS->scale = 1;
	THIS->rotation = 0;

	if (!MISSING(File))
	{
		switch (open_document( _object, STRING(File), LENGTH(File)) )
		{
			case -1: GB.Error("File not found"); return;
			case -2: GB.Error("PDF is encrypted"); return;
			case -3: GB.Error("Bad PDF File"); return;
		}
	}

END_METHOD

BEGIN_METHOD (PDFDOCUMENT_open, GB_STRING File;)

	switch (open_document( _object, STRING(File), LENGTH(File)) )
	{
		case -1: GB.Error("File not found"); return;
		case -2: GB.Error("PDF is encrypted"); return;
		case -3: GB.Error("Bad PDF File"); return;
	}

END_METHOD

BEGIN_METHOD_VOID(PDFDOCUMENT_close)

	free_all(_object);

END_METHOD

BEGIN_METHOD(PDFDOCUMENT_get,GB_INTEGER index;)

	if (!THIS->doc || (VARG(index)<1) || ( VARG(index)>THIS->doc->getNumPages() ) )
	{
		GB.Error("Invalid page number");
		return;
	}

	if (THIS->currpage != (uint32_t)VARG(index) )
	{
		if (THIS->Found)
		{		
			GB.FreeArray(POINTER(&THIS->Found));
			THIS->Found=NULL;
		}

		if (THIS->links)
		{
			delete THIS->links;	
			THIS->links=NULL;
		}

		THIS->page=THIS->doc->getCatalog()->getPage(VARG(index));
		THIS->currpage=VARG(index);
	}
		
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(PDFDOCUMENT_ready)

	GB.ReturnBoolean( (bool)THIS->doc );

END_PROPERTY

BEGIN_PROPERTY(PDFDOCUMENT_count)

	GB.ReturnInteger( (int32_t) (THIS->doc ? THIS->doc->getNumPages() : 0));

END_PROPERTY

BEGIN_PROPERTY(PDFDOCUMENT_info)

	if (THIS->doc) RETURN_SELF();
	else GB.ReturnNull();

END_PROPERTY

/*****************************************************************************

PDF document information

******************************************************************************/

BEGIN_PROPERTY(PDFINFO_title)

	aux_return_string_info(_object,"Title");

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_format)

	char ctx[16];
	#if POPPLER_VERSION_0_11_3
		snprintf(ctx, sizeof(ctx), "%.2g", THIS->doc->getPDFMajorVersion () + THIS->doc->getPDFMinorVersion() / 10.0);
	#else
		snprintf(ctx, sizeof(ctx), "%.2g", THIS->doc->getPDFVersion());
	#endif
	GB.ReturnNewZeroString(ctx);

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_author)

	aux_return_string_info(_object,"Author");

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_subject)

	aux_return_string_info(_object,"Subject");

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_keywords)

	aux_return_string_info(_object,"Keywords");

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_creator)

	aux_return_string_info(_object,"Creator");

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_producer)

	aux_return_string_info(_object,"Producer");

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_linearized)

	GB.ReturnBoolean(THIS->doc->isLinearized());

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_layout)

	Catalog *catalog;

	catalog=THIS->doc->getCatalog();
	if (!catalog) { GB.ReturnInteger(Catalog::pageLayoutNone); return; }
	if (!catalog->isOk())  { GB.ReturnInteger(Catalog::pageLayoutNone); return; }

	GB.ReturnInteger(catalog->getPageLayout());

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_mode)

	Catalog *catalog;

	catalog=THIS->doc->getCatalog();
	if (!catalog) { GB.ReturnInteger(Catalog::pageModeNone); return; }
	if (!catalog->isOk())  { GB.ReturnInteger(Catalog::pageModeNone); return; }

	GB.ReturnInteger(catalog->getPageMode());


END_PROPERTY

BEGIN_PROPERTY(PDFINFO_canprint)
	
	GB.ReturnBoolean(THIS->doc->okToPrint());      

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_canmodify)

	GB.ReturnBoolean(THIS->doc->okToChange());

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_cancopy)

	GB.ReturnBoolean(THIS->doc->okToCopy());

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_canaddnotes)

	GB.ReturnBoolean(THIS->doc->okToAddNotes());

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_creation)

	aux_return_date_info(_object,"CreationDate");

END_PROPERTY

BEGIN_PROPERTY(PDFINFO_modification)

	aux_return_date_info(_object,"ModDate");

END_PROPERTY


/*****************************************************************************

PDF document index

******************************************************************************/


BEGIN_PROPERTY(PDFDOCUMENT_has_index)

	GB.ReturnBoolean(THIS->index && THIS->index->getLength());

END_PROPERTY

BEGIN_PROPERTY(PDFDOCUMENT_index)

	if (!THIS->index) { GB.ReturnNull(); return; }
	
	THIS->action=((OutlineItem*)THIS->index->get(THIS->currindex))->getAction();
	RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_count)

	GB.ReturnInteger(THIS->index->getLength());

END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_has_children)

	OutlineItem *item;

	item = (OutlineItem *)THIS->index->get (THIS->currindex);
	GB.ReturnBoolean(item->getKids() && item->getKids()->getLength());

END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_is_open)

	OutlineItem *item;

	item = (OutlineItem *)THIS->index->get (THIS->currindex);

	if (READ_PROPERTY)
	{	GB.ReturnBoolean(item->isOpen()); return; }

	if (VPROP(GB_INTEGER)) item->open();
	else item->close();

END_PROPERTY

BEGIN_PROPERTY(PDFINDEX_title)

	OutlineItem *item;

	item = (OutlineItem *)THIS->index->get (THIS->currindex);
	return_unicode_string(item->getTitle(), item->getTitleLength());

END_PROPERTY


BEGIN_METHOD_VOID(PDFINDEX_root)

	Outline *outline;

	outline=THIS->doc->getOutline();
	if (outline) THIS->index=outline->getItems();
	THIS->currindex=0;
	if (THIS->pindex) { GB.FreeArray(POINTER(&THIS->pindex)); THIS->pindex=NULL; }
	if (THIS->oldindex) { GB.FreeArray(POINTER(&THIS->oldindex)); THIS->oldindex=NULL; }

END_METHOD

BEGIN_METHOD_VOID(PDFINDEX_prev)

	if (!THIS->currindex) { GB.ReturnBoolean(true); return; }

	THIS->currindex--;
	GB.ReturnBoolean(false);

END_METHOD

BEGIN_METHOD_VOID(PDFINDEX_next)

	if ( (THIS->currindex+1) >= (uint32_t)THIS->index->getLength() )
		 { GB.ReturnBoolean(true); return; }

	THIS->currindex++;
	GB.ReturnBoolean(false);

END_METHOD

BEGIN_METHOD_VOID(PDFINDEX_child)

	OutlineItem *item;

	item = (OutlineItem *)THIS->index->get (THIS->currindex);

	if (!item->hasKids() || item->getKids()->getLength() == 0) { GB.ReturnBoolean(true); return; }

	if (THIS->pindex)
	{
		GB.Add(POINTER(&THIS->pindex));
		GB.Add(POINTER(&THIS->oldindex));
	}
	else
	{
		GB.NewArray(POINTER(&THIS->pindex),sizeof(void*),1);
		GB.NewArray(POINTER(&THIS->oldindex),sizeof(uint32_t),1);
	}	

	if (!item->isOpen()) item->open(); 
	THIS->pindex[GB.Count(POINTER(THIS->pindex))-1]=(void*)THIS->index;
	THIS->oldindex[GB.Count(POINTER(THIS->pindex))-1]=THIS->currindex;
	THIS->index=item->getKids();	
	THIS->currindex=0;

	GB.ReturnBoolean(false);

END_METHOD

BEGIN_METHOD_VOID(PDFINDEX_parent)

	if (!THIS->pindex) { GB.ReturnBoolean(true); return; }

	THIS->index=(GooList*)THIS->pindex[GB.Count(POINTER(THIS->pindex))-1];
	THIS->currindex=THIS->oldindex[GB.Count(POINTER(THIS->pindex))-1];
	if (GB.Count(POINTER(THIS->pindex))==1)
	{
		GB.FreeArray(POINTER(&THIS->pindex));
		GB.FreeArray(POINTER(&THIS->oldindex));
		THIS->oldindex=NULL;
		THIS->pindex=NULL;
	}
	else
	{
		GB.Remove(POINTER(&THIS->pindex),GB.Count(POINTER(THIS->pindex))-1,1);
		GB.Remove(POINTER(&THIS->oldindex),GB.Count(POINTER(THIS->oldindex))-1,1);
	}

	GB.ReturnBoolean(false);

END_METHOD

/*****************************************************************************

 PDF pages

******************************************************************************/

static int get_rotation(void *_object)
{
	return (THIS->rotation + THIS->page->getRotate() + 720) % 360;
}

static void get_page_size(void *_object, int *w, int *h)
{
	int rotation = get_rotation(THIS);

	if (rotation == 90 || rotation == 270)
	{
		if (w) *w =  (int)(THIS->page->getMediaHeight() * THIS->scale);
		if (h) *h = (int)(THIS->page->getMediaWidth() * THIS->scale);
	}
	else
	{
		if (w) *w = (int)(THIS->page->getMediaWidth() * THIS->scale);
		if (h) *h =  (int)(THIS->page->getMediaHeight() * THIS->scale);
	}
}

BEGIN_PROPERTY (PDFPAGE_width)

	int w;
	get_page_size(THIS, &w, NULL);
	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY (PDFPAGE_height)

	int h;
	get_page_size(THIS, NULL, &h);
	GB.ReturnInteger(h);

END_PROPERTY

static uint32_t *get_page_data(CPDFDOCUMENT *_object, int32_t x, int32_t y, int32_t *width, int32_t *height, double scale, int32_t rotation)
{
	SplashBitmap *map;
	uint32_t *data;
	int32_t w, h;
	int rw;
	int rh;

	get_page_size(THIS, &rw, &rh);

	w = *width;
	h = *height;

	if (w < 0) w = rw;
	if (h < 0) h = rh;

	if (x<0) x=0;
	if (y<0) y=0;
	if (w<1) w=1;
	if (h<1) h=1;


	if ( (x+w) > rw ) w=rw-x;
	if ( (y+h) > rh ) h=rh-y;

	if ( (w<0) || (h<0) ) return NULL;

	#if POPPLER_VERSION_0_20
	THIS->page->displaySlice(THIS->dev,72.0*scale,72.0*scale,
			   rotation,
			   gFalse,
			   gTrue,
			   x,y,w,h,
			   gFalse);
	#else
	THIS->page->displaySlice(THIS->dev,72.0*scale,72.0*scale,
			   rotation,
			   gFalse,
			   gTrue,
			   x,y,w,h,
			   gFalse,
			   THIS->doc->getCatalog ());
	#endif
	
	map=THIS->dev->getBitmap();
	
	data=(uint32_t*)map->getDataPtr();


	*width = w;
	*height = h;

	return data;
}

BEGIN_METHOD(PDFPAGE_image, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	uint32_t *data;
	int32_t x,y, w, h;

	x = VARGOPT(x, 0);
	y = VARGOPT(y, 0);
	w = VARGOPT(w, -1);
	h = VARGOPT(h, -1);

	data = get_page_data(THIS, x, y, &w, &h, THIS->scale, THIS->rotation);
	if (!data) { GB.ReturnNull(); return; }
	/*GB.Image.Create(&img, data, w, h, GB_IMAGE_RGB);
	GB.ReturnObject(img);*/

	GB.ReturnObject(IMAGE.Create(w, h, GB_IMAGE_RGB, (unsigned char *)data));

END_METHOD

BEGIN_PROPERTY (PDFPAGE_property_image)

	int32_t w=-1;
	int32_t h=-1;
	uint32_t *data;

	data = get_page_data(THIS, 0, 0, &w, &h, THIS->scale, THIS->rotation);
	if (!data) { GB.ReturnNull(); return; }
	/*GB.Image.Create(&img, data, w, h, GB_IMAGE_RGB);
	GB.ReturnObject(img);*/

	GB.ReturnObject(IMAGE.Create(w, h, GB_IMAGE_RGB, (unsigned char *)data));

END_PROPERTY

BEGIN_METHOD(PDFPAGE_select, GB_INTEGER X; GB_INTEGER Y; GB_INTEGER W; GB_INTEGER H)

	TextOutputDev *dev;
	GooString *str;
	Gfx *gfx;
	int32_t x,y,w,h;

	x = VARGOPT(X, 0);
	y = VARGOPT(Y, 0);
	w = VARGOPT(W, (int32_t)THIS->page->getMediaWidth());
	h = VARGOPT(H, (int32_t)THIS->page->getMediaHeight());

	#if POPPLER_VERSION_0_20
	dev = new TextOutputDev (NULL, gTrue, 0, gFalse, gFalse);
	gfx = THIS->page->createGfx(dev,72.0,72.0,0,gFalse,gTrue,-1, -1, -1, -1, gFalse, NULL, NULL);
	#else
	dev = new TextOutputDev (NULL, gTrue, gFalse, gFalse);
	gfx = THIS->page->createGfx(dev,72.0,72.0,0,gFalse,gTrue,-1, -1, -1, -1, gFalse,THIS->doc->getCatalog (),NULL, NULL, NULL, NULL);
	#endif

	THIS->page->display(gfx);
	dev->endPage();

	str=dev->getText((double)x,(double)y,(double)(w+x),(double)(h+y));

	delete gfx;
	delete dev;

	if (!str)
	{
		GB.ReturnNewZeroString("");
		return;
	}
	
	GB.ReturnNewString(str->getCString(),str->getLength());	
	delete str;

END_METHOD

/*****************************************************************************

 Bookmarks of a PDF page

******************************************************************************/

void aux_fill_links(void *_object)
{
	#if POPPLER_VERSION_0_20
	THIS->links = new Links (THIS->page->getAnnots ());
	#elif POPPLER_VERSION_0_17
	THIS->links = new Links (THIS->page->getAnnots (THIS->doc->getCatalog()));
	#else
	Object obj;
	
	THIS->links = new Links (THIS->page->getAnnots (&obj),THIS->doc->getCatalog()->getBaseURI ());
	obj.free();
	#endif
}

BEGIN_PROPERTY (PDFPAGELINKS_count)

	if (!THIS->links) aux_fill_links(_object);
	if (!THIS->links) { GB.ReturnInteger(0); return; }
	GB.ReturnInteger(THIS->links->getNumLinks());


END_PROPERTY

BEGIN_METHOD (PDFPAGELINKS_get,GB_INTEGER ind;)

	bool pok=true;

	if (!THIS->links) aux_fill_links(_object);
	if (!THIS->links) pok=false;
	else
	{
		if (VARG(ind)<0) pok=false;
		else
		{
			if (VARG(ind)>=THIS->links->getNumLinks()) pok=false;
		}
	}

	if (!pok) { GB.Error("Out of bounds"); return; }

	THIS->lcurrent=VARG(ind);
	THIS->action=THIS->links->getLink(THIS->lcurrent)->getAction();

	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY (PDFPAGELINKDATA_parameters)

	if (THIS->action->getKind() != actionLaunch )
	{
		GB.ReturnNewZeroString("");
		return;	
	}

	GB.ReturnNewZeroString(((LinkLaunch*)THIS->action)->getParams()->getCString());

END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINKDATA_uri)

	char *uri;

	uri=aux_get_target_from_action(THIS->action);

	GB.ReturnNewZeroString(uri);
	if (uri) GB.FreeString(&uri);

END_PROPERTY

BEGIN_PROPERTY(PdfPageLinkData_Rect)

	CPDFRECT *rect = create_rect();
	aux_get_dimensions_from_action(THIS->action, rect);
	GB.ReturnObject(rect);

END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_zoom)

	GB.ReturnFloat(aux_get_zoom_from_action(THIS->action));

END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_page)

	GB.ReturnInteger(aux_get_page_from_action(_object,THIS->action));

END_PROPERTY

BEGIN_PROPERTY (PDFPAGELINKDATA_type)

	GB.ReturnInteger ( (int32_t)THIS->action->getKind() );

END_PROPERTY

BEGIN_PROPERTY(PDFPAGELINKDATA_check)

	if (THIS->action)
		RETURN_SELF();
	else
		GB.ReturnNull();

END_PROPERTY

static void aux_get_link_dimensions(void *_object, CPDFRECT *rect)
{
	double l,t,w,h;
	double pw,ph;

	pw=THIS->page->getMediaWidth();	
	ph=THIS->page->getMediaHeight();

	THIS->links->getLink(THIS->lcurrent)->getRect(&l, &t, &w, &h);
	w=w-l;
	h=h-t;	

	switch (get_rotation(THIS))
	{
		case 0:
			rect->x = (l*THIS->scale);
			rect->y = ((ph-t-h)*THIS->scale);
			rect->w = (w*THIS->scale);
			rect->h = (h*THIS->scale);
			break;
	
		case 90:
			rect->y = (l*THIS->scale);
			rect->x = (t*THIS->scale);
			rect->h = (w*THIS->scale);
			rect->w = (h*THIS->scale);
			break;

		case 180:
			rect->x = ((l-w)*THIS->scale);
			rect->y = (t*THIS->scale);
			rect->w = (w*THIS->scale);
			rect->h = (h*THIS->scale);
			break;

		case 270:
			rect->y = ((pw-l-w)*THIS->scale);
			rect->x = ((ph-t-h)*THIS->scale);
			rect->h = (w*THIS->scale);
			rect->w = (h*THIS->scale);
			break;
	}
}

BEGIN_PROPERTY(PdfPageLink_rect)

	CPDFRECT *rect = create_rect();
	aux_get_link_dimensions(THIS, rect);
	GB.ReturnObject(rect);

END_PROPERTY


/*****************************************************************************

 Finding a text in a PDF page

******************************************************************************/

BEGIN_METHOD (PDFPAGE_find,GB_STRING Text; GB_BOOLEAN Sensitive;)

	TextOutputDev *textdev;
	double x0=0, y0=0;
	double x1, y1;
	CPDFFIND *el;	
	Unicode *block=NULL;
	int nlen=0;
	bool sensitive=false;
	int count;
	double x, y, w, h, wp, hp;
	int rotation;

	// TODO: Use UCS-4BE on big endian systems?
	if (GB.ConvString ((char **)(void *)&block,STRING(Text),LENGTH(Text),"UTF-8",GB_SC_UNICODE))
	{	
		GB.Error("Invalid UTF-8 string");
		return;
	}

	nlen=GB.StringLength((char*)block)/sizeof(Unicode);

	if (!MISSING(Sensitive)) sensitive=VARG(Sensitive);

	#if POPPLER_VERSION_0_20
	textdev = new TextOutputDev (NULL, true, 0, false, false);
	THIS->page->display (textdev, 72, 72, 0, false, false, false);
	#else
	textdev = new TextOutputDev (NULL, true, false, false);
	THIS->page->display (textdev, 72, 72, 0, false, false, false, THIS->doc->getCatalog());
	#endif

	if (THIS->Found) { GB.FreeArray(POINTER(&THIS->Found)); THIS->Found=NULL; }

	count = 0;
	#if POPPLER_VERSION_0_20
	while (textdev->findText (block,nlen,gFalse,gTrue,gTrue,gFalse,sensitive,gFalse,gFalse,&x0,&y0,&x1,&y1))
	#else
	while (textdev->findText (block,nlen,gFalse,gTrue,gTrue,gFalse,sensitive,gFalse,&x0,&y0,&x1,&y1))
	#endif
	{
		if (!THIS->Found)
			GB.NewArray(POINTER(&THIS->Found),sizeof(CPDFFIND),1);
		else
			GB.Add(POINTER(&THIS->Found));

		el = &(THIS->Found[count++]); //(CPDFFIND*)&((CPDFFIND*)THIS->Found)[GB.Count(POINTER(THIS->Found))-1];
		
		x = x0;
		y = y0;
		w = x1 - x0;
		h = y1 - y0;

		wp = THIS->page->getMediaWidth();
		hp = THIS->page->getMediaHeight();
		rotation = THIS->page->getRotate();
		if (rotation == 90 || rotation == 270)
		{
			x0 = wp; wp = hp; hp = x0;
		}

		rotation = THIS->rotation; //get_rotation(THIS);
		while (rotation > 0)
		{
			x0 = wp; wp = hp; hp = x0;

			x0 = wp - y - h;
			y0 = x;

			x = w; w = h; h = x;

			x = x0;
			y = y0;

			rotation -= 90;
		}

		el->x0 = x * THIS->scale;
		el->y0 = y * THIS->scale;
		el->x1 = w * THIS->scale;
		el->y1 = h * THIS->scale;
	}

	delete textdev;

	GB.ReturnBoolean(count == 0);

END_METHOD


BEGIN_METHOD(PDFPAGERESULT_get,GB_INTEGER Index)

	CPDFRECT *rect;
	CPDFFIND *el;
	int index;

	index = VARG(Index);
	
	if (!THIS->Found || index < 0 || index >= GB.Count(THIS->Found))
	{
		GB.Error("Out of bounds");
		return;
	}

	el = &(THIS->Found[index]);
	rect = create_rect();
	
	rect->x = el->x0;
	rect->y = el->y0;
	rect->w = el->x1;
	rect->h = el->y1;
	
	GB.ReturnObject(rect);

END_METHOD

BEGIN_PROPERTY (PDFPAGERESULT_count)

	if (!THIS->Found) { GB.ReturnInteger(0); return; } 
	GB.ReturnInteger( GB.Count(POINTER(THIS->Found)) );

END_PROPERTY


/**********************************************************************

Gambas Interface

***********************************************************************/

GB_DESC PdfRectDesc[] =
{
	GB_DECLARE("PdfRect", sizeof(CPDFRECT)), GB_NOT_CREATABLE(),
	
	GB_PROPERTY_READ("X", "f", PdfRect_X),
	GB_PROPERTY_READ("Y", "f", PdfRect_Y),
	GB_PROPERTY_READ("Width", "f", PdfRect_Width),
	GB_PROPERTY_READ("Height", "f", PdfRect_Height),
	GB_PROPERTY_READ("W", "f", PdfRect_Width),
	GB_PROPERTY_READ("H", "f", PdfRect_Height),
	GB_PROPERTY_READ("Left", "f", PdfRect_X),
	GB_PROPERTY_READ("Top", "f", PdfRect_Y),
	GB_PROPERTY_READ("Right", "f", PdfRect_Right),
	GB_PROPERTY_READ("Bottom", "f", PdfRect_Bottom),
	
	GB_END_DECLARE
};


GB_DESC PdfResultDesc[]=
{
	GB_DECLARE(".PdfDocumentPage.Result",0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get","PdfRect",PDFPAGERESULT_get,"(Index)i"),
	GB_PROPERTY_READ("Count","i",PDFPAGERESULT_count),

	GB_END_DECLARE
};


GB_DESC PdfLinkDataDesc[]=
{
	GB_DECLARE(".PdfDocumentPage.Link.Data",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Type","i",PDFPAGELINKDATA_type),
	GB_PROPERTY_READ("Target","s",PDFPAGELINKDATA_uri),
	GB_PROPERTY_READ("Parameters","s",PDFPAGELINKDATA_parameters),
	GB_PROPERTY_READ("Page","i",PDFPAGELINKDATA_page),
	GB_PROPERTY_READ("Zoom","f",PDFPAGELINKDATA_zoom),
	GB_PROPERTY_READ("Rect", "PdfRect", PdfPageLinkData_Rect),

	GB_END_DECLARE
};


GB_DESC PdfLinkDesc[]=
{
	GB_DECLARE(".PdfDocumentPage.Link",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Rect", "PdfRect", PdfPageLink_rect),
	GB_PROPERTY_READ("Data",".PdfDocumentPage.Link.Data", PDFPAGELINKDATA_check),

	GB_END_DECLARE
};


GB_DESC PdfIndexDesc[]=
{
	GB_DECLARE(".PdfDocument.Index",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Expanded","b",PDFINDEX_is_open),
	GB_PROPERTY_READ("Count","i",PDFINDEX_count),
	GB_PROPERTY_READ("HasChildren","b",PDFINDEX_has_children),
	GB_PROPERTY_READ("Title","s",PDFINDEX_title),
	GB_PROPERTY_READ("Text","s",PDFINDEX_title),

	GB_PROPERTY_READ("Data", ".PdfDocumentPage.Link.Data", PDFPAGELINKDATA_check),
	GB_METHOD("MovePrevious","b",PDFINDEX_prev,0),
	GB_METHOD("MoveNext","b",PDFINDEX_next,0),
	GB_METHOD("MoveChild","b",PDFINDEX_child,0),
	GB_METHOD("MoveParent","b",PDFINDEX_parent,0),
	GB_METHOD("MoveRoot",0,PDFINDEX_root,0),

	GB_END_DECLARE
};


GB_DESC PdfPageDesc[]=
{
	GB_DECLARE(".PdfDocumentPage",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("W","f",PDFPAGE_width),
	GB_PROPERTY_READ("H","f",PDFPAGE_height),
	GB_PROPERTY_READ("Width","f",PDFPAGE_width),
	GB_PROPERTY_READ("Height","f",PDFPAGE_height),
	
	GB_PROPERTY_READ("Image","Image",PDFPAGE_property_image),
	GB_PROPERTY_SELF("Result",".PdfDocumentPage.Result"),

	GB_METHOD("GetImage","Image",PDFPAGE_image,"[(X)i(Y)i(Width)i(Height)i]"),
	GB_METHOD("Find","b",PDFPAGE_find,"(Text)s[(CaseSensitive)b]"),
	GB_METHOD("Select","s",PDFPAGE_select,"[(X)i(Y)i(W)i(H)i]"),

	GB_METHOD("_get",".PdfDocumentPage.Link",PDFPAGELINKS_get,"(Index)i"),
	GB_PROPERTY_READ("Count","i",PDFPAGELINKS_count),

	GB_END_DECLARE
};


GB_DESC PdfDocumentInfo[] =
{
	GB_DECLARE(".PdfDocument.Info",0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Title","s",PDFINFO_title),
	GB_PROPERTY_READ("Format","s",PDFINFO_format),
	GB_PROPERTY_READ("Author","s",PDFINFO_author),
	GB_PROPERTY_READ("Subject","s",PDFINFO_subject),
	GB_PROPERTY_READ("Keywords","s",PDFINFO_keywords),
	GB_PROPERTY_READ("Creator","s",PDFINFO_creator),
	GB_PROPERTY_READ("Producer","s",PDFINFO_producer),
	GB_PROPERTY_READ("CreationDate","d",PDFINFO_creation),
	GB_PROPERTY_READ("ModificationDate","d",PDFINFO_modification),
	GB_PROPERTY_READ("Linearized","b",PDFINFO_linearized),
	GB_PROPERTY_READ("Layout","i",PDFINFO_layout),
	GB_PROPERTY_READ("Mode","i",PDFINFO_mode),
	GB_PROPERTY_READ("CanCopy","b",PDFINFO_cancopy),
	GB_PROPERTY_READ("CanModify","b",PDFINFO_canmodify),
	GB_PROPERTY_READ("CanPrint","b",PDFINFO_canprint),
	GB_PROPERTY_READ("CanAddNotes","b",PDFINFO_canaddnotes),

	GB_END_DECLARE
};


GB_DESC PdfLayoutDesc[] =
{
  GB_DECLARE("PdfLayout", 0), GB_NOT_CREATABLE(),

  GB_CONSTANT("Unset","i",Catalog::pageLayoutNone),
  GB_CONSTANT("SinglePage","i",Catalog::pageLayoutSinglePage),
  GB_CONSTANT("OneColumn","i",Catalog::pageLayoutOneColumn),
  GB_CONSTANT("TwoColumnLeft","i",Catalog::pageLayoutTwoColumnLeft),
  GB_CONSTANT("TwoColumnRight","i",Catalog::pageLayoutTwoColumnRight),
  GB_CONSTANT("TwoPageLeft","i",Catalog::pageLayoutTwoPageLeft),
  GB_CONSTANT("TwoPageRight","i",Catalog::pageLayoutTwoPageRight),

  GB_END_DECLARE
};


GB_DESC PdfModeDesc[] =
{
	GB_DECLARE("PdfPageMode",0), GB_NOT_CREATABLE(),

	GB_CONSTANT("Unset","i",Catalog::pageModeNone),
	GB_CONSTANT("UseOutlines","i",Catalog::pageModeOutlines),
	GB_CONSTANT("UseThumbs","i",Catalog::pageModeThumbs),
	GB_CONSTANT("FullScreen","i",Catalog::pageModeFullScreen),
	GB_CONSTANT("UseOC","i",Catalog::pageModeOC),
	GB_CONSTANT("UseAttachments","i",Catalog::pageModeAttach),

	GB_END_DECLARE
};

GB_DESC PdfDocumentDesc[] =
{
  GB_DECLARE("PdfDocument", sizeof(CPDFDOCUMENT)),

  GB_CONSTANT("Unknown","i",actionUnknown),  /* unknown action */
  GB_CONSTANT("Goto","i",actionGoTo),        /* go to destination */
  GB_CONSTANT("GotoRemote","i",actionGoToR), /* go to destination in new file */
  GB_CONSTANT("Launch","i",actionLaunch),    /* launch app or open doc. */
  GB_CONSTANT("Uri","i",actionURI),          /* URI */
  GB_CONSTANT("Named","i",actionNamed),      /* named action*/
  GB_CONSTANT("Movie","i",actionMovie),      /* movie action */

  GB_CONSTANT("Normal","i",0),
  GB_CONSTANT("Sideways","i",90),
  GB_CONSTANT("Inverted","i",180),
  GB_CONSTANT("SidewaysInverted","i",270),

  GB_METHOD("_new", 0, PDFDOCUMENT_new, "[(File)s]"),
  GB_METHOD("_free", 0, PDFDOCUMENT_free, 0),

  GB_METHOD("Open",0,PDFDOCUMENT_open,"(File)s"),
  GB_METHOD("Close",0,PDFDOCUMENT_close,0),
  GB_METHOD("_get",".PdfDocumentPage",PDFDOCUMENT_get,"(Index)i"),

  GB_PROPERTY("Zoom", "f", PDFDOCUMENT_scale),
  GB_PROPERTY("Orientation", "i", PDFDOCUMENT_rotation),

  GB_PROPERTY_READ("Ready","b",PDFDOCUMENT_ready),
  GB_PROPERTY_READ("Count","i",PDFDOCUMENT_count),
  GB_PROPERTY_READ("HasIndex","b",PDFDOCUMENT_has_index),
  GB_PROPERTY_READ("Index",".PdfDocument.Index",PDFDOCUMENT_index),
  GB_PROPERTY_READ("Info",".PdfDocument.Info",PDFDOCUMENT_info),

  GB_END_DECLARE
};


