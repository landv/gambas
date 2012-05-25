/*
 * c_screen.c - gb.ncurses Screen class
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

#define __C_SCREEN_C

#include <stdio.h>
#include <signal.h>

#include <ncurses.h>
#include <panel.h>

#include "gambas.h"

#include "c_screen.h"
#include "main.h"
#include "input.h"

#define THIS			((CSCREEN *) _object)
#define IS_BUFFERED		(THIS->buffered)

#define E_UNSUPP	"Unsupported value"

static bool _cursor;
static int _echo;

/* Currently active screen */
static CSCREEN *active = NULL;

GB_SIGNAL_CALLBACK *callback;

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Resize);

/*
 * Signal handler for the SIGWINCH signal.
 * @signum: signal number given
 * This routine dispatches the Resize Event
 */
void SCREEN_sigwinch_handler(int signum, intptr_t data)
{
	if (signum == SIGWINCH)
		GB.Raise(active, EVENT_Resize, 0);
}

/**
 * Set mode of @_cursor
 */
static int SCREEN_cursor_mode(int mode)
{
	switch (mode) {
		case CURSOR_HIDDEN:
			curs_set(0);
			break;
		case CURSOR_VISIBLE:
			curs_set(1);
			break;
		case CURSOR_VERY:
			curs_set(2);
			break;
		default:
			return -1;
	}
	_cursor = mode;
	return 0;
}

/**
 * Set mode of @_echo
 */
static int SCREEN_echo_mode(int mode)
{
	switch (mode) {
		case ECHO_NOECHO:
			noecho();
			break;
		case ECHO_ECHO:
			echo();
			break;
		default:
			return -1;
	}
	_echo = mode;
	return 0;
}

/**
 * Screen initialisation
 */
int SCREEN_init()
{
	/* Global variable default setup */
	SCREEN_cursor_mode(CURSOR_VISIBLE);
	SCREEN_echo_mode(ECHO_NOECHO);

	INPUT_init();

/*	callback = GB.Signal.Register(SIGSEGV,
		SCREEN_sigwinch_handler, (intptr_t) NULL);
*/
	return 0;
}

/**
 * Screen cleanup
 */
void SCREEN_exit()
{
	INPUT_exit();

/*	GB.Signal.Unregister(SIGWINCH, callback);
*/
}

/**
 * Get the active Screen
 * If @active is NULL, the default Screen will be made active and then returned
 */
CSCREEN *SCREEN_get_active()
{
	GB_CLASS screen_class;

	if (active)
		return active;
	screen_class = GB.FindClass("Screen");
	active = GB.AutoCreate(screen_class, 0);
	return active;
}

/**
 * Redraw the screen no matter what the buffer settings are.
 * Note that this function may not be used to return into ncurses mode once left.
 */
void SCREEN_real_refresh()
{
	if (!NCURSES_RUNNING)
		return;
	update_panels();
	doupdate();
}

/**
 * Refresh the screen. This respects the currently active buffering wishes
 */
void SCREEN_refresh(void *_object)
{
	if (!NCURSES_RUNNING)
		return;

	if (!_object)
		_object = SCREEN_get_active();

	if (!IS_BUFFERED)
		SCREEN_real_refresh();
}

/**
 * Let the specified screen raise its Read event. If the _object is NULL,
 * the currently active screen will raise.
 */
void SCREEN_raise_read(void *_object)
{
	if (!_object)
		GB.Raise(active, EVENT_Read, 0);
	else
		GB.Raise(_object, EVENT_Read, 0);
}

BEGIN_PROPERTY(Screen_Buffered)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(IS_BUFFERED);
		return;
	}
	THIS->buffered = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Screen_Cursor)

	if (READ_PROPERTY) {
		GB.ReturnInteger(_cursor);
		return;
	}

	if (SCREEN_cursor_mode(VPROP(GB_INTEGER)) == -1)
		GB.Error(E_UNSUPP);

END_PROPERTY

BEGIN_PROPERTY(Screen_Echo)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(_echo);
		return;
	}

	if (SCREEN_echo_mode(!!VPROP(GB_BOOLEAN)) == -1)
		GB.Error(E_UNSUPP);

END_PROPERTY

BEGIN_PROPERTY(Screen_Input)

	if (READ_PROPERTY) {
		GB.ReturnInteger(INPUT_mode(INPUT_RETURN));
		return;
	}

	if (INPUT_mode(VPROP(GB_INTEGER)) == -1)
		GB.Error(E_UNSUPP);

END_PROPERTY

BEGIN_PROPERTY(Screen_IsConsole)

	int fd = INPUT_consolefd();

	if (fd == -1) {
		GB.ReturnBoolean(FALSE);
		return;
	}
	close(fd);
	GB.ReturnBoolean(TRUE);

END_PROPERTY

BEGIN_PROPERTY(Screen_Repeater)

	if (READ_PROPERTY) {
		GB.ReturnInteger(INPUT_repeater_delay(REPEATER_RETURN));
		return;
	}
	if (INPUT_repeater_delay(VPROP(GB_INTEGER)) == -1) {
		GB.Error("Invalid value");
		return;
	}

END_PROPERTY

BEGIN_PROPERTY(Screen_Lines)

	GB.ReturnInteger(LINES);

END_PROPERTY

BEGIN_PROPERTY(Screen_Cols)

	GB.ReturnInteger(COLS);

END_PROPERTY

BEGIN_METHOD_VOID(Screen_new)

	active = THIS;

END_METHOD

BEGIN_METHOD_VOID(Screen_free)

	SCREEN_exit();

END_METHOD

BEGIN_METHOD_VOID(Screen_Refresh)

	SCREEN_real_refresh();

END_METHOD

GB_DESC CScreenDesc[] =
{
	GB_DECLARE("Screen", sizeof(CSCREEN)),
	GB_AUTO_CREATABLE(),

	GB_EVENT("Read", NULL, NULL, &EVENT_Read),
	GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

	GB_CONSTANT("Hidden", "i", CURSOR_HIDDEN),
	GB_CONSTANT("Visible", "i", CURSOR_VISIBLE),

	GB_CONSTANT("Cooked", "i", INPUT_COOKED),
	GB_CONSTANT("CBreak", "i", INPUT_CBREAK),
	GB_CONSTANT("Raw", "i", INPUT_RAW),
	GB_CONSTANT("NoDelay", "i", INPUT_NODELAY),

	GB_PROPERTY("Buffered", "b", Screen_Buffered),

	GB_STATIC_PROPERTY("Cursor", "i", Screen_Cursor),
	GB_STATIC_PROPERTY("Echo", "b", Screen_Echo),
	GB_STATIC_PROPERTY("Input", "i", Screen_Input),
	GB_STATIC_PROPERTY_READ("IsConsole", "b", Screen_IsConsole),
	GB_STATIC_PROPERTY("Repeater", "i", Screen_Repeater),

	GB_STATIC_PROPERTY_READ("Lines", "i", Screen_Lines), //GB_PROPERTY
	GB_STATIC_PROPERTY_READ("Cols", "i", Screen_Cols),

	GB_METHOD("_new", NULL, Screen_new, NULL),
	GB_METHOD("_free", NULL, Screen_free, NULL),

	GB_METHOD("Refresh", NULL, Screen_Refresh, NULL),

	GB_END_DECLARE
};
