/***************************************************************************

	jit.c

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

#define __JIT_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "gb_str.h"
#include "jit.h"

typedef
	struct {
		const char *name;
		char type;
	}
	CLASS_TYPE;

CLASS *JIT_class;
char *JIT_prefix;
bool JIT_last_print_is_label;

static char *_buffer = NULL;
static char *_buffer_decl = NULL;
static char *_buffer_body = NULL;

/*static const CLASS_TYPE _class_type[] = {
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
};*/

static const char *_type_name[] = 
{
	"V" , "b", "c", "h", "i", "l", "g", "f",
	"d", "s", "t", "p", "v", "F", "C", "n",
	"o", "u"
};

static const char *_gtype_name[] = 
{
	"GB_T_VOID" , "GB_T_BOOLEAN", "GB_T_BYTE", "GB_T_SHORT", "GB_T_INTEGER", "GB_T_LONG", "GB_T_SINGLE", "GB_T_FLOAT",
	"GB_T_DATE", "GB_T_CSTRING", "GB_T_STRING", "GB_T_POINTER", "GB_T_VARIANT", "?", "?", "?", 
	"GB_T_OBJECT"
};

static const char *_ctype_name[] = 
{
	"void" , "bool", "uchar", "short", "int", "int64_t", "float", "double",
	"GB_DATE", "GB_STRING", "GB_STRING", "intptr_t", "GB_VARIANT", "?", "?", "?", 
	"GB_OBJECT", "GB_VALUE", "?"
};

const char *JIT_get_type(TYPE type)
{
	return _type_name[TYPEID(type)];
}

const char *JIT_get_gtype(TYPE type)
{
	return _gtype_name[TYPEID(type)];
}

const char *JIT_get_ctype(TYPE type)
{
	return _ctype_name[TYPEID(type)];
}

TYPE JIT_ctype_to_type(CTYPE ctype)
{
	if (ctype.id == T_OBJECT && ctype.value >= 0)
		return (TYPE)(JIT_class->load->class_ref[ctype.value]);
	else if (ctype.id == TC_ARRAY)
		JIT_panic("Static array not implemented");
		//return (TYPE)CARRAY_get_array_class(class, class->load->array[ctype.value]->ctype);
	else if (ctype.id == TC_STRUCT)
		return (TYPE)(JIT_class->load->class_ref[ctype.value]);
	else
		return (TYPE)(ctype.id);
}


static void JIT_begin(void)
{
	JIT_prefix = STR_lower(JIT_class->name);
	_buffer = NULL;
	_buffer_decl = NULL;
	JIT_print("\n//////// %s\n\n", JIT_class->name);
}

static char *JIT_end(void)
{
	char *result = _buffer;
	
	STR_free(JIT_prefix);
	_buffer = NULL;
	
	GB.FreeStringLater(result);
	return result;
}

static void declare_implementation(FUNCTION *func, int index)
{
	int i;
	int nopt;
	int opt;
	
	JIT_print("static %s jit_%s_%d_(", JIT_get_ctype(func->type), JIT_prefix, index);
	
	for (i = 0; i < func->npmin; i++)
	{
		if (i) JIT_print(",");
		JIT_print("%s p%d", JIT_get_ctype(func->param[i].type), i);
	}
	
	if (i < func->n_param)
	{
		opt = nopt = 0;
		
		for (; i < func->n_param; i++)
		{
			if (i) JIT_print(",");
			
			if (nopt == 0)
			{
				JIT_print("uchar o%d,", opt);
				opt++;
			}
			
			JIT_print("%s p%d", JIT_get_ctype(func->param[i].type), i);
			
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


const char *JIT_get_default_value(TYPE type)
{
	switch(TYPEID(type))
	{
		case T_DATE: return "{GB_T_DATE}";
		case T_STRING: return "{GB_T_STRING}";
		case T_OBJECT: return "{GB_T_NULL}";
		case T_VARIANT: return "{GB_T_VARIANT,{GB_T_NULL}}";
		default: return "0";
	}
}


static bool JIT_translate_func(FUNCTION *func, int index)
{
	int i;
	TYPE type;
	int nopt;
		
	if (func->debug)
		JIT_section(func->debug->name);
	
	JIT_print("void jit_%s_%d(uchar n)\n{\n", JIT_prefix, index);
	
	if (func->n_param)
		JIT_print("  VALUE *sp = *((VALUE **)%p);\n", JIT.sp);
	
	JIT_print("  ");
	
	if (!TYPE_is_void(func->type))
		JIT_print("RETURN_%s(", JIT_get_type(func->type));
	
	JIT_print("jit_%s_%d_(", JIT_prefix, index);
	
	for (i = 0; i < func->npmin; i++)
	{
		if (i) JIT_print(",");
		type = func->param[i].type;
		if (TYPE_is_pure_object(type))
			JIT_print("PARAM_O(%d, CLASS(%p))", i, type);
		else
			JIT_print("PARAM_%s(%d)", JIT_get_type(type), i);
	}
	
	if (i < func->n_param)
	{
		nopt = 0;
		
		for (; i < func->n_param; i++)
		{
		if (i) JIT_print(",");
		
			if (nopt == 0)
				JIT_print("OPT(%d,%d),", i, Min(func->n_param, i + 8) - i);
			
			type = func->param[i].type;
			if (TYPE_is_pure_object(type))
				JIT_print("PARAM_OPT_O(%d, CLASS(%p))", i, type);
			else
				JIT_print("PARAM_OPT_%s(%d)", JIT_get_type(type), i);
			
			nopt++;
			if (nopt >= 8)
				nopt = 0;
		}
	}
	
	if (func->vararg)
	{
		GB.Error("Not supported");
		return TRUE;
	}
	
	if (!TYPE_is_void(func->type))
		JIT_print(")");
	
	JIT_print(");\n");
	JIT_print("}\n\n");
	
	declare_implementation(func, index);
	JIT_print("\n{\n");

	_buffer_decl = NULL;
	_buffer_body = NULL;
	
	for (i = -1; i < func->n_local; i++)
	{
		if (i < 0)
		{
			if (TYPE_is_void(func->type))
				continue;
			type = func->type;
			JIT_print_decl("  %s r = ", JIT_get_ctype(type));
		}
		else
		{
			type = JIT_ctype_to_type(func->local[i].type);
			JIT_print_decl("  %s l%d = ", JIT_get_ctype(type), i);
		}
		
		JIT_print_decl(JIT_get_default_value(type));
		JIT_print_decl(";\n");
	}
	
	for (i = 0; i < func->n_param; i++)
	{
		type = func->param[i].type;
		switch(TYPEID(type))
		{
			case T_STRING: case T_OBJECT: case T_VARIANT:
				JIT_print_body("  BORROW_%s(p%d);\n", JIT_get_type(type), i);
		}
	}
	
	if (JIT_translate_body(func, index))
		return TRUE;
	
	if (!TYPE_is_void(func->type))
	{
		switch(TYPEID(func->type))
		{
			case T_STRING:
			case T_OBJECT:
			case T_VARIANT:
				JIT_print_body("  JIT.unborrow((GB_VALUE *)&r);\n");
				break;
		}
		
		JIT_print_body("  return r;\n");
	}
	else
		JIT_print_body("  return;\n");
	
	_buffer = GB.AddString(_buffer, _buffer_decl, GB.StringLength(_buffer_decl));
	JIT_print("\n");
	_buffer = GB.AddString(_buffer, _buffer_body, GB.StringLength(_buffer_body));
	
	GB.FreeString(&_buffer_decl);
	GB.FreeString(&_buffer_body);
	
	JIT_print("}\n");
	
	return FALSE;
}


void JIT_vprint(char **buffer, const char *fmt, va_list args)
{
	int len, add;
	va_list copy;
	
	va_copy(copy, args);
	add = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);
	
	len = GB.StringLength(*buffer);
	
	*buffer = GB.ExtendString(*buffer, len + add);
	
	vsprintf(*buffer + len, fmt, args);
	
	JIT_last_print_is_label = (strncmp(fmt, "__L", 3) == 0);
}


void JIT_print(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
	JIT_vprint(&_buffer, fmt, args);
	va_end(args);
}


void JIT_print_decl(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
	JIT_vprint(&_buffer_decl, fmt, args);
	va_end(args);
}


void JIT_print_body(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
	JIT_vprint(&_buffer_body, fmt, args);
	va_end(args);
}


void JIT_section(const char *str)
{
	JIT_print("\n// %s\n\n", str);
}


void JIT_declare(TYPE type, const char *fmt, ...)
{
	va_list args;
	
	JIT_print_decl("  %s ", JIT_get_ctype(type));
	
  va_start(args, fmt);
	JIT_vprint(&_buffer_decl, fmt, args);
	va_end(args);
	
	switch (TYPEID(type))
	{
		case T_STRING:
		case T_OBJECT:
		case T_VARIANT:
			JIT_print_decl(" = %s", JIT_get_default_value(type));
			break;
	}
	
	JIT_print_decl(";\n");
}



char *JIT_translate(const char *name, const char *from)
{
	CLASS *class;
	int i;
	FUNCTION *func;
	
	JIT_class = class = (CLASS *)GB.LoadClassFrom(name, from);
	
	JIT_begin();
	
	for (i = 0; i < class->load->n_func; i++)
	{
		func = &class->load->func[i];
		if (!func->fast)
			continue;
		JIT_declare_func(func, i);
	}
	
	for (i = 0; i < class->load->n_func; i++)
	{
		func = &class->load->func[i];
		if (!func->fast)
			continue;
		
		JIT_last_print_is_label = FALSE;
		if (JIT_translate_func(func, i))
			return NULL;
	}
	
	return JIT_end();
}

void JIT_panic(const char *fmt, ...)
{
	va_list args;
	fprintf(stderr, "gb.jit: panic: ");
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);
	fputc('\n', stderr);
	fputs(_buffer, stderr);
	fputc('\n', stderr);
	abort();
}

int JIT_get_code_size(FUNCTION *func)
{
	void *code = func->code;
	int size = ((int *)code)[-1] / sizeof(ushort);
	
	if (func->code[size - 1] == 0)
		size--;

	return size;
}


int JIT_find_symbol(CLASS *class, const char *name)
{
	return JIT.find_symbol(class->table, class->sort, class->n_desc, sizeof(CLASS_DESC_SYMBOL), TF_IGNORE_CASE, name, strlen(name), NULL);
}


void JIT_load_class(CLASS *class)
{
	void *save_cp;
	
	if (class->ready || class->in_load)
		return;
	
	save_cp = *(JIT.cp);
	*(JIT.cp) = JIT_class;
	
	//fprintf(stderr, "gb.jit: load class: %s (%p)\n", class->name, class);
	JIT.load_class(class);
	
	*(JIT.cp) = save_cp;
}
