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
	"d", "s", "t", "p", "v", "A", "S", "n", "o",
	"u"
};

static const char *_ctype_name[] = 
{
	"void" , "char", "uchar", "short", "int", "int64_t", "float", "double",
	"GB_DATE", "GB_STRING", "GB_STRING", "intptr_t", "GB_VARIANT", "?", "?", "?", "void *",
	"GB_VALUE"
};

const char *JIT_get_type(TYPE type)
{
	return _type_name[TYPE_get_id(type)];
}

const char *JIT_get_ctype(TYPE type)
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

void JIT_declare_func(FUNCTION *func, int index)
{
	JIT_print("void %s_%d(uchar n);\n", _prefix, index);
	
	JIT_print("static %s %s_%d_IMPL();\n", JIT_get_ctype(func->type), _prefix, index);
}

void JIT_translate_func(FUNCTION *func, int index)
{
	const char *fname = TABLE_get_symbol_name(JOB->class->table, func->name);
	int i;
	TYPE type;
	int nopt;
	int opt;
		
	JIT_section(fname);
	
	JIT_print("void %s_%d(uchar n)\n{\n", _prefix, index);
	
	/*if (func->nlocal)
		JIT_print("  GB_VALUE L[%d];\n", func->nlocal);*/
	
	JIT_print("  ");
	
	if (!TYPE_is_void(func->type))
		JIT_print("RETURN_%s(", JIT_get_type(func->type));
	
	JIT_print("%s_%d_IMPL(", _prefix, index);
	
	for (i = 0; i < func->npmin; i++)
	{
		if (i) JIT_print(", ");
		JIT_print("PARAM_%s(%d)", JIT_get_type(func->param[i].type), i);
	}
	
	if (i < func->nparam)
	{
		nopt = 0;
		
		for (; i < func->nparam; i++)
		{
			if (nopt == 0)
				JIT_print(", OPT(%d,%d)", i, Min(func->nparam, i + 8) - i);
			
			JIT_print(", PARAM_OPT_%s(%d)", JIT_get_type(func->param[i].type), i);
			nopt++;
			if (nopt >= 8)
				nopt == 0;
		}
	}
	
	if (func->vararg)
		THROW("Not supported");
	
	JIT_print(");\n");
	JIT_print("}\n\n");
	
	JIT_print("static %s %s_%d_IMPL(", JIT_get_ctype(func->type), _prefix, index);
	
	for (i = 0; i < func->npmin; i++)
	{
		if (i) JIT_print(", ");
		JIT_print("%s p%d", JIT_get_ctype(func->param[i].type), i);
	}
	
	if (i < func->nparam)
	{
		opt = nopt = 0;
		
		for (; i < func->nparam; i++)
		{
			if (nopt == 0)
			{
				JIT_print(", uchar _o%d", opt);
				opt++;
			}
			
			JIT_print(", %s p%d", JIT_get_ctype(func->param[i].type), i);
			
			nopt++;
			if (nopt >= 8)
				nopt = 0;
		}
	}
	
	JIT_print(")\n{\n");

	for (i = -1; i < func->nlocal; i++)
	{
		if (i < 0)
		{
			if (TYPE_is_void(func->type))
				continue;
			type = func->type;
			JIT_print("  %s r = ", JIT_get_ctype(type));
		}
		else
		{
			type = func->local[i].type;
			JIT_print("  %s l%d = ", JIT_get_ctype(type), i);
		}
		
		switch(TYPE_get_id(type))
		{
			case T_DATE:
			case T_STRING:
			case T_VARIANT:
				JIT_print("{0}");
				break;
			default:
				JIT_print("0");
		}
		JIT_print(";\n");
	}
	if (func->nlocal)
		JIT_print("\n");
	
	JIT_translate_body(func);
	
	if (!TYPE_is_void(func->type))
		JIT_print("  return r;\n");
	else
		JIT_print("  return;\n");
	
	JIT_print("}\n");
}

void JIT_vprint(const char *str, va_list args)
{
	vprintf(str, args);
}

void JIT_print(const char *str, ...)
{
  va_list args;
	va_list copy;
	
  va_start(args, str);
	
	if (JOB->verbose)
	{
		va_copy(copy, args);
		vprintf(str, copy);
		va_end(copy);
	}
	
  vfprintf(_file, str, args);
	va_end(args);
}

void JIT_section(const char *str)
{
	JIT_print("\n// %s\n\n", str);
}
