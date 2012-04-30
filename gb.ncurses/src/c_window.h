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

#define THIS				((CWINDOW *) _object)
#define HAS_BORDER			(THIS->border)
#define IS_WRAPPED			(THIS->wrap)
#define IS_BUFFERED			(THIS->buffered)
/* This will produce final output on terminal screen */
#define REAL_REFRESH()			WINDOW_real_refresh()
/* This macro is mostly called by Gambas implementation functions to request output on screen
   (read: to check if the output is buffered and if not produce output by means of
   REAL_REFRESH(). To check buffering, this needs to get an object parameter.) */
#define REFRESH()			WINDOW_refresh(THIS)

/* Translate linear (absolute) memory addresses and x,y coordinates into each other
   most useful when wrapping is needed. */
#define A2XY(win, a, x, y)	do { \
					(x) = (a) % getmaxx(win); \
					(y) = (a) / getmaxx(win); \
				} while (0)
#define XY2A(win, x, y, a)	do { \
					(a) = (y) * getmaxx(win) + (x); \
				} while (0)
/* Interpret the -1 values in coordinates as to insert the current cursor position */
#define MAKE_COORDS(win, x, y)	do { \
					if ((x) == -1) \
						x = getcurx(win); \
					if ((y) == -1) \
						y = getcury(win); \
				} while (0)
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
	ATTR_DRV_OFF,
	/* Set a color */
	ATTR_DRV_COL
};

#define WIN_ATTR_METHOD(b, a)	do { \
					if (READ_PROPERTY) \
						GB.ReturnBoolean(WINDOW_attrs_driver( \
							THIS, (a), ATTR_DRV_RET) \
							& (a)); \
					else \
						WINDOW_attrs_driver(THIS, (a), \
							(b) ? ATTR_DRV_ON : ATTR_DRV_OFF); \
				} while (0)
#define WIN_ATTR_METHOD_BOOL(a)		WIN_ATTR_METHOD(VPROP(GB_BOOLEAN), a)
/* Notice the wtouchln() line in the following macro. It seems that a chgat() from
   nc_window_char_attrs_driver() doesn't mark anything dirty (no output on screen from
   a REFRESH()). So to make the new attribute available immidiately, we touch the affected
   line manually. A higher-callstack function may call REFRESH() to get output. */
#define CHAR_ATTR_METHOD(b, a)	do { \
					if (READ_PROPERTY) \
						GB.ReturnBoolean(WINDOW_char_attrs_driver( \
							THIS, (a), THIS->pos.col, THIS->pos.line, \
							ATTR_DRV_RET) & (a)); \
					else \
						WINDOW_char_attrs_driver(THIS, (a), THIS->pos.col, \
							THIS->pos.line, (b) ? ATTR_DRV_ON : ATTR_DRV_OFF); \
					wtouchln(THIS->main, THIS->pos.line + (HAS_BORDER ? 1 : 0), 1, 1); \
				} while(0)
#define CHAR_ATTR_METHOD_BOOL(a)	CHAR_ATTR_METHOD(VPROP(GB_BOOLEAN), a)

#define STREAM_PROLOGUE()		void *_object = stream->tag

//TODO: [-] Stream
//	[-] background/foreground colors
typedef struct nc_window {
	GB_BASE ob;
	GB_STREAM stream;	/* Gambas stream structure to enable stream-related syntaxes */
	WINDOW *main;		/* The main window. */
	WINDOW *content;	/* This window is used for all content-related operations. Its purpose is turning
				   the ncurses window borders which are inner-window to outer-window ones thus
				   separating border from content. If there is no border, this is the same as @main
				   otherwise a subwindow of it. */
	PANEL *pan;		/* Panel of the main window to provide overlapping windows */
	bool border;		/* Whether there is a border */
	bool wrap;		/* Whether text shall be truncated or wrapped on line ends */
	bool buffered;		/* Whether the output via REFRESH() macro shall be buffered (only a call to
				   Window.Refresh() will then produce any output) */
	struct {		/* This structure is used to pass a line and a column number to virtual objects */
		int line;
		int col;
	} pos;
} CWINDOW;

#ifndef __C_WINDOW_C
extern GB_DESC CWindowDesc[];
extern GB_DESC CWindowAttrsDesc[];
extern GB_DESC CCharAttrsDesc[];
extern GB_STREAM_DESC WindowStream;
#endif

#define WINDOW_main_to_content()	WINDOW_copy_window(THIS->main, THIS->content, 0, 0, \
								getmaxx(THIS->content), \
								getmaxy(THIS->content), 0, 0)
#define WINDOW_content_to_main()	WINDOW_copy_window(THIS->content, THIS->main, 0, 0, \
								getmaxx(THIS->content), \
								getmaxy(THIS->content), 0, 0)

void WINDOW_real_refresh();
void WINDOW_refresh();

#endif /* __C_WINDOW_C */
