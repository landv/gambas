/*
 * c_screen.h - gb.ncurses Screen class
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

#ifndef __C_SCREEN_H
#define __C_SCREEN_H

#include "gambas.h"
#include "c_input.h"

/* This will produce final output on terminal screen */
#define REAL_REFRESH()			SCREEN_real_refresh()
/* This macro is mostly called by Gambas implementation functions to request output on screen
   (read: to check if the output is buffered and if not produce output by means of
   REAL_REFRESH()) */
#define REFRESH()			SCREEN_refresh(NULL)
/* The Screen - like in ncurses - is the sole source of input */
#define SCREEN_get(t)			INPUT_get(t)

/*
 * Cursor modes
 */
enum {
	CURSOR_RETURN = -1,
	CURSOR_HIDDEN,
	CURSOR_VISIBLE,
	CURSOR_VERY
};

/*
 * Echo modes
 */
enum {
	ECHO_RETURN = -1,
	ECHO_NOECHO,
	ECHO_ECHO
};

#ifndef __C_SCREEN_C
extern GB_DESC CScreenDesc[];
extern GB_DESC CCursorDesc[];
#endif

typedef struct {
	GB_BASE ob;
	bool buffered;		/* Whether output will be buffered, i.e.
				   only done via Screen.Refresh() */
} CSCREEN;

int SCREEN_init();
void SCREEN_exit();
CSCREEN *SCREEN_get_active();
void SCREEN_refresh();
void SCREEN_real_refresh();
void SCREEN_raise_read(void *);

int CURSOR_mode(int);
int ECHO_mode(int);

#endif /* __C_SCREEN_H */
