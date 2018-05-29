/***************************************************************************

	gb.jit.h

	(c) 2018 Benoît Minisini <g4mba5@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

***************************************************************************/

#ifndef __GB_JIT_H
#define __GB_JIT_H

typedef
	union {
		int type;
		struct { int type; double value; } PACKED _float;
		struct { int type; float value; } PACKED _single;
		struct { int type; int value; } PACKED _integer;
		struct { int type; int64_t value; } PACKED _long;
		struct { int type; char *addr; int len; } PACKED _string;
		struct { int type; int val[2]; } PACKED _swap;
		}
	PACKED
	JIT_CONSTANT;

typedef
	struct {
		VALUE **sp;
		PCODE **pc;
		void **cp;
		void **op;
		void (*debug)(const char *fmt, ...);
		void *(*get_static_addr)(int index);
		void *(*get_dynamic_addr)(int index);
		JIT_CONSTANT *(*get_constant)(int index);
		void *(*get_class_ref)(int index);
		void **subr_table;
		const char *char_table;
		void *(*unborrow)(VALUE *val);
		void (*new)(void);
		void (*push_array)(ushort code);
		void (*pop_array)(ushort code);
	}
	JIT_INTERFACE;

#endif
