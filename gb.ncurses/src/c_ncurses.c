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

#if 0
/*
 * Signal handler for the SIGWINCH signal.
 * @signum: signal number given
 * This routine dispatches the Resize Event
 */
void nc_sigwinch_handler(int signum)
{
	/* TODO: I wonder if this works... */

	/* BM: Of course not! :-) You can't raise an event from a signal handler
	 * and moreover you have no sender! */

	if (signum == SIGWINCH) GB.Raise(NULL, EVENT_Resize, 0);
}
#endif

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

	/* global variable default setup */
	nc_cursor = CURSOR_VISIBLE;
	nc_input = INPUT_CBREAK;
	nc_echo = 0;
	/* accordingly... */
	curs_set(1);
	cbreak();
	noecho();

#if 0
	struct sigaction sa;

	sa.sa_handler = nc_sigwinch_handler;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	if (sigaction(SIGWINCH, &sa, NULL) == -1)
	{
		fprintf(stderr, "gb.ncurses: Could not install SIGWINCH signal handler");
	}
#endif

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


BEGIN_PROPERTY(NCurses_Cursor)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(nc_cursor);
		return;
	}

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

BEGIN_PROPERTY(NCurses_Input)

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

BEGIN_PROPERTY(NCurses_Echo)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(nc_echo);
		return;
	}

	nc_echo = VPROP(GB_BOOLEAN);
	if (nc_echo) echo();
	else noecho();

END_PROPERTY

BEGIN_PROPERTY(NCurses_Lines)

	GB.ReturnInteger(LINES);

END_PROPERTY

BEGIN_PROPERTY(NCurses_Cols)

	GB.ReturnInteger(COLS);

END_PROPERTY

BEGIN_METHOD_VOID(NCurses_exit)

	NCURSES_exit();

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

	GB_STATIC_PROPERTY("Cursor", "i", NCurses_Cursor),
	GB_STATIC_PROPERTY("Input", "i", NCurses_Input),
	GB_STATIC_PROPERTY("Echo", "b", NCurses_Echo),

	GB_STATIC_PROPERTY_READ("Lines", "i", NCurses_Lines),
	GB_STATIC_PROPERTY_READ("Cols", "i", NCurses_Cols),

	GB_STATIC_METHOD("_exit", NULL, NCurses_exit, NULL),

	GB_END_DECLARE
};
