/***************************************************************************

  CImage.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_FillRect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height; GB_INTEGER col)

	IMAGE_fill_rect(THIS_IMAGE, VARG(x), VARG(y), VARG(width), VARG(height), VARG(col), TRUE);
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_PaintRect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height; GB_INTEGER col)

	IMAGE_fill_rect(THIS_IMAGE, VARG(x), VARG(y), VARG(width), VARG(height), VARG(col), FALSE);
	GB.ReturnObject(THIS);

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

BEGIN_METHOD_VOID(Image_Desaturate)

	IMAGE_make_gray(THIS_IMAGE);
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD_VOID(CIMAGE_gray)

	fprintf(stderr, "warning: Image.Gray is deprecated, use Image.Desaturate instead.\n");
	Image_Desaturate(_object, _param);

END_METHOD

BEGIN_METHOD_VOID(Image_Clear)

	//IMAGE_fill(THIS_IMAGE, 0XFF000000);
	IMAGE_delete(THIS_IMAGE);

END_METHOD

BEGIN_METHOD(CIMAGE_replace, GB_INTEGER src; GB_INTEGER dst; GB_BOOLEAN noteq)

	IMAGE_replace(THIS_IMAGE, VARG(src), VARG(dst), VARGOPT(noteq, FALSE));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Erase, GB_INTEGER color)

	IMAGE_make_transparent(THIS_IMAGE, VARGOPT(color, 0xFFFFFF));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD_VOID(CIMAGE_transparent)

	fprintf(stderr, "warning: Image.Transparent is deprecated, use Image.Erase instead.\n");
	Image_Erase(_object, _param);

END_METHOD

BEGIN_METHOD(CIMAGE_colorize, GB_INTEGER color)

	IMAGE_colorize(THIS_IMAGE, VARG(color));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(CIMAGE_mask, GB_INTEGER color)

	IMAGE_mask(THIS_IMAGE, VARG(color));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(CIMAGE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CIMAGE *image;
  int x = VARGOPT(x, 0);
  int y = VARGOPT(y, 0);
  int w = VARGOPT(w, THIS_IMAGE->width);
  int h = VARGOPT(h, THIS_IMAGE->height);

  image = GB.New(GB.FindClass("Image"), NULL, NULL);

	IMAGE_create(&image->image, w, h, THIS_IMAGE->format);
  IMAGE_bitblt(&image->image, 0, 0, -1, -1, THIS_IMAGE, x, y, w, h);

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
  IMAGE_bitblt(&tmp, 0, 0, -1, -1, THIS_IMAGE, 0, 0, w, h);

  IMAGE_delete(THIS_IMAGE);
  *THIS_IMAGE = tmp;
  GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(CIMAGE_mirror, GB_BOOLEAN horz; GB_BOOLEAN vert)

  GB_IMG tmp;

  tmp.ob = THIS_IMAGE->ob;
  IMAGE_create(&tmp, THIS_IMAGE->width, THIS_IMAGE->height, THIS_IMAGE->format);
  IMAGE_mirror(THIS_IMAGE, &tmp, VARG(horz), VARG(vert));

  IMAGE_delete(THIS_IMAGE);
  *THIS_IMAGE = tmp;
  GB.ReturnObject(THIS);

END_METHOD

#if 0
BEGIN_METHOD(CIMAGE_transform, GB_FLOAT sx; GB_FLOAT sy; GB_FLOAT dx; GB_FLOAT dy)

	CIMAGE *image;
	int w, h;
	double dx = VARG(dx);
	double dy = VARG(dy);
	double f, s;
	
  GB.New(POINTER(&image), GB.FindClass("Image"), NULL, NULL);
	GB.ReturnObject(image);

	f = fabs(dx);
	if (f > fabs(dy))
		f = fabs(dy);
	
	s = 1 / sqrt(dx * dx + dy * dy);
	
	w = (int)(THIS_IMAGE->width * (1.0 + f) * s + 0.5);
	h = (int)(THIS_IMAGE->height * (1.0 + f) * s + 0.5);
	
	if (!w || !h)
		return;
	
	IMAGE_create(&image->image, w, h, THIS_IMAGE->format);
	IMAGE_transform(&image->image, THIS_IMAGE, VARG(sx), VARG(sy), VARG(dx), VARG(dy));

END_METHOD
#endif

BEGIN_METHOD(Image_DrawAlpha, GB_OBJECT image; GB_INTEGER x; GB_INTEGER y; GB_INTEGER srcx; GB_INTEGER srcy; GB_INTEGER srcw; GB_INTEGER srch)

	CIMAGE *image = VARG(image);
	
	if (GB.CheckObject(image))
		return;
	
  IMAGE_draw_alpha(THIS_IMAGE, VARGOPT(x, 0), VARGOPT(y, 0), &image->image, VARGOPT(srcx, 0), VARGOPT(srcy, 0), VARGOPT(srcw, -1), VARGOPT(srch, -1));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_DrawImage, GB_OBJECT image; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER srcx; GB_INTEGER srcy; GB_INTEGER srcw; GB_INTEGER srch)

	CIMAGE *image = VARG(image);
	
	if (GB.CheckObject(image))
		return;
	
  IMAGE_bitblt(THIS_IMAGE, VARGOPT(x, 0), VARGOPT(y, 0), VARGOPT(w, -1), VARGOPT(h, -1), &image->image, VARGOPT(srcx, 0), VARGOPT(srcy, 0), VARGOPT(srcw, -1), VARGOPT(srch, -1));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_PaintImage, GB_OBJECT image; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER srcx; GB_INTEGER srcy; GB_INTEGER srcw; GB_INTEGER srch)

	CIMAGE *image = VARG(image);
	
	if (GB.CheckObject(image))
		return;
	
  IMAGE_compose(THIS_IMAGE, VARGOPT(x, 0), VARGOPT(y, 0), VARGOPT(w, -1), VARGOPT(h, -1), &image->image, VARGOPT(srcx, 0), VARGOPT(srcy, 0), VARGOPT(srcw, -1), VARGOPT(srch, -1));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_PROPERTY(Image_Format)

	char *format;
	int fmt;

	if (READ_PROPERTY)
	{
		GB.ReturnConstZeroString(IMAGE_format_to_string(THIS_IMAGE->format));
	}
	else
	{
		format = GB.ToZeroString(PROP(GB_STRING));
		
		if (!strcasecmp(format, "BGRA"))
			fmt = GB_IMAGE_BGRA;
		else if (!strcasecmp(format, "BGRP"))
			fmt = GB_IMAGE_BGRP;
		else if (!strcasecmp(format, "RGBA"))
			fmt = GB_IMAGE_RGBA;
		else if (!strcasecmp(format, "RGBP"))
			fmt = GB_IMAGE_RGBP;
		else
		{
			GB.Error("Unsupported conversion");
			return;
		}
		
		IMAGE_convert(THIS_IMAGE, fmt);
	}

END_PROPERTY

BEGIN_PROPERTY(Image_Debug)

	if (READ_PROPERTY)
		GB.ReturnBoolean(IMAGE_debug);
	else
		IMAGE_debug = VPROP(GB_BOOLEAN);

END_PROPERTY

GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

	GB_CONSTANT("Standard", "i", 0),
	GB_CONSTANT("Premultiplied", "i", 1),

  GB_STATIC_PROPERTY("Debug", "b", Image_Debug),
  
	GB_METHOD("_new", NULL, CIMAGE_new, "[(Width)i(Height)i(Color)i(Format)i]"),
	GB_METHOD("_free", NULL, CIMAGE_free, NULL),

	GB_METHOD("_get", "i", CIMAGE_get, "(X)i(Y)i"),
	GB_METHOD("_put", NULL, CIMAGE_put, "(Color)i(X)i(Y)i"),

  GB_PROPERTY_READ("Width", "i", CIMAGE_width),
  GB_PROPERTY_READ("Height", "i", CIMAGE_height),
  GB_PROPERTY_READ("W", "i", CIMAGE_width),
  GB_PROPERTY_READ("H", "i", CIMAGE_height),
  GB_PROPERTY_READ("Depth", "i", CIMAGE_depth),
  GB_PROPERTY_READ("Data", "p", CIMAGE_data),
  GB_PROPERTY("Format", "s", Image_Format),
  
  GB_METHOD("Clear", NULL, Image_Clear, NULL),
  GB_METHOD("Fill", "Image", CIMAGE_fill, "(Color)i"),
  GB_METHOD("Gray", "Image", CIMAGE_gray, NULL),
  GB_METHOD("Transparent", "Image", CIMAGE_transparent, "[(Color)i]"),
  GB_METHOD("Desaturate", "Image", Image_Desaturate, NULL),
  GB_METHOD("Erase", "Image", Image_Erase, "[(Color)i]"),
  GB_METHOD("Replace", "Image", CIMAGE_replace, "(OldColor)i(NewColor)i[(NotEqual)b]"),
  GB_METHOD("Colorize", "Image", CIMAGE_colorize, "(Color)i"),
	GB_METHOD("Mask", "Image", CIMAGE_mask, "(Color)i"),
  
  GB_METHOD("Copy", "Image", CIMAGE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Resize", "Image", CIMAGE_resize, "(Width)i(Height)i"),

  GB_METHOD("Mirror", "Image", CIMAGE_mirror, "(Horizontal)b(Vertical)b"),
  
  GB_METHOD("FillRect", "Image", Image_FillRect, "(X)i(Y)i(Width)i(Height)i(Color)i"),
  GB_METHOD("PaintRect", "Image", Image_PaintRect, "(X)i(Y)i(Width)i(Height)i(Color)i"),
  //GB_METHOD("FillAlpha", "Image", Image_FillAlpha, "(X)i(Y)i(Width)i(Height)i(Alpha)i"),
	GB_METHOD("DrawAlpha", "Image", Image_DrawAlpha, "(Image)Image;[(X)i(Y)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

	//GB_METHOD("Transform", "Image", CIMAGE_transform, "(SrcX)f(SrcY)f(DepX)f(DepY)f"),
  GB_METHOD("DrawImage", "Image", Image_DrawImage, "(Image)Image;[(X)i(Y)i(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_METHOD("PaintImage", "Image", Image_PaintImage, "(Image)Image;[(X)i(Y)i(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  
  GB_END_DECLARE
};
	
