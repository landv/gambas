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

#include "gambas.h"
#include "c_input.h"

#ifndef __C_SCREEN_H
#define __C_SCREEN_H

/* This will produce final output on terminal screen */
#define REAL_REFRESH()		SCREEN_refresh()

#ifndef __C_SCREEN_C
extern GB_DESC CScreenDesc[];
extern GB_DESC CCursorDesc[];
#endif

typedef struct {
	GB_BASE ob;
	int index;
	int echo;
	int cursor;
	int input;
	int buffered;
} CSCREEN;

int SCREEN_init();
void SCREEN_exit();
void SCREEN_refresh();

#endif /* __C_SCREEN_H */
