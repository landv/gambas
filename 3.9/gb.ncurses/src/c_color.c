/*
 * c_color.c - gb.ncurses Color static class
 *
 * Copyright (C) 2012/3 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define __C_COLOR_C

#include <ncurses.h>

#include "../gambas.h"

#include "main.h"
#include "c_screen.h"

#define PAIR_VALID(p)		(p >= 0 && p < COLOR_PAIRS)
#define COLOR_VALID(c)		(c >= -1 && c < COLORS)

static int _color;
static short colors[] = {
	COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
	COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
};

void COLOR_init()
{
	start_color();
	//use_default_colors();

	/*
	 * Initialise all possible pairs
	 */
#define ARRAY_NUM(arr)	(sizeof(arr) / sizeof(arr[0]))
	int i, j, n;

	for (n = 0, i = 0; i < ARRAY_NUM(colors); i++)
		for (j = 0; j < ARRAY_NUM(colors); j++)
			init_pair(++n, colors[i], colors[j]);
}

/*
 * Color
 */

BEGIN_PROPERTY(Color_Available)

	GB.ReturnBoolean(has_colors());

END_PROPERTY

BEGIN_PROPERTY(Color_CanChange)

	GB.ReturnBoolean(can_change_color());

END_PROPERTY

BEGIN_PROPERTY(Color_Count)

	GB.ReturnInteger(COLORS);

END_PROPERTY

BEGIN_METHOD(Color_get, GB_INTEGER index)

	if (!COLOR_VALID(VARG(index))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	_color = VARG(index);
	RETURN_SELF();

END_METHOD

int CCOLOR_setcolor(short index, float r, float g, float b)
{
	return init_color(index, r * 1000, g * 1000, b * 1000);
}

BEGIN_METHOD(Color_Set, GB_INTEGER color; GB_FLOAT r; GB_FLOAT g;GB_FLOAT b)

	if (!COLOR_VALID(VARG(color))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	CCOLOR_setcolor(VARG(color), VARG(r), VARG(g), VARG(b));
	REAL_REFRESH();

END_METHOD

GB_DESC CColorDesc[] = {
	GB_DECLARE("Color", 0),
	GB_NOT_CREATABLE(),

	GB_CONSTANT("Black", "i", COLOR_BLACK),
	GB_CONSTANT("Red", "i", COLOR_RED),
	GB_CONSTANT("Green", "i", COLOR_GREEN),
	GB_CONSTANT("Yellow", "i", COLOR_YELLOW),
	GB_CONSTANT("Blue", "i", COLOR_BLUE),
	GB_CONSTANT("Magenta", "i", COLOR_MAGENTA),
	GB_CONSTANT("Cyan", "i", COLOR_CYAN),
	GB_CONSTANT("White", "i", COLOR_WHITE),

	GB_STATIC_PROPERTY_READ("Available", "b", Color_Available),
	GB_STATIC_PROPERTY_READ("CanChange", "b", Color_CanChange),
	GB_STATIC_PROPERTY_READ("Count", "i", Color_Count),

	GB_STATIC_METHOD("_get", ".ColorInfo", Color_get, "(Index)i"),
	GB_STATIC_METHOD("Set", NULL, Color_Set, "(Color)i(R)i(G)i(B)i"),

	GB_END_DECLARE
};

/*
 * .ColorInfo
 */

enum {
	COLOR_R,
	COLOR_G,
	COLOR_B
};

static int CCOLOR_content(short color, float *r, float *g, float *b)
{
	short ar, ag, ab;

	color_content(color, &ar, &ag, &ab);
	if (r)
		*r = (float) ar / 1000;
	if (g)
		*g = (float) ag / 1000;
	if (b)
		*b = (float) ab / 1000;
	return 0;
}

int CCOLOR_setcolor_one(short index, float val, int which)
{
	short r, g, b;
	float rf, gf, bf;

	color_content(index, &r, &g, &b);
	rf = ((float) r) / 1000;
	gf = ((float) g) / 1000;
	bf = ((float) b) / 1000;
	switch (which) {
		case COLOR_R:
			rf = val;
			break;
		case COLOR_G:
			gf = val;
			break;
		case COLOR_B:
			bf = val;
			break;
		default:
			return -1;
	}
	return CCOLOR_setcolor(index, rf, gf, bf);
}

BEGIN_PROPERTY(ColorInfo_Red)

	float red;

	if (READ_PROPERTY) {
		CCOLOR_content(_color, &red, NULL, NULL);
		GB.ReturnFloat(red);
		return;
	}
	CCOLOR_setcolor_one(_color, VPROP(GB_FLOAT), COLOR_R);
	REAL_REFRESH();

END_PROPERTY

BEGIN_PROPERTY(ColorInfo_Green)

	float green;

	if (READ_PROPERTY) {
		CCOLOR_content(_color, NULL, &green, NULL);
		GB.ReturnFloat(green);
		return;
	}
	CCOLOR_setcolor_one(_color, VPROP(GB_FLOAT), COLOR_G);

END_PROPERTY

BEGIN_PROPERTY(ColorInfo_Blue)

	float blue;

	if (READ_PROPERTY) {
		CCOLOR_content(_color, NULL, NULL, &blue);
		GB.ReturnFloat(blue);
		return;
	}
	CCOLOR_setcolor_one(_color, VPROP(GB_FLOAT), COLOR_B);

END_PROPERTY

GB_DESC CColorInfoDesc[] = {
	GB_DECLARE(".ColorInfo", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY("Red", "i", ColorInfo_Red),
	GB_STATIC_PROPERTY("Green", "i", ColorInfo_Green),
	GB_STATIC_PROPERTY("Blue", "i", ColorInfo_Blue),

	GB_END_DECLARE
};

/*
 * Pair
 */

BEGIN_PROPERTY(Pair_Count)

	GB.ReturnInteger(COLOR_PAIRS);

END_PROPERTY

short CPAIR_get(short fg, short bg)
{
	short i, j;
	int n;

	i = j = -1;
	for (n = 0; n < ARRAY_NUM(colors); n++) {
		if (colors[n] == fg)
			i = fg;
		if (colors[n] == bg)
			j = bg;
		if (i != -1 && j != -1)
			break;
	}
	if (n == ARRAY_NUM(colors))
		return -1;
	/* See COLOR_init() */
	return i * ARRAY_NUM(colors) + j + 1;
}

BEGIN_METHOD(Pair_get, GB_INTEGER fore; GB_INTEGER back)

	short pairn = CPAIR_get(VARG(fore), VARG(back));

	if (pairn == -1) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	GB.ReturnInteger(pairn);

END_METHOD

GB_DESC CPairDesc[] = {
	GB_DECLARE("Pair", 0),
	GB_NOT_CREATABLE(),

	GB_STATIC_PROPERTY_READ("Count", "i", Pair_Count),

	GB_STATIC_METHOD("_get", "i", Pair_get, "(Fore)i(Back)i"),

	GB_END_DECLARE
};
