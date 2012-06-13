/*
 * c_input.h - gb.ncurses opaque input routines for use of either ncurses or
 *           the terminal driver directly
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

#ifndef __C_INPUT_H
#define __C_INPUT_H

enum {
	/* Return the current mode */
	INPUT_RETURN = -1,
	/* Line discipline, signal generation */
	INPUT_COOKED,
	/* No line discipline, signal generation */
	INPUT_CBREAK,
	/* No line discipline, no signal generation */
	INPUT_RAW,
	/* Use terminal driver, enabled to use raw scancodes
	   (which are internally converted to ncurses keys but enable to
	   distinguish key press and release) */
	INPUT_NODELAY
};

enum {
	TIMEOUT_NOTIMEOUT = -1
};

enum {
	REPEATER_RETURN = -1
};

#ifndef __C_INPUT_C
extern GB_DESC CInputDesc[];
#endif

int INPUT_init();
void INPUT_exit();
int INPUT_mode(int);
int INPUT_get(int);
void INPUT_drain();
#ifdef __C_INPUT_C
static int INPUT_watch(int);
static void INPUT_callback(int, int, intptr_t);
static int NODELAY_init();
static int NODELAY_exit();
static void NODELAY_error_hook();
static int NODELAY_consolefd();
static inline void NODELAY_drain();
static int NODELAY_repeater();
static int NODELAY_repeater_delay(int);
static void NODELAY_change_pressed(int);
static int NODELAY_get(int);
#endif

#endif /* __C_INPUT_H */
