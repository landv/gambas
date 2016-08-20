/*
 * c_key.c - gb.ncurses Key static class
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

#define __C_KEY_C

#include <ncurses.h>

#include "../gambas.h"

#include "main.h"
#include "c_key.h"

BEGIN_METHOD(Key_get, GB_STRING key)

	GB.ReturnInteger((int) *STRING(key));

END_METHOD

GB_DESC CKeyDesc[] = {
	GB_DECLARE("Key", 0),
	GB_NOT_CREATABLE(),

	GB_CONSTANT("Return", "i", (int) '\n'),
	GB_CONSTANT("Esc", "i", (int) '\x1b'),

	GB_CONSTANT("Break", "i", KEY_BREAK),
	GB_CONSTANT("Home", "i", KEY_HOME),
	GB_CONSTANT("Backspace", "i", KEY_BACKSPACE),
	GB_CONSTANT("PageUp", "i", KEY_PPAGE),
	GB_CONSTANT("PageDown", "i", KEY_NPAGE),
	GB_CONSTANT("Enter", "i", KEY_ENTER),

	/* Arrow Keys */
	GB_CONSTANT("Left", "i", KEY_LEFT),
	GB_CONSTANT("Right", "i", KEY_RIGHT),
	GB_CONSTANT("Up", "i", KEY_UP),
	GB_CONSTANT("Down", "i", KEY_DOWN),

	/* F keys */
	GB_CONSTANT("F1", "i", KEY_F(1)),
	GB_CONSTANT("F2", "i", KEY_F(2)),
	GB_CONSTANT("F3", "i", KEY_F(3)),
	GB_CONSTANT("F4", "i", KEY_F(4)),
	GB_CONSTANT("F5", "i", KEY_F(5)),
	GB_CONSTANT("F6", "i", KEY_F(6)),
	GB_CONSTANT("F7", "i", KEY_F(7)),
	GB_CONSTANT("F8", "i", KEY_F(8)),
	GB_CONSTANT("F9", "i", KEY_F(9)),
	GB_CONSTANT("F10", "i", KEY_F(10)),
	GB_CONSTANT("F11", "i", KEY_F(11)),
	GB_CONSTANT("F12", "i", KEY_F(12)),

	/* ncurses.h is full of other ones. Just tell me what you need. */

	GB_STATIC_METHOD("_get", "i", Key_get, "(Key)s"),

	GB_END_DECLARE
};
