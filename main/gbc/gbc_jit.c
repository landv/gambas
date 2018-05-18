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

static const char *_type_name[] = 
{
	"" , "b", "c", "h", "i", "l", "g", "f",
	"d", "s", "t", "p", "v", "A", "S", "n", "o"
};

static const char *_ctype_name[] = 
{
	"void" , "char", "uchar", "short", "int", "int64_t", "float", "double",
	"GB_DATE", "string", "string", "intptr_t", "GB_VARIANT", "?", "?", "?", "void *"
};

static const char *get_type(TYPE type)
{
	return _type_name[TYPE_get_id(type)];
}

static const char *get_ctype(TYPE type)
{
	return _ctype_name[TYPE_get_id(type)];
}

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
	
	JIT_print("static %s %s_%s_IMPL();\n", get_ctype(func->type), _prefix, name);
	
	if (!TYPE_is_public(func->type))
		JIT_print("static ");
	JIT_print("void %s_%s();\n", _prefix, name);
	
	STR_free(name);
}

void JIT_translate_func(FUNCTION *func)
{
	const char *fname = TABLE_get_symbol_name(JOB->class->table, func->name);
	char *name = STR_lower(fname);
	int i;
	
	JIT_section(fname);
	
	if (!TYPE_is_public(func->type))
		JIT_print("static ");
	JIT_print("void %s_%s(ushort n)\n{\n", _prefix, name);
	
	/*if (func->nlocal)
		JIT_print("  GB_VALUE L[%d];\n", func->nlocal);*/
	
	JIT_print("  ");
	
	if (!TYPE_is_void(func->type))
		JIT_print("%s ret = ", get_ctype(func->type));
	
	JIT_print("%s_%s_IMPL(", _prefix, name);
	
	for (i = 0; i < func->npmin; i++)
	{
		if (i) JIT_print(", ");
		JIT_print("PARAM_%s(%d)", get_type(func->param[i].type), i);
	}
	
	if (i < func->nparam)
	{
		JIT_print(", OPT(%d, %d)", i, func->nparam);
		
		for (; i < func->nparam; i++)
			JIT_print("; P%s(%d)", get_type(func->param[i].type), i);
	}
	
	if (func->vararg)
		THROW("Not supported");
	
	JIT_print(");\n");
	JIT_print("}\n\n");
	
	JIT_print("static %s %s_%s_IMPL(", get_ctype(func->type), _prefix, name);
	
	for (i = 0; i < func->npmin; i++)
	{
		if (i) JIT_print(", ");
		JIT_print("%s p%d", get_ctype(func->param[i].type), i);
	}
	
	if (i < func->nparam)
	{
		JIT_print(", uint64_t _opt");
		
		for (; i < func->nparam; i++)
			JIT_print("%s p%d", get_ctype(func->param[i].type), i);
	}
	
	JIT_print(")\n{\n");

	for (i = 0; i < func->nlocal; i++)
	{
		JIT_print("  %s l%d;\n", get_ctype(func->local[i].type), i);
	}
	if (func->nlocal)
		JIT_print("\n");
	
	STR_free(name);
	
	JIT_translate_body(func);
	
	JIT_print("}\n");
}

void JIT_print(const char *str, ...)
{
  va_list args;
  va_start(args, str);
  vfprintf(_file, str, args);
  va_start(args, str);
	vprintf(str, args);
}

void JIT_section(const char *str)
{
	JIT_print("\n// %s\n\n", str);
}
