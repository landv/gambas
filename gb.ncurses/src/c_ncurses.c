/*
 * c_ncurses.c - gb.ncurses NCurses static class
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

#define __C_NCURSES_C

#include <stdio.h>
#include <signal.h>

#include <ncurses.h>

#include "c_ncurses.h"
#include "c_color.h"
#include "c_screen.h"
#include "main.h"

#define E_END		"Could not end ncurses"

static bool _init = FALSE;

bool NCURSES_running()
{
	return _init && (!isendwin() || stdscr);
}

void NCURSES_init(void)
{
	if (_init)
		return;

	initscr();
	keypad(stdscr, TRUE);

	/* Color setup */
	COLOR_init(); /* Color._init() would be called before the main hook, apparently */
	/* Screen setup */
	SCREEN_init();

	refresh();

	_init = TRUE;
}

void NCURSES_exit()
{
	if (_init)
	{
		endwin();
		_init = FALSE;
	}
}

BEGIN_METHOD_VOID(NCurses_exit)

	NCURSES_exit();

END_METHOD

GB_DESC CNCursesDesc[] =
{
	GB_DECLARE("NCurses", 0),
	GB_NOT_CREATABLE(),


	GB_STATIC_METHOD("_exit", NULL, NCurses_exit, NULL),

	GB_END_DECLARE
};
