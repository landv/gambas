/***************************************************************************

  CPicture.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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
#include "ggambastag.h"
#include "CDraw.h"
#include "CScreen.h"
#include "CPicture.h"

CIMAGE *CIMAGE_create(gPicture *picture)
{
	CIMAGE *pic;
	GB.New((void **)POINTER(&pic), GB.FindClass("Image"), 0, 0);
	if (picture)
	{
		pic->picture->unref();
		pic->picture = picture;
		picture->setTag(new gGambasTag((void *)pic));
	}
	return pic;
}

CPICTURE *CPICTURE_create(gPicture *picture)
{
	CPICTURE *pic;
	GB.New((void **)POINTER(&pic), GB.FindClass("Picture"), 0, 0);
	if (picture)
	{
		pic->picture->unref();
		pic->picture = picture;
		picture->setTag(new gGambasTag((void *)pic));
	}
	return pic;
}

void* GTK_GetImage(GdkPixbuf *buf)
{
  CIMAGE *pic = CIMAGE_create(new gPicture(buf));
	g_object_ref(buf);
	return (void*)pic;
}

void* GTK_GetPicture(GdkPixbuf *buf)
{
	CPICTURE *pic = CPICTURE_create(new gPicture(buf));
	g_object_ref(buf);
	return (void*)pic;
}

/*******************************************************************************

  Image

*******************************************************************************/

BEGIN_METHOD(CIMAGE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

  int w = VARGOPT(w, 0);
  int h = VARGOPT(h, 0);
  bool trans = VARGOPT(trans, false);

	IMAGE = new gPicture(gPicture::MEMORY, w, h, trans);
	IMAGE->setTag(new gGambasTag((void *)THIS));

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_free)

	IMAGE->unref();

END_METHOD


BEGIN_PROPERTY(CIMAGE_picture)

	CPICTURE *pic = CPICTURE_create(IMAGE->copy());
	pic->picture->getPixmap();
	GB.ReturnObject((void*)pic);

END_PROPERTY


BEGIN_METHOD(CIMAGE_resize, GB_INTEGER width; GB_INTEGER height)

	IMAGE->resize(VARG(width),VARG(height));

END_METHOD


BEGIN_PROPERTY(CIMAGE_width)

	GB.ReturnInteger(IMAGE->width());

END_PROPERTY


BEGIN_PROPERTY(CIMAGE_height)

	GB.ReturnInteger(IMAGE->height());

END_PROPERTY


BEGIN_PROPERTY(CIMAGE_depth)

	GB.ReturnInteger(IMAGE->depth());

END_PROPERTY


BEGIN_PROPERTY(CIMAGE_transparent)

	if (READ_PROPERTY) 
		GB.ReturnBoolean(IMAGE->isTransparent());
	else
		IMAGE->setTransparent(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	CIMAGE *image;
	char *addr;
	int len;

	if (!GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
	{
		gPicture *pic = gPicture::fromMemory(addr, len);
		GB.ReleaseFile(&addr, len);
		
		if (pic)
		{
			image = CIMAGE_create(pic);
			image->picture->getPixbuf();
			GB.ReturnObject(image);
			return;
		}
	}

	GB.Error("Unable to load image");

END_METHOD


BEGIN_METHOD(CIMAGE_save, GB_STRING path; GB_INTEGER quality)

	switch (IMAGE->save(GB.ToZeroString(ARG(path)), VARGOPT(quality, -1)))
	{
		case 0: break;
		case -1: GB.Error("Unknown format"); break;
		case -2: GB.Error("Unable to save picture"); break;
	}

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_clear)

  IMAGE->clear();

END_METHOD


BEGIN_METHOD(CIMAGE_fill, GB_INTEGER col)

	IMAGE->fill(VARG(col));

END_METHOD


BEGIN_METHOD(CIMAGE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CIMAGE *img;
	int x=0,y=0,w=IMAGE->width(),h=IMAGE->height();

	if (!MISSING(x)) x=VARG(x);
	if (!MISSING(y)) y=VARG(y);
	if (!MISSING(w)) w=VARG(w);
	if (!MISSING(h)) h=VARG(h);

  img = CIMAGE_create(IMAGE->copy(x, y, w, h));
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_stretch, GB_INTEGER width; GB_INTEGER height; GB_BOOLEAN smooth)

	CIMAGE *img;
	bool smooth = VARGOPT(smooth, true);

  img = CIMAGE_create(IMAGE->stretch(VARG(width), VARG(height), smooth));
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_flip)

	CIMAGE *img;

  img = CIMAGE_create(IMAGE->flip());
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_mirror)

	CIMAGE *img;

  img = CIMAGE_create(IMAGE->mirror());
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_rotate, GB_FLOAT angle)

	CIMAGE *img;

  img = CIMAGE_create(IMAGE->rotate(VARG(angle)));
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_replace, GB_INTEGER src; GB_INTEGER dst; GB_BOOLEAN noteq)

	IMAGE->replace(VARG(src), VARG(dst), VARGOPT(noteq, false));

END_METHOD



/*******************************************************************************

  .ImagePixels

*******************************************************************************/


BEGIN_METHOD(CIMAGE_pixels_get, GB_INTEGER x; GB_INTEGER y)

	GB.ReturnInteger( IMAGE->getPixel(VARG(x),VARG(y)) );

END_METHOD


BEGIN_METHOD(CIMAGE_pixels_put, GB_INTEGER col; GB_INTEGER x; GB_INTEGER y)

	IMAGE->putPixel(VARG(x),VARG(y),VARG(col));

END_METHOD


BEGIN_PROPERTY(CIMAGE_data)

	GB.ReturnPointer((void *)IMAGE->data());

END_PROPERTY


BEGIN_METHOD(CIMAGE_draw, GB_OBJECT img; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  int x, y, w, h, sx, sy, sw, sh;
  CIMAGE *image = (CIMAGE *)VARG(img);
	gPicture *pic;

  if (GB.CheckObject(image))
    return;

	pic = ((CIMAGE *)image)->picture;

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, -1);
  sh = VARGOPT(sh, -1);

  //DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, pic->width(), pic->height());

	IMAGE->draw(pic, x, y, w, h, sx, sy, sw, sh);

END_METHOD


GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  //GB_STATIC_METHOD("FromString","Image",CIMAGE_fromMemory,"(Data)s"),

  GB_METHOD("_new", 0, CIMAGE_new, "[(Width)i(Height)i(Transparent)b]"),
  GB_METHOD("_free", 0, CIMAGE_free, 0),

  GB_METHOD("_get", "i", CIMAGE_pixels_get, "(X)i(Y)i"),
  GB_METHOD("_put", 0, CIMAGE_pixels_put, "(Color)i(X)i(Y)i"),

  GB_PROPERTY_READ("Width", "i", CIMAGE_width),
  GB_PROPERTY_READ("Height", "i", CIMAGE_height),
  GB_PROPERTY_READ("W", "i", CIMAGE_width),
  GB_PROPERTY_READ("H", "i", CIMAGE_height),
  GB_PROPERTY_READ("Depth", "i", CIMAGE_depth),
  GB_PROPERTY("Transparent", "b", CIMAGE_transparent),
  GB_PROPERTY_READ("Data", "p", CIMAGE_data),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  GB_METHOD("Save", 0, CIMAGE_save, "(Path)s[(Quality)i]"),
  GB_METHOD("Resize", 0, CIMAGE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", 0, CIMAGE_clear, 0),
  GB_METHOD("Fill", 0, CIMAGE_fill, "(Color)i"),
  GB_METHOD("Replace", 0, CIMAGE_replace, "(OldColor)i(NewColor)i[(NotEqual)b]"),

  GB_METHOD("Copy", "Image", CIMAGE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Stretch", "Image", CIMAGE_stretch, "(Width)i(Height)i[(Smooth)b]"),
  GB_METHOD("Flip", "Image", CIMAGE_flip, 0),
  GB_METHOD("Mirror", "Image", CIMAGE_mirror, 0),
  GB_METHOD("Rotate", "Image", CIMAGE_rotate, "(Angle)f"),

  GB_METHOD("Draw", 0, CIMAGE_draw, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

  GB_PROPERTY_READ("Picture", "Picture", CIMAGE_picture),

  GB_END_DECLARE
};


/*******************************************************************************

Picture Cache

********************************************************************************/

static GHashTable *_cache = NULL;

static void destroy_key(char *key)
{
  //fprintf(stderr, "destroy_key: '%s'\n", key);
  g_free(key);
}

static void destroy_value(CPICTURE *pic)
{
  //fprintf(stderr, "destroy_value: %p\n", pic);
  GB.Unref((void **)POINTER(&pic));
}

static CPICTURE *cache_get(char *key)
{
	if (!_cache) 
    return NULL;
	
	return (CPICTURE *)g_hash_table_lookup(_cache, (gconstpointer)key);
}

static void cache_add(char *key, CPICTURE *pic)
{
	if (!_cache) 
    _cache = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)destroy_key, (GDestroyNotify)destroy_value);

  if (!key || !*key) 
    return;
  
	GB.Ref((void *)pic);
	//fprintf(stderr, "cache_add: '%s' %p\n", key, pic);
	g_hash_table_replace(_cache, (gpointer)g_strdup(key), (gpointer)pic);
}

static void cache_flush()
{
	if (!_cache) 
    return;

  g_hash_table_destroy(_cache);
	_cache = NULL;
}

/*******************************************************************************

  class Picture

*******************************************************************************/

#define LOAD_IMAGE_FUNC CPICTURE_load_image

#define IMAGE_TYPE gPicture

#define CREATE_IMAGE_FROM_MEMORY(_image, _addr, _len, _ok) \
	_image = gPicture::fromMemory(_addr, _len);

#define DELETE_IMAGE(_image) 

#define CREATE_PICTURE_FROM_IMAGE(_cpicture, _image) \
	_cpicture = CPICTURE_create(_image);

#define GET_FROM_CACHE(_key) (cache_get(_key))

#define GET_FROM_STOCK(_name, _len) gPicture::fromNamedIcon(_name, _len)

#define INSERT_INTO_CACHE(_key, _cpicture) cache_add((_key), (_cpicture))

#define APPLICATION_THEME CAPPLICATION_Theme

#include "gb.form.picture.h"


BEGIN_METHOD(CPICTURE_get, GB_STRING path)

  GB.ReturnObject(get_picture(STRING(path), LENGTH(path)));

END_METHOD


BEGIN_METHOD(CPICTURE_put, GB_OBJECT picture; GB_STRING path)

  set_picture(STRING(path), LENGTH(path), (CPICTURE *)VARG(picture));

END_METHOD


BEGIN_METHOD_VOID(CPICTURE_flush)

	cache_flush();

END_METHOD


BEGIN_METHOD(CPICTURE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

  int w = VARGOPT(w, 0);
  int h = VARGOPT(h, 0);
  bool trans = VARGOPT(trans, false);

	PICTURE = new gPicture(gPicture::SERVER, w, h, trans);
	PICTURE->setTag(new gGambasTag(THIS));
	
END_METHOD


BEGIN_METHOD_VOID(CPICTURE_free)

	PICTURE->unref();

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

	CPICTURE *picture;
	char *addr;
	int len;

	if (!GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
	{
		gPicture *pic = gPicture::fromMemory(addr, len);
		GB.ReleaseFile(&addr, len);
		
		if (pic)
		{
			picture = CPICTURE_create(pic);
			GB.ReturnObject(picture);
			return;
		}
	}

	GB.Error("Unable to load picture");

END_METHOD

/*
BEGIN_METHOD(CPICTURE_fromMemory,GB_STRING data;)

	CPICTURE *pic=NULL;

	if (!LENGTH(data)) return;

	GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
	pic->picture->fromMemory(STRING(data),LENGTH(data));
	GB.ReturnObject(pic);


END_METHOD
*/

BEGIN_METHOD(CPICTURE_save, GB_STRING path; GB_INTEGER quality)

	switch (PICTURE->save(GB.ToZeroString(ARG(path))), VARGOPT(quality, -1))
	{
		case 0: break;
		case -1: GB.Error("Unknown format"); break;
		case -2: GB.Error("Unable to save picture"); break;
	}

END_METHOD


BEGIN_METHOD_VOID(CPICTURE_clear)

  PICTURE->clear();

END_METHOD


BEGIN_METHOD(CPICTURE_fill, GB_INTEGER col)

	PICTURE->fill(VARG(col));

END_METHOD


BEGIN_METHOD(CPICTURE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CPICTURE *pic=NULL;
	int x=0;
	int y=0;
	int w=PICTURE->width();
	int h=PICTURE->height();

	if (!MISSING(x)) x=VARG(x);
	if (!MISSING(y)) y=VARG(y);
	if (!MISSING(w)) w=VARG(w);
	if (!MISSING(h)) h=VARG(h);

  pic = CPICTURE_create(PICTURE->copy(x, y, w, h));
	GB.ReturnObject(pic);

END_METHOD


BEGIN_PROPERTY(CPICTURE_image)

	CIMAGE *img = CIMAGE_create(IMAGE->copy());
	img->picture->getPixbuf();
	GB.ReturnObject((void*)img);

END_PROPERTY

BEGIN_PROPERTY(CPICTURE_transparent)

	if (READ_PROPERTY) { GB.ReturnBoolean(PICTURE->isTransparent()); return; }
	PICTURE->setTransparent(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CPictureDesc[] =
{
  GB_DECLARE("Picture", sizeof(CPICTURE)),

  //GB_STATIC_METHOD("_exit", NULL, CPICTURE_flush, NULL),

  GB_METHOD("_new", 0, CPICTURE_new, "[(Width)i(Height)i(Transparent)b]"),
  GB_METHOD("_free", 0, CPICTURE_free, 0),
  GB_STATIC_METHOD("_exit",0,CPICTURE_flush,0),

  GB_STATIC_METHOD("_get", "Picture", CPICTURE_get, "(Path)s"),
  GB_STATIC_METHOD("_put", 0, CPICTURE_put, "(Picture)Picture;(Path)s"),
  GB_STATIC_METHOD("Flush", 0, CPICTURE_flush, 0),

  GB_PROPERTY_READ("Width", "i", CPICTURE_width),
  GB_PROPERTY_READ("Height", "i", CPICTURE_height),
  GB_PROPERTY_READ("W", "i", CPICTURE_width),
  GB_PROPERTY_READ("H", "i", CPICTURE_height),
  GB_PROPERTY_READ("Depth", "i", CPICTURE_depth),
  GB_PROPERTY("Transparent", "b", CPICTURE_transparent),

  GB_STATIC_METHOD("Load", "Picture", CPICTURE_load, "(Path)s"),
  GB_METHOD("Save", 0, CPICTURE_save, "(Path)s[(Quality)i]"),
  GB_METHOD("Resize", 0, CPICTURE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", 0, CPICTURE_clear, 0),
  GB_METHOD("Fill", 0, CPICTURE_fill, "(Color)i"),

  GB_METHOD("Copy", "Picture", CPICTURE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_PROPERTY_READ("Image", "Image", CPICTURE_image),

  GB_INTERFACE("Draw", &DRAW_Interface),

  GB_END_DECLARE
};

