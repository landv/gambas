/***************************************************************************

	jit.h

	(c) 2000-2018 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __JIT_H
#define __JIT_H

#define __GB_COMMON_CASE_H
#include "gb_common.h"
#include "gb_str.h"
#include "gb_pcode.h"
#include "gb_reserved.h"
#include "gbx_type.h"
#include "gbx_class.h"
#include "main.h"

#define T_UNKNOWN  17

#undef TYPE_is_pure_object
#define TYPE_is_pure_object(_type) ((_type) > T_UNKNOWN)

#undef TYPE_is_object
#define TYPE_is_object(_type) ((_type) == T_OBJECT || TYPE_is_pure_object(_type))

#define TYPEID(_type) (TYPE_is_pure_object(_type) ? T_OBJECT : (_type))

enum
{
	CALL_UNKNOWN,
	CALL_PRIVATE,
	CALL_EVENT,
	CALL_EXTERN
};

#define PM_STRING  8
#define PM_WAIT    16


#ifndef __GBC_JIT_C
EXTERN char *JIT_prefix;
EXTERN CLASS *JIT_class;
EXTERN bool JIT_last_print_is_label;
#endif


// jit.c

char *JIT_translate(const char *name, const char *from);

void JIT_section(const char *str);

void JIT_print(const char *fmt, ...);
void JIT_print_decl(const char *fmt, ...);
void JIT_print_body(const char *fmt, ...);
void JIT_declare(TYPE type, const char *fmt, ...);

const char *JIT_get_type(TYPE type);
const char *JIT_get_gtype(TYPE type);
const char *JIT_get_ctype(TYPE type);
TYPE JIT_ctype_to_type(CLASS *class, CTYPE ctype);
const char *JIT_get_default_value(TYPE type);

void JIT_panic(const char *fmt, ...) NORETURN;

int JIT_get_code_size(FUNCTION *func);
int JIT_find_symbol(CLASS *class, const char *name);
void JIT_load_class_without_init(CLASS *class);

// jit_body.c

bool JIT_translate_body(FUNCTION *func, int index);


#endif

