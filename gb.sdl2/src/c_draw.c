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
#include "c_draw.h"

#define DRAW_STACK_MAX 8
static CDRAW _draw_stack[DRAW_STACK_MAX];
static CDRAW *_draw_current = NULL;

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

	if (GB.Is(device, CLASS_Window)) 
	{
		THIS->device = device;
		THIS->renderer = ((CWINDOW *)device)->renderer;
		GB.Ref(THIS->device);
		return;
	}
	
	GB.Error("Unsupported device");
}

void DRAW_end(void)
{
	if (!THIS)
		return;

	SDL_RenderPresent(RENDERER);
	
	GB.Unref(POINTER(&THIS->device));
	THIS->device = NULL;
	
	if (THIS == _draw_stack)
		THIS = NULL;
	else
		THIS--;
}

//-------------------------------------------------------------------------

BEGIN_METHOD(Draw_Begin, GB_OBJECT device)

	void *device = VARG(device);
	DRAW_begin(device);

END_METHOD

BEGIN_METHOD_VOID(Draw_End)

	DRAW_end();

END_METHOD

static GB_COLOR get_background()
{
	uchar r, g, b, a;
	SDL_GetRenderDrawColor(RENDERER, &r, &g, &b, &a);
	return GB_COLOR_MAKE(r, g, b, a);
}

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

BEGIN_METHOD(Draw_Clear, GB_INTEGER col)

	CHECK_DEVICE();
	
	set_background(VARGOPT(col, 0));
	SDL_RenderClear(RENDERER);

END_METHOD

BEGIN_PROPERTY(Draw_Background)

	CHECK_DEVICE();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(get_background());
	else
		set_background(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD(Draw_Rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER col)

	SDL_Rect rect;
	
	CHECK_DEVICE();
	
	rect.x = VARG(x);
	rect.y = VARG(y);
	rect.w = VARG(w);
	rect.h = VARG(h);
	
	set_background(VARG(col));
	SDL_RenderDrawRect(RENDERER, &rect);
	
END_METHOD

BEGIN_METHOD(Draw_FillRect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER col)

	SDL_Rect rect;
	
	CHECK_DEVICE();
	
	rect.x = VARG(x);
	rect.y = VARG(y);
	rect.w = VARG(w);
	rect.h = VARG(h);
	
	set_background(VARG(col));
	SDL_RenderFillRect(RENDERER, &rect);
	
END_METHOD

BEGIN_METHOD(Draw_Line, GB_INTEGER x1; GB_INTEGER y1; GB_INTEGER x2; GB_INTEGER y2; GB_INTEGER col)

	CHECK_DEVICE();
	
	set_background(VARG(col));
	SDL_RenderDrawLine(RENDERER, VARG(x1), VARG(y1), VARG(x2), VARG(y2));
	
END_METHOD

BEGIN_METHOD(Draw_Point, GB_INTEGER x; GB_INTEGER y; GB_INTEGER col)

	CHECK_DEVICE();
	
	set_background(VARG(col));
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
	
	set_background(VARG(col));
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
	
	set_background(VARG(col));
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
	
	set_background(VARG(col));
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
	
	set_background(VARG(col));
	SDL_RenderFillRects(RENDERER, (SDL_Rect *)GB.Array.Get(rects, 0), n);
	
END_METHOD


//-------------------------------------------------------------------------


GB_DESC DrawDesc[] =
{
	GB_DECLARE_STATIC("Draw"),
	
	GB_STATIC_METHOD("Begin", NULL, Draw_Begin, "(Device)o"),
	GB_STATIC_METHOD("End", NULL, Draw_End, NULL),
	
	GB_STATIC_METHOD("Clear", NULL, Draw_Clear, "[(Color)i]"),
	GB_STATIC_METHOD("Rect", NULL, Draw_Rect, "(X)i(Y)i(Width)i(Height)i(Color)i"),
	GB_STATIC_METHOD("Rects", NULL, Draw_Rects, "(Rectangles)Integer[];(Color)i"),
	GB_STATIC_METHOD("FillRect", NULL, Draw_FillRect, "(X)i(Y)i(Width)i(Height)i(Color)i"),
	GB_STATIC_METHOD("FillRects", NULL, Draw_FillRects, "(Rectangles)Integer[];(Color)i"),
	GB_STATIC_METHOD("Line", NULL, Draw_Line, "(X1)i(Y1)i(X2)i(Y2)i(Color)i"),
	GB_STATIC_METHOD("Lines", NULL, Draw_Lines, "(Lines)Integer[];(Color)i"),
	GB_STATIC_METHOD("Point", NULL, Draw_Point, "(X)i(Y)i(Color)i"),
	GB_STATIC_METHOD("Points", NULL, Draw_Points, "(Points)Integer[];(Color)i"),
	
	GB_STATIC_PROPERTY("Background", "i", Draw_Background),
	
	GB_END_DECLARE
};
