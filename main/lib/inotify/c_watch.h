/*
 * c_watch.h
 *
 * Copyright (C) 2013, 2014 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __C_WATCH_H
#define __C_WATCH_H

#include "main.h"
#include "gb_list.h"

#ifndef __C_WATCH_C
extern GB_DESC CWatch[];
extern GB_DESC CWatchEvents[];
#endif

typedef
	struct {
		GB_BASE ob;
		LIST list;
		void *root;
		GB_VARIANT_VALUE tag;
		ushort events;
		ushort save_events;
		bool nofollow;
		bool paused;
		} 
	CWATCH;

#endif /* __C_WATCH_H */
