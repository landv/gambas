/***************************************************************************

  c_draw.c

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_DRAW_C

#include "c_window.h"
#include "c_image.h"
#include "c_font.h"
#include "c_draw.h"

#define DRAW_STACK_MAX 8
static CDRAW _draw_stack[DRAW_STACK_MAX];
static CDRAW *_draw_current = NULL;
static CFONT *_default_font = NULL;

#define THIS _draw_current
#define RENDERER _draw_current->renderer

static bool check_device(void)
{
	if (THIS == NULL)
	{
		GB.Error("No device");
		return TRUE;
	}
	else
		return FALSE;
}

#define CHECK_DEVICE() if (check_device()) return

static CFONT *get_default_font()
{
	if (!_default_font)
	{
		_default_font = FONT_create();
		GB.Ref(_default_font);
	}

	return _default_font;
}

/*static GB_COLOR get_background()
{
	uchar r, g, b, a;
	SDL_GetRenderDrawColor(RENDERER, &r, &g, &b, &a);
	return GB_COLOR_MAKE(r, g, b, a);
}*/

static void set_background(GB_COLOR col)
{
	uchar r, g, b, a;

	GB_COLOR_SPLIT(col, r, g, b, a);
	SDL_SetRenderDrawColor(RENDERER, r, g, b, a);

	if (a != 255)
		SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_BLEND);
	else
		SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_NONE);
}

void DRAW_begin(void *device)
{
	if (THIS >= &_draw_stack[DRAW_STACK_MAX - 1]) 
	{
		GB.Error("Too many nested drawings");
		return;
	}

	if (GB.CheckObject(device))
		return;

	if (THIS == 0)
		THIS = _draw_stack;
	else
		THIS++;

	THIS->font = get_default_font();
	GB.Ref(THIS->font);

	if (GB.Is(device, CLASS_Window)) 
	{
		THIS->device = device;
		THIS->renderer = ((CWINDOW *)device)->renderer;
		GB.Ref(THIS->device);
		THIS->color = (GB_COLOR)0xFFFFFF;
		return;
	}
	
	GB.Error("Unsupported device");
}

void DRAW_end(void)
{
	if (!THIS)
		return;

	GB.Unref(POINTER(&THIS->device));
	THIS->device = NULL;
	GB.Unref(POINTER(&THIS->font));
	THIS->font = NULL;

	if (THIS == _draw_stack)
		THIS = NULL;
	else
		THIS--;
}

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(Draw_exit)

	if (_default_font)
		GB.Unref(POINTER(&_default_font));

END_METHOD

BEGIN_METHOD(Draw_Begin, GB_OBJECT device)

	void *device = VARG(device);
	DRAW_begin(device);

END_METHOD

BEGIN_METHOD_VOID(Draw_End)

	CHECK_DEVICE();
	DRAW_end();

END_METHOD

BEGIN_METHOD(Draw_Clear, GB_INTEGER col)

	CHECK_DEVICE();

	set_background(VARGOPT(col, 0));
	SDL_RenderClear(RENDERER);

END_METHOD

BEGIN_PROPERTY(Draw_Background)

	CHECK_DEVICE();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->color);
	else
		THIS->color = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_METHOD(Draw_Rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER col)

	SDL_Rect rect;
	
	CHECK_DEVICE();
	
	rect.x = VARG(x);
	rect.y = VARG(y);
	rect.w = VARG(w);
	rect.h = VARG(h);
	
	set_background(VARGOPT(col, THIS->color));
	SDL_RenderDrawRect(RENDERER, &rect);
	
END_METHOD

BEGIN_METHOD(Draw_FillRect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER col)

	SDL_Rect rect;
	
	CHECK_DEVICE();
	
	rect.x = VARG(x);
	rect.y = VARG(y);
	rect.w = VARG(w);
	rect.h = VARG(h);
	
	set_background(VARGOPT(col, THIS->color));
	SDL_RenderFillRect(RENDERER, &rect);
	
END_METHOD

BEGIN_METHOD(Draw_Line, GB_INTEGER x1; GB_INTEGER y1; GB_INTEGER x2; GB_INTEGER y2; GB_INTEGER col)

	CHECK_DEVICE();
	
	set_background(VARGOPT(col, THIS->color));
	SDL_RenderDrawLine(RENDERER, VARG(x1), VARG(y1), VARG(x2), VARG(y2));
	
END_METHOD

BEGIN_METHOD(Draw_Point, GB_INTEGER x; GB_INTEGER y; GB_INTEGER col)

	CHECK_DEVICE();
	
	set_background(VARGOPT(col, THIS->color));
	SDL_RenderDrawPoint(RENDERER, VARG(x), VARG(y));
	
END_METHOD

BEGIN_METHOD(Draw_Points, GB_OBJECT points; GB_INTEGER col)

	GB_ARRAY points;
	uint n;

	CHECK_DEVICE();
	
	points = (GB_ARRAY)VARG(points);
	if (GB.CheckObject(points))
		return;
	
	n = GB.Array.Count(points) / 2;
	if (n == 0)
		return;
	
	set_background(VARGOPT(col, THIS->color));
	SDL_RenderDrawPoints(RENDERER, (SDL_Point *)GB.Array.Get(points, 0), n);
	
END_METHOD

BEGIN_METHOD(Draw_Lines, GB_OBJECT points; GB_INTEGER col)

	GB_ARRAY points;
	uint n;

	CHECK_DEVICE();
	
	points = (GB_ARRAY)VARG(points);
	if (GB.CheckObject(points))
		return;
	
	n = GB.Array.Count(points) / 2;
	if (n == 0)
		return;
	
	set_background(VARGOPT(col, THIS->color));
	SDL_RenderDrawLines(RENDERER, (SDL_Point *)GB.Array.Get(points, 0), n);
	
END_METHOD

BEGIN_METHOD(Draw_Rects, GB_OBJECT rects; GB_INTEGER col)

	GB_ARRAY rects;
	uint n;

	CHECK_DEVICE();
	
	rects = (GB_ARRAY)VARG(rects);
	if (GB.CheckObject(rects))
		return;
	
	n = GB.Array.Count(rects) / 4;
	if (n == 0)
		return;
	
	set_background(VARGOPT(col, THIS->color));
	SDL_RenderDrawRects(RENDERER, (SDL_Rect *)GB.Array.Get(rects, 0), n);
	
END_METHOD

BEGIN_METHOD(Draw_FillRects, GB_OBJECT rects; GB_INTEGER col)

	GB_ARRAY rects;
	uint n;

	CHECK_DEVICE();
	
	rects = (GB_ARRAY)VARG(rects);
	if (GB.CheckObject(rects))
		return;
	
	n = GB.Array.Count(rects) / 4;
	if (n == 0)
		return;
	
	if (!MISSING(col)) set_background(VARG(col));
	SDL_RenderFillRects(RENDERER, (SDL_Rect *)GB.Array.Get(rects, 0), n);
	
END_METHOD

BEGIN_METHOD(Draw_Image, GB_OBJECT image; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_OBJECT src; GB_FLOAT opacity; GB_FLOAT angle)

	CIMAGE *image;
	SDL_Texture *texture;
	GEOM_RECT *src;
	SDL_Rect *rect;
	SDL_Rect dest;

	CHECK_DEVICE();

	image = VARG(image);
	if (GB.CheckObject(image))
		return;
	
	texture = IMAGE_get_texture(image, (CWINDOW *)THIS->device);
	
	dest.x = VARG(x);
	dest.y = VARG(y);
	dest.w = VARGOPT(w, image->img.width);
	dest.h = VARGOPT(h, image->img.height);
	
	src = VARGOPT(src, NULL);
	if (src)
		rect = (SDL_Rect *)&src->x;
	else
		rect = NULL;
	
	if (MISSING(opacity) && MISSING(angle))
		SDL_RenderCopy(RENDERER, texture, rect, &dest);
	else
	{
		SDL_SetTextureAlphaMod(texture, 255 - VARGOPT(opacity, 1.0) * 255);
		SDL_RenderCopyEx(RENDERER, texture, rect, &dest, VARGOPT(angle, 0.0), NULL, SDL_FLIP_NONE);
		SDL_SetTextureAlphaMod(texture, 255);
	}

END_METHOD

BEGIN_PROPERTY(Draw_Font)

	CHECK_DEVICE();

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->font);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->font));
		if (!THIS->font)
		{
			THIS->font = get_default_font();
			GB.Ref(THIS->font);
		}
	}

END_PROPERTY

BEGIN_METHOD(Draw_Text, GB_STRING text; GB_INTEGER x; GB_INTEGER y)

	SDL_Image *image;
	SDL_Texture *texture;
	SDL_Rect dest;
	uint r, g, b, a;

	CHECK_DEVICE();

	if (!LENGTH(text))
		return;

	dest.x = VARG(x);
	dest.y = VARG(y);

	image = FONT_render_text(THIS->font, (CWINDOW *)THIS->device, STRING(text), LENGTH(text), &dest.w, &dest.h);
	if (!image)
		return;

	texture = SDL_GetTextureFromImage(image, (CWINDOW *)THIS->device);
	if (image->surface)
	{
		SDL_FreeSurface(image->surface);
		image->surface = NULL;
	}

	GB_COLOR_SPLIT(THIS->color, r, g, b, a);
	SDL_SetTextureColorMod(texture, r, g, b);
	SDL_SetTextureAlphaMod(texture, a);

	SDL_RenderCopy(RENDERER, texture, NULL, &dest);

END_METHOD

//-------------------------------------------------------------------------


GB_DESC DrawDesc[] =
{
	GB_DECLARE_STATIC("Draw"),

	GB_STATIC_METHOD("_exit", NULL, Draw_exit, NULL),
	
	GB_STATIC_METHOD("Begin", NULL, Draw_Begin, "(Device)o"),
	GB_STATIC_METHOD("End", NULL, Draw_End, NULL),
	
	GB_STATIC_METHOD("Clear", NULL, Draw_Clear, "[(Color)i]"),
	GB_STATIC_METHOD("Rect", NULL, Draw_Rect, "(X)i(Y)i(Width)i(Height)i[(Color)i]"),
	GB_STATIC_METHOD("Rects", NULL, Draw_Rects, "(Rectangles)Integer[];[(Color)i]"),
	GB_STATIC_METHOD("FillRect", NULL, Draw_FillRect, "(X)i(Y)i(Width)i(Height)i[(Color)i]"),
	GB_STATIC_METHOD("FillRects", NULL, Draw_FillRects, "(Rectangles)Integer[];[(Color)i]"),
	GB_STATIC_METHOD("Line", NULL, Draw_Line, "(X1)i(Y1)i(X2)i(Y2)i[(Color)i]"),
	GB_STATIC_METHOD("Lines", NULL, Draw_Lines, "(Lines)Integer[];[(Color)i]"),
	GB_STATIC_METHOD("Point", NULL, Draw_Point, "(X)i(Y)i[(Color)i]"),
	GB_STATIC_METHOD("Points", NULL, Draw_Points, "(Points)Integer[];[(Color)i]"),
	GB_STATIC_METHOD("Image", NULL, Draw_Image, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(Source)Rect;(Opacity)f(Angle)f"),
	GB_STATIC_METHOD("Text", NULL, Draw_Text, "(Text)s(X)i(Y)i"),

	GB_STATIC_PROPERTY("Background", "i", Draw_Background),
	GB_STATIC_PROPERTY("Font", "Font", Draw_Font),

	GB_END_DECLARE
};
