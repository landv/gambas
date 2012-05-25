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

static int _index;
static int _color;

/**
 * Colour initialisation
 */
void COLOR_init()
{
	start_color();
	use_default_colors();
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

BEGIN_PROPERTY(Color_Capabilities)

	RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(Color_Colors)

	GB.ReturnInteger(COLORS);

END_PROPERTY

BEGIN_PROPERTY(Color_Pairs)

	GB.ReturnInteger(COLOR_PAIRS);

END_PROPERTY

BEGIN_METHOD(Color_get, GB_INTEGER index)

	if (!PAIR_VALID(VARG(index))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	_index = VARG(index);
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(Color_Define, GB_INTEGER color; GB_INTEGER r; GB_INTEGER g; GB_INTEGER b)

	if (!COLOR_VALID(VARG(color))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	init_color(VARG(color), VARG(r), VARG(g), VARG(b));

END_METHOD

BEGIN_METHOD(Color_Content, GB_INTEGER color)

	if (!COLOR_VALID(VARG(color))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	_color = VARG(color);
	RETURN_SELF();

END_METHOD

/*
 * .Color.Capabilities virtual class
 */

BEGIN_PROPERTY(ColorCapabilities_Change)

	GB.ReturnBoolean(can_change_color());

END_PROPERTY

BEGIN_PROPERTY(ColorCapabilities_Color)

	GB.ReturnBoolean(has_colors());

END_PROPERTY

/*
 * .Color.Pair virtual class
 */

BEGIN_PROPERTY(ColorPair_Background)

	short f, b;

	pair_content(_index, &f, &b);
	if (READ_PROPERTY) {
		GB.ReturnInteger(b);
		return;
	}
	if (!COLOR_VALID(VPROP(GB_INTEGER))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	b = VPROP(GB_INTEGER);
	COLOR_setpair(_index, f, b);
	REAL_REFRESH();


END_PROPERTY

BEGIN_PROPERTY(ColorPair_Foreground)

	short f, b;

	pair_content(_index, &f, &b);
	if (READ_PROPERTY) {
		GB.ReturnInteger(f);
		return;
	}
	if (!COLOR_VALID(VPROP(GB_INTEGER))) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	f = VPROP(GB_INTEGER);
	COLOR_setpair(_index, f, b);
	REAL_REFRESH();

END_PROPERTY

/*
 * .Color.Content virtual class
 */

BEGIN_PROPERTY(ColorContent_Red)

	short red;

	COLOR_content(_color, &red, NULL, NULL);
	GB.ReturnInteger(red);

END_PROPERTY

BEGIN_PROPERTY(ColorContent_Green)

	short green;

	COLOR_content(_color, NULL, &green, NULL);
	GB.ReturnInteger(green);

END_PROPERTY

BEGIN_PROPERTY(ColorContent_Blue)

	short blue;

	COLOR_content(_color, NULL, NULL, &blue);
	GB.ReturnInteger(blue);

END_PROPERTY

GB_DESC CColorDesc[] =
{
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

	GB_STATIC_PROPERTY_READ("Capabilities", ".Color.Capabilities", Color_Capabilities),
	GB_STATIC_PROPERTY_READ("Colors", "i", Color_Colors),
	GB_STATIC_PROPERTY_READ("Pairs", "i", Color_Pairs),

	GB_STATIC_METHOD("_get", ".Color.Pair", Color_get, "(Index)i"),
	GB_STATIC_METHOD("Define", NULL, Color_Define, "(Color)i(R)i(G)i(B)i"),
	GB_STATIC_METHOD("Content", ".Color.Content", Color_Content, "(Color)i"),

	GB_END_DECLARE
};

GB_DESC CColorCapabilitiesDesc[] =
{
	GB_DECLARE(".Color.Capabilities", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("Color", "b", ColorCapabilities_Color),
	GB_STATIC_PROPERTY_READ("Change", "b", ColorCapabilities_Change),

	GB_END_DECLARE
};

GB_DESC CColorPairDesc[] =
{
	GB_DECLARE(".Color.Pair", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY("Background", "i", ColorPair_Background),
	GB_STATIC_PROPERTY("Foreground", "i", ColorPair_Foreground),

	GB_END_DECLARE
};

GB_DESC CColorContentDesc[] =
{
	GB_DECLARE(".Color.Content", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("Red", "i", ColorContent_Red),
	GB_STATIC_PROPERTY_READ("Green", "i", ColorContent_Green),
	GB_STATIC_PROPERTY_READ("Blue", "i", ColorContent_Blue),

	GB_END_DECLARE
};
