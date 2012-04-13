/*
 * c_ncurses.h - gb.ncurses NCurses static class
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

#ifndef __C_NCURSES_H
#define __C_NCURSES_H

#include <ncurses.h>

#include "gambas.h"
#include "gb_common.h"

#define NCURSES_RUNNING		(!isendwin() || stdscr)

#ifndef __C_NCURSES_C
extern GB_DESC CNCursesDesc[];
#endif

#endif /* __C_NCURSES_H */
