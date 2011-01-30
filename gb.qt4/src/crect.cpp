/***************************************************************************

	crect.cpp

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

#define __CRECT_CPP

#include "gb_common.h"
#include "crect.h"

static void normalize(CRECT *_object)
{
	if (THIS->w < 0)
	{
		THIS->w = (- THIS->w);
		THIS->x -= THIS->w;
	}

	if (THIS->h < 0)
	{
		THIS->h = (- THIS->h);
		THIS->y -= THIS->h;
	}
}

BEGIN_METHOD(Rect_new, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	if (!MISSING(x) && !MISSING(y) && !MISSING(w) && !MISSING(h))
	{
		THIS->x = VARG(x);
		THIS->y = VARG(y);
		THIS->w = VARG(w);
		THIS->h = VARG(h);
		normalize(THIS);
	}

END_METHOD

BEGIN_PROPERTY(Rect_X)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->x);
	else
		THIS->x = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(Rect_Y)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->y);
	else
		THIS->y = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(Rect_Width)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->w);
	else
	{
		THIS->w = VPROP(GB_INTEGER);
		normalize(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(Rect_Height)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->h);
	else
	{
		THIS->h = VPROP(GB_INTEGER);
		normalize(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(Rect_Left)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->x);
	else
	{
		int dx = VPROP(GB_INTEGER) - THIS->x;
		if (dx > THIS->w)
			dx = THIS->w;
		
		THIS->x += dx;
		THIS->w -= dx;
	}

END_PROPERTY

BEGIN_PROPERTY(Rect_Top)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->y);
	else
	{
		int dy = VPROP(GB_INTEGER) - THIS->y;
		if (dy > THIS->h)
			dy = THIS->h;
		
		THIS->y += dy;
		THIS->h -= dy;
	}

END_PROPERTY

BEGIN_PROPERTY(Rect_Right)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->x + THIS->w);
	else
	{
		int x2 = VPROP(GB_INTEGER);
		if (x2 < THIS->x)
			x2 = THIS->x;
		
		THIS->w = x2 - THIS->x;
	}

END_PROPERTY

BEGIN_PROPERTY(Rect_Bottom)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->y + THIS->h);
	else
	{
		int y2 = VPROP(GB_INTEGER);
		if (y2 < THIS->y)
			y2 = THIS->y;
		
		THIS->h = y2 - THIS->y;
	}

END_PROPERTY

BEGIN_METHOD_VOID(Rect_Clear)

	THIS->x = THIS->y = THIS->w = THIS->h = 0;

END_METHOD

BEGIN_METHOD_VOID(Rect_IsVoid)

	GB.ReturnBoolean(THIS->w <= 0 || THIS->h <= 0);

END_METHOD

BEGIN_METHOD_VOID(Rect_Copy)

	CRECT *copy;
	
	GB.New(POINTER(&copy), GB.FindClass("Rect"), NULL, 0);
	copy->x = THIS->x;
	copy->y = THIS->y;
	copy->w = THIS->w;
	copy->h = THIS->h;
	GB.ReturnObject(copy);

END_METHOD

BEGIN_METHOD(Rect_Move, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	THIS->x = VARG(x);
	THIS->y = VARG(y);
	if (!MISSING(w) && !MISSING(h))
	{
		THIS->w = VARG(w);
		THIS->h = VARG(h);
		normalize(THIS);
	}

END_METHOD

BEGIN_METHOD(Rect_Resize, GB_INTEGER w; GB_INTEGER h)

	THIS->w = VARG(w);
	THIS->h = VARG(h);
	normalize(THIS);

END_METHOD

BEGIN_METHOD(Rect_Translate, GB_INTEGER dx; GB_INTEGER dy)

	THIS->x += VARG(dx);
	THIS->y += VARG(dy);

END_METHOD

BEGIN_METHOD(Rect_Union, GB_OBJECT rect)

	CRECT *dest;
	int x, y, w, h;
	CRECT *rect = (CRECT *)VARG(rect);
	
	if (GB.CheckObject(rect))
		return;

	GB.New(POINTER(&dest), GB.FindClass("Rect"), NULL, 0);
	
	x = Min(THIS->x, rect->x);
	y = Min(THIS->y, rect->y);
	w = Max(THIS->x + THIS->w, rect->x + rect->w) - x;
	h = Max(THIS->y + THIS->h, rect->y + rect->h) - y;
	
	dest->x = x;
	dest->y = y;
	dest->w = w;
	dest->h = h;
	
	GB.ReturnObject(dest);

END_METHOD

BEGIN_METHOD(Rect_Intersection, GB_OBJECT rect)

	CRECT *dest;
	int x, y, x2, y2;
	CRECT *rect = (CRECT *)VARG(rect);
	
	if (GB.CheckObject(rect))
		return;

	x = Max(THIS->x, rect->x);
	y = Max(THIS->y, rect->y);
	x2 = Min(THIS->x + THIS->w, rect->x + rect->w);
	y2 = Min(THIS->y + THIS->h, rect->y + rect->h);

	if (x2 > x && y2 > y)
	{
		GB.New(POINTER(&dest), GB.FindClass("Rect"), NULL, 0);
	
		dest->x = x;
		dest->y = y;
		dest->w = x2 - x;
		dest->h = y2 - y;
		
		GB.ReturnObject(dest);
	}
	else
	{
		GB.ReturnNull();
	}

END_METHOD

BEGIN_METHOD(Rect_Contains, GB_INTEGER x; GB_INTEGER y)

	int x = VARG(x);
	int y = VARG(y);
	
	GB.ReturnBoolean(x >= THIS->x && x < (THIS->x + THIS->w) && y >= THIS->y && y < (THIS->y + THIS->h));

END_METHOD

BEGIN_METHOD(Rect_Adjust, GB_INTEGER left; GB_INTEGER top; GB_INTEGER right; GB_INTEGER bottom)

	int left = VARG(left);
	int top = VARGOPT(top, left);
	int right = VARGOPT(right, left);
	int bottom = VARGOPT(right, top);

	THIS->x += left;
	THIS->w -= (left + right);
	THIS->y += top;
	THIS->h -= (top + bottom);
	
	if (THIS->w < 1 || THIS->h < 1)
		THIS->w = THIS->h = 0;
	
END_METHOD

GB_DESC RectDesc[] =
{
	GB_DECLARE("Rect", sizeof(CRECT)),

	GB_METHOD("_new", NULL, Rect_new, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_PROPERTY("X", "i", Rect_X),
	GB_PROPERTY("Y", "i", Rect_Y),
	GB_PROPERTY("W", "i", Rect_Width),
	GB_PROPERTY("H", "i", Rect_Height),
	GB_PROPERTY("Width", "i", Rect_Width),
	GB_PROPERTY("Height", "i", Rect_Height),
	GB_PROPERTY("Left", "i", Rect_Left),
	GB_PROPERTY("Top", "i", Rect_Top),
	GB_PROPERTY("Right", "i", Rect_Right),
	GB_PROPERTY("Bottom", "i", Rect_Bottom),
	
	GB_METHOD("Clear", NULL, Rect_Clear, NULL),
	GB_METHOD("IsVoid", "b", Rect_IsVoid, NULL),
	GB_METHOD("Copy", "Rect", Rect_Copy, NULL),
	GB_METHOD("Move", NULL, Rect_Move, "(X)i(Y)i[(Width)i(Height)i]"),
	GB_METHOD("Resize", NULL, Rect_Resize, "(Width)i(Height)i"),
	GB_METHOD("Translate", NULL, Rect_Translate, "(DX)i(DY)i"),
	GB_METHOD("Union", "Rect", Rect_Union, "(Rect)Rect;"),
	GB_METHOD("Intersection", "Rect", Rect_Intersection, "(Rect)Rect;"),
	GB_METHOD("Contains", "b", Rect_Contains, "(X)i(Y)i"),
	GB_METHOD("Adjust", NULL, Rect_Adjust, "(Left)i[(Top)i(Right)i(Bottom)i]"),

	GB_END_DECLARE
};

