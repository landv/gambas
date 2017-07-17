/***************************************************************************

	CPicture.cpp

	(c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CPICTURE_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "ggambastag.h"
#include "CDraw.h"
#include "cpaint_impl.h"
#include "CScreen.h"
#include "CImage.h"
#include "CPicture.h"

CPICTURE *CPICTURE_create(gPicture *picture)
{
	CPICTURE *pic;

	pic = (CPICTURE *)GB.New(GB.FindClass("Picture"), 0, 0);
	if (picture)
	{
		pic->picture->unref();
		pic->picture = picture;
		picture->setTag(new gGambasTag((void *)pic));
	}
	return pic;
}

void* GTK_GetPicture(GdkPixbuf *buf)
{
	CPICTURE *pic = CPICTURE_create(new gPicture(buf));
	g_object_ref(buf);
	return (void*)pic;
}


bool CPICTURE_load_image(gPicture **p, const char *path, int lenp)
{
	char *addr;
	int len;
	
	*p = NULL;
	
	if (GB.LoadFile(path, lenp, &addr, &len))
	{
		GB.Error(NULL);
		return FALSE;
	}
			
	*p = gPicture::fromMemory(addr, len);
	GB.ReleaseFile(addr, len);
	return *p != NULL;
}

//---------------------------------------------------------------------------

BEGIN_METHOD(CPICTURE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

	int w = VARGOPT(w, 0);
	int h = VARGOPT(h, 0);
	bool trans = VARGOPT(trans, false);

#ifdef GTK3
	PICTURE = new gPicture(gPicture::SURFACE, w, h, trans);
#else
	PICTURE = new gPicture(gPicture::PIXMAP, w, h, trans);
#endif
	PICTURE->setTag(new gGambasTag(THIS));

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

	CPICTURE *picture;
	char *addr;
	int len;

	if (!GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
	{
		gPicture *pic = gPicture::fromMemory(addr, len);
		GB.ReleaseFile(addr, len);

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

	switch (PICTURE->save(GB.FileName(STRING(path), LENGTH(path)), VARGOPT(quality, -1)))
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

	CIMAGE *img = CIMAGE_create(PICTURE->copy());
	//CIMAGE_get(img)->getPixbuf();
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

	GB_METHOD("_new", NULL, CPICTURE_new, "[(Width)i(Height)i(Transparent)b]"),
	GB_METHOD("_free", NULL, CPICTURE_free, NULL),

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

	GB_INTERFACE("Paint", &PAINT_Interface),

	GB_END_DECLARE
};

