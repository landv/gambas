/*
 * main.c - gb.ncurses main object
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

#define __MAIN_C

#include "c_window.h"
#include "c_key.h"
#include "c_color.h"
#include "c_screen.h"
#include "c_input.h"
#include "main.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT = {
	CScreenDesc,
	CInputDesc,
	CCursorDesc,

	CWindowDesc,
	CWindowAttrsDesc,
	CCharAttrsDesc,
	CBorderDesc,

	CKeyDesc,

	CColorDesc,
	CColorInfoDesc,
	CPairDesc,

	NULL
};

static bool _init = FALSE;

bool MAIN_running()
{
	return _init && (!isendwin() || stdscr);
}

static void MAIN_init()
{
	if (_init)
		return;

	initscr();
	keypad(stdscr, TRUE);

	SCREEN_init();
	COLOR_init();

	refresh();

	_init = TRUE;
}

static void MAIN_exit()
{
	if (_init) {
		SCREEN_exit();
		endwin();
		_init = FALSE;
	}
}

static void MAIN_hook_error(int code, char *error, char *where)
{
	MAIN_exit();
}

static void MAIN_hook_main(int *argc, char **argv)
{
	MAIN_init();
}

int EXPORT GB_INIT()
{
	GB.Hook(GB_HOOK_ERROR, (void *) MAIN_hook_error);
	GB.Hook(GB_HOOK_MAIN, (void *) MAIN_hook_main);
	return 0;
}


void EXPORT GB_EXIT()
{
	MAIN_exit();
}
