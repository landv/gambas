/*
 * c_window.h - gb.ncurses Window class
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

#ifndef __C_WINDOW_H
#define __C_WINDOW_H

#include <ncurses.h>
#include <panel.h>

#include "gambas.h"
#include "gb_common.h"

#include "c_ncurses.h"

#define THIS				((struct nc_window *) _object)
#define HAS_BORDER			(THIS->border)
#define IS_WRAPPED			(THIS->wrap)
/* This will produce final output on terminal screen, shall be called only by Gambas functions
   as they assemble all changes for a single functionality and may then output once. */
#define REFRESH()			{ \
						update_panels(); \
						doupdate(); \
					}

/* Translate linear (absolute) memory addresses and x,y coordinates into each other
   most useful when wrapping is needed. */
#define A2XY(win, a, x, y)	{ \
					(x) = (a) % getmaxx(win); \
					(y) = (a) / getmaxx(win); \
				}
#define XY2A(win, x, y, a)	{ \
					(a) = (y) * getmaxx(win) + (x); \
				}
/* Interpret the -1 values in coordinates as to insert the current cursor position */
#define MAKE_COORDS(win, x, y)	{ \
					if ((x) == -1) x = getcurx(win); \
					if ((y) == -1) y = getcury(win); \
				}
/* Check for out-of-range coordinates */
#define BAD_COORDS(win, x, y)	((x) < 0 || (x) >= getmaxx(win) || \
				 (y) < 0 || (y) >= getmaxy(win))
#define BAD_DIMENSION(w, h)	((w) <= 0 || (w) > COLS || \
				 (h) <= 0 || (h) > LINES)

/* @reqs for *_attrs_driver() */
enum
{
	/* Return current attributes */
	ATTR_DRV_RET,
	/* Enable given attributes */
	ATTR_DRV_ON,
	/* Disable given attributes */
	ATTR_DRV_OFF
};

#define WIN_ATTR_METHOD(b, a)	{ \
					if (READ_PROPERTY) \
						GB.ReturnBoolean(nc_window_attrs_driver( \
							THIS, (a), ATTR_DRV_RET) \
							& (a)); \
					else nc_window_attrs_driver( \
						THIS, (a), (b) ? ATTR_DRV_ON : ATTR_DRV_OFF); \
				}
#define WIN_ATTR_METHOD_BOOL(a)		WIN_ATTR_METHOD(VPROP(GB_BOOLEAN), a);
#define WIN_ATTR_METHOD_INT(a)		WIN_ATTR_METHOD(1, a);
/* Notice the wtouchln() line in the following macro. It seems that a chgat() from
   nc_window_char_attrs_driver() doesn't mark anything dirty (no output on screen from
   a REFRESH()). So to make the new attribute available immidiately, we touch the affected
   line manually. A higher-callstack function may call REFRESH() to get output. */
#define CHAR_ATTR_METHOD(b, a)	{ \
					if (READ_PROPERTY) \
						GB.ReturnBoolean(nc_window_char_attrs_driver( \
							THIS, (a), THIS->pos.col, THIS->pos.line, \
							ATTR_DRV_RET) & (a)); \
					else nc_window_char_attrs_driver( \
						THIS, (a), THIS->pos.col, THIS->pos.line, \
						(b) ? ATTR_DRV_ON : ATTR_DRV_OFF); \
					wtouchln(THIS->main, THIS->pos.line + (HAS_BORDER ? 1 : 0), 1, 1); \
				}
#define CHAR_ATTR_METHOD_BOOL(a)	CHAR_ATTR_METHOD(VPROP(GB_BOOLEAN), a);
#define CHAR_ATTR_METHOD_INT(a)		CHAR_ATTR_METHOD(1, a);

//TODO:	[x] chgat on line and character basis
//	[-] Stream
//	[-] Timer
//	[-] Color

struct nc_window
{
	GB_BASE ob;
	GB_STREAM stream;	/* Gambas stream structure to enable Print #Window, Expr and other stream-related
				   syntaxes */
	WINDOW *main;		/* The main window. */
	WINDOW *content;	/* This window is used for all content-related operations. Its purpose is turning
				   the ncurses window borders which are inner-window to outer-window ones thus
				   separating border from content. If there is no border, this is the same as @main
				   otherwise a subwindow of it. */
	PANEL *pan;		/* Panel of the main window to provide overlapping windows */
	bool border;		/* Whether there is a border */
	bool wrap;		/* Whether text shall be truncated or wrapped on line ends */
	struct			/* This structure is used to pass a line and a column number to virtual objects */
	{
		int line;
		int col;
	} pos;
};

#ifndef __C_WINDOW_C
extern GB_DESC CWindowDesc[];
extern GB_DESC CWindowAttrsDesc[];
extern GB_DESC CCharAttrsDesc[];
#endif

#define nc_window_main_to_content()	(nc_window_copy_window(THIS->main, THIS->content, 0, 0, \
								getmaxx(THIS->content), \
								getmaxy(THIS->content), 0, 0))
#define nc_window_content_to_main()	(nc_window_copy_window(THIS->content, THIS->main, 0, 0, \
								getmaxx(THIS->content), \
								getmaxy(THIS->content), 0, 0))

#endif /* __C_WINDOW_C */
