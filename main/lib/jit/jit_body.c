/***************************************************************************

	jit_body.c

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

#define __JIT_BODY_C

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "jit.h"

#define JIT_print JIT_print_body

#define MAX_STACK 256

typedef
	struct {
		TYPE type;
		char *expr;
		int func;
		int index;
		TYPE call;
	}
	STACK_SLOT;

enum {
	CALL_SUBR,
	CALL_SUBR_CODE,
	CALL_SUBR_UNKNOWN,
	CALL_NEW,
	CALL_PUSH_ARRAY,
	CALL_POP_ARRAY
};
	
static STACK_SLOT _stack[MAX_STACK];
static int _stack_current = 0;

static bool _decl_rs;
static bool _decl_ro;
static bool _decl_rv;

static ushort _pc;

static bool _no_release = FALSE;

static int _loop_count;

static TYPE *_dup_type;

enum { LOOP_UNKNOWN, LOOP_UP, LOOP_DOWN };

static int _loop_type;

static int *_ctrl_index;
static TYPE *_ctrl_type;

static bool _has_gosub;
static bool _has_finally;
static bool _has_catch;

static bool _has_just_dup;


static void enter_function(FUNCTION *func, int index)
{
	_decl_rs = FALSE;
	_decl_ro = FALSE;
	_decl_rv = FALSE;
	_has_gosub = FALSE;
	_loop_count = 0;
	_has_just_dup = FALSE;
	
	_has_catch = FALSE;
	_has_finally = func->error && (func->code[func->error - 1] != C_CATCH);
	
	GB.NewArray((void **)&_dup_type, sizeof(TYPE), 0);
	GB.NewArray((void **)&_ctrl_type, sizeof(TYPE), 0);
	
	if (func->n_ctrl)
		 GB.AllocZero((void **)&_ctrl_index, sizeof(int) * func->n_ctrl);
	else
		_ctrl_index = NULL;
	
	JIT_print_decl("  VALUE **psp = (VALUE **)%p;\n", JIT.sp);
	JIT_print_decl("  VALUE *sp = SP;\n");
	//JIT_print("  VALUE *sp = SP; fprintf(stderr, \"> %d: sp = %%p\\n\", sp);\n", index);
	JIT_print_decl("  ushort *pc = (ushort *)%p;\n", JIT.get_code(JIT_class, index));
	JIT_print_decl("  GB_VALUE_GOSUB *gp = 0;\n");
	JIT_print_decl("  bool error;\n");
	
	JIT_print("\n  TRY {\n\n");
}


static void print_catch(void)
{
	JIT_print("\n  } CATCH {\n\n");
	if (_has_catch || _has_finally)
		JIT_print("  JIT.error_set_last(FALSE); \n");
	JIT_print("  error = TRUE;\n");
	JIT_print("  goto __FINALLY;\n");
	JIT_print("\n  } END_TRY\n\n");
	JIT_print("  error = FALSE;\n\n");
	JIT_print("__FINALLY:\n");
}

#define RELEASE_FAST(_expr, _type, _index) ({ \
	TYPE _t = (_type); \
  switch(TYPEID(_t)) \
	{ \
		case T_STRING: case T_OBJECT: case T_VARIANT: \
			JIT_print((_expr), JIT_get_type(_t), (_index)); \
	} \
})

static bool leave_function(FUNCTION *func, int index)
{
	int i;
	
	STR_free_later(NULL);
	JIT_print("__RETURN:\n");
	//JIT_print("__RETURN: fprintf(stderr, \"< %d: sp = %%p\\n\", sp);\n", ind);
	JIT_print("  SP = sp;\n");
		
	if (_stack_current)
		JIT_panic("Stack mismatch: stack is not void");
	
	if (!_has_catch && !_has_finally)
		print_catch();
	
	JIT_print("\n__RELEASE:;\n");
	
	for (i = 0; i < func->n_local; i++)
		RELEASE_FAST("  RELEASE_FAST_%s(l%d);\n", JIT_ctype_to_type(func->local[i].type), i); 
	
	for (i = 0; i < func->n_param; i++)
		RELEASE_FAST("  RELEASE_FAST_%s(p%d);\n", func->param[i].type, i);
	
	for (i = 0; i < GB.Count(_ctrl_type); i++)
		RELEASE_FAST("  RELEASE_FAST_%s(c%d);\n", _ctrl_type[i], i);
	
	for (i = 0; i < GB.Count(_dup_type); i++)
		RELEASE_FAST("  RELEASE_FAST_%s(d%d);\n", _dup_type[i], i);
	
	if (!_has_catch && !_has_finally)
		JIT_print("  if (error) GB.Propagate();\n");
	
	GB.Free((void **)&_ctrl_index);
	GB.FreeArray((void **)&_ctrl_type);
	GB.FreeArray((void **)&_dup_type);
	
	return FALSE;
}


static TYPE get_local_type(FUNCTION *func, int index)
{
	TYPE type;
	
	if (index < func->n_local)
		type = JIT_ctype_to_type(func->local[index].type);
	else
		type = _ctrl_type[_ctrl_index[index - func->n_local]];
	
	return type;
}


static void free_stack(int n)
{
	if (n < 0) n += _stack_current;
	STR_free(_stack[n].expr);
	_stack[n].expr = NULL;
}


static void check_stack(int n)
{
	if (_stack_current < n)
		JIT_panic("Stack mismatch: stack is void");
}


static void pop_stack(int n)
{
	int i;
	
	for (i = 1; i <= n; i++)
		free_stack(-i);
	
	_stack_current -= n;
}


static void declare(bool *flag, const char *expr)
{
	if (*flag)
		return;
	
	JIT_print_decl("  %s;\n", expr);
	*flag = TRUE;
}


static void print_label(ushort pc)
{
	JIT_print("__L%d:;\n", pc);
}

static void push_one(TYPE type, const char *fmt, va_list args)
{
	if (_stack_current > MAX_STACK)
		JIT_panic("Expression too complex");
	
	CLEAR(&_stack[_stack_current]);
	
	if (fmt)
		STR_vadd(&_stack[_stack_current].expr, fmt, args);
	
	_stack[_stack_current].type = type;
	_stack[_stack_current].call = T_UNKNOWN;
	_stack_current++;
}


static void push(TYPE type, const char *fmt, ...)
{
  va_list args;
	
  va_start(args, fmt);
	
	push_one(type, fmt, args);
	
	va_end(args);
}


static TYPE get_type(int n)
{
	TYPE type;
	
	if (n < 0) n += _stack_current;
	
	type = _stack[n].type;
	
	if (TYPE_is_pure_object(type))
		JIT_load_class((CLASS *)type);
	
	return type;
}


static char *get_expr(int n)
{
	if (n < 0) n += _stack_current;
	return _stack[n].expr;
}


static void set_expr(int n, char *expr)
{
	if (n < 0) n += _stack_current;
	_stack[n].expr = expr;
}


static CLASS *get_class(int n)
{
	TYPE type = get_type(n);
	
	if (type == T_CLASS)
		sscanf(get_expr(n), "CLASS(%p)", (void **)&type);
	else if (!TYPE_is_pure_object(type))
		type = 0;
	
	return (CLASS *)type;
}


static char *get_conv(TYPE src, TYPE dest)
{
	static char buffer[64];
	
	switch(dest)
	{
		case T_VOID:
			
			return "((void)%s)";
			
		case T_BOOLEAN:
			
			switch(TYPEID(src))
			{
				case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG: case T_SINGLE: case T_FLOAT: case T_POINTER:
					return "((%s)!=0)";
				case T_OBJECT:
					return "((%s).value!=0)";
			}
			break;
			
		case T_BYTE:
			
			switch(src)
			{
				case T_BOOLEAN:
					return "((uchar)(%s)?255:0)";
				case T_SHORT: case T_INTEGER: case T_LONG: case T_SINGLE: case T_FLOAT:
					return "((uchar)(%s))";
			}
			break;
			
		case T_SHORT:
			
			switch(src)
			{
				case T_BOOLEAN:
					return "((short)(%s)?-1:0)";
				case T_BYTE: case T_INTEGER: case T_LONG: case T_SINGLE: case T_FLOAT:
					return "((short)(%s))";
			}
			break;

		case T_INTEGER:
			
			switch(src)
			{
				case T_BOOLEAN:
					return "((int)(%s)?-1:0)";
				case T_BYTE: case T_SHORT: case T_LONG: case T_SINGLE: case T_FLOAT: case T_POINTER:
					return "((int)(%s))";
			}
			break;
		
		case T_LONG:
			
			switch(src)
			{
				case T_BOOLEAN:
					return "((int64_t)(%s)?-1:0)";
				case T_BYTE: case T_SHORT: case T_INTEGER: case T_SINGLE: case T_FLOAT: case T_POINTER:
					return "((int64_t)(%s))";
			}
			break;
			
		case T_SINGLE:
			
			switch(src)
			{
				case T_BOOLEAN:
					return "((float)(%s)?-1:0)";
				case T_BYTE: case T_SHORT: case T_INTEGER:
					return "((float)(%s))";
				case T_LONG: case T_FLOAT:
					return "(CHECK_FINITE((float)(%s)))";
			}
			break;
			
		case T_FLOAT:
			
			switch(src)
			{
				case T_BOOLEAN:
					return "((double)(%s)?-1:0)";
				case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG: case T_SINGLE:
					return "((double)(%s))";
			}
			break;
			
		case T_STRING:
			
			switch(src)
			{
				case T_CSTRING: return "%s";
			}
			break;

		case T_CSTRING:
			
			switch(src)
			{
				case T_STRING: return "%s";
			}
			break;
		
		default:
			
			if (src == T_NULL)
			{
				if (dest == T_OBJECT)
					return "GET_OBJECT(NULL, GB_T_OBJECT)";
				else
				{
					sprintf(buffer, "GET_OBJECT(NULL, CLASS(%p))", (CLASS *)dest);
					return buffer;
				}
			}
			
			if (TYPE_is_object(dest) && TYPE_is_object(src))
			{
				if (TYPE_is_pure_object(dest))
				{
					sprintf(buffer, "CONV_o_O(%%s, CLASS(%p))", (CLASS *)dest);
					return buffer;
				}
				else
					return "%s";
			}
			break;
	}
	
	if (TYPE_is_pure_object(dest))
		sprintf(buffer, "CONV(%%s, %s, %s, CLASS(%p))", JIT_get_type(src), JIT_get_type(dest), (CLASS *)dest);
	else
		sprintf(buffer, "CONV(%%s, %s, %s, %s)", JIT_get_type(src), JIT_get_type(dest), JIT_get_gtype(dest));
	
	return buffer;
}


static char *borrow_expr(char *expr, TYPE type)
{
	const char *type_name = JIT_get_type(type);
	int len;
	char *new_expr;
	
	len = strlen(expr);
	if ((strncmp(&expr[len - 3], "())", 3) == 0) && (strncmp(&expr[len - 8], "POP_", 4) == 0) && (expr[len - 4] == *type_name))
		new_expr = STR_print("%.*sPOP_BORROW_%s())", len - 8, expr, type_name);
	else
		new_expr = STR_print("BORROW_%s(%s)", type_name, expr);
	
	STR_free(expr);
	return new_expr;
}


static char *peek(int n, TYPE conv)
{
	char *expr;
	TYPE type;
	
	if (n < 0) n += _stack_current;
	
	expr = _stack[n].expr;
	type = _stack[n].type;
	
	if (type != conv)
	{
		char *expr2 = NULL;
		STR_add(&expr2, get_conv(type, conv), expr);
		STR_free(expr);
		expr = expr2;
		_stack[n].expr = expr2;
	}
	
	return expr;
}


static char *peek_pop(int n, TYPE conv, const char *fmt, va_list args)
{
	char *dest = NULL;
	char *expr;
	TYPE type;
	char *op;
	
	if (n < 0) n += _stack_current;
	
	expr = _stack[n].expr;
	type = _stack[n].type;
	
	if (fmt)
	{
		STR_vadd(&dest, fmt, args);
	
		if (!_no_release)
		{
			switch (TYPEID(conv))
			{
				case T_STRING:
					declare(&_decl_rs, "char *rs");
					JIT_print("  rs = (%s).value.addr;\n", dest);
					break;
					
				case T_OBJECT:
					declare(&_decl_ro, "void *ro");
					JIT_print("  ro = (%s).value;\n", dest);
					break;

				case T_VARIANT:
					declare(&_decl_rv, "GB_VARIANT rv");
					JIT_print("  rv = (%s);\n", dest);
					break;
			}
		}
	}
	
	if (type != conv)
	{
		char *expr2 = NULL;
		STR_add(&expr2, get_conv(type, conv), expr);
		STR_free(expr);
		expr = expr2;
		_stack[n].expr = expr2;
	}
	
	if (fmt)
	{
		if (_no_release)
		{
			JIT_print("  ");
			JIT_print(dest, expr);
			JIT_print(";\n");
		}
		else
		{
			if (dest[strlen(dest) - 1] != '=')
				op = " =";
			else
				op = "";
			
			switch (TYPEID(conv))
			{
				case T_STRING:
				case T_OBJECT:
				case T_VARIANT:
					_stack[n].expr = expr = borrow_expr(expr, conv);
					break;
			}
					
			JIT_print("  %s%s %s;\n", dest, op, expr);
			
			if (!_no_release)
			{
				switch (TYPEID(conv))
				{
					case T_STRING: JIT_print("  GB.FreeString(&rs);\n"); break;
					case T_OBJECT: JIT_print("  GB.Unref(&ro);\n"); break;
					case T_VARIANT:  JIT_print("  GB.ReleaseValue((GB_VALUE *)&rv);\n"); break;
				}
			}
		}
		
		STR_free(dest);
	}
	
	return expr;
}


static char *push_expr(int n, TYPE type)
{
	const char *type_name;
	char *expr;
	char *new_expr;
	int len;
	
	type_name = JIT_get_type(type);
	
	expr = peek(n, type);
	
	if (type == T_VOID)
		return "PUSH_V()";
	
	len = strlen(expr);
	if ((strncmp(&expr[len - 3], "())", 3) == 0) && (strncmp(&expr[len - 8], "POP_", 4) == 0) && (expr[len - 4] == *type_name))
		new_expr = STR_print("%.*s)", len - 9, expr);
	else
		new_expr = STR_print("PUSH_%s(%s)", type_name, expr);
	
	STR_free(expr);
	set_expr(n, new_expr);
	
	//fprintf(stderr, "push_expr %s ===> %s\n", expr, new_expr);
	
	return new_expr;
}


static void pop(TYPE type, const char *fmt, ...)
{
	va_list args;
	char *expr;
	
	check_stack(1);
	
	_stack_current--;
	
	va_start(args, fmt);
	expr = peek_pop(_stack_current, type, fmt, args);
	va_end(args);

	if (!fmt)
		JIT_print("  %s;\n", expr);
	
	free_stack(_stack_current);
}


static bool check_swap(TYPE type, const char *fmt, ...)
{
	va_list args;
	char *expr = NULL;
	char *swap = NULL;
	
	if (_has_just_dup)
	{
		_has_just_dup = FALSE;
		return TRUE;
	}
	
	if (_stack_current < 2)
		return TRUE;
	
	STR_add(&expr, "({ %s _t = %s; ", JIT_get_ctype(type), peek(-2, type));
	
	va_start(args, fmt);
	STR_vadd(&swap, fmt, args);
	va_end(args);
	STR_add(&expr, swap, peek(-1, type));
	STR_add(&expr, "; _t; })");
	
	pop_stack(2);
	
	push(type, "%s", expr);
	
	STR_free(swap);
	STR_free(expr);
	
	return FALSE;
}


static int add_ctrl(int index, TYPE type)
{
	int index_ctrl;
	
	index_ctrl = GB.Count(_ctrl_type);
	
	*(TYPE *)GB.Add(&_ctrl_type) = type;
	_ctrl_index[index] = index_ctrl;
	
	//JIT_print_decl("  %s c%d;\n", JIT_get_ctype(type), index_ctrl);
	JIT_declare(type, "c%d", index_ctrl);
	
	return index_ctrl;
}


static void pop_ctrl(int index, TYPE type)
{
	int index_ctrl;

	if (type == T_VOID)
		type = get_type(-1);
	
	index_ctrl = add_ctrl(index, type);
	
	//_no_release = TRUE;
	pop(type, "c%d", index_ctrl);
	//_no_release = FALSE;
}


static void push_constant(CLASS *class, int index)
{
	CLASS_CONST *cc = &class->load->cst[index];
	
	switch(cc->type)
	{
		case T_BOOLEAN: push(T_BOOLEAN, "(bool)%d", cc->_integer.value); break;
		case T_BYTE: push(T_BYTE, "(uchar)%d", cc->_integer.value); break;
		case T_SHORT: push(T_SHORT, "(short)%d", cc->_integer.value); break;
		case T_INTEGER: push(T_INTEGER, "(int)%d", cc->_integer.value); break;
		case T_LONG: push(T_LONG, "(int64_t)%d", cc->_long.value); break;
		case T_SINGLE: push(T_SINGLE, "(*(float *)%p)", &cc->_single.value); break;
		case T_FLOAT: push(T_FLOAT, "(*(double *)%p)", &cc->_float.value); break;
		case T_STRING: push(T_CSTRING, "CONSTANT_s(%p, %d)", cc->_string.addr, cc->_string.len); break;
		case T_CSTRING: push(T_CSTRING, "CONSTANT_t(%p, %d)", cc->_string.addr, 0); break;
		case T_POINTER: push(T_POINTER, "(intptr_t)0"); break;
		default: JIT_panic("unknown constant type");
	}
}


static void push_function(int func, int index)
{
	push(T_FUNCTION, NULL);
	_stack[_stack_current - 1].func = func;
	_stack[_stack_current - 1].index = index;
}


static void push_unknown(int index)
{
	TYPE type = T_UNKNOWN;
	TYPE call_type = T_UNKNOWN;
	char *expr;
	CLASS *class;
	
	check_stack(1);

	class = get_class(-1);
	
	if (class)
	{
		CLASS_DESC *desc;
		void *addr;
		int pos;
		TYPE utype;
		char *sym;
		
		sym = JIT_class->load->unknown[index];
		index = JIT_find_symbol(class, sym);
		
		if (index != NO_SYMBOL)
		{
			desc = class->table[index].desc;
			utype = JIT_ctype_to_type(desc->variable.ctype);

			switch (CLASS_DESC_get_type(desc))
			{
				case CD_STATIC_VARIABLE:
					
					pop_stack(1);
					
					addr = (char *)desc->variable.class->stat + desc->variable.offset;
					
					if (TYPE_is_object(utype))
					{
						if (TYPE_is_pure_object(utype))
							push(utype, "GET_o(%p, CLASS(%p))", addr, (CLASS *)utype);
						else
							push(utype, "GET_o(%p, GB_T_OBJECT)", addr);
					}
					else
						push(utype, "GET_%s(%p)", JIT_get_type(utype), addr);
					
					return;
					
				case CD_VARIABLE:
					
					expr = STR_copy(peek(-1, (TYPE)class));
					pop_stack(1);
					
					pos = desc->variable.offset;
					
					if (TYPE_is_object(utype))
					{
						if (TYPE_is_pure_object(utype))
							push(utype, "GET_o(ADDR(%s) + %d, CLASS(%p))", expr, pos, (CLASS *)utype);
						else
							push(utype, "GET_o(ADDR(%s) + %d, GB_T_OBJECT)", expr, pos);
					}
					else
						push(utype, "GET_%s(ADDR(%s) + %d)", JIT_get_type(utype), expr, pos);
					
					goto __RETURN;
					
				case CD_CONSTANT:
					
					pop_stack(1);
					
					switch(desc->constant.type)
					{
						case T_BOOLEAN: push(T_BOOLEAN, "(bool)%d", desc->constant.value._integer); break;
						case T_BYTE: push(T_BYTE, "(uchar)%d", desc->constant.value._integer); break;
						case T_SHORT: push(T_SHORT, "(short)%d", desc->constant.value._integer); break;
						case T_INTEGER: push(T_INTEGER, "(int)%d", desc->constant.value._integer); break;
						case T_LONG: push(T_LONG, "(int64_t)%d", desc->constant.value._long); break;
						case T_SINGLE: push(T_SINGLE, "(*(float *)%p)", &desc->constant.value._single); break;
						case T_FLOAT: push(T_FLOAT, "(*(double *)%p)", &desc->constant.value._float); break;
						case T_POINTER: push(T_POINTER, "(intptr_t)%p", desc->constant.value._pointer); break;
						case T_STRING: case T_CSTRING:
							if (desc->constant.translate)
								push(T_CSTRING, "CONSTANT_t(%p, %d)", desc->constant.value._string, strlen(desc->constant.value._string));
							else
								push(T_CSTRING, "CONSTANT_s(%p, %d)", desc->constant.value._string, strlen(desc->constant.value._string));
							break;
						default: JIT_panic("unknown constant type");
					}
					
					return;
					
				case CD_PROPERTY:
				case CD_STATIC_PROPERTY:
				case CD_PROPERTY_READ:
				case CD_STATIC_PROPERTY_READ:
					
					type = desc->property.type;
					break;
					
				case CD_METHOD:
				case CD_STATIC_METHOD:
					
					call_type = desc->property.type;
					break;
			}
		}
		else
			JIT_print("  // %s.%s ?\n", class->name, sym);
	}
	
	expr = STR_copy(push_expr(-1, get_type(-1)));
	pop_stack(1);
	push(type, "(%s,PUSH_UNKNOWN(%d),POP_%s())", expr, _pc, JIT_get_type(type));
	
	_stack[_stack_current - 1].call = call_type;

__RETURN:

	STR_free(expr);
}


static void pop_unknown(int index)
{
	CLASS *class;
	char *expr = NULL;
	char *arg;
	
	check_stack(2);
	
	class = get_class(-1);

	if (class)
	{
		CLASS_DESC *desc;
		void *addr;
		int pos;
		TYPE utype;
		const char *sym;
		
		sym = JIT_class->load->unknown[index];
		index = JIT_find_symbol(class, sym);
		if (index != NO_SYMBOL)
		{
			desc = class->table[index].desc;
			utype = JIT_ctype_to_type(desc->variable.ctype);

			switch (CLASS_DESC_get_type(desc))
			{
				case CD_STATIC_VARIABLE:
					
					pop_stack(1);
					
					addr = (char *)desc->variable.class->stat + desc->variable.offset;
					
					_no_release = TRUE;
					if (check_swap(utype, "SET_%s(%p, %%s)", JIT_get_type(utype), addr))
						pop(utype, "SET_%s(%p, %%s)", JIT_get_type(utype), addr);
					_no_release = FALSE;
					
					return;
					
				case CD_VARIABLE:
					
					expr = STR_copy(peek(-1, (TYPE)class));
					pop_stack(1);
					
					pos = desc->variable.offset;
					
					_no_release = TRUE;
					if (check_swap(utype, "SET_%s(ADDR(%s) + %d, %%s)", JIT_get_type(utype), expr, pos))
						pop(utype, "SET_%s(ADDR(%s) + %d, %%s)", JIT_get_type(utype), expr, pos);
					_no_release = FALSE;
					
					goto __RETURN;
			}
		}
		else
			JIT_print("  // %s.%s ?\n", class->name, sym);
	}

	arg = push_expr(-2, get_type(-2));
	STR_add(&expr,"(%s", arg);
	
	arg = push_expr(-1, get_type(-1));
	STR_add(&expr, ",%s,POP_UNKNOWN(%d))", arg, _pc);

	pop_stack(2);
	
	push(T_VOID, "%s", expr);
	
	if (check_swap(T_UNKNOWN, "%s", expr))
		pop(T_VOID, NULL);
	
__RETURN:
	
	STR_free(expr);
}


static void push_array(ushort code)
{
	TYPE type;
	int i, narg;
	char *expr = NULL;
	char *expr1, *expr2;
	
	narg = code & 0x3F;
	check_stack(narg);
	
	type = get_type(-narg);
	
	if (TYPE_is_pure_object(type))
	{
		CLASS *class = (CLASS *)type;

		//JIT_print("  // %s %d\n", class->name, class->is_array);
		
		if (class->is_array)
		{
			type = class->array_type;
			
			if (narg == 2)
			{
				expr1 = peek(-2, get_type(-2));
				expr2 = peek(-1, T_INTEGER);
				
				if (TYPE_is_pure_object(type))
					expr = STR_print("PUSH_ARRAY_o(%s,%s,CLASS(%p))", expr1, expr2, (CLASS *)type);
				else
					expr = STR_print("PUSH_ARRAY_%s(%s,%s)", JIT_get_type(type), expr1, expr2);
				
				pop_stack(2);
				
				push(type, "(%s)", expr);
				
				STR_free(expr);
				
				return;
			}
		}
		else
			type = T_UNKNOWN;
	}
	else
		type = T_UNKNOWN;
	
	//declare_sp();
	
	for (i = _stack_current - narg; i < _stack_current; i++)
	{
		STR_add(&expr, "%s,", push_expr(i, get_type(i)));
		free_stack(i);
	}
	
	_stack_current -= narg;
	
	STR_add(&expr, "CALL_PUSH_ARRAY(%d, 0x%04X),POP_%s()", _pc, code, JIT_get_type(type));
	
	push(type, "(%s)", expr);
	
	STR_free(expr);
}


static void pop_array(ushort code)
{
	TYPE type;
	int i, narg;
	char *expr = NULL;
	char *expr1, *expr2;
	
	narg = code & 0x3F;
	check_stack(narg + 1);
	
	type = get_type(-narg);
	
	if (TYPE_is_pure_object(type))
	{
		CLASS *class = (CLASS *)type;
		
		if (class->is_array)
		{
			type = class->array_type;
		
			if (narg == 2)
			{
				expr1 = peek(-2, get_type(-2));
				expr2 = peek(-1, T_INTEGER);
				
				STR_add(&expr, "POP_ARRAY_%s(%s,%s,%s)", JIT_get_type(type), expr1, expr2, peek(-3, type));
				
				pop_stack(3);
				
				goto CHECK_SWAP;
			}
		}
		
	}
	else
		type = T_UNKNOWN;
	
	//declare_sp();
	
	narg++;
	
	for (i = _stack_current - narg; i < _stack_current; i++)
	{
		STR_add(&expr, "%s,", push_expr(i, get_type(i)));
		free_stack(i);
	}
	
	_stack_current -= narg;
	
	STR_add(&expr, "CALL_POP_ARRAY(%d, 0x%04X),sp--", _pc, code);
	
CHECK_SWAP:
	
	push(T_VOID, "(%s)", expr);
	
	if (check_swap(type, "(%s)", expr))
		pop(T_VOID, NULL);
	
	STR_free(expr);
}

static void push_subr(char mode, ushort code)
{
	const char *call;
	TYPE type;
	int i, narg;
	char *expr = NULL;
	ushort op;
	bool rst = FALSE;
	char type_id = 0;
	void *addr;
	
	//declare_sp();
	
	//JIT_print("  static ushort s%d = 0x%04X;\n", _subr_count, code);
	
	op = code >> 8;
	
	switch(mode)
	{
		case CALL_SUBR_CODE:
			call = "CALL_SUBR_CODE(%d, %p, 0x%04X)";
			addr = JIT.subr_table[op];
			break;
		case CALL_SUBR:
			call = "CALL_SUBR(%d, %p)";
			addr = JIT.subr_table[op];
			break;
		case CALL_SUBR_UNKNOWN:
			call = "CALL_SUBR_UNKNOWN(%d)";
			addr = NULL;
			break;
		case CALL_NEW:
			call = "CALL_SUBR_CODE(%d, %p, 0x%04X)";
			addr = JIT.new;
			break;
	}
	
	if (op == (C_NEW >> 8))
	{
		narg = code & 0x3F;
		type = get_type(-narg);
	}
	else if (op < CODE_FIRST_SUBR)
	{
		int index = RESERVED_get_from_opcode(code);
		
		if (index < 0)
			JIT_panic("Unknown operator");
		
		if (RES_is_unary(index))
			narg = 1;
		else if (RES_is_binary(index))
			narg = 2;
		else
			narg = code & 0x3F;

		type_id = COMP_res_info[index].type;
		rst = TRUE;
	}
	else
	{
		SUBR_INFO *info = SUBR_get_from_opcode(op - CODE_FIRST_SUBR, code & 0x3F);
		if (!info)
			JIT_panic("unknown subroutine");
		if (info->min_param <= info->max_param)
			narg = code & 0x3F;
		else
			narg = info->min_param;
		
		type_id = info->type;
		rst = TRUE;
	}

	check_stack(narg);
	
	if (rst)
	{
		switch(type_id)
		{
			case RST_SAME:
			case RST_BCLR:
				type = get_type(-narg);
				break;
				
			case RST_MIN:
				type = Max(get_type(-1), get_type(-2));
				if (type > T_DATE && type != T_VARIANT)
					type = T_UNKNOWN;
			
			default:
				type = (type_id >= T_UNKNOWN) ? T_UNKNOWN : type_id;
		}
	}
	
	if (op == (C_NEW >> 8))
	{
		if (type == T_CLASS)
			type = (TYPE)get_class(-narg);
		else
			type = T_OBJECT;
	}
	
	for (i = _stack_current - narg; i < _stack_current; i++)
	{
		STR_add(&expr, "%s,", push_expr(i, get_type(i)));
		free_stack(i);
	}
	
	_stack_current -= narg;
	
	STR_add(&expr, call, _pc, addr, code);
	STR_add(&expr, ",POP_%s()", JIT_get_type(type));
	
	push(type, "(%s)", expr);
	
	STR_free(expr);
}


static void push_subr_add(ushort code, const char *op, const char *opb, bool allow_pointer)
{
	char *expr;
	char *expr1, *expr2;
	TYPE type1, type2, type;
	
	check_stack(2);
	
	type1 = get_type(-2);
	type2 = get_type(-1);
	
	if (TYPEID(type1) > TYPEID(type2))
		type = type1;
	else
		type = type2;
	
	switch(type)
	{
		case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG: case T_SINGLE: case T_FLOAT:
			break;
			
		case T_DATE: case T_STRING: case T_CSTRING:
			type = T_FLOAT;
			break;
			
		case T_POINTER:
			if (allow_pointer)
				break;
			
		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}
	
	expr1 = peek(-2, type);
	expr2 = peek(-1, type);

	if (type == T_BOOLEAN)
		op = opb;
	
	expr = STR_print("({%s _a = %s; %s _b = %s; _a %s _b;})", JIT_get_ctype(type), expr1, JIT_get_ctype(type), expr2, op);
	
	pop_stack(2);
	
	push(type, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_div(ushort code)
{
	char *expr;
	char *expr1, *expr2;
	TYPE type1, type2, type;
	
	check_stack(2);
	
	type1 = get_type(-2);
	type2 = get_type(-1);
	
	if (TYPEID(type1) > TYPEID(type2))
		type = type1;
	else
		type = type2;
	
	switch(type)
	{
		case T_SINGLE: case T_FLOAT:
			break;
			
		case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG:
			type = T_FLOAT;
			break;
			
		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}
	
	expr1 = peek(-2, type);
	expr2 = peek(-1, type);
	expr = STR_print("({%s _a = %s; %s _b = %s; _a /= _b; if (!isfinite(_a)) JIT.throw(E_ZERO); _a;})", JIT_get_ctype(type), expr1, JIT_get_ctype(type), expr2);

	pop_stack(2);
	
	push(type, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_neg(ushort code)
{
	TYPE type;
	char *expr;
	
	check_stack(1);
	
	type = get_type(-1);
	
	switch(type)
	{
		case T_BOOLEAN:
			return;

		case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG: case T_SINGLE: case T_FLOAT:
			break;

		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}
	
	expr = STR_copy(peek(-1, type));
	
	pop_stack(1);
	push(type, "(- %s)", expr);
	STR_free(expr);
}


static void push_subr_quo(ushort code, const char *op)
{
	char *expr;
	char *expr1, *expr2;
	TYPE type1, type2, type;
	
	check_stack(2);
	
	type1 = get_type(-2);
	type2 = get_type(-1);
	
	if (TYPEID(type1) > TYPEID(type2))
		type = type1;
	else
		type = type2;
	
	switch(type)
	{
		case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG:
			break;
			
		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}
	
	expr1 = peek(-2, type);
	expr2 = peek(-1, type);

	expr = STR_print("({%s _a = %s; %s _b = %s; if (_b == 0) JIT.throw(E_ZERO); _a %s _b;})", JIT_get_ctype(type), expr1, JIT_get_ctype(type), expr2, op);
	
	pop_stack(2);
	push(type, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_and(ushort code, const char *op)
{
	char *expr;
	char *expr1, *expr2;
	TYPE type1, type2, type;
	
	check_stack(2);
	
	type1 = get_type(-2);
	type2 = get_type(-1);
	
	if (TYPEID(type1) > TYPEID(type2))
		type = type1;
	else
		type = type2;
	
	switch(type)
	{
		case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG:
			break;
			
		case T_DATE: case T_STRING: case T_CSTRING:
			type = T_BOOLEAN;
			break;
			
		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}
	
	expr1 = peek(-2, type);
	expr2 = peek(-1, type);

	expr = STR_print("({%s _a = %s; %s _b = %s; _a %s _b;})", JIT_get_ctype(type), expr1, JIT_get_ctype(type), expr2, op);
	
	pop_stack(2);
	push(type, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_not(ushort code)
{
	TYPE type;
	char *expr;
	char *op;
	
	check_stack(1);
	
	type = get_type(-1);
	
	switch(type)
	{
		case T_BOOLEAN:
			op = "!";
			break;

		case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG:
			op = "~";
			break;

		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}
	
	expr = STR_print("%s%s", op, peek(-1, type));
	
	pop_stack(1);
	push(type, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_comp(ushort code)
{
	char *op = NULL;
	char *expr;
	char *expr1, *expr2;
	TYPE type1, type2, type;
	
	check_stack(2);
	
	type1 = get_type(-2);
	type2 = get_type(-1);
	
	if (TYPEID(type1) > TYPEID(type2))
		type = type1;
	else
		type = type2;
	
	switch(type)
	{
		case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER: case T_LONG: case T_SINGLE: case T_FLOAT: case T_POINTER:
			
			switch(code & 0xFF00)
			{
				case C_EQ: op = "=="; break;
				case C_NE: op = "!="; break;
				case C_GT: op = ">"; break;
				case C_LT: op = "<"; break;
				case C_GE: op = ">="; break;
				case C_LE: op = "<="; break;
			}
			break;
	}
	
	if (!op)
	{
		switch(code & 0xFF00)
		{
			case C_EQ: case C_NE:
				push_subr(CALL_SUBR_CODE, code);
				break;
				
			case C_GT: case C_LT: case C_GE: case C_LE:
				push_subr(CALL_SUBR_UNKNOWN, code);
				break;
		}
		
		return;
	}
	
	expr1 = peek(-2, type);
	expr2 = peek(-1, type);

	expr = STR_print("({%s _a = %s; %s _b = %s; _a %s _b;})", JIT_get_ctype(type), expr1, JIT_get_ctype(type), expr2, op);

	pop_stack(2);
	
	push(T_BOOLEAN, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_bit(ushort code)
{
	static const char *_actions[] = {
		NULL,
		"_v &= ~((%s)1 << _b)", // BClr
		"_v |= ((%s)1 << _b)", // BSet
		"((_v & ((%s)1 << _b)) != 0)", // BTst
		"_v ^= ((%s)1 << _b)", // BChg
		"_v = (((%s)_v << _b) & 0x%s) | (((%s)_v) & 0x%s)", // Asl
		"_v = (((%s)_v >> _b) & 0x%s) | (((%s)_v) & 0x%s)", // Asr
		"_v = ((%s)_v << _b) | ((%s)_v >> (%d - _b))", // Rol
		"_v = ((%s)_v >> _b) | ((%s)_v << (%d - _b))", // Ror
		"_v = ((%s)_v << _b)", // Lsl
		"_v = ((%s)_v >> _b)", // Lsr
	};

	const char *action;
	char *expr;
	char *expr1, *expr2;
	TYPE type;
	const char *ctype, *uctype, *mask, *mask2;
	int nbits;
	
	check_stack(2);
	
	type = get_type(-2);
	
	switch(type)
	{
		case T_BYTE:
			ctype = "uchar"; uctype = "uchar";
			mask = "7F"; mask2 = "80";
			nbits = 8;
			break;

		case T_SHORT:
			ctype = "short"; uctype=  "ushort";
			mask = "7FFF"; mask2 = "8000";
			nbits = 16;
			break;

		case T_INTEGER:
			ctype = "int"; uctype =  "uint"; 
			mask = "7FFFFFFF"; mask2 = "80000000";
			nbits = 32;
			break;

		case T_LONG:
			ctype = "int64_t"; uctype=  "uint64_t";
			mask = "7FFFFFFFFFFFFFFFLL"; mask2 = "8000000000000000LL";
			nbits = 64;
			break;

		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}
			
	expr1 = peek(-2, type);
	expr2 = peek(-1, T_INTEGER);

	code &= 0x3F;
	
	action = _actions[code];
	
	expr = STR_print("({ %s _v = %s; int _b = %s; if ((_b < 0) || (_b >= %d)) JIT.throw(E_ARG); ", ctype, expr1, expr2, nbits);
	
	
	switch(code)
	{
		case 1: case 2: case 3: case 4:
			STR_add(&expr, action, uctype);
			break;
			
		case 5: case 6:
			STR_add(&expr, action, uctype, mask, uctype, mask2);
			break;
			
		case 7: case 8:
			STR_add(&expr, action, uctype, uctype, nbits);
			break;

		case 9: case 10:
			STR_add(&expr, action, uctype);
			break;
	}
	
	if (code == 3)
		STR_add(&expr, "; })");
	else
		STR_add(&expr, "; _v; })");

	pop_stack(2);
	
	push(code == 3 ? T_BOOLEAN : type, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_conv(ushort code)
{
	char *expr;
	TYPE type;
	TYPE conv = code & 0x3F;

	check_stack(1);
	type = get_type(-1);
	
	if (type == conv)
		return;
	
	expr = STR_copy(peek(-1, conv));
	pop_stack(1);
	push(conv, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_len(ushort code)
{
	char *expr;

	check_stack(1);
	
	expr = STR_copy(peek(-1, T_STRING));
	pop_stack(1);
	push(T_INTEGER, "((%s).value.len)", expr);
	STR_free(expr);
}


/*static void push_subr_left(ushort code)
{
	char *expr = NULL;
	TYPE type;
	char *expr_str, *expr_len = NULL;
	
	declare_sp();
	
	JIT_print("  static ushort s%d = 0x%04X;\n", _subr_count, code);
	
	check_stack(1);
	
	if (_stack_current >= 2)
	{
		expr_len = peek(-1, T_INTEGER);
		pop_stack(1);
	}
	
	type = get_type(-1);
	expr_str = peek(-1, T_CSTRING);
	pop_stack(1);
	
	STR_add(&expr, "(PUSH_t(%s)", expr_str);
	STR_free(expr_str);
	if (expr_len)
	{
		STR_add(&expr, ",PUSH_i(%s)", expr_len);
		STR_free(expr_len);
	}

	push(type, "%s,CALL_SUBR_CODE(s%d),POP_t())", expr, _subr_count, code);
	_subr_count++;
}*/


static void push_call(ushort code)
{
	char *call = NULL;
	const char *def;
	int i, j;
	int narg;
	FUNCTION *func;
	int func_kind, func_index;
	TYPE func_type;
	int nopt, opt;
	
	narg = code & 0x3F;
	
	if (_stack_current > narg && get_type(- narg - 1) == T_FUNCTION)
	{
		STACK_SLOT *s = &_stack[_stack_current - narg - 1];
		func_kind = s->func;
		func_index = s->index;
		func_type = T_UNKNOWN;
	}
	else
	{
		STACK_SLOT *s = &_stack[_stack_current - narg - 1];
		func_kind = CALL_UNKNOWN;
		func_type = s->call;
	}
	
	switch (func_kind)
	{
		case CALL_PRIVATE:
			
			func = &JIT_class->load->func[func_index];
			if (func->fast)
			{
				if (narg < func->npmin)
				{
					pop_stack(narg + 1);
					push(T_UNKNOWN, "(JIT.throw(E_NEPARAM))");
				}
				else if (narg > func->n_param)
				{
					pop_stack(narg + 1);
					push(T_UNKNOWN, "(JIT.throw(E_TMPARAM))");
				}
				else
				{
					STR_add(&call,"SP = sp, jit_%s_%d_(", JIT_prefix, func_index);
					
					for (i = 0; i < func->npmin; i++)
					{
						if (i) STR_add(&call, ",");
						STR_add(&call, "%s", peek(i - narg, func->param[i].type));
					}
					
					nopt = 0;
					for (; i < func->n_param; i++)
					{
						if (i) STR_add(&call, ",");
						
						if (nopt == 0)
						{
							opt = 0;
							for (j = 0; j < 8; j++)
							{
								if ((i + j) >= func->n_param)
									break;
								if (((i + j) >= narg) || get_type(i + j - narg) == T_VOID)
									opt |= 1 << j;
							}
							STR_add(&call, "%d,", opt);
						}
						
						if (i < narg)
							STR_add(&call, "%s", peek(i - narg, func->param[i].type));
						else
						{
							def = JIT_get_default_value(func->param[i].type);
							STR_add(&call, "%s", def);
						}
						
						nopt++;
						if (nopt >= 8)
							nopt = 0;
					}
					
					STR_add(&call, ")");
					
					pop_stack(narg + 1);
				
					push(func->type, "(%s)", call);
				}
			}
			else
			{
				STR_add(&call, "PUSH_PRIVATE_FUNCTION(%d),", func_index);
				
				for (i = 0; i < narg; i++)
					STR_add(&call, "%s,", push_expr(i - narg, get_type(i - narg)));
				
				pop_stack(narg + 1);
				
				STR_add(&call, "CALL_UNKNOWN(%d),POP_%s()", _pc, JIT_get_type(func->type));
		
				push(func->type, "(%s)", call);
			}
			
			break;
				
		case CALL_UNKNOWN:
			
			narg++;
			for (i = 0; i < narg; i++)
				STR_add(&call, "%s,", push_expr(i - narg, get_type(i - narg)));
			
			pop_stack(narg);
			
			STR_add(&call, "CALL_UNKNOWN(%d),POP_%s()", _pc, JIT_get_type(func_type));
	
			push(func_type, "(%s)", call);
			
			break;
	
		default:
			
			JIT_panic("Unsupported call");
	}
	
	STR_free(call);
}


static void push_subr_isnan(ushort code)
{
	char *func;
	char *expr;
	
	check_stack(1);
	
	switch (code & 0xFF)
	{
		case 1: // IsNan
			func = "isnan";
			break;

		case 2: // IsInf
			func = "isinf";
			break;
		
		default:
			push_subr(CALL_SUBR_CODE, code);
			return;
	}

	expr = STR_print("%s(%s) != 0", func, peek(-1, T_FLOAT));
	
	pop_stack(1);
	push(T_BOOLEAN, "(%s)", expr);
	STR_free(expr);
}


static void push_subr_math(ushort code)
{
	static const char *func[] = {
		NULL, "frac", "log", "exp", "sqrt", "sin", "cos", "tan", "atan", "asin", "acos",
		"deg", "rad", "log10", "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
		"exp2", "exp10", "log2", "cbrt", "expm1", "log1p", "floor", "ceil"
	};

	char *expr;
	
	check_stack(1);
	
	expr = STR_copy(peek(-1, T_FLOAT));
	pop_stack(1);
	
	push(T_FLOAT, "CALL_MATH(%s, %s)", func[code & 0x1F], expr);
	
	STR_free(expr);
}


static void push_complex(void)
{
	char *expr;
	
	expr = STR_copy(peek(-1, T_FLOAT));
	pop_stack(1);
	
	push(T_OBJECT, "PUSH_COMPLEX(%s)", expr);
	
	STR_free(expr);
}


#define GET_XXX()   (((signed short)(code << 4)) >> 4)
#define GET_UXX()   (code & 0xFFF)
#define GET_7XX()   (code & 0x7FF)
#define GET_XX()    ((signed char)code)
#define GET_UX()    ((unsigned char)code)
#define GET_3X()    (code & 0x3F)
#define TEST_XX()   (code & 1)
#define PC (&func->code[p])

bool JIT_translate_body(FUNCTION *func, int ind)
{
	static const void *jump_table[256] =
	{
		/* 00 NOP             */  &&_MAIN,
		/* 01 PUSH LOCAL      */  &&_PUSH_LOCAL,
		/* 02 PUSH PARAM      */  &&_PUSH_PARAM,
		/* 03 PUSH ARRAY      */  &&_PUSH_ARRAY,
		/* 04 PUSH UNKNOWN    */  &&_PUSH_UNKNOWN,
		/* 05 PUSH EXTERN     */  &&_PUSH_EXTERN,
		/* 06 BYREF           */  &&_BYREF,
		/* 07 PUSH EVENT      */  &&_PUSH_EVENT,
		/* 08 QUIT            */  &&_QUIT,
		/* 09 POP LOCAL       */  &&_POP_LOCAL,
		/* 0A POP PARAM       */  &&_POP_PARAM,
		/* 0B POP ARRAY       */  &&_POP_ARRAY,
		/* 0C POP UNKNOWN     */  &&_POP_UNKNOWN,
		/* 0D POP OPTIONAL    */  &&_POP_OPTIONAL,
		/* 0E POP CTRL        */  &&_POP_CTRL,
		/* 0F BREAK           */  &&_BREAK,
		/* 10 RETURN          */  &&_RETURN,
		/* 11 PUSH SHORT      */  &&_PUSH_SHORT,
		/* 12 PUSH INTEGER    */  &&_PUSH_INTEGER,
		/* 13 PUSH CHAR       */  &&_PUSH_CHAR,
		/* 14 PUSH MISC       */  &&_PUSH_MISC,
		/* 15 PUSH ME         */  &&_PUSH_ME,
		/* 16 TRY             */  &&_TRY,
		/* 17 END TRY         */  &&_END_TRY,
		/* 18 CATCH           */  &&_CATCH,
		/* 19 DUP             */  &&_DUP,
		/* 1A DROP            */  &&_DROP,
		/* 1B NEW             */  &&_NEW,
		/* 1C CALL            */  &&_CALL,
		/* 1D CALL QUICK      */  &&_CALL_QUICK,
		/* 1E CALL EASY       */  &&_CALL_SLOW,
		/* 1F ON              */  &&_ON_GOTO_GOSUB,
		/* 20 JUMP            */  &&_JUMP,
		/* 21 JUMP IF TRUE    */  &&_JUMP_IF_TRUE,
		/* 22 JUMP IF FALSE   */  &&_JUMP_IF_FALSE,
		/* 23 GOSUB           */  &&_GOSUB,
		/* 24 JUMP FIRST      */  &&_JUMP_FIRST,
		/* 25 JUMP NEXT       */  &&_JUMP_NEXT,
		/* 26 FIRST           */  &&_ENUM_FIRST,
		/* 27 NEXT            */  &&_ENUM_NEXT,
		/* 28 =               */  &&_SUBR_COMPE,
		/* 29 <>              */  &&_SUBR_COMPN,
		/* 2A >               */  &&_SUBR_COMPGT,
		/* 2B <=              */  &&_SUBR_COMPLE,
		/* 2C <               */  &&_SUBR_COMPLT,
		/* 2D >=              */  &&_SUBR_COMPGE,
		/* 2E ==              */  &&_SUBR,
		/* 2F CASE            */  &&_SUBR_CODE,
		/* 30 +               */  &&_SUBR_ADD,
		/* 31 -               */  &&_SUBR_SUB,
		/* 32 *               */  &&_SUBR_MUL,
		/* 33 /               */  &&_SUBR_DIV,
		/* 34 NEG             */  &&_SUBR_NEG,
		/* 35 \               */  &&_SUBR_QUO,
		/* 36 MOD             */  &&_SUBR_REM,
		/* 37 ^               */  &&_SUBR_CODE,
		/* 38 AND             */  &&_SUBR_AND,
		/* 39 OR              */  &&_SUBR_OR,
		/* 3A XOR             */  &&_SUBR_XOR,
		/* 3B NOT             */  &&_SUBR_NOT,
		/* 3C &               */  &&_SUBR_CODE,
		/* 3D LIKE            */  &&_SUBR_CODE,
		/* 3E &/              */  &&_SUBR_CODE,
		/* 3F Is              */  &&_SUBR_CODE,
		/* 40 Left$           */  &&_SUBR_CODE,
		/* 41 Mid$            */  &&_SUBR_CODE,
		/* 42 Right$          */  &&_SUBR_CODE,
		/* 43 Len             */  &&_SUBR_LEN,
		/* 44 Space$          */  &&_SUBR,
		/* 45 String$         */  &&_SUBR,
		/* 46 Trim$           */  &&_SUBR_CODE,
		/* 47 UCase$          */  &&_SUBR_CODE,
		/* 48 Oct$            */  &&_SUBR_CODE,
		/* 49 Chr$            */  &&_SUBR,
		/* 4A Asc             */  &&_SUBR_CODE,
		/* 4B InStr           */  &&_SUBR_CODE,
		/* 4C RInStr          */  &&_SUBR_CODE,
		/* 4D Subst$          */  &&_SUBR_CODE,
		/* 4E Replace$        */  &&_SUBR_CODE,
		/* 4F Split           */  &&_SUBR_CODE,
		/* 50 Scan            */  &&_SUBR,
		/* 51 Comp            */  &&_SUBR_CODE,
		/* 52 Conv            */  &&_SUBR,
		/* 53 DConv           */  &&_SUBR_CODE,
		/* 54 Abs             */  &&_SUBR_CODE,
		/* 55 Int             */  &&_SUBR_CODE,
		/* 56 Fix             */  &&_SUBR_CODE,
		/* 57 Sgn             */  &&_SUBR_CODE,
		/* 58 Frac...         */  &&_SUBR_MATH,
		/* 59 Pi              */  &&_SUBR_CODE,
		/* 5A Round           */  &&_SUBR_CODE,
		/* 5B Randomize       */  &&_SUBR_CODE,
		/* 5C Rnd             */  &&_SUBR_CODE,
		/* 5D Min             */  &&_SUBR_CODE,
		/* 5E Max             */  &&_SUBR_CODE,
		/* 5F IIf             */  &&_SUBR_CODE,
		/* 60 Choose          */  &&_SUBR_CODE,
		/* 61 Array           */  &&_SUBR_CODE,
		/* 62 ATan2...        */  &&_SUBR_CODE,
		/* 63 IsAscii...      */  &&_SUBR_CODE,
		/* 64 BClr...         */  &&_SUBR_BIT,
		/* 65 IsBoolean...    */  &&_SUBR_CODE,
		/* 66 TypeOf          */  &&_SUBR_CODE,
		/* 67 CBool...        */  &&_SUBR_CONV,
		/* 68 Bin$            */  &&_SUBR_CODE,
		/* 69 Hex$            */  &&_SUBR_CODE,
		/* 6A Val             */  &&_SUBR,
		/* 6B Str             */  &&_SUBR,
		/* 6C Format          */  &&_SUBR_CODE,
		/* 6D Timer           */  &&_SUBR,
		/* 6E Now             */  &&_SUBR,
		/* 6F Year...         */  &&_SUBR_CODE,
		/* 70 Week            */  &&_SUBR_CODE,
		/* 71 Date            */  &&_SUBR_CODE,
		/* 72 Time...         */  &&_SUBR_CODE,
		/* 73 DateAdd...      */  &&_SUBR_CODE,
		/* 74 Eval            */  &&_SUBR_CODE,
		/* 75 Error           */  &&_SUBR,
		/* 76 Debug           */  &&_SUBR,
		/* 77 Wait            */  &&_SUBR_CODE,
		/* 78 Open            */  &&_SUBR_CODE,
		/* 79 Close           */  &&_SUBR,
		/* 7A Input           */  &&_SUBR_CODE,
		/* 7B LineInput       */  &&_SUBR,
		/* 7C Print           */  &&_SUBR_CODE,
		/* 7D Read            */  &&_SUBR_CODE,
		/* 7E Write           */  &&_SUBR_CODE,
		/* 7F Flush           */  &&_SUBR,
		/* 80 Lock...         */  &&_SUBR_CODE,
		/* 81 InputFrom...    */  &&_SUBR_CODE,
		/* 82 Eof             */  &&_SUBR_CODE,
		/* 83 Lof             */  &&_SUBR_CODE,
		/* 84 Seek            */  &&_SUBR_CODE,
		/* 85 Kill            */  &&_SUBR_CODE,
		/* 86 Mkdir           */  &&_SUBR_CODE,
		/* 87 Rmdir           */  &&_SUBR_CODE,
		/* 88 Move            */  &&_SUBR_CODE,
		/* 89 Copy            */  &&_SUBR_CODE,
		/* 8A Link            */  &&_SUBR_ISNAN,
		/* 8B Exist           */  &&_SUBR_CODE,
		/* 8C Access          */  &&_SUBR_CODE,
		/* 8D Stat            */  &&_SUBR_CODE,
		/* 8E Dfree           */  &&_SUBR,
		/* 8F Temp$           */  &&_SUBR_CODE,
		/* 90 IsDir           */  &&_SUBR,
		/* 91 Dir             */  &&_SUBR_CODE,
		/* 92 RDir            */  &&_SUBR_CODE,
		/* 93 Exec...         */  &&_SUBR_CODE,
		/* 94 Alloc           */  &&_SUBR_CODE,
		/* 95 Free            */  &&_SUBR,
		/* 96 Realloc         */  &&_SUBR_CODE,
		/* 97 StrPtr          */  &&_SUBR_CODE,
		/* 98 Sleep...        */  &&_SUBR_CODE,
		/* 99 VarPtr          */  &&_SUBR_CODE,
		/* 9A Collection      */  &&_SUBR_CODE,
		/* 9B Tr$             */  &&_SUBR,
		/* 9C Quote$...       */  &&_SUBR_CODE,
		/* 9D Unquote$...     */  &&_SUBR_CODE,
		/* 9E MkInt$...       */  &&_SUBR_CODE,
		/* 9F Byte@...        */  &&_SUBR_CODE,
		/* A0 ADD QUICK       */  &&_ADD_QUICK,
		/* A1 ADD QUICK       */  &&_ADD_QUICK,
		/* A2 ADD QUICK       */  &&_ADD_QUICK,
		/* A3 ADD QUICK       */  &&_ADD_QUICK,
		/* A4 ADD QUICK       */  &&_ADD_QUICK,
		/* A5 ADD QUICK       */  &&_ADD_QUICK,
		/* A6 ADD QUICK       */  &&_ADD_QUICK,
		/* A7 ADD QUICK       */  &&_ADD_QUICK,
		/* A8 ADD QUICK       */  &&_ADD_QUICK,
		/* A9 ADD QUICK       */  &&_ADD_QUICK,
		/* AA ADD QUICK       */  &&_ADD_QUICK,
		/* AB ADD QUICK       */  &&_ADD_QUICK,
		/* AC ADD QUICK       */  &&_ADD_QUICK,
		/* AD ADD QUICK       */  &&_ADD_QUICK,
		/* AE ADD QUICK       */  &&_ADD_QUICK,
		/* AF ADD QUICK       */  &&_ADD_QUICK,
		/* B0 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B1 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B2 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B3 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B4 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B5 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B6 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B7 PUSH CLASS      */  &&_PUSH_CLASS,
		/* B8 PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* B9 PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* BA PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* BB PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* BC PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* BD PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* BE PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* BF PUSH FUNCTION   */  &&_PUSH_FUNCTION,
		/* C0 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C1 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C2 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C3 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C4 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C5 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C6 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C7 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
		/* C8 PUSH STATIC     */  &&_PUSH_STATIC,
		/* C9 PUSH STATIC     */  &&_PUSH_STATIC,
		/* CA PUSH STATIC     */  &&_PUSH_STATIC,
		/* CB PUSH STATIC     */  &&_PUSH_STATIC,
		/* CC PUSH STATIC     */  &&_PUSH_STATIC,
		/* CD PUSH STATIC     */  &&_PUSH_STATIC,
		/* CE PUSH STATIC     */  &&_PUSH_STATIC,
		/* CF PUSH STATIC     */  &&_PUSH_STATIC,
		/* D0 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D1 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D2 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D3 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D4 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D5 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D6 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D7 POP DYNAMIC     */  &&_POP_DYNAMIC,
		/* D8 POP STATIC      */  &&_POP_STATIC,
		/* D9 POP STATIC      */  &&_POP_STATIC,
		/* DA POP STATIC      */  &&_POP_STATIC,
		/* DB POP STATIC      */  &&_POP_STATIC,
		/* DC POP STATIC      */  &&_POP_STATIC,
		/* DD POP STATIC      */  &&_POP_STATIC,
		/* DE POP STATIC      */  &&_POP_STATIC,
		/* DF POP STATIC      */  &&_POP_STATIC,
		/* E0 PUSH CONST      */  &&_PUSH_CONST,
		/* E1 PUSH CONST      */  &&_PUSH_CONST,
		/* E2 PUSH CONST      */  &&_PUSH_CONST,
		/* E3 PUSH CONST      */  &&_PUSH_CONST,
		/* E4 PUSH CONST      */  &&_PUSH_CONST,
		/* E5 PUSH CONST      */  &&_PUSH_CONST,
		/* E6 PUSH CONST      */  &&_PUSH_CONST,
		/* E7 PUSH CONST      */  &&_PUSH_CONST,
		/* E8 PUSH CONST      */  &&_PUSH_CONST,
		/* E9 PUSH CONST      */  &&_PUSH_CONST,
		/* EA PUSH CONST      */  &&_PUSH_CONST,
		/* EB PUSH CONST      */  &&_PUSH_CONST,
		/* EC PUSH CONST      */  &&_PUSH_CONST,
		/* ED PUSH CONST      */  &&_PUSH_CONST,
		/* EE PUSH CONST      */  &&_PUSH_CONST,
		/* EF PUSH CONST      */  &&_PUSH_CONST_EX,
		/* F0 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F1 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F2 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F3 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F4 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F5 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F6 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F7 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F8 PUSH QUICK      */  &&_PUSH_QUICK,
		/* F9 PUSH QUICK      */  &&_PUSH_QUICK,
		/* FA PUSH QUICK      */  &&_PUSH_QUICK,
		/* FB PUSH QUICK      */  &&_PUSH_QUICK,
		/* FC PUSH QUICK      */  &&_PUSH_QUICK,
		/* FD PUSH QUICK      */  &&_PUSH_QUICK,
		/* FE PUSH QUICK      */  &&_PUSH_QUICK,
		/* FF PUSH QUICK      */  &&_PUSH_QUICK
	};

	CLASS *class = JIT_class;
	TYPE type;
	uint p = 0;
	ushort code;
	int index;
	int size = JIT_get_code_size(func);
	void *addr;
	int pos;
	int i;
	
	enter_function(func, ind);
	
	//JIT_print("  JIT.debug(\"SP = %%p\\n\", SP);\n");
	
	goto _MAIN;
	
_MAIN:

	//fprintf(stderr, "[%d] %d\n", p, _stack_current);
	
	if (_has_finally && p == func->error)
		print_catch();

	if (!JIT_last_print_is_label)
		print_label(p);
	
	if (p >= (size - 1)) // ignore the last opcode which is RETURN
		return leave_function(func, ind);
	
	_pc = p;
	code = func->code[p++];
	//fprintf(stderr, "--------—-------------------- %d / %04X\n", _stack_current, code);
	goto *jump_table[code >> 8];

_DUP:

	check_stack(1);
	
	_has_just_dup = TRUE;
	
	index = GB.Count(_dup_type);
	*(TYPE *)GB.Add(&_dup_type) = type = get_type(-1);

	//JIT_print_decl("  %s d%d;\n", JIT_get_ctype(type), index);
	JIT_declare(type, "d%d", index);
	//_no_release = TRUE;
	pop(type, "d%d", index);
	//_no_release = FALSE;
	push(type, "d%d", index);
	push(type, "d%d", index);
	
	goto _MAIN;

_PUSH_LOCAL:

	index = GET_XX();
	type = get_local_type(func, index);

	if (index >= func->n_local)
		push(type, "c%d", _ctrl_index[index - func->n_local]);
	else
		push(type, "l%d", index);
	
	goto _MAIN;

_POP_LOCAL:

	index = GET_XX();
	type = get_local_type(func, index);

	if (index >= func->n_local)
	{
		pop(type, "c%d", _ctrl_index[index - func->n_local]);
	}
	else
	{
		if (check_swap(type, "l%d = %%s", index))
			pop(type, "l%d", index);
	}
	
	goto _MAIN;

_POP_CTRL:

	pop_ctrl(GET_XX() - func->n_local, T_VOID);
	goto _MAIN;

_PUSH_PARAM:

	index = func->n_param + GET_XX();
	push(func->param[index].type, "p%d", index);
	goto _MAIN;

_POP_PARAM:

	index = func->n_param + GET_XX();
	type = func->param[index].type;
	
	if (check_swap(type, "p%d = %%s", index))
		pop(type, "p%d", index);
	
	goto _MAIN;

_PUSH_QUICK:

	push(T_INTEGER, "%d", GET_XXX());
	goto _MAIN;

_PUSH_SHORT:

	push(T_INTEGER, "%d", (short)PC[0]);
	p++;
	goto _MAIN;

_PUSH_INTEGER:

	push(T_INTEGER, "%d", PC[0] | ((uint)PC[1] << 16));
	p += 2;
	goto _MAIN;

_PUSH_STATIC:

	index = GET_7XX();
	type = JIT_ctype_to_type(class->load->stat[index].type);
	addr = &class->stat[class->load->stat[index].pos];

	if (TYPE_is_object(type))
	{
		if (TYPE_is_pure_object(type))
			push(type, "GET_o(%p, CLASS(%p))", addr, (CLASS *)type);
		else
			push(type, "GET_o(%p, GB_T_OBJECT)", addr);
	}
	else
		push(type, "GET_%s(%p)", JIT_get_type(type), addr);

	goto _MAIN;

_POP_STATIC:

	index = GET_7XX();
	type = JIT_ctype_to_type(class->load->stat[index].type);
	addr = &class->stat[class->load->stat[index].pos];

	_no_release = TRUE;
	if (check_swap(type, "SET_%s(%p, %%s)", JIT_get_type(type), addr))
		pop(type, "SET_%s(%p, %%s)", JIT_get_type(type), addr);
	_no_release = FALSE;
	
	goto _MAIN;

_PUSH_DYNAMIC:

	index = GET_7XX();
	type = JIT_ctype_to_type(class->load->dyn[index].type);
	pos = class->load->dyn[index].pos;

	if (TYPE_is_object(type))
	{
		if (TYPE_is_pure_object(type))
			push(type, "GET_o(&OP[%d], CLASS(%p))", pos, (CLASS *)type);
		else
			push(type, "GET_o(&OP[%d], GB_T_OBJECT)", pos);
	}
	else
		push(type, "GET_%s(&OP[%d])", JIT_get_type(type), pos);
	
	goto _MAIN;

_POP_DYNAMIC:

	index = GET_7XX();
	type = JIT_ctype_to_type(class->load->dyn[index].type);
	pos = class->load->dyn[index].pos;
	
	_no_release = TRUE;
	if (check_swap(type, "SET_%s(&OP[%d], %%s)", JIT_get_type(type), pos))
		pop(type, "SET_%s(&OP[%d], %%s)", JIT_get_type(type), pos);
	_no_release = FALSE;
	
	goto _MAIN;

_PUSH_MISC:

	switch (GET_UX())
	{
		case 0:
			push(T_NULL, "NULL");
			break;
			
		case 1:
			push(T_VOID, NULL);
			break;
			
		case 2:
			push(T_BOOLEAN, "0");
			break;
		
		case 3:
			push(T_BOOLEAN, "1");
			break;
			
		case 4:
			push(T_OBJECT, "GET_LAST()");
			break;
			
		case 5:
			push(T_CSTRING, "GET_CSTRING(\"\", 0, 0)");
			break;
		
		case 6:
			push(T_FLOAT, "INFINITY");
			break;

		case 7:
			push(T_FLOAT, "-INFINITY");
			break;

		case 8:
			push_complex();
			break;

		/*case 9:
			EXEC_push_vargs();
			break;

		case 10:
			EXEC_drop_vargs();
			break;*/
			
		default:
			goto _ILLEGAL;
	}
	goto _MAIN;
	
_PUSH_CHAR:

	push(T_CSTRING, "GET_CHAR(%d)", GET_UX());
	goto _MAIN;

_POP_OPTIONAL:

	check_stack(1);
	if (get_type(-1) == T_VOID)
		pop_stack(1);
	else
	{
		index = func->n_param + GET_XX() - func->npmin;
		JIT_print("  if (o%d & %d)\n  ", index / 8, (1 << (index % 8)));
		index = func->n_param + GET_XX();
		pop(func->param[index].type, "p%d", index);
	}
	goto _MAIN;
	
_PUSH_CLASS:

	index = GET_7XX();
	type = (TYPE)class->load->class_ref[index];
	push(T_CLASS, "CLASS(%p)", (CLASS *)type);
	goto _MAIN;

_PUSH_FUNCTION:

	push_function(CALL_PRIVATE, GET_7XX());
	goto _MAIN;

_PUSH_ME:

	index = GET_UX();
	push(T_OBJECT, "GET_ME()");
	
	// TODO: SUPER
	
	goto _MAIN;
	
	/*if (GET_UX() & 2)
	{
		// The used class must be in the stack, because it is tested by exec_push && exec_pop
		if (LIKELY(OP != NULL))
		{
			SP->_object.class = SP->_object.class->parent;
			SP->_object.super = EXEC_super;
		}
		else
		{
			SP->_class.class = SP->_class.class->parent;
			SP->_class.super = EXEC_super;
		}

		EXEC_super = SP;

		//fprintf(stderr, "%s\n", DEBUG_get_current_position());
		//BREAKPOINT();
	}

	PUSH();
	goto _NEXT;*/

_PUSH_UNKNOWN:

	push_unknown(PC[0]);
	p++;
	goto _MAIN;
	
_POP_UNKNOWN:

	pop_unknown(PC[0]);
	p++;
	goto _MAIN;
	
_CALL:

	push_call(code);
	goto _MAIN;

_SUBR:

	push_subr(CALL_SUBR, code);
	goto _MAIN;

_SUBR_CODE:

	push_subr(CALL_SUBR_CODE, code);
	goto _MAIN;

_DROP:

	pop(T_VOID, NULL);
	goto _MAIN;

_NEW:

	push_subr(CALL_NEW, code);
	goto _MAIN;

_RETURN:

	switch(code & 0xFF)
	{
		case 0:
			JIT_print("  RETURN();\n");
			break;
			
		case 1:
			pop(func->type, "r");
			JIT_print("  goto __RETURN;\n");
			break;
		
		case 2:
			JIT_print("  goto __RETURN;\n");
			break;
			
		default:
			goto _ILLEGAL;
	}
	goto _MAIN;

_GOSUB:

	JIT_print("  PUSH_GOSUB(__L%d); goto __L%d;\n", p + 1, p + (signed short)PC[0] + 1);
	JIT_print("__L%d:;\n", p + 1);
	p++;
	goto _MAIN;

_JUMP:

	JIT_print("  goto __L%d;\n", p + (signed short)PC[0] + 1);
	p++;
	goto _MAIN;
	
_JUMP_IF_TRUE:

	JIT_print("  if (%s) goto __L%d;\n", peek(-1, T_BOOLEAN), p + (signed short)PC[0] + 1);
	pop_stack(1);
	p++;
	goto _MAIN;

_JUMP_IF_FALSE:

	JIT_print("  if (!(%s)) goto __L%d;\n", peek(-1, T_BOOLEAN), p + (signed short)PC[0] + 1);
	pop_stack(1);
	p++;
	goto _MAIN;

_JUMP_FIRST:

	index = PC[2] & 0xFF;
	type = get_local_type(func, index);
	if (strcmp(get_expr(-1), "1") == 0)
		_loop_type = LOOP_UP;
	else if (strcmp(get_expr(-1), "-1") == 0)
		_loop_type = LOOP_DOWN;
	else
		_loop_type = LOOP_UNKNOWN;
	
	pop(type, "const %s i%d", JIT_get_ctype(type), _loop_count);
	pop(type, "const %s e%d", JIT_get_ctype(type), _loop_count);
	
	JIT_print("  goto __L%ds;\n", p + 1);
	
	goto _MAIN;

_JUMP_NEXT:

	index = PC[1] & 0xFF;
	JIT_print("  l%d += i%d;\n", index, _loop_count);
	JIT_print("__L%ds:\n", p);
	
	switch(_loop_type)
	{
		case LOOP_UP:
			JIT_print("  if (l%d > e%d) goto __L%d;\n", index, _loop_count, p + (signed short)PC[0] + 1);
			break;
			
		case LOOP_DOWN:
			JIT_print("  if (l%d < e%d) goto __L%d;\n", index, _loop_count, p + (signed short)PC[0] + 1);
			break;
			
		case LOOP_UNKNOWN:
			JIT_print("  if (((i%d > 0) && (l%d > e%d)) || ((i%d < 0) && (l%d < e%d))) goto __L%d;\n", _loop_count, index, _loop_count, _loop_count, index, _loop_count, p + (signed short)PC[0] + 1);
	}
	
	_loop_count++;
	
	p +=2;
	goto _MAIN;

_ENUM_FIRST:

	index = GET_XX() - func->n_local;
	pop_ctrl(index, T_OBJECT);
	add_ctrl(index + 1, T_OBJECT);
	JIT_print("  ENUM_FIRST(0x%04X, c%d, c%d);\n", code, _ctrl_index[index], _ctrl_index[index + 1]);
	goto _MAIN;

_ENUM_NEXT:

	index = (PC[-2] & 0xFF) - func->n_local;
	
	JIT_print("  ENUM_NEXT(0x%04X, c%d, c%d, __L%d);\n", code, _ctrl_index[index], _ctrl_index[index + 1], p + (signed short)PC[0] + 1);
	if ((code & 1) == 0) 
		push(T_UNKNOWN, "POP_u()");
	
	p++;
	goto _MAIN;

_PUSH_CONST:

	index = GET_UXX();
	push_constant(class, index);
	goto _MAIN;

_PUSH_CONST_EX:

	push_constant(class, PC[0]);
	p++;
	goto _MAIN;

_PUSH_ARRAY:

	push_array(code);
	goto _MAIN;

_POP_ARRAY:

	pop_array(code);
	goto _MAIN;

_ADD_QUICK:

	index = GET_XXX();
	push(T_INTEGER, "%d", abs(index));
	if (index < 0)
	{
		PC[-1] = code = 0x3100;
		goto _SUBR_SUB;
	}
	else
		PC[-1] = code = 0x3000;

_SUBR_ADD:

	push_subr_add(code, "+", "|", TRUE);
	goto _MAIN;

_SUBR_SUB:

	push_subr_add(code, "-", "^", TRUE);
	goto _MAIN;

_SUBR_MUL:

	push_subr_add(code, "*", "&", FALSE);
	goto _MAIN;

_SUBR_DIV:

	push_subr_div(code);
	goto _MAIN;

_SUBR_NEG:

	push_subr_neg(code);
	goto _MAIN;

_SUBR_QUO:

	push_subr_quo(code, "/");
	goto _MAIN;

_SUBR_REM:

	push_subr_quo(code, "%");
	goto _MAIN;

_SUBR_AND:

	push_subr_and(code, "&");
	goto _MAIN;
	
_SUBR_OR:

	push_subr_and(code, "|");
	goto _MAIN;
	
_SUBR_XOR:

	push_subr_and(code, "^");
	goto _MAIN;
	
_SUBR_NOT:

	push_subr_not(code);
	goto _MAIN;

_SUBR_COMPE:
_SUBR_COMPN:
_SUBR_COMPGT:
_SUBR_COMPLE:
_SUBR_COMPLT:
_SUBR_COMPGE:

	push_subr_comp(code);
	goto _MAIN;
	
_SUBR_ISNAN:

	push_subr_isnan(code);
	goto _MAIN;

_SUBR_CONV:
	
	push_subr_conv(code);
	goto _MAIN;
	
_SUBR_LEN:

	push_subr_len(code);
	goto _MAIN;
	
_SUBR_MATH:

	push_subr_math(code);
	goto _MAIN;
	
_SUBR_BIT:

	push_subr_bit(code);
	goto _MAIN;

_BREAK:

	/*if (*(JIT.exec_debug))
		JIT_print("  BREAK(%d,0x%04X);\n", _pc, code);*/
	goto _MAIN;

_QUIT:

	JIT_print("  ");
	
	if ((code & 3) == 3)
	{
		check_stack(1);
		JIT_print("%s,", push_expr(-1, T_BYTE));
		pop_stack(1);
	}
	
	JIT_print("  QUIT(0x%04X);\n", code);
	goto _MAIN;

_TRY:

	JIT_print("  TRY {\n");
	p++;
	goto _MAIN;
	
_END_TRY:

	JIT_print("  *JIT.got_error = 0;\n");
	JIT_print("  } CATCH {\n");
	JIT_print("  *JIT.got_error = 1;\n");
	JIT_print("  JIT.error_set_last(FALSE); \n");
	JIT_print("  } END_TRY\n");
	goto _MAIN;

_CATCH:

	_has_catch = TRUE;
	if (!_has_finally)
		print_catch();

	JIT_print("  if (!error) goto __RELEASE;\n");
	
	goto _MAIN;

_ON_GOTO_GOSUB:

	index = GET_XX();

	JIT_print("  {\n");
	JIT_print("    static void *jump[] = { ");
	
	for (i = 0; i < index; i++)
	{
		if (i) JIT_print(", ");
		JIT_print("&&__L%d", PC[i] + p + i);
	}
	
	JIT_print(" };\n");
	
	pop(T_INTEGER, "  int n");
	
	p += index + 2;
	
	JIT_print("    if (n >= 0 && n < %d)\n", index);
	
	if ((PC[index + 1] & 0xFF00) == C_GOSUB)
	{
		JIT_print("    {\n");
		JIT_print("      PUSH_GOSUB(__L%d);\n", p);
	}
	
	JIT_print("      goto *jump[n];\n");
	
	if ((PC[index + 1] & 0xFF00) == C_GOSUB)
		JIT_print("    }\n");
	
	JIT_print("  }\n");
	
	JIT_print("__L%d:;\n", p);
	goto _MAIN;

_PUSH_EVENT:
_PUSH_EXTERN:
_BYREF:
_CALL_QUICK:
_CALL_SLOW:
_ILLEGAL:

	JIT_panic("unsupported opcode %04X", code);
}

