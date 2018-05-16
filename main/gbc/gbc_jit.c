/***************************************************************************

	gbc_jit.c

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

#define __GBC_JIT_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "gb_file.h"
#include "gb_str.h"
#include "gb_error.h"
#include "gbc_compile.h"
#include "gbc_chown.h"
#include "gbc_class.h"
#include "gbc_jit.h"

static FILE *_file = NULL;
static char *_prefix;

void JIT_begin(void)
{
	const char *path;
	
	if (_file)
		return;
	
	if (mkdir(".jit", 0777) == 0)
		FILE_set_owner(".jit", COMP_project);
	
	_prefix = STR_lower(JOB->class->name);
	path = FILE_cat(".jit", _prefix, NULL);
	path = FILE_set_ext(path, "c");
	
	_file = fopen(path, "w");
	if (!_file)
		THROW("Cannot create file: &1", path);
}

void JIT_end(void)
{
	if (!_file)
		return;
	
	fclose(_file);
	_file = NULL;
	STR_free(_prefix);
}

void JIT_declare_func(FUNCTION *func)
{
	char *name = STR_lower(TABLE_get_symbol_name(JOB->class->table, func->name));
	
	if (!TYPE_is_public(func->type))
		JIT_print("static ");
	JIT_print("void %s_%s();\n", _prefix, name);
	
	STR_free(name);
}

void JIT_begin_func(FUNCTION *func)
{
	const char *fname = TABLE_get_symbol_name(JOB->class->table, func->name);
	char *name = STR_lower(fname);
	
	JIT_section(fname);
	
	if (!TYPE_is_public(func->type))
		JIT_print("static ");
	JIT_print("void %s_%s(ushort code)\n{\n", _prefix, name);
	
	STR_free(name);
}

void JIT_end_func(void)
{
	JIT_print("}\n");
}

void JIT_print(const char *str, ...)
{
  va_list args;
  va_start(args, str);
  vfprintf(_file, str, args);
}

void JIT_section(const char *str)
{
	JIT_print("\n// %s\n\n", str);
}
