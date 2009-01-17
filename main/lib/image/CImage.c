/***************************************************************************

  CImage.c

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CIMAGE_C

#include "main.h"
#include "image.h"
#include "CImage.h"

BEGIN_METHOD(CIMAGE_new, GB_INTEGER w; GB_INTEGER h; GB_INTEGER format; GB_INTEGER col)

	int format = VARGOPT(format, 0) ? GB_IMAGE_RGBP : GB_IMAGE_RGBA;

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

GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

	GB_CONSTANT("Standard", "i", 0),
	GB_CONSTANT("Premultiplied", "i", 1),

	GB_METHOD("_new", NULL, CIMAGE_new, "[(Width)i(Height)i(Format)i(Color)i]"),
	GB_METHOD("_free", NULL, CIMAGE_free, NULL),

	GB_METHOD("_get", "i", CIMAGE_get, "(X)i(Y)i"),
	GB_METHOD("_put", NULL, CIMAGE_put, "(Color)i(X)i(Y)i"),

  //GB_PROPERTY_READ("Format", "i", CIMAGE_format),
  GB_PROPERTY_READ("Width", "i", CIMAGE_width),
  GB_PROPERTY_READ("Height", "i", CIMAGE_height),
  GB_PROPERTY_READ("W", "i", CIMAGE_width),
  GB_PROPERTY_READ("H", "i", CIMAGE_height),
  GB_PROPERTY_READ("Depth", "i", CIMAGE_depth),
  GB_PROPERTY_READ("Data", "i", CIMAGE_data),
  
  GB_METHOD("Clear", NULL, CIMAGE_clear, NULL),
  GB_METHOD("Fill", NULL, CIMAGE_fill, "(Color)i"),
  GB_METHOD("MakeGray", NULL, CIMAGE_make_gray, NULL),
  GB_METHOD("MakeTransparent", NULL, CIMAGE_make_transparent, "[(Color)i]"),
  GB_METHOD("Replace", NULL, CIMAGE_replace, "(OldColor)i(NewColor)i[(NotEqual)b]"),
  
  GB_END_DECLARE
};
	
