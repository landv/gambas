/*
 * c_color.c - gb.ncurses Color static class
 *
 * Copyright (C) 2012 Tobias Boege <tobias@gambas-buch.de>
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
#include "c_color.h"
#include "c_screen.h"

static int _color;
static int _pair;

/**
 * Colour initialisation
 */
void COLOR_init()
{
	start_color();
	use_default_colors();
}

/**
 * Initialise a colour
 * @index: the color number
 * @r: red
 * @g: green
 * @b: blue
 * Note that one gives colour amounts in floats 0.0 - 1.0
 */
int COLOR_setcolor(short index, float r, float g, float b)
{
	return init_color(index, r * 1000, g * 1000, b * 1000);
}

/**
 * Initialise one field of a colour
 * @index: color number
 * @val: value as float 0.0 - 1.0
 * @what: field number
 */
int COLOR_setcolor_one(short index, float val, int what)
{
	short r, g, b;
	float rf, gf, bf;

	color_content(index, &r, &g, &b);
	rf = ((float) r) / 1000;
	gf = ((float) g) / 1000;
	bf = ((float) b) / 1000;
	switch (what) {
		case 0:
			rf = val;
			break;
		case 1:
			gf = val;
			break;
		case 2:
			bf = val;
			break;
		default:
			return -1;
	}
	return COLOR_setcolor(index, rf, gf, bf);
}

/**
 * Initialise a colour pair
 * @index: colour pair index
 * @fg: foreground value
 * @bg: background value
 */
int COLOR_setpair(short index, short fg, short bg)
{
	/* FIXME: on my setup, giving -1 doesn't work after
	   use_default_colors(). It gives no error but does nothing. I
	   workaround this issue by assuming COLOR_BLACK and COLOR_WHITE as
	   defaults for back- and foreground respectively */
	if (fg == -1)
		fg = COLOR_WHITE;
	if (bg == -1)
		bg = COLOR_BLACK;

	if (index)
		init_pair(index, fg, bg);
	else
		assume_default_colors(fg, bg);
	return 0;
}

/**
 * Set either foreground or background of a colour pair
 * @index: colour pair
 * @val: colour
 * @what: SETPAIR_* constant to indicate what should be set
 */
int COLOR_setpair_one(short index, short val, int what)
{
	short f, b;

	pair_content(index, &f, &b);
	switch (what) {
		case SETPAIR_FORE:
			return COLOR_setpair(index, val, b);
		case SETPAIR_BACK:
			return COLOR_setpair(index, f, val);
	}
	return -1;
}

/**
 * Return the RGB contents of a colour
 * @color: colour value to examine
 * @r: pointer to variable to be containing red portion
 * @g: green
 * @b: blue
 * The @r, @g, @b values may be NULL in which case the value is discarded
 */
static int COLOR_content(short color, short *r, short *g, short *b)
{
	short ar, ag, ab;

	color_content(color, r ? r : &ar, g ? g : &ag, b ? b : &ab);
	return 0;
}

/*
 * Color static class
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

BEGIN_METHOD(Color_Set, GB_INTEGER color; GB_INTEGER r; GB_INTEGER g; GB_INTEGER b)

	if (!COLOR_VALID(VARG(color))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	init_color(VARG(color), VARG(r), VARG(g), VARG(b));
	REFRESH();

END_METHOD

/*
 * .ColorInfo virtual class
 */

BEGIN_PROPERTY(ColorInfo_Red)

	short red;

	if (READ_PROPERTY) {
		COLOR_content(_color, &red, NULL, NULL);
		GB.ReturnInteger(red);
		return;
	}
	COLOR_setcolor_one(_color, VPROP(GB_FLOAT), 0);
	REAL_REFRESH();

END_PROPERTY

BEGIN_PROPERTY(ColorInfo_Green)

	short green;

	if (READ_PROPERTY) {
		COLOR_content(_color, NULL, &green, NULL);
		GB.ReturnInteger(green);
		return;
	}
	COLOR_setcolor_one(_color, VPROP(GB_FLOAT), 1);

END_PROPERTY

BEGIN_PROPERTY(ColorInfo_Blue)

	short blue;

	if (READ_PROPERTY) {
		COLOR_content(_color, NULL, NULL, &blue);
		GB.ReturnInteger(blue);
		return;
	}
	COLOR_setcolor_one(_color, VPROP(GB_FLOAT), 2);

END_PROPERTY

/*
 * Pair static class
 */

BEGIN_PROPERTY(Pair_Count)

	GB.ReturnInteger(COLOR_PAIRS);

END_PROPERTY

BEGIN_METHOD(Pair_get, GB_INTEGER index)

	if (!PAIR_VALID(VARG(index))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	_pair = VARG(index);
	RETURN_SELF();

END_METHOD

/*
 * .PairInfo virtual class
 */

BEGIN_PROPERTY(PairInfo_Background)

	short f, b;

	pair_content(_pair, &f, &b);
	if (READ_PROPERTY) {
		GB.ReturnInteger(b);
		return;
	}
	if (!COLOR_VALID(VPROP(GB_INTEGER))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	b = VPROP(GB_INTEGER);
	COLOR_setpair(_pair, f, b);
	REAL_REFRESH();


END_PROPERTY

BEGIN_PROPERTY(PairInfo_Foreground)

	short f, b;

	pair_content(_pair, &f, &b);
	if (READ_PROPERTY) {
		GB.ReturnInteger(f);
		return;
	}
	if (!COLOR_VALID(VPROP(GB_INTEGER))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	f = VPROP(GB_INTEGER);
	COLOR_setpair(_pair, f, b);
	REAL_REFRESH();

END_PROPERTY

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

GB_DESC CColorInfoDesc[] = {
	GB_DECLARE(".ColorInfo", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY("Red", "i", ColorInfo_Red),
	GB_STATIC_PROPERTY("Green", "i", ColorInfo_Green),
	GB_STATIC_PROPERTY("Blue", "i", ColorInfo_Blue),

	GB_END_DECLARE
};

GB_DESC CPairDesc[] = {
	GB_DECLARE("Pair", 0),
	GB_NOT_CREATABLE(),

	GB_STATIC_PROPERTY_READ("Count", "i", Pair_Count),

	GB_STATIC_METHOD("_get", ".PairInfo", Pair_get, "(Index)i"),

	GB_END_DECLARE
};

GB_DESC CPairInfoDesc[] = {
	GB_DECLARE(".PairInfo", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY("Background", "i", PairInfo_Background),
	GB_STATIC_PROPERTY("Foreground", "i", PairInfo_Foreground),

	GB_END_DECLARE
};
