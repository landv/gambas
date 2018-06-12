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

typedef
	struct {
		const char *name;
		char type;
	}
	CLASS_TYPE;

char *JIT_prefix;

static FILE *_file = NULL;

static const CLASS_TYPE _class_type[] = {
	{ "Boolean[]", T_BOOLEAN },
	{ "Byte[]", T_BYTE },
	{ "Short[]", T_SHORT },
	{ "Integer[]", T_INTEGER },
	{ "Long[]", T_LONG },
	{ "Single[]", T_SINGLE },
	{ "Float[]", T_FLOAT },
	{ "Date[]", T_DATE },
	{ "String[]", T_STRING },
	{ "Pointer[]", T_POINTER },
	{ "Object[]", T_OBJECT },
	{ "Variant[]", T_VARIANT },
	{ NULL, 0 }
};

static const char *_type_name[] = 
{
	"V" , "b", "c", "h", "i", "l", "g", "f",
	"d", "s", "t", "p", "v", "A", "S", "n",
	"o", "u", "C"
};

static const char *_gtype_name[] = 
{
	"GB_T_VOID" , "GB_T_BOOLEAN", "GB_T_BYTE", "GB_T_SHORT", "GB_T_INTEGER", "GB_T_LONG", "GB_T_SINGLE", "GB_T_FLOAT",
	"GB_T_DATE", "GB_T_CSTRING", "GB_T_STRING", "GB_T_POINTER", "GB_T_VARIANT", "?", "?", "?", 
	"GB_T_OBJECT"
};

static const char *_ctype_name[] = 
{
	"void" , "char", "uchar", "short", "int", "int64_t", "float", "double",
	"GB_DATE", "GB_STRING", "GB_STRING", "intptr_t", "GB_VARIANT", "?", "?", "?", 
	"GB_OBJECT", "GB_VALUE", "?"
};

const char *JIT_get_type(TYPE type)
{
	int index = TYPE_get_id(type);
	//if (index <= 0 || index > 18) THROW("Unknown type");
	return _type_name[index];
}

const char *JIT_get_gtype(TYPE type)
{
	return _gtype_name[TYPE_get_id(type)];
}

const char *JIT_get_ctype(TYPE type)
{
	return _ctype_name[TYPE_get_id(type)];
}

static TYPE get_array_type(SYMBOL *sym)
{
	const CLASS_TYPE *p;
	
	for(p = _class_type; p->name; p++)
	{
		if (SYMBOL_compare_ignore_case(sym, p->name) == 0)
			return TYPE_make_simple(p->type);
	}
	
	return (TYPE)0;
}

void JIT_begin(bool has_fast)
{
	const char *path;
	int i;
	CLASS_REF *cr;
	SYMBOL *sym;
	
	JIT_prefix = STR_lower(JOB->class->name);
	path = FILE_cat(".jit", JIT_prefix, NULL);
	path = FILE_set_ext(path, "c");

	if (!has_fast)
	{
		FILE_unlink(path);
		return;
	}
	
	if (mkdir(".jit", 0777) == 0)
		FILE_set_owner(".jit", COMP_project);
	
	_file = fopen(path, "w");
	if (!_file)
		THROW("Cannot create file: &1", path);
	
	// detect classes for optimizations
	
	for (i = 0; i < ARRAY_count(JOB->class->class); i++)
	{
		cr = &JOB->class->class[i];
		if (!TYPE_is_null(cr->type))
			continue;
		
		sym = (SYMBOL *)CLASS_get_symbol(JOB->class, cr->index);
		
		if (SYMBOL_compare_ignore_case(sym, "Collection") == 0)
		{
			cr->is_collection = TRUE;
			continue;
		}
		
		cr->type = get_array_type(sym);
	}
}

void JIT_end(void)
{
	STR_free(JIT_prefix);

	if (_file)
	{
		fclose(_file);
		_file = NULL;
	}
}

static void declare_implementation(FUNCTION *func, int index)
{
	int i;
	int nopt;
	int opt;
	
	JIT_print("static %s jit_%s_%d_(", JIT_get_ctype(func->type), JIT_prefix, index);
	
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
	
	JIT_print(")");
}

void JIT_declare_func(FUNCTION *func, int index)
{
	JIT_print("void jit_%s_%d(uchar n);\n", JIT_prefix, index);
	
	declare_implementation(func, index);
	JIT_print(";\n");
}

void JIT_translate_func(FUNCTION *func, int index)
{
	const char *fname = TABLE_get_symbol_name(JOB->class->table, func->name);
	int i;
	TYPE type;
	int nopt;
		
	JIT_section(fname);
	
	JIT_print("void jit_%s_%d(uchar n)\n{\n", JIT_prefix, index);
	
	if (func->nparam)
		JIT_print("  VALUE *sp = *(JIT.sp);\n");
	
	JIT_print("  ");
	
	if (!TYPE_is_void(func->type))
		JIT_print("RETURN_%s(", JIT_get_type(func->type));
	
	JIT_print("jit_%s_%d_(", JIT_prefix, index);
	
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
	
	if (!TYPE_is_void(func->type))
		JIT_print(")");
	
	JIT_print(");\n");
	JIT_print("}\n\n");
	
	declare_implementation(func, index);
	JIT_print("\n{\n");

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
			type = func->local[func->nparam + i].type;
			JIT_print("  %s l%d = ", JIT_get_ctype(type), i);
		}
		
		switch(TYPE_get_id(type))
		{
			case T_DATE: JIT_print("{GB_T_DATE}"); break;
			case T_STRING: JIT_print("{GB_T_STRING}"); break;
			case T_OBJECT: JIT_print("{GB_T_NULL}"); break;
			case T_VARIANT:  JIT_print("{GB_T_VARIANT,{GB_T_NULL}}"); break;
			default: JIT_print("0");
		}
		JIT_print(";\n");
	}
	if (func->nlocal)
		JIT_print("\n");
	
	JIT_translate_body(func, index);
	
	if (func->nlocal)
	{
		for (i = 0; i < func->nlocal; i++)
		{
			type = func->local[func->nparam + i].type;
			switch(TYPE_get_id(type))
			{
				case T_STRING:
				case T_OBJECT:
				case T_VARIANT:
					JIT_print("  RELEASE_FAST_%s(l%d);\n", JIT_get_type(type), i);
					break;
			}
		}
	}
	
	if (!TYPE_is_void(func->type))
	{
		switch(TYPE_get_id(func->type))
		{
			case T_STRING:
			case T_OBJECT:
			case T_VARIANT:
				JIT_print("  JIT.unborrow((GB_VALUE *)&r);\n");
				break;
		}
		
		JIT_print("  return r;\n");
	}
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
