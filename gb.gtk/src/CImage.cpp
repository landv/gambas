/***************************************************************************

  CImage.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CIMAGE_CPP

#include "gambas.h"
#include "main.h"
#include "widgets.h"
#include "CImage.h"
#include "CPicture.h"


void* GTK_GetImage(GdkPixbuf *buf)
{
	CIMAGE *pic;

	GB.New((void **)&pic, GB.FindClass("Image"), 0, 0);
	pic->image->image=buf;
	g_object_ref(buf);

	return (void*)pic;
}

/*******************************************************************************

  Image

*******************************************************************************/


BEGIN_METHOD(CIMAGE_new, GB_INTEGER w; GB_INTEGER h)

	int myw=0,myh=0;

	if (!MISSING(w))
	{
		myw=VARG(w);

		if (!MISSING(h))
			myh=VARG(h);
		else
			myh=myw;
	}

	IMAGE=new gImage(myw,myh);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_free)

	delete IMAGE;

END_METHOD


BEGIN_PROPERTY(CIMAGE_picture)

	CPICTURE *img;

	GB.New((void **)&img, GB.FindClass("Picture"), 0, 0);
	if (img->picture) img->picture->unref();
	img->picture=IMAGE->getPicture();
	GB.ReturnObject((void*)img);

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



END_PROPERTY


BEGIN_METHOD(CIMAGE_fromMemory,GB_STRING Data;)

	CIMAGE *img;

	GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
	img->image->fromMemory(STRING(Data),LENGTH(Data));
	GB.ReturnObject((void*)img);

END_METHOD

BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	CIMAGE *image;
	char *addr;
	long len;
	bool ok;

	if (!GB.LoadFile (STRING(path),LENGTH(path),&addr,&len))
	{
		GB.New((void **)&image, GB.FindClass("Image"), 0, 0);
		image->image->fromMemory(addr,len);
		ok = TRUE;
		GB.ReleaseFile (&addr,len);

		if (ok)
		{
			GB.ReturnObject(image);
			return;
		}
		else
			GB.Unref((void **)&image);
	}

	GB.Error("Unable to load image");

END_METHOD


BEGIN_METHOD(CIMAGE_save, GB_STRING path)

	char *path=NULL;

	if (LENGTH(path)) path=STRING(path);
	switch (IMAGE->save(path))
	{
		case 0: break;
		case -1: GB.Error("Unknown format"); break;
		case -2: GB.Error("Unable to save picture"); break;
	}

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_clear)


END_METHOD


BEGIN_METHOD(CIMAGE_fill, GB_INTEGER col)

	IMAGE->fill(VARG(col));

END_METHOD


BEGIN_METHOD(CIMAGE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CIMAGE *img;
	long x=0,y=0,w=IMAGE->width(),h=IMAGE->height();

	if (!MISSING(x)) x=VARG(x);
	if (!MISSING(y)) y=VARG(y);
	if (!MISSING(w)) w=VARG(w);
	if (!MISSING(h)) h=VARG(h);

	GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
	if (img->image) delete img->image;
	img->image=IMAGE->copy(x,y,w,h);
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_stretch, GB_INTEGER width; GB_INTEGER height; GB_BOOLEAN smooth)

	CIMAGE *img;
	bool smooth=true;

	if (!MISSING(smooth)) smooth=VARG(smooth);

	GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
	if (img->image) delete img->image;
	img->image=IMAGE->stretch(VARG(width),VARG(height),smooth);
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_flip)

	CIMAGE *img;

	GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
	if (img->image) delete img->image;
	img->image=IMAGE->flip();
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_mirror)

	CIMAGE *img;

	GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
	if (img->image) delete img->image;
	img->image=IMAGE->mirror();
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_rotate, GB_FLOAT angle)

	CIMAGE *img;

	GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
	if (img->image) delete img->image;
	img->image=IMAGE->rotate(VARG(angle));
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_replace, GB_INTEGER src; GB_INTEGER dst)

	IMAGE->replace(VARG(src),VARG(dst));

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



GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_STATIC_METHOD("FromString","Image",CIMAGE_fromMemory,"(Data)s"),

  GB_METHOD("_new", NULL, CIMAGE_new, "[(Width)i(Height)i]"),
  GB_METHOD("_free", NULL, CIMAGE_free, NULL),

  GB_METHOD("_get", "i", CIMAGE_pixels_get, "(X)i(Y)i"),
  GB_METHOD("_put", NULL, CIMAGE_pixels_put, "(Color)i(X)i(Y)i"),


  GB_PROPERTY_READ("Width", "i", CIMAGE_width),
  GB_PROPERTY_READ("Height", "i", CIMAGE_height),
  GB_PROPERTY("Depth", "i", CIMAGE_depth),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  GB_METHOD("Save", NULL, CIMAGE_save, "(Path)s"),
  GB_METHOD("Resize", NULL, CIMAGE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", NULL, CIMAGE_clear, NULL),
  GB_METHOD("Fill", NULL, CIMAGE_fill, "(Color)i"),
  GB_METHOD("Replace", NULL, CIMAGE_replace, "(OldColor)i(NewColor)i"),

  GB_METHOD("Copy", "Image", CIMAGE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Stretch", "Image", CIMAGE_stretch, "(Width)i(Height)i[(Smooth)b]"),
  GB_METHOD("Flip", "Image", CIMAGE_flip, NULL),
  GB_METHOD("Mirror", "Image", CIMAGE_mirror, NULL),
  GB_METHOD("Rotate", "Image", CIMAGE_rotate, "(Angle)f"),

  GB_PROPERTY_READ("Picture", "Picture", CIMAGE_picture),


  GB_END_DECLARE
};

