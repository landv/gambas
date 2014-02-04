/*
 * c_screen.c - gb.ncurses Screen class
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

#define __C_SCREEN_C

#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include <ncurses.h>
#include <panel.h>

#include "gambas.h"

#include "main.h"
#include "c_screen.h"
#include "c_input.h"

#define THIS			_active

#define E_UNSUPP	"Unsupported value"

static CSCREEN *_active;

GB_SIGNAL_CALLBACK *_sigwinch_cb;

DECLARE_EVENT(EVENT_Resize);

static void SCREEN_sigwinch(int signum, intptr_t data)
{
	/* TODO: ncurses may be configured to provide its own SIGWINCH
	 *       handler. See resizeterm(3X). */
	if (signum == SIGWINCH)
		GB.Raise(_active, EVENT_Resize, 0);
}

int SCREEN_init()
{
	_sigwinch_cb = GB.Signal.Register(SIGWINCH, SCREEN_sigwinch,
					  (intptr_t) NULL);
	return 0;
}

void SCREEN_exit()
{
	GB.Signal.Unregister(SIGWINCH, _sigwinch_cb);
}

void SCREEN_refresh()
{
	if (!NCURSES_RUNNING)
		return;
	update_panels();
	doupdate();
}

BEGIN_METHOD_VOID(Screen_init)

	_active = GB.AutoCreate(GB.FindClass("Screen"), 0);

END_METHOD

static int CSCREEN_cursor(CSCREEN *scr, int mode)
{
	if (mode >= 0 && mode <= 2)
		curs_set(mode);
	else
		return -1;
	scr->cursor = mode;
	return 0;
}

static void CSCREEN_echo(CSCREEN *scr, int mode)
{
	if (mode)
		echo();
	else
		noecho();
	scr->echo = mode;
}

BEGIN_METHOD_VOID(Screen_new)

	CSCREEN_cursor(THIS, 1);
	CSCREEN_echo(THIS, 1);
	INPUT_mode(THIS, INPUT_CBREAK);

END_METHOD

#if 0
BEGIN_METHOD(Screen_new, GB_STRING termpath)

	char path[LENGTH(termpath) + 1];
	FILE *finout;

	strncpy(path, STRING(termpath), LENGTH(termpath));
	path[LENGTH(termpath)] = 0;

	finout = fopen(path, "r+");
	if (!finout) {
		GB.Error(NULL);
		return;
	}
	assert(_maxscreen < MAX_SCREENS);
	_screens[_maxscreen] = THIS;
	THIS->index = _curscreen = _maxscreen++;
	THIS->screen = newterm(NULL, finout, finout);
	set_term(THIS->screen);
	THIS->finout = finout;
	THIS->buffered = 1;
	CSCREEN_cursor(THIS, 0);
	CSCREEN_echo(THIS, 0);
	INPUT_mode(THIS, INPUT_CBREAK);
	refresh();

END_METHOD

BEGIN_METHOD_VOID(Screen_free)

	void *dst, *src;

	if (THIS->index != _maxscreen - 1) {
		dst = &_screens[THIS->index];
		src = dst + sizeof(_screens[0]);
		memmove(dst, src, _maxscreen - THIS->index);
	}
	_screens[THIS->index] = NULL;
	if (_curscreen)
		_curscreen--;
	_maxscreen--;
	endwin();
	fclose(THIS->finout);
	delscreen(THIS->screen);
	if (!_maxscreen) {
		SCREEN_exit();
		return;
	}
	set_term(_active->screen);

END_METHOD
#endif

BEGIN_METHOD_VOID(Screen_Refresh)

	SCREEN_refresh();

END_METHOD

BEGIN_METHOD(Screen_Resize, GB_INTEGER lines; GB_INTEGER cols)

	resizeterm(VARG(lines), VARG(cols));

END_METHOD

GB_DESC CCursorDesc[] = {
	GB_DECLARE("Cursor", 0),
	GB_NOT_CREATABLE(),

	/* According to curs_set */
	GB_CONSTANT("Hidden", "i", 0),
	GB_CONSTANT("Visible", "i", 1),
	GB_CONSTANT("VeryVisible", "i", 2),

	GB_END_DECLARE
};

BEGIN_PROPERTY(Screen_Cursor)

	if (READ_PROPERTY) {
		GB.ReturnInteger(THIS->cursor);
		return;
	}

	if (CSCREEN_cursor(THIS, VPROP(GB_INTEGER)) == -1)
		GB.Error(E_UNSUPP);

END_PROPERTY

BEGIN_PROPERTY(Screen_Echo)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(THIS->echo);
		return;
	}
	CSCREEN_echo(THIS, !!VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Screen_Input)

	if (READ_PROPERTY) {
		GB.ReturnInteger(THIS->input);
		return;
	}
	INPUT_mode(THIS, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Screen_Lines)

	if (READ_PROPERTY) {
		GB.ReturnInteger(LINES);
		return;
	}
	resizeterm(VPROP(GB_INTEGER), COLS);

END_PROPERTY

BEGIN_PROPERTY(Screen_Cols)

	if (READ_PROPERTY) {
		GB.ReturnInteger(COLS);
		return;
	}
	resizeterm(LINES, VPROP(GB_INTEGER));

END_PROPERTY

GB_DESC CScreenDesc[] = {
	GB_DECLARE("Screen", sizeof(CSCREEN)),
	GB_AUTO_CREATABLE(),
	GB_NOT_CREATABLE(),

	GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

	GB_STATIC_METHOD("_init", NULL, Screen_init, NULL),
	GB_STATIC_METHOD("Refresh", NULL, Screen_Refresh, NULL),
	GB_STATIC_METHOD("Resize", NULL, Screen_Resize, "(Lines)i(Cols)i"),

	GB_STATIC_PROPERTY("Cursor", "i", Screen_Cursor),
	GB_STATIC_PROPERTY("Echo", "b", Screen_Echo),
	GB_STATIC_PROPERTY("Input", "i", Screen_Input),

	GB_STATIC_PROPERTY("Height", "i", Screen_Lines),
	GB_STATIC_PROPERTY("H", "i", Screen_Lines),
	GB_STATIC_PROPERTY("Width", "i", Screen_Cols),
	GB_STATIC_PROPERTY("W", "i", Screen_Cols),

	GB_END_DECLARE
};
