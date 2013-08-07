/***************************************************************************

	CImage.c

	(c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

static int _balance = 0;

static double _brightness = 0;
static double _contrast = 0;
static double _gamma = 0;
static double _hue = 0;
static double _saturation = 0;
static double _lightness = 0;

BEGIN_METHOD(Image_new, GB_INTEGER w; GB_INTEGER h; GB_INTEGER col; GB_INTEGER format)

	int format = IMAGE_get_default_format();
	
	if (VARGOPT(format, 0) == 1)
		format = GB_IMAGE_FMT_SET_PREMULTIPLIED(format);

	IMAGE_create(THIS_IMAGE, VARGOPT(w, 0), VARGOPT(h, 0), format);
	
	if (!MISSING(col))
		IMAGE_fill(THIS_IMAGE, VARG(col));

END_METHOD

BEGIN_METHOD_VOID(Image_free)

	IMAGE_delete(THIS_IMAGE);

END_METHOD

BEGIN_PROPERTY(Image_Width)

	GB.ReturnInteger(THIS_IMAGE->width);

END_PROPERTY

BEGIN_PROPERTY(Image_Height)

	GB.ReturnInteger(THIS_IMAGE->height);

END_PROPERTY

BEGIN_PROPERTY(Image_Depth)

	GB.ReturnInteger(32);

END_PROPERTY

BEGIN_METHOD(Image_Fill, GB_INTEGER col)

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

BEGIN_METHOD(Image_get, GB_INTEGER x; GB_INTEGER y)

	GB.ReturnInteger(IMAGE_get_pixel(THIS_IMAGE, VARG(x), VARG(y)));

END_METHOD

BEGIN_METHOD(Image_put, GB_INTEGER col; GB_INTEGER x; GB_INTEGER y)

	IMAGE_set_pixel(THIS_IMAGE, VARG(x), VARG(y), VARG(col));

END_METHOD

BEGIN_PROPERTY(Image_Data)

	GB.ReturnPointer((void *)THIS_IMAGE->data);

END_PROPERTY

BEGIN_METHOD_VOID(Image_Desaturate)

	IMAGE_make_gray(THIS_IMAGE);
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD_VOID(Image_Gray)

	fprintf(stderr, "warning: Image.Gray is deprecated, use Image.Desaturate instead.\n");
	Image_Desaturate(_object, _param);

END_METHOD

BEGIN_METHOD_VOID(Image_Clear)

	//IMAGE_fill(THIS_IMAGE, 0XFF000000);
	IMAGE_delete(THIS_IMAGE);

END_METHOD

BEGIN_METHOD(Image_Replace, GB_INTEGER src; GB_INTEGER dst; GB_BOOLEAN noteq)

	IMAGE_replace(THIS_IMAGE, VARG(src), VARG(dst), VARGOPT(noteq, FALSE));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Erase, GB_INTEGER color)

	IMAGE_make_transparent(THIS_IMAGE, VARGOPT(color, 0xFFFFFF));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD_VOID(Image_Transparent)

	fprintf(stderr, "warning: Image.Transparent is deprecated, use Image.Erase instead.\n");
	Image_Erase(_object, _param);

END_METHOD

BEGIN_METHOD(Image_Colorize, GB_INTEGER color)

	IMAGE_colorize(THIS_IMAGE, VARG(color));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Mask, GB_INTEGER color)

	IMAGE_mask(THIS_IMAGE, VARG(color));
	GB.ReturnObject(THIS);

END_METHOD

// Parameter correction
#define CHECK_PARAMETERS(dst, dx, dy, dw, dh, src, sx, sy, sw, sh) \
	if ( sw < 0 ) sw = src->width; \
	if ( sh < 0 ) sh = src->height; \
	if (dw < 0) dw = sw; \
	if (dh < 0) dh = sh; \
	if (dw != sw || dh != sh) \
	{ \
		GB.Error("Stretching images is not implemented in gb.image"); \
		return; \
	} \
	if ( sx < 0 ) { dx -= sx; dw += sx; sw += sx; sx = 0; } \
	if ( sy < 0 ) { dy -= sy; dh += sy; sh += sy; sy = 0; } \
	if ( dx < 0 ) { sx -= dx; sw += dx; dx = 0; } \
	if ( dy < 0 ) { sy -= dy; sh += dy; dy = 0; } \
	if ( (sx + sw) > src->width ) sw = src->width - sx; \
	if ( (sy + sh) > src->height ) sh = src->height - sy; \
	if ( (dx + sw) > dst->width ) sw = dst->width - dx; \
	if ( (dy + sh) > dst->height ) sh = dst->height - dy; \
	if (sw <= 0 || sh <= 0) \
		return;

BEGIN_METHOD(Image_Copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CIMAGE *image;
	int x = VARGOPT(x, 0);
	int y = VARGOPT(y, 0);
	int w = VARGOPT(w, THIS_IMAGE->width);
	int h = VARGOPT(h, THIS_IMAGE->height);

	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if ((x + w) > THIS_IMAGE->width) { w = THIS_IMAGE->width - x; }
	if ((y + h) > THIS_IMAGE->height) { h = THIS_IMAGE->height - y; }

	image = GB.New(GB.FindClass("Image"), NULL, NULL);

	IMAGE_create(&image->image, w, h, THIS_IMAGE->format);
	if (w > 0 && h > 0)
		IMAGE_bitblt(&image->image, 0, 0, -1, -1, THIS_IMAGE, x, y, w, h);

	GB.ReturnObject(image);

END_METHOD

BEGIN_METHOD(Image_Resize, GB_INTEGER width; GB_INTEGER height)

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

BEGIN_METHOD(Image_Mirror, GB_BOOLEAN horz; GB_BOOLEAN vert)

	GB_IMG tmp;

	tmp.ob = THIS_IMAGE->ob;
	IMAGE_create(&tmp, THIS_IMAGE->width, THIS_IMAGE->height, THIS_IMAGE->format);
	IMAGE_mirror(THIS_IMAGE, &tmp, VARG(horz), VARG(vert));
	IMAGE_delete(THIS_IMAGE);
	*THIS_IMAGE = tmp;
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD_VOID(Image_RotateLeft)

	GB_IMG tmp;

	tmp.ob = THIS_IMAGE->ob;
	IMAGE_create(&tmp, THIS_IMAGE->height, THIS_IMAGE->width, THIS_IMAGE->format);
	IMAGE_rotate(THIS_IMAGE, &tmp, TRUE);
	IMAGE_delete(THIS_IMAGE);
	*THIS_IMAGE = tmp;
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD_VOID(Image_RotateRight)

	GB_IMG tmp;

	tmp.ob = THIS_IMAGE->ob;
	IMAGE_create(&tmp, THIS_IMAGE->height, THIS_IMAGE->width, THIS_IMAGE->format);
	IMAGE_rotate(THIS_IMAGE, &tmp, FALSE);
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

BEGIN_METHOD(Image_Opacity, GB_FLOAT opacity)

	int alpha = VARG(opacity) * 255;
	
	if (alpha < 0)
		alpha = 0;
	else if (alpha > 255)
		alpha = 255;

	IMAGE_set_opacity(THIS_IMAGE, (uchar)alpha);
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Blur, GB_INTEGER radius)

	IMAGE_blur(THIS_IMAGE, VARGOPT(radius, 8));
	GB.ReturnObject(THIS);

END_METHOD

BEGIN_PROPERTY(Image_Pixels)

	GB_ARRAY array;
	int size;
	
	if (!GB_IMAGE_FMT_IS_32_BITS(THIS_IMAGE->format))
	{
		GB.Error("Image format must be 32 bits");
		return;
	}

	size = THIS_IMAGE->width * THIS_IMAGE->height;

	if (READ_PROPERTY)
	{
		GB.Array.New(&array, GB_T_INTEGER, size);
		IMAGE_get_pixels(THIS_IMAGE, GB.Array.Get(array, 0));
		GB.ReturnObject(array);
	}
	else
	{
		array = VPROP(GB_OBJECT);
		
		if (GB.CheckObject(array))
			return;
		
		if (GB.Array.Count(array) < size)
		{
			GB.Error("Not enough pixels");
			return;
		}
		
		IMAGE_set_pixels(THIS_IMAGE, GB.Array.Get(array, 0));
	}

END_PROPERTY

BEGIN_METHOD_VOID(Image_BeginBalance)

	_balance++;
	GB.ReturnObject(THIS);


END_METHOD

BEGIN_METHOD_VOID(Image_EndBalance)

	if (_balance <= 0)
	{
		GB.Error("Missing call to BeginBalance");
		return;
	}

	_balance--;

	if (_balance == 0)
		IMAGE_balance(THIS_IMAGE, _brightness * 255, _contrast * 255, _gamma * 255, _hue * 180, _saturation * 255, _lightness * 255);

	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Brightness, GB_FLOAT value)

	if (_balance)
		_brightness = VARG(value);
	else
		IMAGE_balance(THIS_IMAGE, VARG(value) * 255, 0, 0, 0, 0, 0);

	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Contrast, GB_FLOAT value)

	if (_balance)
		_contrast = VARG(value);
	else
		IMAGE_balance(THIS_IMAGE, 0, VARG(value) * 255, 0, 0, 0, 0);

	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Gamma, GB_FLOAT value)

	if (_balance)
		_gamma = VARG(value);
	else
		IMAGE_balance(THIS_IMAGE, 0, 0, VARG(value) * 255, 0, 0, 0);

	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Hue, GB_FLOAT value)

	if (_balance)
		_hue = VARG(value);
	else
		IMAGE_balance(THIS_IMAGE, 0, 0, 0, VARG(value) * 180, 0, 0);

	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Saturation, GB_FLOAT value)

	if (_balance)
		_saturation = VARG(value);
	else
		IMAGE_balance(THIS_IMAGE, 0, 0, 0, 0, VARG(value) * 255, 0);

	GB.ReturnObject(THIS);

END_METHOD

BEGIN_METHOD(Image_Lightness, GB_FLOAT value)

	if (_balance)
		_lightness = VARG(value);
	else
		IMAGE_balance(THIS_IMAGE, 0, 0, 0, 0, 0, VARG(value) * 255);

	GB.ReturnObject(THIS);

END_METHOD



//---------------------------------------------------------------------------

GB_DESC CImageDesc[] =
{
	GB_DECLARE("Image", sizeof(CIMAGE)),

	GB_CONSTANT("Standard", "i", 0),
	GB_CONSTANT("Premultiplied", "i", 1),

	GB_STATIC_PROPERTY("Debug", "b", Image_Debug),
	
	GB_METHOD("_new", NULL, Image_new, "[(Width)i(Height)i(Color)i(Format)i]"),
	GB_METHOD("_free", NULL, Image_free, NULL),

	GB_METHOD("_get", "i", Image_get, "(X)i(Y)i"),
	GB_METHOD("_put", NULL, Image_put, "(Color)i(X)i(Y)i"),

	GB_PROPERTY_READ("Width", "i", Image_Width),
	GB_PROPERTY_READ("Height", "i", Image_Height),
	GB_PROPERTY_READ("W", "i", Image_Width),
	GB_PROPERTY_READ("H", "i", Image_Height),
	GB_PROPERTY_READ("Depth", "i", Image_Depth),
	GB_PROPERTY_READ("Data", "p", Image_Data),
	GB_PROPERTY("Format", "s", Image_Format),
	GB_PROPERTY("Pixels", "Integer[]", Image_Pixels),
	
	GB_METHOD("Clear", NULL, Image_Clear, NULL),
	GB_METHOD("Fill", "Image", Image_Fill, "(Color)i"),
	GB_METHOD("Gray", "Image", Image_Gray, NULL),
	GB_METHOD("Transparent", "Image", Image_Transparent, "[(Color)i]"),
	GB_METHOD("Desaturate", "Image", Image_Desaturate, NULL),
	GB_METHOD("Erase", "Image", Image_Erase, "[(Color)i]"),
	GB_METHOD("Replace", "Image", Image_Replace, "(OldColor)i(NewColor)i[(NotEqual)b]"),
	GB_METHOD("Colorize", "Image", Image_Colorize, "(Color)i"),
	GB_METHOD("Mask", "Image", Image_Mask, "(Color)i"),
	GB_METHOD("Opacity", "Image", Image_Opacity, "(Opacity)f"),
	
	GB_METHOD("Copy", "Image", Image_Copy, "[(X)i(Y)i(Width)i(Height)i]"),
	GB_METHOD("Resize", "Image", Image_Resize, "(Width)i(Height)i"),

	GB_METHOD("Mirror", "Image", Image_Mirror, "(Horizontal)b(Vertical)b"),
	GB_METHOD("RotateLeft", "Image", Image_RotateLeft, NULL),
	GB_METHOD("RotateRight", "Image", Image_RotateRight, NULL),
	
	GB_METHOD("FillRect", "Image", Image_FillRect, "(X)i(Y)i(Width)i(Height)i(Color)i"),
	GB_METHOD("PaintRect", "Image", Image_PaintRect, "(X)i(Y)i(Width)i(Height)i(Color)i"),
	GB_METHOD("DrawAlpha", "Image", Image_DrawAlpha, "(Image)Image;[(X)i(Y)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

	GB_METHOD("DrawImage", "Image", Image_DrawImage, "(Image)Image;[(X)i(Y)i(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
	GB_METHOD("PaintImage", "Image", Image_PaintImage, "(Image)Image;[(X)i(Y)i(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
	
	GB_METHOD("Fuzzy", "Image", Image_Blur, "[(Radius)i]"),

	GB_METHOD("BeginBalance", "Image", Image_BeginBalance, NULL),
	GB_METHOD("Brightness", "Image", Image_Brightness, "(Brightness)f"),
	GB_METHOD("Contrast", "Image", Image_Contrast, "(Contrast)f"),
	GB_METHOD("Gamma", "Image", Image_Gamma, "(Gamma)f"),
	GB_METHOD("Hue", "Image", Image_Hue, "(Hue)f"),
	GB_METHOD("Saturation", "Image", Image_Saturation, "(Saturation)f"),
	GB_METHOD("Lightness", "Image", Image_Lightness, "(Lightness)f"),
	GB_METHOD("EndBalance", "Image", Image_EndBalance, NULL),

	GB_END_DECLARE
};
	
