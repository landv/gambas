/***************************************************************************

  CPicture.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>

  GTK+ component

  Realizado para la Junta de Extremadura.
  Consejería de Educación Ciencia y Tecnología.
  Proyecto gnuLinEx

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

#define __CPICTURE_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CPicture.h"
#include "CImage.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


void* GTK_GetPicture(GdkPixbuf *buf)
{
	CPICTURE *pic;

	GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
	if (pic->picture) pic->picture->unref();
	pic->picture=gPicture::fromPixbuf(buf);

	return (void*)pic;
}

/*******************************************************************************

Picture Cache

********************************************************************************/
GHashTable *PICT_table=NULL;

CPICTURE* PICTURE_cache_get(char *key)
{
	if (!PICT_table) return NULL;
	return (CPICTURE*)g_hash_table_lookup(PICT_table,(gconstpointer)key);
}

void PICTURE_cache_add(char *key,CPICTURE *pic)
{
	CPICTURE *test;

	if (!PICT_table) PICT_table=g_hash_table_new(g_str_hash,g_str_equal);

	test=PICTURE_cache_get(key);
	if (test) {
		if (test==pic) return;
		GB.Unref((void**)&test);
		g_hash_table_remove(PICT_table,(gconstpointer)pic);
	}

	g_hash_table_insert(PICT_table,(gpointer)key,(gpointer)pic);
	if (pic) GB.Ref((void*)pic);
}

gboolean PICTURE_cache_unref(char *key,CPICTURE *pic, gpointer data)
{
	GB.Unref((void**)&pic);
	return true;
}

void PICTURE_cache_flush()
{
	if (!PICT_table) return;
	g_hash_table_foreach_remove(PICT_table,(GHRFunc)PICTURE_cache_unref,NULL);
	g_hash_table_destroy(PICT_table);
	PICT_table=NULL;
}

/*******************************************************************************

  class Picture

*******************************************************************************/
static GB_FUNCTION _stock_get_func;

static bool init_stock()
{
	static bool init = false;
	static bool error = false;

	if (!init)
	{
		error = GB.GetFunction(&_stock_get_func, GB.FindClass("Stock"), "_get", "s", "Picture");
		init = true;
	}

	return error;
}

BEGIN_METHOD(CPICTURE_get, GB_STRING path;)

	char *addr;
	long len;
	CPICTURE *pic=NULL;
	gPicture *buf;
	gPicture *img;
	GB_VALUE *value;

	pic=PICTURE_cache_get( GB.ToZeroString(ARG(path)) );
	if (pic)
	{
		GB.ReturnObject(pic);
		return;
	}


	if (GB.LoadFile (STRING(path),LENGTH(path),&addr,&len))
	{

		buf=gPicture::fromNamedIcon( GB.ToZeroString(ARG(path)) );
		if (buf)
		{
			GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
			if (pic->picture) pic->picture->unref();
			pic->picture=buf;
			GB.ReturnObject(pic);
			return;

		}

		if (LENGTH(path) >= STOCK_PREFIX_LEN && strncmp(STRING(path), STOCK_PREFIX, STOCK_PREFIX_LEN) == 0)
		{
			if (LENGTH(path) == STOCK_PREFIX_LEN) { GB.Error("Unable to load image");return; }
			if (init_stock()) { GB.Error("Unable to load image");return; }
			
			GB.Push(1, GB_T_STRING, &STRING(path)[STOCK_PREFIX_LEN], LENGTH(path) - STOCK_PREFIX_LEN);
			value = GB.Call(&_stock_get_func, 1, false);
			if (value->type >= GB_T_OBJECT) pic = (CPICTURE *)((GB_OBJECT *)value)->value;
			if (!pic) { GB.Error("Unable to load image");return; }
			GB.ReturnObject(pic);
			return;
				
		}

		GB.Error("Unable to load image");
		return;
	}

	GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
	pic->picture->fromMemory(addr,len);
	GB.ReleaseFile (&addr,len);
	if (pic) PICTURE_cache_add(STRING(path),pic);
	GB.ReturnObject(pic);

END_METHOD


BEGIN_METHOD(CPICTURE_put, GB_OBJECT picture; GB_STRING path;)

	CPICTURE *picture=(CPICTURE*)VARG(picture);

	if (picture) PICTURE_cache_add(STRING(path),picture);

END_METHOD



BEGIN_METHOD_VOID(CPICTURE_flush)

	PICTURE_cache_flush();

END_METHOD


BEGIN_METHOD(CPICTURE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

	gint myw=0,myh=0;
	bool mytrans=false;

	if (!MISSING(w)) myw=(gint)VARG(w);
	if (!MISSING(h)) myh=(gint)VARG(h);
	if (!MISSING(trans)) mytrans=VARG(trans);

	PICTURE=new gPicture(myw,myh,mytrans);

END_METHOD


BEGIN_METHOD_VOID(CPICTURE_free)

	if (PICTURE) PICTURE->unref();

END_METHOD


BEGIN_METHOD(CPICTURE_resize, GB_INTEGER width; GB_INTEGER height)

	PICTURE->resize(VARG(width),VARG(height));

END_METHOD


BEGIN_PROPERTY(CPICTURE_width)

	GB.ReturnInteger(PICTURE->width());

END_PROPERTY


BEGIN_PROPERTY(CPICTURE_height)

	GB.ReturnInteger(PICTURE->height());

END_PROPERTY


BEGIN_PROPERTY(CPICTURE_depth)

	GB.ReturnInteger(PICTURE->depth());

END_PROPERTY


BEGIN_METHOD(CPICTURE_load, GB_STRING path)

	CPICTURE *pict;
	char *addr;
	long len;
	bool ok;

	if (!GB.LoadFile (STRING(path),LENGTH(path),&addr,&len))
	{
		GB.New((void **)&pict, GB.FindClass("Picture"), 0, 0);
		pict->picture->fromMemory(addr, len);
		ok = TRUE;
		GB.ReleaseFile(&addr,len);

		if (ok)
		{
			GB.ReturnObject(pict);
			return;
		}
		else
			GB.Unref((void **)&pict);
	}

	GB.Error("Unable to load image");



END_METHOD

BEGIN_METHOD(CPICTURE_fromMemory,GB_STRING data;)

	CPICTURE *pic=NULL;

	if (!LENGTH(data)) return;

	GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
	pic->picture->fromMemory(STRING(data),LENGTH(data));
	GB.ReturnObject(pic);


END_METHOD


BEGIN_METHOD(CPICTURE_save, GB_STRING path)

	char *path=NULL;

	if (LENGTH(path)) path=STRING(path);
	switch (PICTURE->save(path))
	{
		case 0: break;
		case -1: GB.Error("Unknown format"); break;
		case -2: GB.Error("Unable to save picture"); break;
	}



END_METHOD


BEGIN_METHOD_VOID(CPICTURE_clear)

	bool trans;

	if (PICTURE)
	{
		trans=PICTURE->transparent();
		PICTURE->unref();
		PICTURE=new gPicture(0,0,trans);
	}

END_METHOD


BEGIN_METHOD(CPICTURE_fill, GB_INTEGER col)

	PICTURE->Fill(VARG(col));

END_METHOD


BEGIN_METHOD(CPICTURE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CPICTURE *pic=NULL;
	long x=0;
	long y=0;
	long w=PICTURE->width();
	long h=PICTURE->height();

	if (!MISSING(x)) x=VARG(x);
	if (!MISSING(y)) y=VARG(y);
	if (!MISSING(w)) w=VARG(w);
	if (!MISSING(h)) h=VARG(h);

	GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
	if (pic->picture) pic->picture->unref();
	pic->picture=PICTURE->copy(x,y,w,h);
	GB.ReturnObject(pic);

END_METHOD


BEGIN_PROPERTY(CPICTURE_image)

	CIMAGE *img;

	GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
	if (img->image) delete img->image;
	img->image=PICTURE->getImage();
	GB.ReturnObject((void*)img);

END_PROPERTY


BEGIN_PROPERTY(CPICTURE_transparent)

	if (READ_PROPERTY) { GB.ReturnBoolean(PICTURE->transparent()); return; }
	PICTURE->setTransparent(VPROP(GB_INTEGER));

END_PROPERTY


GB_DESC CPictureDesc[] =
{
  GB_DECLARE("Picture", sizeof(CPICTURE)),

  GB_STATIC_METHOD("_exit", NULL, CPICTURE_flush, NULL),

  GB_METHOD("_new", NULL, CPICTURE_new, "[(Width)i(Height)i(Transparent)b]"),
  GB_METHOD("_free", NULL, CPICTURE_free, NULL),
  GB_STATIC_METHOD("_exit",NULL,CPICTURE_flush,NULL),

  GB_STATIC_METHOD("_get", "Picture", CPICTURE_get, "(Path)s"),
  GB_STATIC_METHOD("_put", NULL, CPICTURE_put, "(Picture)Picture;(Path)s"),
  GB_STATIC_METHOD("Flush", NULL, CPICTURE_flush, NULL),

  GB_PROPERTY_READ("Width", "i", CPICTURE_width),
  GB_PROPERTY_READ("Height", "i", CPICTURE_height),
  GB_PROPERTY_READ("Depth", "i", CPICTURE_depth),

  GB_STATIC_METHOD("FromString", "Picture", CPICTURE_fromMemory, "(Data)s"),
  GB_STATIC_METHOD("Load", "Picture", CPICTURE_load, "(Path)s"),
  GB_METHOD("Save", NULL, CPICTURE_save, "(Path)s"),
  GB_METHOD("Resize", NULL, CPICTURE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", NULL, CPICTURE_clear, NULL),
  GB_METHOD("Fill", NULL, CPICTURE_fill, "(Color)i"),

  GB_PROPERTY("Transparent", "b", CPICTURE_transparent),

  GB_METHOD("Copy", "Picture", CPICTURE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_PROPERTY_READ("Image", "Image", CPICTURE_image),

  GB_END_DECLARE
};

