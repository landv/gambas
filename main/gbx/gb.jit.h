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
	ushort JIT_PCODE;
	
typedef
	struct {
		void *next;
		GB_VALUE *bp;
		GB_VALUE *pp;
		void *cp;
		char *op;
		GB_VALUE *ep;
		void *fp;
		JIT_PCODE *pc;
		JIT_PCODE *ec;
		JIT_PCODE *et;
		GB_VALUE *gp;
		}
	JIT_CONTEXT;
	
typedef
	struct {
		GB_VALUE **sp;
		JIT_CONTEXT *exec;
		GB_VALUE *ret;
		bool *exec_debug;
		GB_VALUE **exec_super;
		void (*debug)(const char *fmt, ...);
		JIT_PCODE *(*get_code)(void *func);
		void (*throw)(int code, ...) NORETURN;
		JIT_CONSTANT *(*get_constant)(int index);
		void *(*get_class_ref)(int index);
		void **subr_table;
		const char *char_table;
		void *(*unborrow)(GB_VALUE *val);
		void (*new)(void);
		void (*push_array)(ushort code);
		void (*pop_array)(ushort code);
		void (*conv)(GB_VALUE *value, GB_TYPE type);
		void (*push_unknown)(void);
		void (*call_unknown)(ushort *pc, GB_VALUE *sp);
		void (*pop_unknown)(void);
		void (*enum_first)(ushort code, GB_VALUE *local, GB_VALUE *penum);
		bool (*enum_next)(ushort code, GB_VALUE *local, GB_VALUE *penum);
		int (*find_symbol)(void *symbol, ushort *sort, int n_symbol, size_t s_symbol, int flag, const char *name, int len, const char *prefix);
		void (*load_class)(void *class);
		void *error_current;
		void *error_handler;
		void (*error_reset)(void *);
		void (*error_set_last)(bool);
		bool *got_error;
		void **event_last;
		void (*push_complex)(void);
		void (*push_vargs)(void);
		void (*pop_vargs)(void);
		void (*exec_quit)(ushort code);
		void (*push_unknown_event)(bool);
		void *(*get_extern)(void *ext);
		const char *(*get_position)(void *cp, void *fp, ushort *pc);
		void (*release_many)(GB_VALUE *, int);
	}
	JIT_INTERFACE;

#endif
