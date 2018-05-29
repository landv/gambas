/***************************************************************************

	gbc_jit.h

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

#ifndef __GBC_JIT_H
#define __GBC_JIT_H

#include "gbc_class.h"

#define T_UNKNOWN (T_OBJECT + 1)

// gbc_jit.c

void JIT_begin(void);
void JIT_end(void);
void JIT_declare_func(FUNCTION *func, int index);
void JIT_translate_func(FUNCTION *func, int index);

void JIT_section(const char *str);

void JIT_print(const char *str, ...);
void JIT_vprint(const char *str, va_list args);

const char *JIT_get_type(TYPE type);
const char *JIT_get_ctype(TYPE type);

// gbc_jit_body.c

void JIT_translate_body(FUNCTION *func);


#endif

