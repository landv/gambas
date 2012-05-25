/*
 * input.h - gb.ncurses opaque input routines for use of either ncurses or
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

#ifndef __INPUT_H
#define __INPUT_H

enum {
	/* Return the current mode */
	INPUT_RETURN,
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
	/* Let INPUT_repeater_delay() return the current value of the
	   repeater delay */
	REPEATER_RETURN = -1
};

int INPUT_init();
void INPUT_exit();
int INPUT_consolefd();
int INPUT_mode(int mode);
int INPUT_get(int timeout);
int INPUT_repeater_delay(int val);
#ifdef __INPUT_C
static int INPUT_init_nodelay();
static int INPUT_exit_nodelay();
static void INPUT_nodelay_error_hook();
static int INPUT_nodelay_repeater();
static void INPUT_callback(int fd, int flag, intptr_t arg);
#endif

#endif /* __INPUT_H */
