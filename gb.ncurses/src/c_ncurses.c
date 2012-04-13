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
#include "main.h"

#define E_END		"Could not end ncurses"
#define E_UNSUPP	"Unsupported value"

#define CURSOR_HIDDEN	0
#define CURSOR_VISIBLE	1

#define INPUT_COOKED	0
#define INPUT_CBREAK	1
#define INPUT_RAW	2

static bool nc_cursor;
static int nc_input;
static int nc_echo;

DECLARE_EVENT(EVENT_Resize);

/*
 * Signal handler for the SIGWINCH signal.
 * @signum: signal number given
 * This routine dispatches the Resize Event
 */
void nc_sigwinch_handler(int signum)
{
	/* TODO: I wonder if this works... */
	if (signum == SIGWINCH) GB.Raise(NULL, EVENT_Resize, 0);
}

BEGIN_PROPERTY(CNCurses_cursor)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(nc_cursor);
		return;
	}

	/* Hide if setting cursor is not supported */
	switch (VPROP(GB_INTEGER))
	{
		case CURSOR_HIDDEN:
			curs_set(0);
			break;
		case CURSOR_VISIBLE:
			curs_set(1);
			break;
		default:
			GB.Error(E_UNSUPP);
			return;
	}
	nc_cursor = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CNCurses_input)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(nc_input);
		return;
	}

	switch (VPROP(GB_INTEGER))
	{
		case INPUT_COOKED:
			noraw();
			nocbreak();
			break;
		case INPUT_CBREAK:
			cbreak();
			break;
		case INPUT_RAW:
			raw();
			break;
		default:
			GB.Error(E_UNSUPP);
			return;
	}
	nc_input = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CNCurses_echo)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(nc_echo);
		return;
	}

	nc_echo = VPROP(GB_BOOLEAN);
	if (nc_echo) echo();
	else noecho();

END_PROPERTY

BEGIN_PROPERTY(CNCurses_lines)

	GB.ReturnInteger(LINES);

END_PROPERTY

BEGIN_PROPERTY(CNCurses_cols)

	GB.ReturnInteger(COLS);

END_PROPERTY

BEGIN_METHOD_VOID(CNCurses_init)

	struct sigaction sa;

	initscr();

	/* global variable default setup */
	nc_cursor = CURSOR_VISIBLE;
	nc_input = INPUT_CBREAK;
	nc_echo = 0;
	/* accordingly... */
	curs_set(1);
	cbreak();
	noecho();

	sa.sa_handler = nc_sigwinch_handler;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	if (sigaction(SIGWINCH, &sa, NULL) == -1)
	{
		fprintf(stderr, "gb.ncurses: Could not install SIGWINCH signal handler");
	}

	refresh();

END_METHOD

DECLARE_METHOD(CNCurses_off);

BEGIN_METHOD_VOID(CNCurses_exit)

	CALL_METHOD_VOID(CNCurses_off);

END_METHOD

BEGIN_METHOD_VOID(CNCurses_on)

	if (NCURSES_RUNNING) return;
	refresh();

END_METHOD

BEGIN_METHOD_VOID(CNCurses_off)

	if (!NCURSES_RUNNING) return;
	if (endwin() == ERR) GB.Error(E_END);

END_METHOD

GB_DESC CNCursesDesc[] =
{
	GB_DECLARE("NCurses", 0),
	GB_NOT_CREATABLE(),

	GB_CONSTANT("Hidden", "i", CURSOR_HIDDEN),
	GB_CONSTANT("Visible", "i", CURSOR_VISIBLE),

	GB_CONSTANT("Cooked", "i", INPUT_COOKED),
	GB_CONSTANT("CBreak", "i", INPUT_CBREAK),
	GB_CONSTANT("Raw", "i", INPUT_RAW),

	GB_STATIC_PROPERTY("Cursor", "i", CNCurses_cursor),
	GB_STATIC_PROPERTY("Input", "i", CNCurses_input),
	GB_STATIC_PROPERTY("Echo", "b", CNCurses_echo),

	GB_STATIC_PROPERTY_READ("Lines", "i", CNCurses_lines),
	GB_STATIC_PROPERTY_READ("Cols", "i", CNCurses_cols),

	GB_STATIC_METHOD("_init", NULL, CNCurses_init, NULL),
	GB_STATIC_METHOD("_exit", NULL, CNCurses_exit, NULL),
	GB_STATIC_METHOD("On", NULL, CNCurses_on, NULL),
	GB_STATIC_METHOD("Off", NULL, CNCurses_off, NULL),

	GB_END_DECLARE
};
