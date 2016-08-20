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

/* Border constants */
enum {
	BORDER_NONE = 0,
	BORDER_ASCII,
	BORDER_ACS
};

typedef struct {
	GB_BASE ob;
	WINDOW *main;
	WINDOW *content;
	PANEL *pan;
	bool has_border;
	int border;
	bool buffered;
	bool wrap;
	char *caption;
	struct {
		int line;
		int col;
	} pos;
} CWINDOW;

#ifndef __C_WINDOW_C
extern GB_DESC CWindowDesc[];
extern GB_DESC CWindowAttrsDesc[];
extern GB_DESC CCharAttrsDesc[];
extern GB_DESC CBorderDesc[];
#endif

void CWINDOW_raise_read(CWINDOW *win);

#endif /* __C_WINDOW_C */
