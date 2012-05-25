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

#include "c_screen.h"

/* Border constants */
enum {
	BORDER_NONE = 0,
	BORDER_ASCII,
	BORDER_ACS
};

/* Timeout constants */
enum {
	TIMEOUT_NOTIMEOUT = -1
};

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

typedef struct {
	GB_BASE ob;
	CSCREEN *parent;	/* The parent Screen */
	WINDOW *main;		/* The main window */
	WINDOW *content;	/* This window is used for all content-related operations. Its purpose is turning
				   the ncurses window borders which are inner-window to outer-window ones thus
				   separating border from content. If there is no border, this is the same as @main
				   otherwise a subwindow of it */
	PANEL *pan;		/* Panel of the main window to provide overlapping windows */
	int border;		/* What kind of border */
	bool wrap;		/* Whether text shall be truncated or wrapped on line ends */
	char *caption;		/* Text to be displayed in the main window if there is a border */
	struct {		/* This structure is used to pass a line and a column number to virtual objects */
		int line;
		int col;
	} pos;
} CWINDOW;

#ifndef __C_WINDOW_C
extern GB_DESC CWindowDesc[];
extern GB_DESC CWindowAttrsDesc[];
extern GB_DESC CCharAttrsDesc[];
#endif

void WINDOW_raise_read(void *);

#endif /* __C_WINDOW_C */
