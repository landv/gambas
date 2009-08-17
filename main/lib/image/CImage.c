/***************************************************************************

  CImage.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CIMAGE_C

#include "main.h"
#include "image.h"
#include "CImage.h"

BEGIN_METHOD(CIMAGE_new, GB_INTEGER w; GB_INTEGER h; GB_INTEGER col; GB_INTEGER format)

	int format = IMAGE_get_default_format();
	
	if (VARGOPT(format, 0) == 1)
		format = GB_IMAGE_FMT_SET_PREMULTIPLIED(format);

	IMAGE_create(THIS_IMAGE, VARGOPT(w, 0), VARGOPT(h, 0), format);
	
	if (!MISSING(col))
		IMAGE_fill(THIS_IMAGE, VARG(col));

END_METHOD

BEGIN_METHOD_VOID(CIMAGE_free)

	IMAGE_delete(THIS_IMAGE);

END_METHOD

BEGIN_PROPERTY(CIMAGE_width)

	GB.ReturnInteger(THIS_IMAGE->width);

END_PROPERTY

BEGIN_PROPERTY(CIMAGE_height)

	GB.ReturnInteger(THIS_IMAGE->height);

END_PROPERTY

BEGIN_PROPERTY(CIMAGE_depth)

	GB.ReturnInteger(32);

END_PROPERTY

BEGIN_METHOD(CIMAGE_fill, GB_INTEGER col)

	IMAGE_fill(THIS_IMAGE, VARG(col));

END_METHOD

BEGIN_METHOD(CIMAGE_get, GB_INTEGER x; GB_INTEGER y)

	GB.ReturnInteger(IMAGE_get_pixel(THIS_IMAGE, VARG(x), VARG(y)));

END_METHOD

BEGIN_METHOD(CIMAGE_put, GB_INTEGER col; GB_INTEGER x; GB_INTEGER y)

	IMAGE_set_pixel(THIS_IMAGE, VARG(x), VARG(y), VARG(col));

END_METHOD

BEGIN_PROPERTY(CIMAGE_data)

  GB.ReturnPointer((void *)THIS_IMAGE->data);

END_PROPERTY

BEGIN_METHOD_VOID(CIMAGE_make_gray)

	IMAGE_make_gray(THIS_IMAGE);

END_METHOD

BEGIN_METHOD_VOID(CIMAGE_clear)

	IMAGE_delete(THIS_IMAGE);

END_METHOD

BEGIN_METHOD(CIMAGE_replace, GB_INTEGER src; GB_INTEGER dst; GB_BOOLEAN noteq)

	IMAGE_replace(THIS_IMAGE, VARG(src), VARG(dst), VARGOPT(noteq, FALSE));

END_METHOD

BEGIN_METHOD(CIMAGE_make_transparent, GB_INTEGER color)

	IMAGE_make_transparent(THIS_IMAGE, VARGOPT(color, 0xFFFFFF));

END_METHOD

BEGIN_METHOD(CIMAGE_colorize, GB_INTEGER color)

	IMAGE_colorize(THIS_IMAGE, VARG(color));

END_METHOD


BEGIN_METHOD(CIMAGE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CIMAGE *image;
  int x = VARGOPT(x, 0);
  int y = VARGOPT(y, 0);
  int w = VARGOPT(w, THIS_IMAGE->width);
  int h = VARGOPT(h, THIS_IMAGE->height);

  GB.New(POINTER(&image), GB.FindClass("Image"), NULL, NULL);

	IMAGE_create(&image->image, w, h, THIS_IMAGE->format);
  IMAGE_bitblt(&image->image, 0, 0, THIS_IMAGE, x, y, w, h);

  GB.ReturnObject(image);

END_METHOD

BEGIN_METHOD(CIMAGE_resize, GB_INTEGER width; GB_INTEGER height)

	GB_IMG tmp;
  int w = VARG(width);
  int h = VARG(height);

	if (w < 0) w = THIS_IMAGE->width;
	if (h < 0) h = THIS_IMAGE->height;

	//IMAGE_convert(THIS_IMAGE, IMAGE_get_default_format());
	//fprintf(stderr, "format = %d\n", THIS_IMAGE->format);
	tmp.ob = THIS_IMAGE->ob;
	IMAGE_create(&tmp, w, h, THIS_IMAGE->format);
  IMAGE_bitblt(&tmp, 0, 0, THIS_IMAGE, 0, 0, w, h);

  IMAGE_delete(THIS_IMAGE);
  *THIS_IMAGE = tmp;

END_METHOD

BEGIN_METHOD(CIMAGE_mirror, GB_BOOLEAN horz; GB_BOOLEAN vert)

  CIMAGE *image;

  GB.New(POINTER(&image), GB.FindClass("Image"), NULL, NULL);

	IMAGE_create(&image->image, THIS_IMAGE->width, THIS_IMAGE->height, THIS_IMAGE->format);
  IMAGE_mirror(THIS_IMAGE, &image->image, VARG(horz), VARG(vert));

  GB.ReturnObject(image);

END_METHOD

GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

	GB_CONSTANT("Standard", "i", 0),
	GB_CONSTANT("Premultiplied", "i", 1),

	GB_METHOD("_new", NULL, CIMAGE_new, "[(Width)i(Height)i(Color)i(Format)i]"),
	GB_METHOD("_free", NULL, CIMAGE_free, NULL),

	GB_METHOD("_get", "i", CIMAGE_get, "(X)i(Y)i"),
	GB_METHOD("_put", NULL, CIMAGE_put, "(Color)i(X)i(Y)i"),

  //GB_PROPERTY_READ("Format", "i", CIMAGE_format),
  GB_PROPERTY_READ("Width", "i", CIMAGE_width),
  GB_PROPERTY_READ("Height", "i", CIMAGE_height),
  GB_PROPERTY_READ("W", "i", CIMAGE_width),
  GB_PROPERTY_READ("H", "i", CIMAGE_height),
  GB_PROPERTY_READ("Depth", "i", CIMAGE_depth),
  GB_PROPERTY_READ("Data", "p", CIMAGE_data),
  
  GB_METHOD("Clear", NULL, CIMAGE_clear, NULL),
  GB_METHOD("Fill", NULL, CIMAGE_fill, "(Color)i"),
  GB_METHOD("MakeGray", NULL, CIMAGE_make_gray, NULL),
  GB_METHOD("MakeTransparent", NULL, CIMAGE_make_transparent, "[(Color)i]"),
  GB_METHOD("Replace", NULL, CIMAGE_replace, "(OldColor)i(NewColor)i[(NotEqual)b]"),
  GB_METHOD("Colorize", NULL, CIMAGE_colorize, "(Color)i"),
  
  GB_METHOD("Copy", "Image", CIMAGE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Resize", NULL, CIMAGE_resize, "(Width)i(Height)i"),

  GB_METHOD("Mirror", "Image", CIMAGE_mirror, "(Horizontal)b(Vertical)b"),

  //GB_METHOD("Draw", NULL, CIMAGE_draw, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  
  GB_END_DECLARE
};
	
